/***************************************************************************
 *   Copyright (C) 2006 by Peter Penz                                      *
 *   peter.penz@gmx.at                                                     *
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

#include "bookmarkssettingspage.h"

#include <assert.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qvbox.h>

#include <kbookmark.h>
#include <kbookmarkmanager.h>
#include <kdialog.h>
#include <klistview.h>
#include <klocale.h>
#include <kpushbutton.h>

#include "dolphinsettings.h"
#include "editbookmarkdialog.h"

BookmarksSettingsPage::BookmarksSettingsPage(QWidget*parent) :
    SettingsPageBase(parent),
    m_addButton(0),
    m_removeButton(0),
    m_moveUpButton(0),
    m_moveDownButton(0)
{
    QVBoxLayout* topLayout = new QVBoxLayout(parent, 2, KDialog::spacingHint());

    const int spacing = KDialog::spacingHint();

    QHBox* hBox = new QHBox(parent);
    hBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    hBox->setSpacing(spacing);
    hBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);

    m_listView = new KListView(hBox);
    m_listView->addColumn(i18n("Icon"));
    m_listView->addColumn(i18n("Name"));
    m_listView->addColumn(i18n("Location"));
    m_listView->setResizeMode(QListView::LastColumn);
    m_listView->setColumnAlignment(0, Qt::AlignHCenter);
    m_listView->setAllColumnsShowFocus(true);
    m_listView->setSorting(-1);
    connect(m_listView, SIGNAL(selectionChanged()),
            this, SLOT(updateButtons()));
    connect(m_listView, SIGNAL(pressed(QListViewItem*)),
            this, SLOT(slotBookmarkPressed(QListViewItem*)));
    connect(m_listView, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)),
            this, SLOT(slotBookmarkDoubleClicked(QListViewItem*, const QPoint&, int)));

    QVBox* buttonBox = new QVBox(hBox);
    buttonBox->setSpacing(spacing);

    const QSizePolicy buttonSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    m_addButton = new KPushButton(i18n("Add..."), buttonBox);
    connect(m_addButton, SIGNAL(clicked()),
            this, SLOT(slotAddButtonClicked()));
    m_addButton->setSizePolicy(buttonSizePolicy);

    m_editButton = new KPushButton(i18n("Edit..."), buttonBox);
    connect(m_editButton, SIGNAL(clicked()),
            this, SLOT(slotEditButtonClicked()));
    m_editButton->setSizePolicy(buttonSizePolicy);

    m_removeButton = new KPushButton(i18n("Remove"), buttonBox);
    connect(m_removeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveButtonClicked()));
    m_removeButton->setSizePolicy(buttonSizePolicy);

    m_moveUpButton = new KPushButton(i18n("Move Up"), buttonBox);
    connect(m_moveUpButton, SIGNAL(clicked()),
            this, SLOT(slotMoveUpButtonClicked()));
    m_moveUpButton->setSizePolicy(buttonSizePolicy);

    m_moveDownButton = new KPushButton(i18n("Move Down"), buttonBox);
    connect(m_moveDownButton, SIGNAL(clicked()),
            this, SLOT(slotMoveDownButtonClicked()));
    m_moveDownButton->setSizePolicy(buttonSizePolicy);

    // Add a dummy widget with no restriction regarding a vertical resizing.
    // This assures that the spacing between the buttons is not increased.
    new QWidget(buttonBox);

    topLayout->addWidget(hBox);

    // insert all editable bookmarks.
    KBookmarkGroup root = DolphinSettings::instance().bookmarkManager()->root();
    KBookmark bookmark = root.first();

    QListViewItem* prev = 0;
    while (!bookmark.isNull()) {
        QListViewItem* item = new QListViewItem(m_listView);
        item->setPixmap(PixmapIdx, SmallIcon(bookmark.icon()));
        item->setText(NameIdx, bookmark.text());
        item->setText(URLIdx, bookmark.url().prettyURL());

        // add hidden column to be able to retrieve the icon name again
        item->setText(IconIdx, bookmark.icon());

        m_listView->insertItem(item);
        if (prev != 0) {
            item->moveItem(prev);
        }
        prev = item;

        bookmark = root.next(bookmark);
    }
    m_listView->setSelected(m_listView->firstChild(), true);

    updateButtons();
}


BookmarksSettingsPage::~BookmarksSettingsPage()
{
}

void BookmarksSettingsPage::applySettings()
{
    // delete all bookmarks
    KBookmarkManager* manager = DolphinSettings::instance().bookmarkManager();
    KBookmarkGroup root = manager->root();
    KBookmark bookmark = root.first();
    while (!bookmark.isNull()) {
        root.deleteBookmark(bookmark);
        bookmark = root.first();
    }

    // add all items as bookmarks
    QListViewItem* item = m_listView->firstChild();
    while (item != 0) {
        root.addBookmark(manager,
                         item->text(NameIdx),
                         KURL(item->text(URLIdx)),
                         item->text(IconIdx)); // hidden column
        item = item->itemBelow();
    }

    manager->emitChanged(root);
}

void BookmarksSettingsPage::updateButtons()
{
    const QListViewItem* selectedItem = m_listView->selectedItem();
    const bool hasSelection = (selectedItem != 0);

    m_editButton->setEnabled(hasSelection);
    m_removeButton->setEnabled(hasSelection);

    const bool enableMoveUp = hasSelection &&
                              (selectedItem != m_listView->firstChild());
    m_moveUpButton->setEnabled(enableMoveUp);

    const bool enableMoveDown = hasSelection &&
                                (selectedItem != m_listView->lastChild());
    m_moveDownButton->setEnabled(enableMoveDown);
}

void BookmarksSettingsPage::slotBookmarkDoubleClicked(QListViewItem*,
                                                      const QPoint&,
                                                      int)
{
    slotEditButtonClicked();
}

void BookmarksSettingsPage::slotBookmarkPressed(QListViewItem* item)
{
    if (item == 0) {
        m_listView->setSelected(m_listView->currentItem(), true);
    }
}

void BookmarksSettingsPage::slotAddButtonClicked()
{
    KBookmark bookmark = EditBookmarkDialog::getBookmark(i18n("Add Bookmark"),
                                                         i18n("New bookmark"),
                                                         KURL(),
                                                         "bookmark");
    if (!bookmark.isNull()) {
        // insert bookmark into listview
        QListViewItem* item = new QListViewItem(m_listView);
        item->setPixmap(PixmapIdx, SmallIcon(bookmark.icon()));
        item->setText(NameIdx, bookmark.text());
        item->setText(URLIdx, bookmark.url().prettyURL());
        item->setText(IconIdx, bookmark.icon());
        m_listView->insertItem(item);

        QListViewItem* lastItem = m_listView->lastChild();
        if (lastItem != 0) {
            item->moveItem(lastItem);
        }

        m_listView->setSelected(item, true);
        updateButtons();
    }
}

void BookmarksSettingsPage::slotEditButtonClicked()
{
    QListViewItem* item = m_listView->selectedItem();
    assert(item != 0); // 'edit' may not get invoked when having no items

    KBookmark bookmark = EditBookmarkDialog::getBookmark(i18n("Edit Bookmark"),
                                                         item->text(NameIdx),
                                                         KURL(item->text(URLIdx)),
                                                         item->text(IconIdx));
    if (!bookmark.isNull()) {
        item->setPixmap(PixmapIdx, SmallIcon(bookmark.icon()));
        item->setText(NameIdx, bookmark.text());
        item->setText(URLIdx, bookmark.url().prettyURL());
        item->setText(IconIdx, bookmark.icon());
    }
}

void BookmarksSettingsPage::slotRemoveButtonClicked()
{
    QListViewItem* selectedItem = m_listView->selectedItem();
    assert(selectedItem != 0);
    QListViewItem* nextItem = selectedItem->itemBelow();
    if (nextItem == 0) {
        nextItem = selectedItem->itemAbove();
    }

    m_listView->takeItem(selectedItem);
    if (nextItem != 0) {
        m_listView->setSelected(nextItem, true);
    }
}

void BookmarksSettingsPage::slotMoveUpButtonClicked()
{
    moveBookmark(-1);
}

void BookmarksSettingsPage::slotMoveDownButtonClicked()
{
    moveBookmark(+1);
}

int BookmarksSettingsPage::selectedBookmarkIndex() const
{
    int index = -1;

    QListViewItem* selectedItem = m_listView->selectedItem();
    if (selectedItem != 0) {
        index = 0;
        QListViewItem* item = m_listView->firstChild();
        while (item != selectedItem) {
            item = item->nextSibling();
            ++index;
        }
    }

    return index;
}

void BookmarksSettingsPage::moveBookmark(int direction)
{
    // this implementation currently only allows moving of bookmarks
    // one step up or down
    assert((direction >= -1) && (direction <= +1));

    // swap bookmarks in listview
    QListViewItem* selectedItem = m_listView->selectedItem();
    assert(selectedItem != 0);
    QListViewItem* item = (direction < 0) ? selectedItem->itemAbove() :
                                            selectedItem->itemBelow();
    assert(item != 0);

    QPixmap pixmap;
    if (item->pixmap(0) != 0) {
        pixmap = *(item->pixmap(0));
    }
    QString name(item->text(NameIdx));
    QString url(item->text(URLIdx));
    QString icon(item->text(IconIdx));

    if (selectedItem->pixmap(0) != 0) {
        item->setPixmap(PixmapIdx, *(selectedItem->pixmap(0)));
    }
    item->setText(NameIdx, selectedItem->text(NameIdx));
    item->setText(URLIdx, selectedItem->text(URLIdx));
    item->setText(IconIdx, selectedItem->text(IconIdx));

    selectedItem->setPixmap(PixmapIdx, pixmap);
    selectedItem->setText(NameIdx, name);
    selectedItem->setText(URLIdx, url);
    selectedItem->setText(IconIdx, icon);

    m_listView->setSelected(item, true);
}

#include "bookmarkssettingspage.moc"
