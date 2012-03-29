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

#ifndef SIDEBARSETTINGS_H
#define SIDEBARSETTINGS_H

#include <qstring.h>
#include <dolphinsettingsbase.h>


class leftSidebarSettings
{
public:
    leftSidebarSettings();
    virtual ~leftSidebarSettings();
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }

    void setWidth(int width);
    int width() const { return m_width; }

    void setSelectedPage(const QString& pageName) { m_selectedPage = pageName; }
    const QString& selectedPage() const { return m_selectedPage; }

    virtual void save();

protected:
    bool m_visible;
    int m_width;
    QString m_selectedPage;
};

class rightSidebarSettings
{
public:
    rightSidebarSettings();
    virtual ~rightSidebarSettings();
    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }

    void setWidth(int width);
    int width() const { return m_width; }

    void setSelectedPage(const QString& pageName) { m_selectedPage = pageName; }
    const QString& selectedPage() const { return m_selectedPage; }

    virtual void save();

protected:
    bool m_visible;
    int m_width;
    QString m_selectedPage;
};

#endif
