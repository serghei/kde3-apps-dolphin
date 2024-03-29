/***************************************************************************
 *   Copyright (C) 2006 by Peter Penz <peter.penz@gmx.at>                  *
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

#include "infosidebarpage.h"
#include <assert.h>

#include <qlayout.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qpushbutton.h>
#include <qvbox.h>
#include <qvgroupbox.h>
#include <qpopupmenu.h>
#include <qpainter.h>
#include <qfontmetrics.h>
#include <qgrid.h>
#include <qhgroupbox.h>

#include <kbookmarkmanager.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kio/previewjob.h>
#include <kfileitem.h>
#include <kdialog.h>
#include <kglobalsettings.h>
#include <kfilemetainfo.h>

#include "dolphin.h"
#include "pixmapviewer.h"
#include "dolphinsettings.h"

InfoSidebarPage::InfoSidebarPage(QWidget* parent) :
    SidebarPage(parent),
    m_multipleSelection(false),
    m_pendingPreview(false),
    m_timer(0),
    m_preview(0),
    m_name(0),
    m_currInfoLineIdx(0),
    m_infoGrid(0),
    m_actionBox(0)
{
    const int spacing = KDialog::spacingHint();

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()),
            this, SLOT(slotTimeout()));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(spacing);

    // preview
    m_preview = new PixmapViewer(this);
    m_preview->setMinimumWidth(KIcon::SizeEnormous);
    m_preview->setFixedHeight(KIcon::SizeEnormous);

    // name
    m_name = new QLabel(this);
    m_name->setTextFormat(Qt::RichText);
    m_name->setAlignment(m_name->alignment() | Qt::AlignHCenter);
    QFontMetrics fontMetrics(m_name->font());
    m_name->setMinimumHeight(fontMetrics.height() * 3);
    m_name->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

    QWidget* sep1 = new QHGroupBox(this);  // TODO: check whether default widget exist for this?
    sep1->setFixedHeight(1);

    // general information
    m_infoGrid = new QGrid(2, this);
    m_infoGrid->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QWidget* sep2 = new QHGroupBox(this);  // TODO: check whether default widget exist for this?
    sep2->setFixedHeight(1);

    // actions
    m_actionBox = new QVBox(this);
    m_actionBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // Add a dummy widget with no restriction regarding a vertical resizing.
    // This assures that information is always top aligned.
    QWidget* dummy = new QWidget(this);

    layout->addItem(new QSpacerItem(spacing, spacing, QSizePolicy::Preferred, QSizePolicy::Fixed));
    layout->addWidget(m_preview);
    layout->addWidget(m_name);
    layout->addWidget(sep1);
    layout->addWidget(m_infoGrid);
    layout->addWidget(sep2);
    layout->addWidget(m_actionBox);
    layout->addWidget(dummy);

    connect(&Dolphin::mainWin(), SIGNAL(selectionChanged()),
            this, SLOT(showItemInfo()));

    connectToActiveView();
}

InfoSidebarPage::~InfoSidebarPage()
{
}

void InfoSidebarPage::activeViewChanged()
{
    connectToActiveView();
}

void InfoSidebarPage::requestDelayedItemInfo(const KURL& url)
{
    cancelRequest();

    if (!url.isEmpty() && !m_multipleSelection) {
        m_urlCandidate = url;
        m_timer->start(300, true);
    }
}

void InfoSidebarPage::requestItemInfo(const KURL& url)
{
    cancelRequest();

    if (!url.isEmpty() && !m_multipleSelection) {
        m_shownURL = url;
        showItemInfo();
    }
}

void InfoSidebarPage::showItemInfo()
{
    cancelRequest();

    m_multipleSelection = false;

    // show the preview...
    DolphinView* view = Dolphin::mainWin().activeView();
    const KFileItemList* selectedItems = view->selectedItems();
    if ((selectedItems != 0) && selectedItems->count() > 1) {
        m_multipleSelection = true;
    }

    if (m_multipleSelection) {
        KIconLoader iconLoader;
        QPixmap icon = iconLoader.loadIcon("exec",
                                           KIcon::NoGroup,
                                           KIcon::SizeEnormous);
        m_preview->setPixmap(icon);
        m_name->setText(i18n("%n items selected", "%n items selected", selectedItems->count()));
    }
    else if (!applyBookmark()) {
        // try to get a preview pixmap from the item...
        KURL::List list;
        list.append(m_shownURL);

        m_pendingPreview = true;
        m_preview->setPixmap(QPixmap());

        KIO::PreviewJob* job = KIO::filePreview(list,
                                                m_preview->width(),
                                                KIcon::SizeEnormous);
        connect(job, SIGNAL(gotPreview(const KFileItem*, const QPixmap&)),
                this, SLOT(gotPreview(const KFileItem*, const QPixmap&)));
        connect(job, SIGNAL(failed(const KFileItem*)),
                this, SLOT(slotPreviewFailed(const KFileItem*)));

        QString text("<b>");
        text.append(m_shownURL.fileName());
        text.append("</b>");
        m_name->setText(text);
    }

    createMetaInfo();
    insertActions();
}

void InfoSidebarPage::slotTimeout()
{
    m_shownURL = m_urlCandidate;
    showItemInfo();
}

void InfoSidebarPage::slotPreviewFailed(const KFileItem* item)
{
    m_pendingPreview = false;
    if (!applyBookmark()) {
        m_preview->setPixmap(item->pixmap(KIcon::SizeEnormous));
    }
}

void InfoSidebarPage::gotPreview(const KFileItem* /* item */,
                                 const QPixmap& pixmap)
{
    if (m_pendingPreview) {
        m_preview->setPixmap(pixmap);
        m_pendingPreview = false;
    }
}

void InfoSidebarPage::startService(int index)
{
    DolphinView* view = Dolphin::mainWin().activeView();
    if (view->hasSelection()) {
        KURL::List selectedURLs = view->selectedURLs();
        KDEDesktopMimeType::executeService(selectedURLs, m_actionsVector[index]);
    }
    else {
        KDEDesktopMimeType::executeService(m_shownURL, m_actionsVector[index]);
    }
}

void InfoSidebarPage::connectToActiveView()
{
    cancelRequest();

    DolphinView* view = Dolphin::mainWin().activeView();
    connect(view, SIGNAL(signalRequestItemInfo(const KURL&)),
            this, SLOT(requestDelayedItemInfo(const KURL&)));
    connect(view, SIGNAL(signalURLChanged(const KURL&)),
            this, SLOT(requestItemInfo(const KURL&)));

    m_shownURL = view->url();
    showItemInfo();
}

bool InfoSidebarPage::applyBookmark()
{
    KBookmarkGroup root = DolphinSettings::instance().bookmarkManager()->root();
    KBookmark bookmark = root.first();
    while (!bookmark.isNull()) {
        if (m_shownURL.equals(bookmark.url(), true)) {
            QString text("<b>");
            text.append(bookmark.text());
            text.append("</b>");
            m_name->setText(text);

            KIconLoader iconLoader;
            QPixmap icon = iconLoader.loadIcon(bookmark.icon(),
                                               KIcon::NoGroup,
                                               KIcon::SizeEnormous);
            m_preview->setPixmap(icon);
            return true;
        }
        bookmark = root.next(bookmark);
    }

    return false;
}

void InfoSidebarPage::cancelRequest()
{
    m_timer->stop();
    m_pendingPreview = false;
}

void InfoSidebarPage::createMetaInfo()
{
    // To prevent a flickering it's important to reuse available
    // labels instead of deleting them and recreate them afterwards.
    // The methods beginInfoLines(), addInfoLine() and endInfoLines()
    // take care of this.
    beginInfoLines();
    DolphinView* view = Dolphin::mainWin().activeView();
    if (!view->hasSelection()) {
        KFileItem fileItem(S_IFDIR, KFileItem::Unknown, m_shownURL);
        fileItem.refresh();

        if (fileItem.isDir()) {
            addInfoLine(i18n("Type:"), i18n("Directory"));
        }
        else {
            addInfoLine(i18n("Type:"), fileItem.mimeComment());

            QString sizeText(KIO::convertSize(fileItem.size()));
            addInfoLine(i18n("Size:"), sizeText);
            addInfoLine(i18n("Modified:"), fileItem.timeString());

            const KFileMetaInfo& metaInfo = fileItem.metaInfo();
            if (metaInfo.isValid()) {
                QStringList keys = metaInfo.supportedKeys();
                for (QStringList::Iterator it = keys.begin(); it != keys.end(); ++it) {
                    if (showMetaInfo(*it)) {
                        KFileMetaInfoItem metaInfoItem = metaInfo.item(*it);
                        addInfoLine(*it, metaInfoItem.string());
                    }
                }
            }
        }
    }
    endInfoLines();
}

void InfoSidebarPage::beginInfoLines()
{
    m_currInfoLineIdx = 0;
}

void InfoSidebarPage::endInfoLines()
{
    if (m_currInfoLineIdx <= 0) {
        return;
    }

    // remove labels which have not been used
    if (m_currInfoLineIdx < static_cast<int>(m_infoWidgets.count())) {
        QPtrListIterator<QLabel> deleteIter(m_infoWidgets);
        deleteIter += m_currInfoLineIdx;

        QWidget* widget = 0;
        int removeCount = 0;
        while ((widget = deleteIter.current()) != 0) {
            widget->close();
            widget->deleteLater();
            ++deleteIter;
            ++removeCount;
        }
        for (int i = 0; i < removeCount; ++i) {
            m_infoWidgets.removeLast();
        }
    }
}

bool InfoSidebarPage::showMetaInfo(const QString& key) const
{
    // sorted list of keys, where it's data should be shown
    static const char* keys[] = {
        "Album",
        "Artist",
        "Author",
        "Bitrate",
        "Date",
        "Dimensions",
        "Genre",
        "Length",
        "Lines",
        "Pages",
        "Title",
        "Words"
    };

    // do a binary search for the key...
    int top = 0;
    int bottom = sizeof(keys) / sizeof(char*) - 1;
    while (top < bottom) {
        const int middle = (top + bottom) / 2;
        const int result = key.compare(keys[middle]);
        if (result < 0) {
            bottom = middle - 1;
        }
        else if (result > 0) {
            top = middle + 1;
        }
        else {
            return true;
        }
    }

    return false;
}

void InfoSidebarPage::addInfoLine(const QString& labelText, const QString& infoText)
{
    QString labelStr("<b>");
    labelStr.append(labelText);
    labelStr.append("</b>&nbsp;");

    const int count = m_infoWidgets.count();
    if (m_currInfoLineIdx < count - 1) {
        // reuse available labels
        m_infoWidgets.at(m_currInfoLineIdx++)->setText(labelStr);
        m_infoWidgets.at(m_currInfoLineIdx++)->setText(infoText);
    }
    else {
        // no labels are available anymore, hence create 2 new ones
        QLabel* label = new QLabel(labelStr, m_infoGrid);
        label->setTextFormat(Qt::RichText);
        label->setAlignment(Qt::AlignRight |
                            Qt::AlignTop);
        label->show();
        m_infoWidgets.append(label);

        QLabel* info = new QLabel(infoText, m_infoGrid);
        info->setTextFormat(Qt::RichText);
        info->setAlignment(Qt::AlignTop | Qt::WordBreak);
        info->show();
        m_infoWidgets.append(info);

        m_currInfoLineIdx += 2;
    }
}

void InfoSidebarPage::insertActions()
{
    // delete all existing action widgets
    // TODO: just use children() from QObject...
    QPtrListIterator<QWidget> deleteIter(m_actionWidgets);
    QWidget* widget = 0;
    while ((widget = deleteIter.current()) != 0) {
        widget->close();
        widget->deleteLater();
        ++deleteIter;
    }

    m_actionWidgets.clear();
    m_actionsVector.clear();

    int actionsIndex = 0;

    // The algorithm for searching the available actions works on a list
    // of KFileItems. If no selection is given, a temporary KFileItem
    // by the given URL 'url' is created and added to the list.
    KFileItem fileItem(S_IFDIR, KFileItem::Unknown, m_shownURL);
    KFileItemList localList;
    const KFileItemList* itemList = Dolphin::mainWin().activeView()->selectedItems();
    if ((itemList == 0) || itemList->isEmpty()) {
        fileItem.refresh();
        localList.append(&fileItem);
        itemList = &localList;
    }

    // 'itemList' contains now all KFileItems, where an item information should be shown.
    // TODO: the following algorithm is quite equal to DolphinContextMenu::insertActionItems().
    // It's open yet whether they should be merged or whether they have to work slightly different.
    QStringList dirs = KGlobal::dirs()->findDirs("data", "d3lphin/servicemenus/");
    for (QStringList::ConstIterator dirIt = dirs.begin(); dirIt != dirs.end(); ++dirIt) {
        QDir dir(*dirIt);
        QStringList entries = dir.entryList("*.desktop", QDir::Files);

        for (QStringList::ConstIterator entryIt = entries.begin(); entryIt != entries.end(); ++entryIt) {
            KSimpleConfig cfg(*dirIt + *entryIt, true);
            cfg.setDesktopGroup();
            if ((cfg.hasKey("Actions") || cfg.hasKey("X-KDE-GetActionMenu")) && cfg.hasKey("ServiceTypes")) {
                const QStringList types = cfg.readListEntry("ServiceTypes");
                for (QStringList::ConstIterator it = types.begin(); it != types.end(); ++it) {
                    // check whether the mime type is equal or whether the
                    // mimegroup (e. g. image/*) is supported

                    bool insert = false;
                    if ((*it) == "all/allfiles") {
                        // The service type is valid for all files, but not for directories.
                        // Check whether the selected items only consist of files...
                        KFileItemListIterator mimeIt(*itemList);
                        KFileItem* item = 0;
                        insert = true;
                        while (insert && ((item = mimeIt.current()) != 0)) {
                            insert = !item->isDir();
                            ++mimeIt;
                        }
                    }

                    if (!insert) {
                        // Check whether the MIME types of all selected files match
                        // to the mimetype of the service action. As soon as one MIME
                        // type does not match, no service menu is shown at all.
                        KFileItemListIterator mimeIt(*itemList);
                        KFileItem* item = 0;
                        insert = true;
                        while (insert && ((item = mimeIt.current()) != 0)) {
                            const QString mimeType((*mimeIt)->mimetype());
                            const QString mimeGroup(mimeType.left(mimeType.find('/')));

                            insert  = (*it == mimeType) ||
                                      ((*it).right(1) == "*") &&
                                      ((*it).left((*it).find('/')) == mimeGroup);
                            ++mimeIt;
                        }
                    }

                    if (insert) {
                        const QString submenuName = cfg.readEntry( "X-KDE-Submenu" );
                        QPopupMenu* popup = 0;
                        if (!submenuName.isEmpty()) {
                            // create a sub menu containing all actions
                            popup = new QPopupMenu();
                            connect(popup, SIGNAL(activated(int)),
                                    this, SLOT(startService(int)));

                            QPushButton* button = new QPushButton(submenuName, m_actionBox);
                            button->setFlat(true);
                            button->setPopup(popup);
                            button->show();
                            m_actionWidgets.append(button);
                        }

                        QValueList<KDEDesktopMimeType::Service> userServices =
                            KDEDesktopMimeType::userDefinedServices(*dirIt + *entryIt, true);

                        // iterate through all actions and add them to a widget
                        QValueList<KDEDesktopMimeType::Service>::Iterator serviceIt;
                        for (serviceIt = userServices.begin(); serviceIt != userServices.end(); ++serviceIt) {
                            KDEDesktopMimeType::Service service = (*serviceIt);
                            if (popup == 0) {
                                ServiceButton* button = new ServiceButton(SmallIcon(service.m_strIcon),
                                                                          service.m_strName,
                                                                          m_actionBox,
                                                                          actionsIndex);
                                connect(button, SIGNAL(requestServiceStart(int)),
                                        this, SLOT(startService(int)));
                                m_actionWidgets.append(button);
                                button->show();
                            }
                            else {
                                popup->insertItem(SmallIcon(service.m_strIcon), service.m_strName, actionsIndex);
                            }

                            m_actionsVector.append(service);
                            ++actionsIndex;
                        }
                    }
                }
            }
        }
    }
}

ServiceButton::ServiceButton(const QIconSet& icon,
                             const QString& text,
                             QWidget* parent,
                             int index) :
    QPushButton(icon, text, parent),
    m_hover(false),
    m_index(index)
{
    setEraseColor(colorGroup().background());
    setFocusPolicy(QWidget::NoFocus);
    connect(this, SIGNAL(released()),
            this, SLOT(slotReleased()));
}

ServiceButton::~ServiceButton()
{
}

void ServiceButton::drawButton(QPainter* painter)
{
    const int buttonWidth  = width();
    const int buttonHeight = height();

    QColor backgroundColor;
    QColor foregroundColor;
    if (m_hover) {
        backgroundColor = KGlobalSettings::highlightColor();
        foregroundColor = KGlobalSettings::highlightedTextColor();
    }
    else {
        backgroundColor = colorGroup().background();
        foregroundColor = KGlobalSettings::buttonTextColor();
    }

    // draw button background
    painter->setPen(NoPen);
    painter->setBrush(backgroundColor);
    painter->drawRect(0, 0, buttonWidth, buttonHeight);

    const int spacing = KDialog::spacingHint();

    // draw icon
    int x = spacing;
    const int y = (buttonHeight - KIcon::SizeSmall) / 2;
    const QIconSet* set = iconSet();
    if (set != 0) {
        painter->drawPixmap(x, y, set->pixmap(QIconSet::Small, QIconSet::Normal));
    }
    x += KIcon::SizeSmall + spacing;

    // draw text
    painter->setPen(foregroundColor);

    const int textWidth = buttonWidth - x;
    QFontMetrics fontMetrics(font());
    const bool clipped = fontMetrics.width(text()) >= textWidth;
    //const int align = clipped ? Qt::AlignVCenter : Qt::AlignCenter;
    painter->drawText(QRect(x, 0, textWidth, buttonHeight), Qt::AlignVCenter, text());

    if (clipped) {
        // Blend the right area of the text with the background, as the
        // text is clipped.
        // TODO #1: use alpha blending in Qt4 instead of drawing the text that often
        // TODO #2: same code as in URLNavigatorButton::drawButton() -> provide helper class?
        const int blendSteps = 16;

        QColor blendColor(backgroundColor);
        const int redInc   = (foregroundColor.red()   - backgroundColor.red())   / blendSteps;
        const int greenInc = (foregroundColor.green() - backgroundColor.green()) / blendSteps;
        const int blueInc  = (foregroundColor.blue()  - backgroundColor.blue())  / blendSteps;
        for (int i = 0; i < blendSteps; ++i) {
            painter->setClipRect(QRect(x + textWidth - i, 0, 1, buttonHeight));
            painter->setPen(blendColor);
            painter->drawText(QRect(x, 0, textWidth, buttonHeight), Qt::AlignVCenter, text());

            blendColor.setRgb(blendColor.red()   + redInc,
                              blendColor.green() + greenInc,
                              blendColor.blue()  + blueInc);
        }
    }
}

void ServiceButton::enterEvent(QEvent* event)
{
    QPushButton::enterEvent(event);
    m_hover = true;
    update();
}

void ServiceButton::leaveEvent(QEvent* event)
{
    QPushButton::leaveEvent(event);
    m_hover = false;
    update();
}

void ServiceButton::slotReleased()
{
    emit requestServiceStart(m_index);
}

#include "infosidebarpage.moc"

