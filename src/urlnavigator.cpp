/***************************************************************************
 *   Copyright (C) 2006 by Peter Penz (peter.penz@gmx.at)                  *
 *   Copyright (C) 2006 by Aaron J. Seigo (<aseigo@kde.org>)               *
 *   Copyright (C) 2006 by Patrice Tremblay                                *
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

#include "urlnavigator.h"

#include <assert.h>
#include <kurl.h>
#include <qobjectlist.h>
#include <qcombobox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <qpopupmenu.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qsizepolicy.h>
#include <qtooltip.h>
#include <qfont.h>
#include <qlistbox.h>

#include <kio/job.h>
#include <kurlcombobox.h>
#include <kurlcompletion.h>
#include <kbookmarkmanager.h>

#include "dolphin.h"
#include "dolphinsettings.h"
#include "bookmarkselector.h"
#include "dolphinstatusbar.h"
#include "urlnavigatorbutton.h"
#include "dolphinview.h"

URLNavigator::HistoryElem::HistoryElem()
 :  m_url(),
    m_currentFileName(),
    m_contentsX(0),
    m_contentsY(0)
{
}

URLNavigator::HistoryElem::HistoryElem(const KURL& url)
 :  m_url(url),
    m_currentFileName(),
    m_contentsX(0),
    m_contentsY(0)
{
}

URLNavigator::HistoryElem::~HistoryElem()
{
}

URLNavigator::URLNavigator(const KURL& url,
                           DolphinView* dolphinView) :
    QHBox(dolphinView),
    m_historyIndex(0),
    m_dolphinView(dolphinView)
{
    m_history.prepend(HistoryElem(url));

    QFontMetrics fontMetrics(font());
    setMinimumHeight(fontMetrics.height() + 8);

    m_toggleButton = new QPushButton(SmallIcon("editurl"), 0, this);
    m_toggleButton->setFlat(true);
    m_toggleButton->setToggleButton(true);
    m_toggleButton->setFocusPolicy(QWidget::NoFocus);
    m_toggleButton->setMinimumHeight(minimumHeight());
    connect(m_toggleButton, SIGNAL(clicked()),
            this, SLOT(slotClicked()));
    if (DolphinSettings::instance().isURLEditable()) {
        m_toggleButton->toggle();
    }

    m_bookmarkSelector = new BookmarkSelector(this);
    connect(m_bookmarkSelector, SIGNAL(bookmarkActivated(int)),
            this, SLOT(slotBookmarkActivated(int)));

    m_pathBox = new KURLComboBox(KURLComboBox::Directories, true, this);

    KURLCompletion* kurlCompletion = new KURLCompletion(KURLCompletion::DirCompletion);
    m_pathBox->setCompletionObject(kurlCompletion);
    m_pathBox->setAutoDeleteCompletionObject(true);

    connect(m_pathBox, SIGNAL(returnPressed(const QString&)),
            this, SLOT(slotReturnPressed(const QString&)));
    connect(m_pathBox, SIGNAL(urlActivated(const KURL&)),
            this, SLOT(slotURLActivated(const KURL&)));

    connect(dolphinView, SIGNAL(contentsMoved(int, int)),
            this, SLOT(slotContentsMoved(int, int)));
    updateContent();
}

URLNavigator::~URLNavigator()
{
}

void URLNavigator::setURL(const KURL& url)
{
    QString urlStr(url.prettyURL());

    if (url.protocol() == "zip") {
       bool stillInside = false;
       if (KMimeType::findByPath(url.url(-1))
           ->is("application/x-zip")) {
           stillInside = true;
       }
       else {
           KURL url1 = url.upURL();
           while (url1 != url1.upURL()) {
               if (KMimeType::findByPath(url1.url(-1))
                   ->is("application/x-zip")) {
                   stillInside = true;
                   break;
               }
               url1 = url1.upURL();
           }
       }
       if (!stillInside)
       {
           // Drop the zip:/ protocol since we are not in the zip anymore
           urlStr = url.path();
       }
    }
    else if (url.protocol() == "tar")
    {
       bool stillInside = false;
       KMimeType::Ptr kmp = 
           KMimeType::findByPath(url.url(-1));
       if (kmp->is("application/x-tar") ||
           kmp->is("application/x-tarz") ||
           kmp->is("application/x-tbz") || 
           kmp->is("application/x-tgz") || 
           kmp->is("application/x-tzo")
           ) {
           stillInside = true;
       }
       else {
           KURL url1 = url.upURL();
           while (url1 != url1.upURL()) {
               KMimeType::Ptr kmp =
                   KMimeType::findByPath(url1.url(-1));
               if (kmp->is("application/x-tar") ||
                   kmp->is("application/x-tarz") ||
                   kmp->is("application/x-tbz") || 
                   kmp->is("application/x-tgz") || 
                   kmp->is("application/x-tzo")
                   ) {
                   stillInside = true;
                   break;
               }
               url1 = url1.upURL();
           }
       }
       if (!stillInside)
       {
           // Drop the tar:/ protocol since we are not in the tar anymore
           urlStr = url.path();
       }
    }


    if (urlStr.at(0) == '~') {
        // replace '~' by the home directory
        urlStr.remove(0, 1);
        urlStr.insert(0, QDir::home().path());
    }

    const KURL transformedURL(urlStr);

    if (m_historyIndex > 0) {
        // Check whether the previous element of the history has the same URL.
        // If yes, just go forward instead of inserting a duplicate history
        // element.
        const KURL& nextURL = m_history[m_historyIndex - 1].url();
        if (transformedURL == nextURL) {
            goForward();
            return;
        }
    }

    const KURL& currURL = m_history[m_historyIndex].url();
    if (currURL == transformedURL) {
        // don't insert duplicate history elements
        return;
    }

    updateHistoryElem();

    const QValueListIterator<URLNavigator::HistoryElem> it = m_history.at(m_historyIndex);
    m_history.insert(it, HistoryElem(transformedURL));
    updateContent();
    emit urlChanged(transformedURL);
    emit historyChanged();

    // Prevent an endless growing of the history: remembering
    // the last 100 URLs should be enough...
    if (m_historyIndex > 100) {
        m_history.erase(m_history.begin());
        --m_historyIndex;
    }
}

const KURL& URLNavigator::url() const
{
    assert(!m_history.empty());
    return m_history[m_historyIndex].url();
}

KURL URLNavigator::url(int index) const
{
    assert(index >= 0);
    QString path(url().prettyURL());
    path = path.section('/', 0, index);

    if (path.at(path.length()) != '/')
    {
        path.append('/');
    }

    return path;
}

const QValueList<URLNavigator::HistoryElem>& URLNavigator::history(int& index) const
{
    index = m_historyIndex;
    return m_history;
}

void URLNavigator::goBack()
{
    updateHistoryElem();

    const int count = m_history.count();
    if (m_historyIndex < count - 1) {
        ++m_historyIndex;
        updateContent();
        emit urlChanged(url());
        emit historyChanged();
    }
}

void URLNavigator::goForward()
{
    if (m_historyIndex > 0) {
        --m_historyIndex;
        updateContent();
        emit urlChanged(url());
        emit historyChanged();
    }
}

void URLNavigator::goUp()
{
    setURL(url().upURL());
}

void URLNavigator::goHome()
{
    setURL(DolphinSettings::instance().homeURL());
}

void URLNavigator::setURLEditable(bool editable)
{
    if (isURLEditable() != editable) {
        m_toggleButton->toggle();
        slotClicked();
    }
}

bool URLNavigator::isURLEditable() const
{
    return m_toggleButton->state() == QButton::On;
}

void URLNavigator::editURL()
{
    setURLEditable(true);
    m_pathBox->setFocus();
}

DolphinView* URLNavigator::dolphinView() const
{
    return m_dolphinView;
}

void URLNavigator::keyReleaseEvent(QKeyEvent* event)
{
    QHBox::keyReleaseEvent(event);
    if (isURLEditable() && (event->key() == Qt::Key_Escape)) {
        setURLEditable(false);
    }
}

void URLNavigator::slotReturnPressed(const QString& text)
{
    // Parts of the following code have been taken
    // from the class KateFileSelector located in
    // kate/app/katefileselector.hpp of Kate.
    // Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
    // Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
    // Copyright (C) 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>

    KURL typedURL(text);
    if (typedURL.hasPass()) {
        typedURL.setPass(QString::null);
    }

    QStringList urls = m_pathBox->urls();
    urls.remove(typedURL.url());
    urls.prepend(typedURL.url());
    m_pathBox->setURLs(urls, KURLComboBox::RemoveBottom);

    setURL(typedURL);
    // The URL might have been adjusted by URLNavigator::setURL(), hence
    // synchronize the result in the path box.
    m_pathBox->setURL(url());
}

void URLNavigator::slotURLActivated(const KURL& url)
{
    setURL(url);
}

void URLNavigator::slotRequestActivation()
{
    m_dolphinView->requestActivation();
}

void URLNavigator::slotBookmarkActivated(int index)
{
    m_dolphinView->statusBar()->clear();
    m_dolphinView->requestActivation();

    KBookmark bookmark = DolphinSettings::instance().bookmark(index);
    m_dolphinView->setURL(bookmark.url());
}

void URLNavigator::slotContentsMoved(int x, int y)
{
    m_history[m_historyIndex].setContentsX(x);
    m_history[m_historyIndex].setContentsY(y);
}

void URLNavigator::slotClicked()
{
    updateContent();
    if (isURLEditable()) {
        m_pathBox->setFocus();
    }
    else {
        m_dolphinView->setFocus();
    }
}

void URLNavigator::updateHistoryElem()
{
    assert(m_historyIndex >= 0);
    const KFileItem* item = m_dolphinView->currentFileItem();
    if (item != 0) {
        m_history[m_historyIndex].setCurrentFileName(item->name());
    }
    m_history[m_historyIndex].setContentsX(m_dolphinView->contentsX());
    m_history[m_historyIndex].setContentsY(m_dolphinView->contentsY());
}

void URLNavigator::updateContent()
{
    const QObjectList* list = children();
    if (list == 0) {
        return;
    }

    // set the iterator to the first URL navigator button
    QObjectListIterator it(*list);
    QObject* object = 0;
    while ((object = it.current()) != 0) {
        if (object->inherits("URLNavigatorButton")) {
            break;
        }
        ++it;
    }

    // delete all existing URL navigator buttons
    QPtrList<QWidget> deleteList;
    while ((object = it.current()) != 0) {
        if (object->inherits("URLNavigatorButton")) {
            // Don't close and delete the navigator button immediatly, otherwise
            // the iterator won't work anymore and an object would get deleted more
            // than once (-> crash).
            deleteList.append(static_cast<QWidget*>(object));
        }
        ++it;
    }

    // now close and delete all unused navigator buttons
    QPtrListIterator<QWidget> deleteIter(deleteList);
    QWidget* widget = 0;
    while ((widget = deleteIter.current()) != 0) {
        widget->close();
        widget->deleteLater();
        ++deleteIter;
    }

    m_bookmarkSelector->updateSelection(url());

    QToolTip::remove(m_toggleButton);
    QString path(url().prettyURL());
    if (m_toggleButton->state() == QButton::On) {
        // TODO: don't hardcode the shortcut as part of the text
        QToolTip::add(m_toggleButton, i18n("Browse (Ctrl+B, Escape)"));

        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        m_pathBox->show();
        m_pathBox->setURL(url());
    }
    else {
        // TODO: don't hardcode the shortcut as part of the text
        QToolTip::add(m_toggleButton, i18n("Edit location (Ctrl+L)"));

        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_pathBox->hide();
        QString dir_name;

        // get the data from the currently selected bookmark
        KBookmark bookmark = m_bookmarkSelector->selectedBookmark();
        //int bookmarkIndex = m_bookmarkSelector->selectedIndex();

        QString bookmarkPath;
        if (bookmark.isNull()) {
            // No bookmark is a part of the current URL.
            // The following code tries to guess the bookmark
            // path. E. g. "fish://root@192.168.0.2/var/lib" writes
            // "fish://root@192.168.0.2" to 'bookmarkPath', which leads to the
            // navigation indication 'Custom Path > var > lib".
            int idx = path.find(QString("//"));
            idx = path.find("/", (idx < 0) ? 0 : idx + 2);
            bookmarkPath = (idx < 0) ? path : path.left(idx);
        }
        else {
            bookmarkPath = bookmark.url().prettyURL();
        }
        const uint len = bookmarkPath.length();

        // calculate the start point for the URL navigator buttons by counting
        // the slashs inside the bookmark URL
        int slashCount = 0;
        for (uint i = 0; i < len; ++i) {
            if (bookmarkPath.at(i) == QChar('/')) {
                ++slashCount;
            }
        }
        if ((len > 0) && bookmarkPath.at(len - 1) == QChar('/')) {
            assert(slashCount > 0);
            --slashCount;
        }

        // create URL navigator buttons
        int idx = slashCount;
        bool hasNext = true;
        do {
            dir_name = path.section('/', idx, idx);
            const bool isFirstButton = (idx == slashCount);
            hasNext = isFirstButton || !dir_name.isEmpty();
            if (hasNext) {
                URLNavigatorButton* button = new URLNavigatorButton(idx, this);
                if (isFirstButton) {
                    // the first URL navigator button should get the name of the bookmark
                    // instead of the directory name
                    QString text = bookmark.text();
                    if (text.isEmpty()) {
                        text = bookmarkPath;
                    }
                    button->setText(text);
                }
                button->show();
                ++idx;
            }
        } while (hasNext);
    }
}

#include "urlnavigator.moc"
