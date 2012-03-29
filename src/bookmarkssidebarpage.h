/***************************************************************************
 *   Copyright (C) 2006 by Peter Penz <peter.penz@gmx.at>
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
#ifndef _BOOKMARKSSIDEBARPAGE_H_
#define _BOOKMARKSSIDEBARPAGE_H_

#include <sidebarpage.h>
#include <qlistbox.h>
#include <kurl.h>
#include <kbookmark.h>
#include <kiconloader.h>

class BookmarksListBox;

/**
 * @brief Sidebar page for accessing bookmarks.
 *
 * It is possible to add, remove and edit bookmarks
 * by a context menu. The selection of the bookmark
 * is automatically adjusted to the URL given by
 * the active view.
 */
class BookmarksSidebarPage : public SidebarPage
{
        Q_OBJECT

public:
    BookmarksSidebarPage(QWidget* parent);
    virtual ~BookmarksSidebarPage();

protected:
    /** @see SidebarPage::activeViewChanged() */
    virtual void activeViewChanged();

private slots:
    /** Fills the listbox with the bookmarks stored in DolphinSettings. */
    void updateBookmarks();

    /**
     * Checks whether the left mouse button has been clicked above a bookmark.
     * If this is the case, the URL for the currently active view is adjusted.
     */
    void slotMouseButtonClicked(int button, QListBoxItem* item);

    /** @see QListBox::slotContextMenuRequested */
    void slotContextMenuRequested(QListBoxItem* item, const QPoint& pos);

    /**
     * Is invoked whenever the URL of the active view has been changed. Adjusts
     * the selection of the listbox to the bookmark which is part of the current URL.
     */
    void slotURLChanged(const KURL& url);

private:
    /**
     * Updates the selection dependent from the given URL \a url. The
     * URL must not match exactly to one of the available bookmarks:
     * The bookmark which is equal to the URL or at least is a parent URL
     * is selected. If there are more than one possible parent URL candidates,
     * the bookmark which covers the bigger range of the URL is selected.
     */
    void adjustSelection(const KURL& url);

    /**
     * Connects to signals from the currently active Dolphin view to get
     * informed about URL and bookmark changes.
     */
    void connectToActiveView();

    BookmarksListBox* m_bookmarksList;
};

/**
 * @brief Listbox which contains a list of bookmarks.
 *
 * Only QListBox::paintEvent() has been overwritten to prevent
 * that a (not wanted) frameborder is drawn.
 */
class BookmarksListBox : public QListBox
{
    Q_OBJECT

public:
    BookmarksListBox(QWidget* parent);
    virtual ~BookmarksListBox();

protected:
    //drag
    void contentsMousePressEvent(QMouseEvent *event); 
    void contentsMouseMoveEvent(QMouseEvent *event);
    //drop
    void dragEnterEvent( QDragEnterEvent *evt );
    void dropEvent( QDropEvent *evt );
//    void mousePressEvent( QMouseEvent *evt );
//    void mouseMoveEvent( QMouseEvent * );
    /** @see QWidget::paintEvent() */
    virtual void paintEvent(QPaintEvent* event);
private:
    QPoint dragPos;

    void startDrag();
};

/**
 * @brief Item which can be added to a BookmarksListBox.
 *
 * Only QListBoxPixmap::height() has been overwritten to get
 * a spacing between the items.
 */
class BookmarkItem : public QListBoxPixmap
{
public:
    BookmarkItem(const QPixmap& pixmap, const QString& text, const KURL& url);
    virtual ~BookmarkItem();
    virtual int height(const QListBox* listBox) const;
    const KURL& url() const;

    static BookmarkItem* fromKbookmark(const KBookmark& bookmark, const KIconLoader& iconLoader);

private:
    KURL m_url;
};

#endif // _BOOKMARKSSIDEBARPAGE_H_
