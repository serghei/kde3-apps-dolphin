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

#include "dolphindetailsviewsettings.h"
#include "dolphindetailsview.h"
#include <kglobalsettings.h>
#include <kapplication.h>

DolphinDetailsViewSettings::DolphinDetailsViewSettings() :
    m_columnEnabled(0),
    m_iconSize(0),
    m_fontSize(0)
{
    KConfig* config = kapp->config();
    config->setGroup("Details Mode");

    // read which columns should be shown
    const bool showName = config->readBoolEntry("Show Name", true);
    const bool showSize = config->readBoolEntry("Show Size", true);
    const bool showDate = config->readBoolEntry("Show Date", true);
    const bool showPermissions = config->readBoolEntry("Show Permissions", false);
    const bool showOwner = config->readBoolEntry("Show Owner", false);
    const bool showGroup = config->readBoolEntry("Show Group", false);

    setColumnEnabled(DolphinDetailsView::NameColumn, showName);
    setColumnEnabled(DolphinDetailsView::SizeColumn, showSize);
    setColumnEnabled(DolphinDetailsView::DateColumn, showDate);
    setColumnEnabled(DolphinDetailsView::PermissionsColumn, showPermissions);
    setColumnEnabled(DolphinDetailsView::OwnerColumn, showOwner);
    setColumnEnabled(DolphinDetailsView::GroupColumn, showGroup);

    // read icon size
    m_iconSize = config->readNumEntry("Icon Size", KIcon::SizeSmall);

    // read font size and font family
    m_fontSize = config->readNumEntry("Font Size", -1);
    m_fontFamily = config->readEntry("Font Family");

    const QFont font(KGlobalSettings::generalFont());
    if (m_fontSize < 0) {
        m_fontSize = font.pointSize();
    }

    if (m_fontFamily.isEmpty()) {
        m_fontFamily = font.family();
    }
}

DolphinDetailsViewSettings::~DolphinDetailsViewSettings()
{
    m_columnEnabled = 0;
    m_fontSize = 0;
}

void DolphinDetailsViewSettings::setColumnEnabled(int column,
                                                  bool enable)
{
    if (enable) {
        m_columnEnabled = m_columnEnabled | (1 << column);
    }
    else {
        m_columnEnabled = m_columnEnabled & ~(1 << column);
    }
}

bool DolphinDetailsViewSettings::isColumnEnabled(int column) const
{
    return (m_columnEnabled & (1 << column)) > 0;
}

void DolphinDetailsViewSettings::save()
{
    KConfig* config = kapp->config();
    config->setGroup("Details Mode");

    config->writeEntry("Show Name",
                       isColumnEnabled(DolphinDetailsView::NameColumn));
    config->writeEntry("Show Size",
                       isColumnEnabled(DolphinDetailsView::SizeColumn));
    config->writeEntry("Show Date",
                       isColumnEnabled(DolphinDetailsView::DateColumn));
    config->writeEntry("Show Permissions",
                       isColumnEnabled(DolphinDetailsView::PermissionsColumn));
    config->writeEntry("Show Owner",
                       isColumnEnabled(DolphinDetailsView::OwnerColumn));
    config->writeEntry("Show Group",
                       isColumnEnabled(DolphinDetailsView::GroupColumn));
    config->writeEntry("Icon Size", m_iconSize);
    config->writeEntry("Font Size", m_fontSize);
    config->writeEntry("Font Family", m_fontFamily);
}
