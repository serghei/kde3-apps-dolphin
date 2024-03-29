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

#ifndef DOLPHINCONTEXTMENU_H
#define DOLPHINCONTEXTMENU_H

#include <kpopupmenu.h>
#include <qpoint.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <kmountpoint.h>
#include <qvaluevector.h>
#include <kservice.h>
#include <kpropertiesdialog.h>

class KPopupMenu;
class KFileItem;
class QPoint;
class QWidget;
class DolphinView;

/**
 * @brief Represents the context menu which appears when doing a right
 * click on an item or the viewport of the file manager.
 *
 * Beside static menu entries (e. g. 'Paste' or 'Properties') two
 * dynamic sub menus are shown when opening a context menu above
 * an item:
 * - 'Open With': Contains all applications which are registered to
 *                open items of the given MIME type.
 * - 'Actions':   Contains all actions which can be applied to the
 *                given item.
 *
 *	@author Peter Penz <peter.penz@gmx.at>
 */
class DolphinContextMenu
{
public:
    /**
     * @parent    Pointer to the dolphin view the context menu
     *            belongs to.
     * @fileInfo  Pointer to the file item the context menu
     *            is applied. If 0 is passed, the context menu
     *            is above the viewport.
     * @pos       Position of the upper left edge of the context menu.
     */
    DolphinContextMenu(DolphinView* parent,
                       KFileItem* fileInfo,
                       const QPoint& pos);

    virtual ~DolphinContextMenu();

    /** Opens the context menu modal. */
    void open();

private:
    void openViewportContextMenu();
    void openItemContextMenu();

    /**
     * Inserts the 'Open With...' submenu to \a popup.
     * @param popup          Menu where the 'Open With...' sub menu should
     *                       be added.
     * @param openWithVector Output parameter which contains all 'Open with...'
     *                       services.
     * @return               Identifier of the first 'Open With...' entry.
     *                       All succeeding identifiers have an increased value of 1
     *                       to the predecessor.
     */
    int insertOpenWithItems(KPopupMenu* popup,
                            QValueVector<KService::Ptr>& openWithVector);

    /**
     * Inserts the 'Actions...' submenu to \a popup.
     * @param popup          Menu where the 'Actions...' sub menu should
     *                       be added.
     * @param openWithVector Output parameter which contains all 'Actions...'
     *                       services.
     */
    void insertActionItems(KPopupMenu* popup,
                          QValueVector<KDEDesktopMimeType::Service>& actionsVector);

    /**
     * Returns true, if 'menu' contains already
     * an entry with the name 'entryName'.
     */
    bool containsEntry(const KPopupMenu* menu,
                       const QString& entryName) const;

    enum {
        restoreID       =   80,	
        emptyID         =   85,
        submenuID       =   90,
        bookmarkID      =   91,
        openWithIDStart =  100,
        actionsIDStart  = 1000,
    };

    DolphinView* m_dolphinView;
    KFileItem* m_fileInfo;
    QPoint m_pos;

    struct Entry {
        int type;
        QString name;
        QString filePath;     // empty for separator
        QString templatePath; // same as filePath for template
        QString icon;
        QString comment;
    };
};

#endif
