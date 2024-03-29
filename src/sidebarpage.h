/***************************************************************************
 *   Copyright (C) 2006 by Cvetoslav Ludmiloff <ludmiloff@gmail.com>       *
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

#ifndef _SIDEBARPAGE_H_
#define _SIDEBARPAGE_H_

#include <qwidget.h>

class Sidebar;

/**
 * @brief Base widget for all pages that can be embedded into the Sidebar.
 *
 * TODO
 */
class SidebarPage : public QWidget
{
	Q_OBJECT

public:
    SidebarPage(QWidget* parent);
    virtual ~SidebarPage();

protected slots:
    /**
     * Is invoked whenever the active view from Dolphin has been changed.
     * The active view can be retrieved by Dolphin::mainWin().activeView();
     */
    virtual void activeViewChanged();
};

#endif // _SIDEBARPAGE_H_
