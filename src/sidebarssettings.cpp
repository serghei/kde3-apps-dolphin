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

#include "sidebarssettings.h"
#include <kapplication.h>
#include <kconfig.h>
#include <assert.h>

leftSidebarSettings::leftSidebarSettings() :
    m_visible(true),
    m_width(0)
{
    KConfig* config = kapp->config();
    config->setGroup("leftSidebar");

    m_visible = config->readBoolEntry("Visible", true);
    m_width = config->readNumEntry("Width", 150);
    m_selectedPage = config->readEntry("Selected Page", "Bookmarks");
}

leftSidebarSettings::~leftSidebarSettings()
{
}

void leftSidebarSettings::setWidth(int width)
{
    if (width < 50) {
        // prevent that a sidebar gets width which makes
        // it look invisible
        width = 50;
    }
    m_width = width;
}

void leftSidebarSettings::save()
{
    KConfig* config = kapp->config();
    config->setGroup("leftSidebar");

    config->writeEntry("Visible", m_visible);
    config->writeEntry("Width", m_width);
    config->writeEntry("Selected Page", m_selectedPage);
}

rightSidebarSettings::rightSidebarSettings() :
    m_visible(true),
    m_width(0)
{
    KConfig* config = kapp->config();
    config->setGroup("rightSidebar");

    m_visible = config->readBoolEntry("Visible", true);
    m_width = config->readNumEntry("Width", 150);
    m_selectedPage = config->readEntry("Selected Page", "Bookmarks");
}

rightSidebarSettings::~rightSidebarSettings()
{
}

void rightSidebarSettings::setWidth(int width)
{
    if (width < 50) {
        // prevent that a sidebar gets width which makes
        // it look invisible
        width = 50;
    }
    m_width = width;
}

void rightSidebarSettings::save()
{
    KConfig* config = kapp->config();
    config->setGroup("rightSidebar");

    config->writeEntry("Visible", m_visible);
    config->writeEntry("Width", m_width);
    config->writeEntry("Selected Page", m_selectedPage);
}
