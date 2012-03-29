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

#include "bookmarkssidebarpage.h"

#include <qlistbox.h>
#include <qlayout.h>
#include <qpainter.h>
#include <assert.h>
#include <qpopupmenu.h>

#include <kbookmarkmanager.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kurldrag.h>

#include "dolphinsettings.h"
#include "dolphin.h"
#include "dolphinview.h"
#include "editbookmarkdialog.h"

BookmarksSidebarPage::BookmarksSidebarPage(QWidget* parent) :
    SidebarPage(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    m_bookmarksList = new BookmarksListBox(this);
    m_bookmarksList->setPaletteBackgroundColor(colorGroup().background());

    layout->addWidget(m_bookmarksList);
    connect(m_bookmarksList, SIGNAL(mouseButtonClicked(int, QListBoxItem*, const QPoint&)),
            this, SLOT(slotMouseButtonClicked(int, QListBoxItem*)));
    connect(m_bookmarksList, SIGNAL(contextMenuRequested(QListBoxItem*, const QPoint&)),
            this, SLOT(slotContextMenuRequested(QListBoxItem*, const QPoint&)));

    KBookmarkManager* manager = DolphinSettings::instance().bookmarkManager();
    connect(manager, SIGNAL(changed(const QString&, const QString&)),
            this, SLOT(updateBookmarks()));

    updateBookmarks();
}

BookmarksSidebarPage::~BookmarksSidebarPage()
{
}

void BookmarksSidebarPage::activeViewChanged()
{
    connectToActiveView();
}

void BookmarksSidebarPage::updateBookmarks()
{
    m_bookmarksList->clear();

    KIconLoader iconLoader;

    KBookmarkGroup root = DolphinSettings::instance().bookmarkManager()->root();
    KBookmark bookmark = root.first();
    while (!bookmark.isNull()) {
        m_bookmarksList->insertItem( BookmarkItem::fromKbookmark(bookmark, iconLoader) );

        bookmark = root.next(bookmark);
    }

    connectToActiveView();
}

void BookmarksSidebarPage::slotMouseButtonClicked(int button, QListBoxItem* item)
{
    if ((button != Qt::LeftButton) || (item == 0)) {
        return;
    }

    const int index = m_bookmarksList->index(item);
    KBookmark bookmark = DolphinSettings::instance().bookmark(index);
    Dolphin::mainWin().activeView()->setURL(bookmark.url());
}

void BookmarksSidebarPage::slotContextMenuRequested(QListBoxItem* item,
                                                    const QPoint& pos)
{
    const int insertID = 1;
    const int editID = 2;
    const int deleteID = 3;
    const int addID = 4;

    QPopupMenu* popup = new QPopupMenu();
    if (item == 0) {
        popup->insertItem(SmallIcon("filenew"), i18n("Add Bookmark..."), addID);
    }
    else {
        popup->insertItem(SmallIcon("filenew"), i18n("Insert Bookmark..."), insertID);
        popup->insertItem(SmallIcon("edit"), i18n("Edit..."), editID);
        popup->insertItem(SmallIcon("editdelete"), i18n("Delete"), deleteID);
    }

    KBookmarkManager* manager = DolphinSettings::instance().bookmarkManager();
    KBookmarkGroup root = manager->root();
    const int index = m_bookmarksList->index(m_bookmarksList->selectedItem());

    const int result = popup->exec(pos);
    switch (result) {
        case insertID: {
            KBookmark newBookmark = EditBookmarkDialog::getBookmark(i18n("Insert Bookmark"),
                                                                    i18n("New bookmark"),
                                                                    KURL(),
                                                                    "bookmark");
            if (!newBookmark.isNull()) {
                root.addBookmark(manager, newBookmark);
                if (index > 0) {
                    KBookmark prevBookmark = DolphinSettings::instance().bookmark(index - 1);
                    root.moveItem(newBookmark, prevBookmark);
                }
                else {
                    // insert bookmark at first position (is a little bit tricky as KBookmarkGroup
                    // only allows to move items after existing items)
                    KBookmark firstBookmark = root.first();
                    root.moveItem(newBookmark, firstBookmark);
                    root.moveItem(firstBookmark, newBookmark);
                }
                manager->emitChanged(root);
            }
            break;
        }

        case editID: {
            KBookmark oldBookmark = DolphinSettings::instance().bookmark(index);
            KBookmark newBookmark = EditBookmarkDialog::getBookmark(i18n("Edit Bookmark"),
                                                                    oldBookmark.text(),
                                                                    oldBookmark.url(),
                                                                    oldBookmark.icon());
            if (!newBookmark.isNull()) {
                root.addBookmark(manager, newBookmark);
                root.moveItem(newBookmark, oldBookmark);
                root.deleteBookmark(oldBookmark);
                manager->emitChanged(root);
            }
            break;
        }

        case deleteID: {
            KBookmark bookmark = DolphinSettings::instance().bookmark(index);
            root.deleteBookmark(bookmark);
            manager->emitChanged(root);
            break;
        }

        case addID: {
            KBookmark bookmark = EditBookmarkDialog::getBookmark(i18n("Add Bookmark"),
                                                                 "New bookmark",
                                                                 KURL(),
                                                                 "bookmark");
            if (!bookmark.isNull()) {
                root.addBookmark(manager, bookmark);
                manager->emitChanged(root);
            }
        }

        default: break;
    }

    delete popup;
    popup = 0;

    DolphinView* view = Dolphin::mainWin().activeView();
    adjustSelection(view->url());
}


void BookmarksSidebarPage::adjustSelection(const KURL& url)
{
    // TODO (remarked in dolphin/TODO): the following code is quite equal
    // to BookmarkSelector::updateSelection().

    KBookmarkGroup root = DolphinSettings::instance().bookmarkManager()->root();
    KBookmark bookmark = root.first();

    int maxLength = 0;
    int selectedIndex = -1;

    // Search the bookmark which is equal to the URL or at least is a parent URL.
    // If there are more than one possible parent URL candidates, choose the bookmark
    // which covers the bigger range of the URL.
    int i = 0;
    while (!bookmark.isNull()) {
        const KURL bookmarkURL = bookmark.url();
        if (bookmarkURL.isParentOf(url)) {
            const int length = bookmarkURL.prettyURL().length();
            if (length > maxLength) {
                selectedIndex = i;
                maxLength = length;
            }
        }
        bookmark = root.next(bookmark);
        ++i;
    }

    const bool block = m_bookmarksList->signalsBlocked();
    m_bookmarksList->blockSignals(true);
    if (selectedIndex < 0) {
        // no bookmark matches, hence deactivate any selection
        const int currentIndex = m_bookmarksList->index(m_bookmarksList->selectedItem());
        m_bookmarksList->setSelected(currentIndex, false);
    }
    else {
        // select the bookmark which is part of the current URL
        m_bookmarksList->setSelected(selectedIndex, true);
    }
    m_bookmarksList->blockSignals(block);
}

void BookmarksSidebarPage::slotURLChanged(const KURL& url)
{
    adjustSelection(url);
}

void BookmarksSidebarPage::connectToActiveView()
{
    DolphinView* view = Dolphin::mainWin().activeView();
    adjustSelection(view->url());
    connect(view, SIGNAL(signalURLChanged(const KURL&)),
            this, SLOT(slotURLChanged(const KURL&)));
}

BookmarksListBox::BookmarksListBox(QWidget* parent) :
    QListBox(parent)
{
    setAcceptDrops(true);
}
BookmarksListBox::~BookmarksListBox()
{
}

void BookmarksListBox::paintEvent(QPaintEvent* /* event */)
{
    // don't invoke QListBox::paintEvent(event) to prevent
    // that any kind of frame is drawn
}

void BookmarksListBox::contentsMousePressEvent(QMouseEvent *event) 
{ 
    if (event->button() == LeftButton) 
        dragPos = event->pos();
    QListBox::contentsMousePressEvent(event);
}

void BookmarksListBox::contentsMouseMoveEvent(QMouseEvent *event)
{
    if (event->state() & LeftButton) {
        int distance = (event->pos() - dragPos).manhattanLength();
        if (distance > QApplication::startDragDistance())
            startDrag();
    }
    QListBox::contentsMouseMoveEvent(event); 
}

void BookmarksListBox::startDrag()
{
    int currentItem = QListBox::currentItem();
    if (currentItem != -1) {
        BookmarkItem* bookmark = (BookmarkItem*)item(currentItem);
	if (bookmark!=0){
            KURL::List lst;
            lst.append( bookmark->url() );
            KURLDrag *drag = new KURLDrag(lst, this);
            drag->drag();
        }
    }
}

void BookmarksListBox::dragEnterEvent( QDragEnterEvent *event )
{
    event->accept(KURLDrag::canDecode(event));
}

void BookmarksListBox::dropEvent( QDropEvent *event )
{
    KURL::List urls;
    if (KURLDrag::decode(event, urls) && !urls.isEmpty()) {
        KBookmarkManager* manager = DolphinSettings::instance().bookmarkManager();
        KBookmarkGroup root = manager->root();

        KURL::List::iterator it;
        for(it=urls.begin(); it!=urls.end(); ++it) {
            root.addBookmark(manager, (*it).fileName(), (*it), "", false);
	}
	manager->emitChanged(root);
    }
}

BookmarkItem::BookmarkItem(const QPixmap& pixmap, const QString& text, const KURL& url) :
    QListBoxPixmap(pixmap, text),
    m_url(url)
{
}

BookmarkItem::~BookmarkItem()
{
}

int BookmarkItem::height(const QListBox* listBox) const
{
    return QListBoxPixmap::height(listBox) + 8;
}

const KURL& BookmarkItem::url() const
{
    return m_url;
}

BookmarkItem* BookmarkItem::fromKbookmark(const KBookmark& bookmark, const KIconLoader& iconLoader)
{
    QPixmap icon(iconLoader.loadIcon(bookmark.icon(), KIcon::NoGroup, KIcon::SizeMedium));
    return new BookmarkItem(icon, bookmark.text(), bookmark.url());
}

#include "bookmarkssidebarpage.moc"

