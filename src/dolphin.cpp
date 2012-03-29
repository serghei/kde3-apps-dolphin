/***************************************************************************
 *   Copyright (C) 2006 by Peter Penz <peter.penz@gmx.at>                  *
 *   Copyright (C) 2006 by Stefan Monov <logixoul@gmail.com>               *
 *   Copyright (C) 2006 by Cvetoslav Ludmiloff <ludmiloff@gmail.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/

#include "dolphin.h"

#include <assert.h>

#include <kbookmarkmanager.h>
#include <kglobal.h>
#include <kpropertiesdialog.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdeversion.h>
#include <kstatusbar.h>
#include <kaccel.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kactionclasses.h>
#include <kpopupmenu.h>
#include <kio/renamedlg.h>
#include <kinputdialog.h>
#include <kshell.h>
#include <kdesktopfile.h>
#include <kstandarddirs.h>
#include <kprotocolinfo.h>
#include <kmessagebox.h>
#include <kservice.h>
#include <kstandarddirs.h>
#include <krun.h>

#include <qclipboard.h>
#include <qdragobject.h>

#include "urlnavigator.h"
#include "viewpropertiesdialog.h"
#include "viewproperties.h"
#include "dolphinsettings.h"
#include "dolphinsettingsdialog.h"
#include "dolphinstatusbar.h"
#include "undomanager.h"
#include "progressindicator.h"
#include "dolphinsettings.h"
#include "sidebars.h"
#include "sidebarssettings.h"


Dolphin& Dolphin::mainWin()
{
    static Dolphin* instance = 0;
    if (instance == 0) {
        instance = new Dolphin();
        instance->init();
    }
    return *instance;
}

Dolphin::~Dolphin()
{
}

void Dolphin::setActiveView(DolphinView* view)
{
    assert((view == m_view[PrimaryIdx]) || (view == m_view[SecondaryIdx]));
    if (m_activeView == view) {
        return;
    }

    m_activeView = view;

    updateHistory();
    updateEditActions();
    updateViewActions();
    updateGoActions();

    setCaption(m_activeView->url().fileName());

    emit activeViewChanged();
}

void Dolphin::dropURLs(const KURL::List& urls,
                       const KURL& destination)
{
    const ButtonState keyboardState = KApplication::keyboardMouseState();
    const bool shiftPressed = (keyboardState & ShiftButton) > 0;
    const bool controlPressed = (keyboardState & ControlButton) > 0;

    int selectedIndex = -1;
    if (shiftPressed && controlPressed) {
        // shortcut for 'Linke Here' is used
        selectedIndex = 2;
    }
    else if (controlPressed) {
        // shortcut for 'Copy Here' is used
        selectedIndex = 1;
    }
    else if (shiftPressed) {
        // shortcut for 'Move Here' is used
        selectedIndex = 0;
    }
    else {
        // no shortcut is used, hence open a popup menu
        KPopupMenu popup(this);

        popup.insertItem(SmallIcon("goto"), i18n("&Move Here") + "\t" + KKey::modFlagLabel(KKey::SHIFT), 0);
        popup.insertItem(SmallIcon("editcopy"), i18n( "&Copy Here" ) + "\t" + KKey::modFlagLabel(KKey::CTRL), 1);
        popup.insertItem(i18n("&Link Here") + "\t" + KKey::modFlagLabel((KKey::ModFlag)(KKey::CTRL|KKey::SHIFT)), 2);
        popup.insertSeparator();
        popup.insertItem(SmallIcon("stop"), i18n("Cancel"), 3);
        popup.setAccel(i18n("Escape"), 3);

        selectedIndex = popup.exec(QCursor::pos());
    }

    if (selectedIndex < 0) {
        return;
    }

    switch (selectedIndex) {
        case 0: {
            // 'Move Here' has been selected
            updateViewProperties(urls);
            moveURLs(urls, destination);
            break;
        }

        case 1: {
            // 'Copy Here' has been selected
            updateViewProperties(urls);
            copyURLs(urls, destination);
            break;
        }

        case 2: {
            // 'Link Here' has been selected
            KIO::Job* job = KIO::link(urls, destination);
            addPendingUndoJob(job, DolphinCommand::Link, urls, destination);
            break;
        }

        default:
            // 'Cancel' has been selected
            break;
    }
}

void Dolphin::refreshViews()
{
    const bool split = DolphinSettings::instance().isViewSplit();
    const bool isPrimaryViewActive = (m_activeView == m_view[PrimaryIdx]);
    DolphinSettings& settings = DolphinSettings::instance();
    KURL url;
    for (int i = PrimaryIdx; i <= SecondaryIdx; ++i) {
       if (m_view[i] != 0) {
            url = m_view[i]->url();

            // delete view instance...
            m_view[i]->close();
            m_view[i]->deleteLater();
            m_view[i] = 0;
        }

        if (split || (i == PrimaryIdx)) {
            // ... and recreate it
            ViewProperties props(url);
            m_view[i] = new DolphinView(m_splitter,
                                        url,
                                        props.viewMode(),
                                        props.isShowHiddenFilesEnabled());
            m_view[i]->show();
        }

        rightSidebarSettings* rightsidebarSettings = settings.rightsidebar();
        assert(rightsidebarSettings != 0);
        if (rightsidebarSettings->isVisible()) {
            m_splitter->moveToLast(m_rightsidebar);
        }
    }

    m_activeView = isPrimaryViewActive ? m_view[PrimaryIdx] : m_view[SecondaryIdx];
    assert(m_activeView != 0);

    updateViewActions();
    emit activeViewChanged();
}

void Dolphin::slotHistoryChanged()
{
    updateHistory();
}

void Dolphin::slotURLChanged(const KURL& url)
{
    updateEditActions();
    updateGoActions();
    setCaption(url.fileName());
}

void Dolphin::slotURLChangeRequest(const KURL& url)
{
	clearStatusBar();
	m_activeView->setURL(url);
}

void Dolphin::slotViewModeChanged()
{
    updateViewActions();
}

void Dolphin::slotShowHiddenFilesChanged()
{
    KToggleAction* showHiddenFilesAction =
        static_cast<KToggleAction*>(actionCollection()->action("show_hidden_files"));
    showHiddenFilesAction->setChecked(m_activeView->isShowHiddenFilesEnabled());
}

void Dolphin::slotShowFilterBarChanged()
{
    KToggleAction* showFilterBarAction =
        static_cast<KToggleAction*>(actionCollection()->action("show_filter_bar"));
    showFilterBarAction->setChecked(m_activeView->isFilterBarVisible());
}

void Dolphin::slotSortingChanged(DolphinView::Sorting sorting)
{
    KAction* action = 0;
    switch (sorting) {
        case DolphinView::SortByName:
            action = actionCollection()->action("by_name");
            break;
        case DolphinView::SortBySize:
            action = actionCollection()->action("by_size");
            break;
        case DolphinView::SortByDate:
            action = actionCollection()->action("by_date");
            break;
        default:
            break;
    }

    if (action != 0) {
        KToggleAction* toggleAction = static_cast<KToggleAction*>(action);
        toggleAction->setChecked(true);
    }
}

void Dolphin::slotSortOrderChanged(Qt::SortOrder order)
{
    KToggleAction* descending = static_cast<KToggleAction*>(actionCollection()->action("descending"));
    const bool sortDescending = (order == Qt::Descending);
    descending->setChecked(sortDescending);
}

void Dolphin::slotSelectionChanged()
{
    updateEditActions();

    assert(m_view[PrimaryIdx] != 0);
    int selectedURLsCount = m_view[PrimaryIdx]->selectedURLs().count();
    if (m_view[SecondaryIdx] != 0) {
        selectedURLsCount += m_view[SecondaryIdx]->selectedURLs().count();
    }

    KAction* compareFilesAction = actionCollection()->action("compare_files");
    compareFilesAction->setEnabled(selectedURLsCount == 2);

    m_activeView->updateStatusBar();

    emit selectionChanged();
}

void Dolphin::closeEvent(QCloseEvent* event)
{
    KConfig* config = kapp->config();
    config->setGroup("General");
    config->writeEntry("First Run", false);

    DolphinSettings& settings = DolphinSettings::instance();

    leftSidebarSettings* leftsidebarSettings = settings.leftsidebar();
    const bool isleftSidebarVisible = (m_leftsidebar != 0);
    leftsidebarSettings->setVisible(isleftSidebarVisible);
    if (isleftSidebarVisible) {
        leftsidebarSettings->setWidth(m_leftsidebar->width());
    }
    
    rightSidebarSettings* rightsidebarSettings = settings.rightsidebar();
    const bool isrightSidebarVisible = (m_rightsidebar != 0);
    rightsidebarSettings->setVisible(isrightSidebarVisible);
    if (isrightSidebarVisible) {
        rightsidebarSettings->setWidth(m_rightsidebar->width());
    }

    settings.save();

    config->sync();
    KMainWindow::closeEvent(event);
}

void Dolphin::saveProperties(KConfig* config)
{
    config->setGroup("Primary view");
    config->writeEntry("URL", m_view[PrimaryIdx]->url().url());
    config->writeEntry("Editable URL", m_view[PrimaryIdx]->isURLEditable());
    if (m_view[SecondaryIdx] != 0) {
        config->setGroup("Secondary view");
        config->writeEntry("URL", m_view[SecondaryIdx]->url().url());
        config->writeEntry("Editable URL", m_view[SecondaryIdx]->isURLEditable());
    }
}

void Dolphin::readProperties(KConfig* config)
{
    config->setGroup("Primary view");
    m_view[PrimaryIdx]->setURL(config->readEntry("URL"));
    m_view[PrimaryIdx]->setURLEditable(config->readBoolEntry("Editable URL"));
    if (config->hasGroup("Secondary view")) {
        config->setGroup("Secondary view");
        if (m_view[SecondaryIdx] == 0) {
            toggleSplitView();
        }
        m_view[SecondaryIdx]->setURL(config->readEntry("URL"));
        m_view[SecondaryIdx]->setURLEditable(config->readBoolEntry("Editable URL"));
    }
    else if (m_view[SecondaryIdx] != 0) {
        toggleSplitView();
    }
}

void Dolphin::createFolder()
{
    // Parts of the following code have been taken
    // from the class KonqPopupMenu located in
    // libqonq/konq_popupmenu.h of Konqueror.
    // (Copyright (C) 2000  David Faure <faure@kde.org>,
    // Copyright (C) 2001 Holger Freyther <freyther@yahoo.com>)

    clearStatusBar();

    DolphinStatusBar* statusBar = m_activeView->statusBar();
    const KURL baseURL(m_activeView->url());

    QString name(i18n("New Folder"));
    if (baseURL.isLocalFile() && QFileInfo(baseURL.path(+1) + name).exists()) {
        name = KIO::RenameDlg::suggestName(baseURL, i18n("New Folder"));
    }

    bool ok = false;
    name = KInputDialog::getText(i18n("New Folder"),
                                 i18n("Enter folder name:" ),
                                 name,
                                 &ok,
                                 this);

    if (!ok) {
        // the user has pressed 'Cancel'
        return;
    }

    assert(!name.isEmpty());

    KURL url;
    if ((name[0] == '/') || (name[0] == '~')) {
        url.setPath(KShell::tildeExpand(name));
    }
    else {
        name = KIO::encodeFileName(name);
        url = baseURL;
        url.addPath(name);
    }
    ok = KIO::NetAccess::mkdir(url, this);

    // TODO: provide message type hint
    if (ok) {
        statusBar->setMessage(i18n("Created folder %1.").arg(url.path()),
                              DolphinStatusBar::OperationCompleted);

        DolphinCommand command(DolphinCommand::CreateFolder, KURL::List(), url);
        UndoManager::instance().addCommand(command);
    }
    else {
        // Creating of the folder has been failed. Check whether the creating
        // has been failed because a folder with the same name exists...
        if (KIO::NetAccess::exists(url, true, this)) {
            statusBar->setMessage(i18n("A folder named %1 already exists.").arg(url.path()),
                                  DolphinStatusBar::Error);
        }
        else {
            statusBar->setMessage(i18n("Creating of folder %1 failed.").arg(url.path()),
                                  DolphinStatusBar::Error);
        }

    }
}

void Dolphin::createFile()
{
    // Parts of the following code have been taken
    // from the class KonqPopupMenu located in
    // libqonq/konq_popupmenu.h of Konqueror.
    // (Copyright (C) 2000  David Faure <faure@kde.org>,
    // Copyright (C) 2001 Holger Freyther <freyther@yahoo.com>)

    clearStatusBar();

    // TODO: const Entry& entry = m_createFileTemplates[QString(sender->name())];
    // should be enough. Anyway: the implemantation of [] does a linear search internally too.
    KSortableValueList<CreateFileEntry, QString>::ConstIterator it = m_createFileTemplates.begin();
    KSortableValueList<CreateFileEntry, QString>::ConstIterator end = m_createFileTemplates.end();

    const QString senderName(sender()->name());
    bool found = false;
    CreateFileEntry entry;
    while (!found && (it != end)) {
        if ((*it).index() == senderName) {
            entry = (*it).value();
            found = true;
        }
        else {
            ++it;
        }
    }

    DolphinStatusBar* statusBar = m_activeView->statusBar();
    if (!found || !QFile::exists(entry.templatePath)) {
        statusBar->setMessage(i18n("Could not create file."), DolphinStatusBar::Error);
       return;
    }

    // Get the source path of the template which should be copied.
    // The source path is part of the URL entry of the desktop file.
    const int pos = entry.templatePath.findRev('/');
    QString sourcePath(entry.templatePath.left(pos + 1));
    sourcePath += KDesktopFile(entry.templatePath, true).readPathEntry("URL");

    QString name(i18n(entry.name));
    // Most entry names end with "..." (e. g. "HTML File..."), which is ok for
    // menus but no good choice for a new file name -> remove the dots...
    name.replace("...", QString::null);

    // add the file extension to the name
    name.append(sourcePath.right(sourcePath.length() - sourcePath.findRev('.')));

    // Check whether a file with the current name already exists. If yes suggest automatically
    // a unique file name (e. g. "HTML File" will be replaced by "HTML File_1").
    const KURL viewURL(m_activeView->url());
    const bool fileExists = viewURL.isLocalFile() &&
                            QFileInfo(viewURL.path(+1) + KIO::encodeFileName(name)).exists();
    if (fileExists) {
        name = KIO::RenameDlg::suggestName(viewURL, name);
    }

    // let the user change the suggested file name
    bool ok = false;
    name = KInputDialog::getText(entry.name,
                                 entry.comment,
                                 name,
                                 &ok,
                                 this);
    if (!ok) {
        // the user has pressed 'Cancel'
        return;
    }

    // before copying the template to the destination path check whether a file
    // with the given name already exists
    const QString destPath(viewURL.prettyURL() + "/" + KIO::encodeFileName(name));
    const KURL destURL(destPath);
    if (KIO::NetAccess::exists(destURL, false, this)) {
        statusBar->setMessage(i18n("A file named %1 already exists.").arg(name),
                              DolphinStatusBar::Error);
        return;
    }

    // copy the template to the destination path
    const KURL sourceURL(sourcePath);
    KIO::CopyJob* job = KIO::copyAs(sourceURL, destURL);
    job->setDefaultPermissions(true);
    if (KIO::NetAccess::synchronousRun(job, this)) {
        statusBar->setMessage(i18n("Created file %1.").arg(name),
                              DolphinStatusBar::OperationCompleted);

        KURL::List list;
        list.append(sourceURL);
        DolphinCommand command(DolphinCommand::CreateFile, list, destURL);
        UndoManager::instance().addCommand(command);

    }
    else {
        statusBar->setMessage(i18n("Creating of file %1 failed.").arg(name),
                              DolphinStatusBar::Error);
    }
}

void Dolphin::rename()
{
    clearStatusBar();
    m_activeView->renameSelectedItems();
}

void Dolphin::moveToTrash()
{
    clearStatusBar();
    KURL::List selectedURLs = m_activeView->selectedURLs();
    KIO::Job* job = KIO::trash(selectedURLs);
    addPendingUndoJob(job, DolphinCommand::Trash, selectedURLs, m_activeView->url());
}

void Dolphin::deleteItems()
{
    clearStatusBar();

    KURL::List list = m_activeView->selectedURLs();
    const uint itemCount = list.count();
    assert(itemCount >= 1);

    QString text;
    if (itemCount > 1) {
        text = i18n("Do you really want to delete the %1 selected items?").arg(itemCount);
    }
    else {
        const KURL& url = list.first();
        text = i18n("Do you really want to delete '%1'?").arg(url.fileName());
    }

    const bool del = KMessageBox::warningContinueCancel(this,
                                                        text,
                                                        QString::null,
                                                        KGuiItem(i18n("Delete"), SmallIcon("editdelete"))
                                                       ) == KMessageBox::Continue;
    if (del) {
        KIO::Job* job = KIO::del(list);
        connect(job, SIGNAL(result(KIO::Job*)),
                this, SLOT(slotHandleJobError(KIO::Job*)));
        connect(job, SIGNAL(result(KIO::Job*)),
                this, SLOT(slotDeleteFileFinished(KIO::Job*)));
    }
}

void Dolphin::properties()
{
    const KFileItemList* sourceList = m_activeView->selectedItems();
    if (sourceList == 0) {
        return;
    }

    KFileItemList list;
    KFileItemListIterator it(*sourceList);
    KFileItem* item = 0;
    while ((item = it.current()) != 0) {
        list.append(item);
        ++it;
    }

    new KPropertiesDialog(list, this);
}

void Dolphin::quit()
{
    close();
}

void Dolphin::slotHandleJobError(KIO::Job* job)
{
    if (job->error() != 0) {
        m_activeView->statusBar()->setMessage(job->errorString(),
                                              DolphinStatusBar::Error);
    }
}

void Dolphin::slotDeleteFileFinished(KIO::Job* job)
{
    if (job->error() == 0) {
        m_activeView->statusBar()->setMessage(i18n("Delete operation completed."),
                                               DolphinStatusBar::OperationCompleted);

        // TODO: In opposite to the 'Move to Trash' operation in the class KFileIconView
        // no rearranging of the item position is done when a file has been deleted.
        // This is bypassed by reloading the view, but it might be worth to investigate
        // deeper for the root of this issue.
        m_activeView->reload();
    }
}

void Dolphin::slotUndoAvailable(bool available)
{
    KAction* undoAction = actionCollection()->action(KStdAction::stdName(KStdAction::Undo));
    if (undoAction != 0) {
        undoAction->setEnabled(available);
    }
}

void Dolphin::slotUndoTextChanged(const QString& text)
{
    KAction* undoAction = actionCollection()->action(KStdAction::stdName(KStdAction::Undo));
    if (undoAction != 0) {
        undoAction->setText(text);
    }
}

void Dolphin::slotRedoAvailable(bool available)
{
    KAction* redoAction = actionCollection()->action(KStdAction::stdName(KStdAction::Redo));
    if (redoAction != 0) {
        redoAction->setEnabled(available);
    }
}

void Dolphin::slotRedoTextChanged(const QString& text)
{
    KAction* redoAction = actionCollection()->action(KStdAction::stdName(KStdAction::Redo));
    if (redoAction != 0) {
        redoAction->setText(text);
    }
}

void Dolphin::cut()
{
    m_clipboardContainsCutData = true;
    QDragObject* data = new KURLDrag(m_activeView->selectedURLs(),
                                     widget());
    QApplication::clipboard()->setData(data);
}

void Dolphin::copy()
{
    m_clipboardContainsCutData = false;
    QDragObject* data = new KURLDrag(m_activeView->selectedURLs(),
                                     widget());
    QApplication::clipboard()->setData(data);
}

void Dolphin::paste()
{
    QClipboard* clipboard = QApplication::clipboard();
    QMimeSource* data = clipboard->data();
    if (!KURLDrag::canDecode(data)) {
        return;
    }

    clearStatusBar();

    KURL::List sourceURLs;
    KURLDrag::decode(data, sourceURLs);

    // per default the pasting is done into the current URL of the view
    KURL destURL(m_activeView->url());

    // check whether the pasting should be done into a selected directory
    KURL::List selectedURLs = m_activeView->selectedURLs();
    if (selectedURLs.count() == 1) {
        const KFileItem fileItem(S_IFDIR,
                                 KFileItem::Unknown,
                                 selectedURLs.first(),
                                 true);
        if (fileItem.isDir()) {
            // only one item is selected which is a directory, hence paste
            // into this directory
            destURL = selectedURLs.first();
        }
    }


    updateViewProperties(sourceURLs);
    if (m_clipboardContainsCutData) {
        moveURLs(sourceURLs, destURL);
        m_clipboardContainsCutData = false;
        clipboard->clear();
    }
    else {
        copyURLs(sourceURLs, destURL);
    }
}

void Dolphin::updatePasteAction()
{
    KAction* pasteAction = actionCollection()->action(KStdAction::stdName(KStdAction::Paste));
    if (pasteAction == 0) {
        return;
    }

    QString text(i18n("Paste"));
    QClipboard* clipboard = QApplication::clipboard();
    QMimeSource* data = clipboard->data();
    if (KURLDrag::canDecode(data)) {
        pasteAction->setEnabled(true);

        KURL::List urls;
        KURLDrag::decode(data, urls);
        const int count = urls.count();
        if (count == 1) {
            pasteAction->setText(i18n("Paste 1 File"));
        }
        else {
            pasteAction->setText(i18n("Paste %1 Files").arg(count));
        }
    }
    else {
        pasteAction->setEnabled(false);
        pasteAction->setText(i18n("Paste"));
    }

    if (pasteAction->isEnabled()) {
        KURL::List urls = m_activeView->selectedURLs();
        const uint count = urls.count();
        if (count > 1) {
            // pasting should not be allowed when more than one file
            // is selected
            pasteAction->setEnabled(false);
        }
        else if (count == 1) {
            // Only one file is selected. Pasting is only allowed if this
            // file is a directory.
            const KFileItem fileItem(S_IFDIR,
                                     KFileItem::Unknown,
                                     urls.first(),
                                     true);
            pasteAction->setEnabled(fileItem.isDir());
        }
    }
}

void Dolphin::selectAll()
{
    clearStatusBar();
    m_activeView->selectAll();
}

void Dolphin::invertSelection()
{
    clearStatusBar();
    m_activeView->invertSelection();
}
void Dolphin::setIconsView()
{
    m_activeView->setMode(DolphinView::IconsView);
}

void Dolphin::setDetailsView()
{
    m_activeView->setMode(DolphinView::DetailsView);
}

void Dolphin::setPreviewsView()
{
    m_activeView->setMode(DolphinView::PreviewsView);
}

void Dolphin::sortByName()
{
    m_activeView->setSorting(DolphinView::SortByName);
}

void Dolphin::sortBySize()
{
    m_activeView->setSorting(DolphinView::SortBySize);
}

void Dolphin::sortByDate()
{
    m_activeView->setSorting(DolphinView::SortByDate);
}

void Dolphin::toggleSortOrder()
{
    const Qt::SortOrder order = (m_activeView->sortOrder() == Qt::Ascending) ?
                                Qt::Descending :
                                Qt::Ascending;
    m_activeView->setSortOrder(order);
}

void Dolphin::toggleSplitView()
{
    if (m_view[SecondaryIdx] == 0) {
        const int newWidth = (m_view[PrimaryIdx]->width() - m_splitter->handleWidth()) / 2;

        // create a secondary view
        m_view[SecondaryIdx] = new DolphinView(m_splitter,
                                               m_view[PrimaryIdx]->url(),
                                               m_view[PrimaryIdx]->mode(),
                                               m_view[PrimaryIdx]->isShowHiddenFilesEnabled());

        QValueList<int> list = m_splitter->sizes();
        assert(!list.isEmpty());
        list.pop_back();
        list.append(newWidth);
        list.append(newWidth);
        m_splitter->setSizes(list);
        m_view[SecondaryIdx]->show();
        if(m_rightsidebar != 0){
            closerightSidebar();
            openrightSidebar();
        }
    }
    else {
        // remove secondary view
        if (m_activeView == m_view[PrimaryIdx]) {
            m_view[SecondaryIdx]->close();
            m_view[SecondaryIdx]->deleteLater();
            m_view[SecondaryIdx] = 0;
            setActiveView(m_view[PrimaryIdx]);
        }
        else {
            // The secondary view is active, hence from the users point of view
            // the content of the secondary view should be moved to the primary view.
            // From an implementation point of view it is more efficient to close
            // the primary view and exchange the internal pointers afterwards.
            m_view[PrimaryIdx]->close();
            m_view[PrimaryIdx]->deleteLater();
            m_view[PrimaryIdx] = m_view[SecondaryIdx];
            m_view[SecondaryIdx] = 0;
            setActiveView(m_view[PrimaryIdx]);
        }
    }
}

void Dolphin::reloadView()
{
    clearStatusBar();
    m_activeView->reload();
}

void Dolphin::stopLoading()
{
}

void Dolphin::showHiddenFiles()
{
    clearStatusBar();

    const KToggleAction* showHiddenFilesAction =
        static_cast<KToggleAction*>(actionCollection()->action("show_hidden_files"));
    const bool show = showHiddenFilesAction->isChecked();
    m_activeView->setShowHiddenFilesEnabled(show);
}

void Dolphin::showFilterBar()
{
    const KToggleAction* showFilterBarAction =
        static_cast<KToggleAction*>(actionCollection()->action("show_filter_bar"));
    const bool show = showFilterBarAction->isChecked();
    m_activeView->slotShowFilterBar(show);
}

void Dolphin::zoomIn()
{
    m_activeView->zoomIn();
    updateViewActions();
}

void Dolphin::zoomOut()
{
    m_activeView->zoomOut();
    updateViewActions();
}

void Dolphin::editLocation()
{
    clearStatusBar();
    m_activeView->editURL();
}

void Dolphin::browse()
{
    clearStatusBar();
    m_activeView->setURLEditable(false);
}

void Dolphin::adjustViewProperties()
{
    clearStatusBar();
    ViewPropertiesDialog dlg(m_activeView);
    dlg.exec();
}

void Dolphin::goBack()
{
    clearStatusBar();
    m_activeView->goBack();
}

void Dolphin::goForward()
{
    clearStatusBar();
    m_activeView->goForward();
}

void Dolphin::goUp()
{
    clearStatusBar();
    m_activeView->goUp();
}

void Dolphin::goHome()
{
    clearStatusBar();
    m_activeView->goHome();
}

void Dolphin::openTerminal()
{
    QString command("konsole --workdir \"");
    command.append(m_activeView->url().path());
    command.append('\"');

    KRun::runCommand(command, "Konsole", "konsole");
}

void Dolphin::findFile()
{
    KRun::run("kfind", m_activeView->url());
}

void Dolphin::compareFiles()
{
    // The method is only invoked if exactly 2 files have
    // been selected. The selected files may be:
    // - both in the primary view
    // - both in the secondary view
    // - one in the primary view and the other in the secondary
    //   view
    assert(m_view[PrimaryIdx] != 0);

    KURL urlA;
    KURL urlB;
    KURL::List urls = m_view[PrimaryIdx]->selectedURLs();

    switch (urls.count()) {
        case 0: {
            assert(m_view[SecondaryIdx] != 0);
            urls = m_view[SecondaryIdx]->selectedURLs();
            assert(urls.count() == 2);
            urlA = urls[0];
            urlB = urls[1];
            break;
        }

        case 1: {
            urlA = urls[0];
            assert(m_view[SecondaryIdx] != 0);
            urls = m_view[SecondaryIdx]->selectedURLs();
            assert(urls.count() == 1);
            urlB = urls[0];
            break;
        }

        case 2: {
            urlA = urls[0];
            urlB = urls[1];
            break;
        }

        default: {
            // may not happen: compareFiles may only get invoked if 2
            // files are selected
            assert(false);
        }
    }

    QString command("kompare -c \"");
    command.append(urlA.prettyURL());
    command.append("\" \"");
    command.append(urlB.prettyURL());
    command.append('\"');
    KRun::runCommand(command, "Kompare", "kompare");

}

void Dolphin::editSettings()
{
    // TODO: make a static method for opening the settings dialog
    DolphinSettingsDialog dlg;
    dlg.exec();
}

void Dolphin::addUndoOperation(KIO::Job* job)
{
    if (job->error() != 0) {
        slotHandleJobError(job);
    }
    else {
        const int id = job->progressId();

        // set iterator to the executed command with the current id...
        QValueList<UndoInfo>::Iterator it = m_pendingUndoJobs.begin();
        const QValueList<UndoInfo>::Iterator end = m_pendingUndoJobs.end();
        bool found = false;
        while (!found && (it != end)) {
            if ((*it).id == id) {
                found = true;
            }
            else {
                ++it;
            }
        }

        if (found) {
            DolphinCommand command = (*it).command;
            if (command.type() == DolphinCommand::Trash) {
                // To be able to perform an undo for the 'Move to Trash' operation
                // all source URLs must be updated with the trash URL. E. g. when moving
                // a file "test.txt" and a second file "test.txt" to the trash,
                // then the filenames in the trash are "0-test.txt" and "1-test.txt".
                QMap<QString, QString> metaData = job->metaData();
                KURL::List newSourceURLs;

                KURL::List sourceURLs = command.source();
                KURL::List::Iterator sourceIt = sourceURLs.begin();
                const KURL::List::Iterator sourceEnd = sourceURLs.end();

                while (sourceIt != sourceEnd) {
                    QMap<QString, QString>::ConstIterator metaIt = metaData.find("trashURL-" + (*sourceIt).path());
                    if (metaIt != metaData.end()) {
                        newSourceURLs.append(KURL(metaIt.data()));
                    }
                    ++sourceIt;
                }
                command.setSource(newSourceURLs);
            }

            UndoManager::instance().addCommand(command);
            m_pendingUndoJobs.erase(it);

            DolphinStatusBar* statusBar = m_activeView->statusBar();
            switch (command.type()) {
                case DolphinCommand::Copy:
                    statusBar->setMessage(i18n("Copy operation completed."),
                                          DolphinStatusBar::OperationCompleted);
                    break;
                case DolphinCommand::Move:
                    statusBar->setMessage(i18n("Move operation completed."),
                                          DolphinStatusBar::OperationCompleted);
                    break;
                case DolphinCommand::Trash:
                    statusBar->setMessage(i18n("Move to trash operation completed."),
                                          DolphinStatusBar::OperationCompleted);
                    break;
                default:
                    break;
            }
        }
    }
}

Dolphin::Dolphin() :
    KMainWindow(0, "D3lphin"),
    m_splitter(0),
    m_leftsidebar(0),
    m_rightsidebar(0),
    m_activeView(0),
    m_clipboardContainsCutData(false)
{
    m_view[PrimaryIdx] = 0;
    m_view[SecondaryIdx] = 0;

    m_fileGroupActions.setAutoDelete(true);

    // TODO: the following members are not used yet. See documentation
    // of Dolphin::linkGroupActions() and Dolphin::linkToDeviceActions()
    // in the header file for details.
    //m_linkGroupActions.setAutoDelete(true);
    //m_linkToDeviceActions.setAutoDelete(true);
}

void Dolphin::init()
{
    // Check whether Dolphin runs the first time. If yes then
    // a proper default window size is given at the end of Dolphin::init().
    KConfig* config = kapp->config();
    config->setGroup("General");
    const bool firstRun = config->readBoolEntry("First Run", true);

    setAcceptDrops(true);

    m_splitter = new QSplitter(this);

    DolphinSettings& settings = DolphinSettings::instance();

    KBookmarkManager* manager = settings.bookmarkManager();
    assert(manager != 0);
    KBookmarkGroup root = manager->root();
    if (root.first().isNull()) {
        root.addBookmark(manager, i18n("Home"), settings.homeURL(), "folder_home");
        root.addBookmark(manager, i18n("System"), KURL("system:/"), "system");
        root.addBookmark(manager, i18n("Storage Media"), KURL("system:/media"), "blockdevice");
        root.addBookmark(manager, i18n("Network"), KURL("remote:/"), "network_local");
        root.addBookmark(manager, i18n("Users' Folders"), KURL("system:/users"), "folder_home2");
        root.addBookmark(manager, i18n("Root"), KURL("/"), "folder_red");
        root.addBookmark(manager, i18n("Trash"), KURL("trash:/"), "trashcan_full");
    }

    const KURL& homeURL = root.first().url();
    setCaption(homeURL.fileName());
    ViewProperties props(homeURL);
    m_view[PrimaryIdx] = new DolphinView(m_splitter,
                                         homeURL,
                                         props.viewMode(),
                                         props.isShowHiddenFilesEnabled());

    m_activeView = m_view[PrimaryIdx];

    setCentralWidget(m_splitter);

    // open sidebars
    leftSidebarSettings* leftsidebarSettings = settings.leftsidebar();
    assert(leftsidebarSettings != 0);
    if (leftsidebarSettings->isVisible()) {
        openleftSidebar();
    }
    
    rightSidebarSettings* rightsidebarSettings = settings.rightsidebar();
    assert(rightsidebarSettings != 0);
    if (rightsidebarSettings->isVisible()) {
        openrightSidebar();
    }

    setupActions();
    setupGUI(Keys|Save|Create|ToolBar);
    createGUI(0, false);

    stateChanged("new_file");
    setAutoSaveSettings();

    QClipboard* clipboard = QApplication::clipboard();
    connect(clipboard, SIGNAL(dataChanged()),
            this, SLOT(updatePasteAction()));
    updatePasteAction();
    updateGoActions();

    setupCreateNewMenuActions();

    loadSettings();

    if (firstRun) {
        // assure a proper default size if Dolphin runs the first time
        resize(640, 480);
    }
}

void Dolphin::loadSettings()
{
    DolphinSettings& settings = DolphinSettings::instance();

    KToggleAction* splitAction = static_cast<KToggleAction*>(actionCollection()->action("split_view"));
    if (settings.isViewSplit()) {
        splitAction->setChecked(true);
        toggleSplitView();
    }

    updateViewActions();
}

void Dolphin::setupActions()
{
    // setup 'File' menu
    KAction* createFolder = new KAction(i18n("Folder..."), "Ctrl+N",
                                        this, SLOT(createFolder()),
                                        actionCollection(), "create_folder");
    createFolder->setIcon("folder");

    new KAction(i18n("Rename"), KKey(Key_F2),
                this, SLOT(rename()),
                actionCollection(), "rename");

    KAction* moveToTrashAction = new KAction(i18n("Move to Trash"), KKey(Key_Delete),
                                             this, SLOT(moveToTrash()),
                                             actionCollection(), "move_to_trash");
    moveToTrashAction->setIcon("edittrash");

    KAction* deleteAction = new KAction(i18n("Delete"), "Shift+Delete",
                                        this, SLOT(deleteItems()),
                                        actionCollection(), "delete");
    deleteAction->setIcon("editdelete");

    new KAction(i18n("Propert&ies"), "Alt+Return",
                     this, SLOT(properties()),
                     actionCollection(), "properties");

    KStdAction::quit(this, SLOT(quit()), actionCollection());

    // setup 'Edit' menu
    UndoManager& undoManager = UndoManager::instance();
    KStdAction::undo(&undoManager,
                     SLOT(undo()),
                     actionCollection());
    connect(&undoManager, SIGNAL(undoAvailable(bool)),
            this, SLOT(slotUndoAvailable(bool)));
    connect(&undoManager, SIGNAL(undoTextChanged(const QString&)),
            this, SLOT(slotUndoTextChanged(const QString&)));

    KStdAction::redo(&undoManager,
                     SLOT(redo()),
                     actionCollection());
    connect(&undoManager, SIGNAL(redoAvailable(bool)),
            this, SLOT(slotRedoAvailable(bool)));
    connect(&undoManager, SIGNAL(redoTextChanged(const QString&)),
            this, SLOT(slotRedoTextChanged(const QString&)));

    KStdAction::cut(this, SLOT(cut()), actionCollection());
    KStdAction::copy(this, SLOT(copy()), actionCollection());
    KStdAction::paste(this, SLOT(paste()), actionCollection());

    new KAction(i18n("Select All"), "Ctrl+A",
                this, SLOT(selectAll()),
                actionCollection(), "select_all");

    new KAction(i18n("Invert Selection"), "Ctrl+Shift+A",
                this, SLOT(invertSelection()),
                actionCollection(), "invert_selection");

    // setup 'View' menu
    KStdAction::zoomIn(this,
                       SLOT(zoomIn()),
                       actionCollection());

    KStdAction::zoomOut(this,
                        SLOT(zoomOut()),
                        actionCollection());

    KRadioAction* iconsView = new KRadioAction(i18n("Icons"), "Ctrl+1",
                                                this, SLOT(setIconsView()),
                                                actionCollection(), "icons");
    iconsView->setExclusiveGroup("view_mode");
    iconsView->setIcon("view_icon");

    KRadioAction* detailsView = new KRadioAction(i18n("Details"), "Ctrl+2",
                                                 this, SLOT(setDetailsView()),
                                                 actionCollection(), "details");
    detailsView->setExclusiveGroup("view_mode");
    detailsView->setIcon("view_text");

    KRadioAction* previewsView = new KRadioAction(i18n("Previews"), "Ctrl+3",
                                                   this, SLOT(setPreviewsView()),
                                                   actionCollection(), "previews");
    previewsView->setExclusiveGroup("view_mode");
    previewsView->setIcon("gvdirpart");

    KRadioAction* sortByName = new KRadioAction(i18n("By Name"), 0,
                                                this, SLOT(sortByName()),
                                                actionCollection(), "by_name");
    sortByName->setExclusiveGroup("sort");

    KRadioAction* sortBySize = new KRadioAction(i18n("By Size"), 0,
                                                this, SLOT(sortBySize()),
                                                actionCollection(), "by_size");
    sortBySize->setExclusiveGroup("sort");

    KRadioAction* sortByDate = new KRadioAction(i18n("By Date"), 0,
                                                this, SLOT(sortByDate()),
                                                actionCollection(), "by_date");
    sortByDate->setExclusiveGroup("sort");

    new KToggleAction(i18n("Descending"), 0, this, SLOT(toggleSortOrder()),
                      actionCollection(), "descending");

    new KToggleAction(i18n("Show Hidden Files"), "Alt+.",
                      this, SLOT(showHiddenFiles()),
                      actionCollection(), "show_hidden_files");

    KToggleAction* splitAction = new KToggleAction(i18n("Split View"), "F10",
                                                   this, SLOT(toggleSplitView()),
                                                   actionCollection(), "split_view");
    splitAction->setIcon("view_left_right");

    KAction* reloadAction = new KAction(i18n("Reload"), "F5",
                                        this, SLOT(reloadView()),
                                        actionCollection(), "reload");
    reloadAction->setIcon("reload");

    KAction* stopAction = new KAction(i18n("Stop"), 0,
                                      this, SLOT(stopLoading()),
                                      actionCollection(), "stop");
    stopAction->setIcon("stop");

    new KAction(i18n("Edit Location"), "Ctrl+L",
                this, SLOT(editLocation()),
                actionCollection(), "edit_location");

    new KAction(i18n("Browse"), "Ctrl+B",
                this, SLOT(browse()),
                actionCollection(), "browse");

    new KToggleAction(i18n("Left Sidebar"), "F8",
                      this, SLOT(toggleleftSidebar()),
                      actionCollection(), "leftsidebar");
                      
    new KToggleAction(i18n("Right Sidebar"), "F9",
                      this, SLOT(togglerightSidebar()),
                      actionCollection(), "rightsidebar");

    new KAction(i18n("Adjust View Properties..."), 0,
                this, SLOT(adjustViewProperties()),
                actionCollection(), "view_properties");

    // setup 'Go' menu
    KStdAction::back(this, SLOT(goBack()), actionCollection());
    KStdAction::forward(this, SLOT(goForward()), actionCollection());
    KStdAction::up(this, SLOT(goUp()), actionCollection());
    KStdAction::home(this, SLOT(goHome()), actionCollection());

    // setup 'Tools' menu
    KAction* openTerminalAction = new KAction(i18n("Open Terminal"), "F4",
                                         this, SLOT(openTerminal()),
                                         actionCollection(), "open_terminal");
    openTerminalAction->setIcon("konsole");

    KAction* findFileAction = new KAction(i18n("Find File..."), "Ctrl+F",
                                         this, SLOT(findFile()),
                                         actionCollection(), "find_file");
    findFileAction->setIcon("filefind");

    new KToggleAction(i18n("Show Filter Bar"), "filter", "/",
                      this, SLOT(showFilterBar()),
                      actionCollection(), "show_filter_bar");

    KAction* compareFilesAction = new KAction(i18n("Compare Files"), 0,
                                          this, SLOT(compareFiles()),
                                          actionCollection(), "compare_files");
    compareFilesAction->setIcon("kompare");
    compareFilesAction->setEnabled(false);

    // setup 'Settings' menu
    KStdAction::preferences(this, SLOT(editSettings()), actionCollection());
}

void Dolphin::setupCreateNewMenuActions()
{
    // Parts of the following code have been taken
    // from the class KNewMenu located in
    // libqonq/knewmenu.h of Konqueror.
    //  Copyright (C) 1998, 1999 David Faure <faure@kde.org>
    //                2003       Sven Leiber <s.leiber@web.de>

    QStringList files = actionCollection()->instance()->dirs()->findAllResources("templates");
    for (QStringList::Iterator it = files.begin() ; it != files.end(); ++it) {
        if ((*it)[0] != '.' ) {
            KSimpleConfig config(*it, true);
            config.setDesktopGroup();

            // tricky solution to ensure that TextFile is at the beginning
            // because this filetype is the most used (according kde-core discussion)
            const QString name(config.readEntry("Name"));
            QString key(name);

            const QString path(config.readPathEntry("URL"));
            if (!path.endsWith("emptydir")) {
                if (path.endsWith("TextFile.txt")) {
                    key = "1" + key;
                }
                else if (!KDesktopFile::isDesktopFile(path)) {
                    key = "2" + key;
                }
                else if (path.endsWith("URL.desktop")){
                    key = "3" + key;
                }
                else if (path.endsWith("Program.desktop")){
                    key = "4" + key;
                }
                else {
                    key = "5";
                }

                const QString icon(config.readEntry("Icon"));
                const QString comment(config.readEntry("Comment"));
                const QString type(config.readEntry("Type"));

                const QString filePath(*it);


                if (type == "Link") {
                    CreateFileEntry entry;
                    entry.name = name;
                    entry.icon = icon;
                    entry.comment = comment;
                    entry.templatePath = filePath;
                    m_createFileTemplates.insert(key, entry);
                }
            }
        }
    }
    m_createFileTemplates.sort();

    unplugActionList("create_actions");
    KSortableValueList<CreateFileEntry, QString>::ConstIterator it = m_createFileTemplates.begin();
    KSortableValueList<CreateFileEntry, QString>::ConstIterator end = m_createFileTemplates.end();
    while (it != end) {
        CreateFileEntry entry = (*it).value();
        KAction* action = new KAction(entry.name);
        action->setIcon(entry.icon);
        action->setName((*it).index());
        connect(action, SIGNAL(activated()),
                this, SLOT(createFile()));

        const QChar section = ((*it).index()[0]);
        switch (section) {
            case '1':
            case '2': {
                m_fileGroupActions.append(action);
                break;
            }

            case '3':
            case '4': {
                // TODO: not used yet. See documentation of Dolphin::linkGroupActions()
                // and Dolphin::linkToDeviceActions() in the header file for details.
                //m_linkGroupActions.append(action);
                break;
            }

            case '5': {
                // TODO: not used yet. See documentation of Dolphin::linkGroupActions()
                // and Dolphin::linkToDeviceActions() in the header file for details.
                //m_linkToDeviceActions.append(action);
                break;
            }
            default:
                break;
        }
        ++it;
    }

    plugActionList("create_file_group", m_fileGroupActions);
    //plugActionList("create_link_group", m_linkGroupActions);
    //plugActionList("link_to_device", m_linkToDeviceActions);
}

void Dolphin::updateHistory()
{
    int index = 0;
    const QValueList<URLNavigator::HistoryElem> list = m_activeView->urlHistory(index);

    KAction* backAction = actionCollection()->action("go_back");
    if (backAction != 0) {
        backAction->setEnabled(index < static_cast<int>(list.count()) - 1);
    }

    KAction* forwardAction = actionCollection()->action("go_forward");
    if (forwardAction != 0) {
        forwardAction->setEnabled(index > 0);
    }
}

void Dolphin::updateEditActions()
{
    const KFileItemList* list = m_activeView->selectedItems();
    if ((list == 0) || (*list).isEmpty()) {
        stateChanged("has_no_selection");
    }
    else {
        stateChanged("has_selection");

        KAction* renameAction = actionCollection()->action("rename");
        if (renameAction != 0) {
            renameAction->setEnabled(list->count() >= 1);
        }

        bool enableMoveToTrash = true;

        KFileItemListIterator it(*list);
        KFileItem* item = 0;
        while ((item = it.current()) != 0) {
            const KURL& url = item->url();
            // only enable the 'Move to Trash' action for local files
            if (!url.isLocalFile()) {
                enableMoveToTrash = false;
            }
            ++it;
        }

        KAction* moveToTrashAction = actionCollection()->action("move_to_trash");
        moveToTrashAction->setEnabled(enableMoveToTrash);
    }
    updatePasteAction();
}

void Dolphin::updateViewActions()
{
    KAction* zoomInAction = actionCollection()->action(KStdAction::stdName(KStdAction::ZoomIn));
    if (zoomInAction != 0) {
        zoomInAction->setEnabled(m_activeView->isZoomInPossible());
    }

    KAction* zoomOutAction = actionCollection()->action(KStdAction::stdName(KStdAction::ZoomOut));
    if (zoomOutAction != 0) {
        zoomOutAction->setEnabled(m_activeView->isZoomOutPossible());
    }

    KAction* action = 0;
    switch (m_activeView->mode()) {
        case DolphinView::IconsView:
            action = actionCollection()->action("icons");
            break;
        case DolphinView::DetailsView:
            action = actionCollection()->action("details");
            break;
        case DolphinView::PreviewsView:
            action = actionCollection()->action("previews");
            break;
        default:
            break;
    }

    if (action != 0) {
        KToggleAction* toggleAction = static_cast<KToggleAction*>(action);
        toggleAction->setChecked(true);
    }

    slotSortingChanged(m_activeView->sorting());
    slotSortOrderChanged(m_activeView->sortOrder());

    KToggleAction* showFilterBarAction =
        static_cast<KToggleAction*>(actionCollection()->action("show_filter_bar"));
    showFilterBarAction->setChecked(m_activeView->isFilterBarVisible());

    KToggleAction* showHiddenFilesAction =
        static_cast<KToggleAction*>(actionCollection()->action("show_hidden_files"));
    showHiddenFilesAction->setChecked(m_activeView->isShowHiddenFilesEnabled());

    KToggleAction* splitAction = static_cast<KToggleAction*>(actionCollection()->action("split_view"));
    splitAction->setChecked(m_view[SecondaryIdx] != 0);

    KToggleAction* leftsidebarAction = static_cast<KToggleAction*>(actionCollection()->action("leftsidebar"));
    leftsidebarAction->setChecked(m_leftsidebar != 0);
    
    KToggleAction* rightsidebarAction = static_cast<KToggleAction*>(actionCollection()->action("rightsidebar"));
    rightsidebarAction->setChecked(m_rightsidebar != 0);
}

void Dolphin::updateGoActions()
{
    KAction* goUpAction = actionCollection()->action(KStdAction::stdName(KStdAction::Up));
    const KURL& currentURL = m_activeView->url();
    goUpAction->setEnabled(currentURL.upURL() != currentURL);
}

void Dolphin::updateViewProperties(const KURL::List& urls)
{
    if (urls.isEmpty()) {
        return;
    }

    // Updating the view properties might take up to several seconds
    // when dragging several thousand URLs. Writing a KIO slave for this
    // use case is not worth the effort, but at least the main widget
    // must be disabled and a progress should be shown.
    ProgressIndicator progressIndicator(i18n("Updating view properties..."),
                                        QString::null,
                                        urls.count());

    KURL::List::ConstIterator end = urls.end();
    for(KURL::List::ConstIterator it = urls.begin(); it != end; ++it) {
        progressIndicator.execOperation();

        ViewProperties props(*it);
        props.save();
    }
}

void Dolphin::copyURLs(const KURL::List& source, const KURL& dest)
{
    KIO::Job* job = KIO::copy(source, dest);
    addPendingUndoJob(job, DolphinCommand::Copy, source, dest);
}

void Dolphin::moveURLs(const KURL::List& source, const KURL& dest)
{
    KIO::Job* job = KIO::move(source, dest);
    addPendingUndoJob(job, DolphinCommand::Move, source, dest);
}

void Dolphin::addPendingUndoJob(KIO::Job* job,
                                DolphinCommand::Type commandType,
                                const KURL::List& source,
                                const KURL& dest)
{
    connect(job, SIGNAL(result(KIO::Job*)),
            this, SLOT(addUndoOperation(KIO::Job*)));

    UndoInfo undoInfo;
    undoInfo.id = job->progressId();
    undoInfo.command = DolphinCommand(commandType, source, dest);
    m_pendingUndoJobs.append(undoInfo);
}

void Dolphin::clearStatusBar()
{
    m_activeView->statusBar()->clear();
}

void Dolphin::openleftSidebar()
{
    if (m_leftsidebar != 0) {
        // the sidebar is already open
        return;
    }

    m_leftsidebar = new leftSidebar(m_splitter);
    m_leftsidebar->show();

    connect(m_leftsidebar, SIGNAL(urlChanged(const KURL&)),
            this, SLOT(slotURLChangeRequest(const KURL&)));
    m_splitter->setCollapsible(m_leftsidebar, false);
    m_splitter->setResizeMode(m_leftsidebar, QSplitter::KeepSize);
    m_splitter->moveToFirst(m_leftsidebar);

    leftSidebarSettings* settings = DolphinSettings::instance().leftsidebar();
    settings->setVisible(true);
}

void Dolphin::openrightSidebar()
{
    if (m_rightsidebar != 0) {
        // the sidebar is already open
        return;
    }

    m_rightsidebar = new rightSidebar(m_splitter);
    m_rightsidebar->show();

    connect(m_rightsidebar, SIGNAL(urlChanged(const KURL&)),
            this, SLOT(slotURLChangeRequest(const KURL&)));
    m_splitter->setCollapsible(m_rightsidebar, false);
    m_splitter->setResizeMode(m_rightsidebar, QSplitter::KeepSize);
    m_splitter->moveToLast(m_rightsidebar);

    rightSidebarSettings* settings = DolphinSettings::instance().rightsidebar();
    settings->setVisible(true);
}

void Dolphin::closeleftSidebar()
{
    if (m_leftsidebar == 0) {
        // the sidebar has already been closed
        return;
    }

    // store width of sidebar and remember that the sidebar has been closed
    leftSidebarSettings* settings = DolphinSettings::instance().leftsidebar();
    settings->setVisible(false);
    settings->setWidth(m_leftsidebar->width());

    m_leftsidebar->deleteLater();
    m_leftsidebar = 0;
}

void Dolphin::closerightSidebar()
{
    if (m_rightsidebar == 0) {
        // the sidebar has already been closed
        return;
    }

    // store width of sidebar and remember that the sidebar has been closed
    rightSidebarSettings* settings = DolphinSettings::instance().rightsidebar();
    settings->setVisible(false);
    settings->setWidth(m_rightsidebar->width());

    m_rightsidebar->deleteLater();
    m_rightsidebar = 0;
}

void Dolphin::toggleleftSidebar()
{
    if (m_leftsidebar == 0) {
        openleftSidebar();
    }
    else {
        closeleftSidebar();
    }

    KToggleAction* leftsidebarAction = static_cast<KToggleAction*>(actionCollection()->action("leftsidebar"));
    leftsidebarAction->setChecked(m_leftsidebar != 0);
}

void Dolphin::togglerightSidebar()
{
    if (m_rightsidebar == 0) {
        openrightSidebar();
    }
    else {
        closerightSidebar();
    }

    KToggleAction* rightsidebarAction = static_cast<KToggleAction*>(actionCollection()->action("rightsidebar"));
    rightsidebarAction->setChecked(m_rightsidebar != 0);
}

#include "dolphin.moc"
