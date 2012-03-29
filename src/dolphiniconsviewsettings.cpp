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

#include "dolphiniconsviewsettings.h"
#include <kicontheme.h>
#include <kglobalsettings.h>
#include <kapplication.h>
#include <assert.h>

DolphinIconsViewSettings::DolphinIconsViewSettings(DolphinIconsView::LayoutMode mode) :
    m_arrangement(QIconView::LeftToRight),
    m_layoutMode(mode),
    m_iconSize(KIcon::SizeMedium),
    m_previewSize(KIcon::SizeMedium),
    m_gridWidth(0),
    m_gridHeight(KIcon::SizeMedium),
    m_gridSpacing(8),
    m_fontSize(0),
    m_textlinesCount(2)
{
    KConfig* config = kapp->config();
    setConfigGroup(config);

    // read icon size
    m_iconSize = config->readNumEntry("Icon Size", -1);
    if (m_iconSize < 0) {
        m_iconSize = KIcon::SizeMedium;
    }

    // read arrangement
    const QString arrangement(config->readEntry("Arrangement"));
    if (arrangement == "Left to Right") {
        m_arrangement = QIconView::LeftToRight;
    }
    else if (arrangement == "Top to Bottom") {
        m_arrangement = QIconView::TopToBottom;
    }

    // read preview size, grid width and grid height
    m_previewSize = config->readNumEntry("Preview Size", -1);
    m_gridWidth = config->readNumEntry("Grid Width", -1);
    m_gridHeight = config->readNumEntry("Grid Height", -1);
    m_gridSpacing = config->readNumEntry("Grid Spacing", -1);

    if (mode == DolphinIconsView::Previews) {
        if (m_previewSize < 0) {
            m_previewSize = KIcon::SizeEnormous;
        }
        if (m_gridWidth < 0) {
            m_gridWidth = m_previewSize + (m_previewSize / 2);
        }
    }
    else if (m_gridWidth < 0) {
        m_gridWidth = m_iconSize + (m_iconSize / 2) + (KIcon::SizeLarge * 2);
    }

    if (m_gridHeight < 0) {
        m_gridHeight = m_iconSize * 2;
    }

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

    // read textlines count
    m_textlinesCount = config->readNumEntry("Number of Textlines", 2);
}


DolphinIconsViewSettings::~DolphinIconsViewSettings()
{
}

void DolphinIconsViewSettings::setIconSize(int size)
{
    // TODO: add boundaries check
    m_iconSize = size;
}

void DolphinIconsViewSettings::setPreviewSize(int size)
{
    // TODO: add boundaries check
    m_previewSize = size;
}

void DolphinIconsViewSettings::setGridSpacing(int spacing)
{
    // TODO: add boundaries check
    m_gridSpacing = spacing;
}

void DolphinIconsViewSettings::save()
{
    KConfig* config = kapp->config();
    setConfigGroup(config);

    config->writeEntry("Icon Size", m_iconSize);
    if (m_arrangement == QIconView::LeftToRight) {
        config->writeEntry("Arrangement", "Left to Right");
    }
    else {
        config->writeEntry("Arrangement", "Top to Bottom");
    }

    config->writeEntry("Preview Size", m_previewSize);
    config->writeEntry("Grid Width", m_gridWidth);
    config->writeEntry("Grid Height", m_gridHeight);
    config->writeEntry("Grid Spacing", m_gridSpacing);
    config->writeEntry("Font Size", m_fontSize);
    config->writeEntry("Font Family", m_fontFamily);
    config->writeEntry("Number of Textlines", m_textlinesCount);
}

void DolphinIconsViewSettings::calculateGridSize(int hint)
{
    const int maxSize = (m_previewSize > m_iconSize) ? m_previewSize : m_iconSize;
    if (m_arrangement == QIconView::LeftToRight) {
        int widthUnit = maxSize + (maxSize / 2);
        if (widthUnit < KIcon::SizeLarge) {
            widthUnit = KIcon::SizeLarge;
        }
        //m_gridWidth = widthUnit + hint * KIcon::SizeLarge;
        m_gridWidth = widthUnit + hint * KIcon::SizeLarge;

        m_gridHeight = m_iconSize;
        if (m_gridHeight <= KIcon::SizeMedium) {
            m_gridHeight = m_gridHeight * 2;
        }
        else {
            m_gridHeight += maxSize / 2;
        }
    }
    else {
        assert(m_arrangement == QIconView::TopToBottom);
        m_gridWidth = maxSize + (hint + 1) * (8 * m_fontSize);

        // The height-setting is ignored yet by KFileIconView if the TopToBottom
        // arrangement is active. Anyway write the setting to have a defined value.
        m_gridHeight = maxSize;
    }
}

int DolphinIconsViewSettings::textWidthHint() const
{
    const int maxSize = (m_previewSize > m_iconSize) ? m_previewSize : m_iconSize;
    int hint = 0;
    if (m_arrangement == QIconView::LeftToRight) {
        int widthUnit = maxSize + (maxSize / 2);
        if (widthUnit < KIcon::SizeLarge) {
            widthUnit = KIcon::SizeLarge;
        }
        hint = (m_gridWidth - widthUnit) / KIcon::SizeLarge;
    }
    else {
        assert(m_arrangement == QIconView::TopToBottom);
        hint = (m_gridWidth - maxSize) / (8 * m_fontSize) - 1;
        if (hint > 2) {
            hint = 2;
        }
    }
    return hint;
}

void DolphinIconsViewSettings::setConfigGroup(KConfig* config)
{
    if (m_layoutMode == DolphinIconsView::Previews) {
        config->setGroup("Previews Mode");
    }
    else {
        config->setGroup("Icons Mode");
    }
}

