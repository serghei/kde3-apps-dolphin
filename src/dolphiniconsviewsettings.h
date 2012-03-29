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
#ifndef DOLPHINICONSVIEWSETTINGS_H
#define DOLPHINICONSVIEWSETTINGS_H

#include <qstring.h>
#include <qiconview.h>
#include <dolphinview.h>
#include <dolphiniconsview.h>
#include <dolphinsettingsbase.h>

/**
 * @brief Contains the settings for the icons view.
 *
 * The following properties are stored:
 * - layout mode (icons or previews)
 * - icon size
 * - preview size
 * - grid width, height and spacing
 * - arrangement (left to right or top to bottom)
 * - font family
 * - font size
 * - number of text lines
 *
 * @see DolphinIconsView
 * @author Peter Penz <peter.penz@gmx.at>
 */
class DolphinIconsViewSettings : public DolphinSettingsBase
{
public:
    DolphinIconsViewSettings(DolphinIconsView::LayoutMode mode);
    virtual ~DolphinIconsViewSettings();

    void setIconSize(int size);
    int iconSize() const { return m_iconSize; }

    void setPreviewSize(int size);
    int previewSize() const { return m_previewSize; }

    /**
     * Returns the width of the grid. For setting the width
     * DolphinIconsviewSettings::calculateGridSize() must be used.
     */
    int gridWidth() const { return m_gridWidth; }

    /**
     * Returns the height of the grid. For setting the height
     * DolphinIconsviewSettings::calculateGridSize() must be used.
     */
    int gridHeight() const { return m_gridHeight; }

    void setGridSpacing(int spacing);
    int gridSpacing() const { return m_gridSpacing; }

    void setArrangement(QIconView::Arrangement arrangement) { m_arrangement = arrangement; }
    QIconView::Arrangement arrangement() const { return m_arrangement; }

    void setFontFamily(const QString& family) { m_fontFamily = family; }
    const QString& fontFamily() const { return m_fontFamily; }

    void setFontSize(int size) { m_fontSize = size; }
    int fontSize() const { return m_fontSize; }

    void setTextlinesCount(int count) { m_textlinesCount = count; }
    int textlinesCount() const { return m_textlinesCount; }

    /** @see DolphinSettingsBase::save */
    virtual void save();

    /**
     * Calculates the width and the height of the grid dependant from \a hint and
     * the current settings. The hint gives information about the wanted text
     * width, where a lower value indicates a smaller text width. Currently
     * in Dolphin the values 0, 1 and 2 are used. See also
     * DolhinIconsViewSettings::textWidthHint.
     *
     * The calculation of the grid width and grid height is a little bit tricky,
     * as the user model does not fit to the implementation model of QIconView. The user model
     * allows to specify icon-, preview- and text width sizes, whereas the implementation
     * model expects only a grid width and height. The nasty thing is that the specified
     * width and height varies dependant from the arrangement (e. g. the height is totally
     * ignored for the top-to-bottom arrangement inside QIconView).
     */
    void calculateGridSize(int hint);

    /**
     * Returns the text width hint dependant from the given settings.
     * A lower value indicates a smaller text width. Currently
     * in Dolphin the values 0, 1 and 2 are used. The text width hint can
     * be used later for DolphinIconsViewSettings::calculateGridSize().
     */
    int textWidthHint() const;

private:
    QIconView::Arrangement m_arrangement;
    DolphinIconsView::LayoutMode m_layoutMode;
    int m_iconSize;
    int m_previewSize;
    int m_gridWidth;
    int m_gridHeight;
    int m_gridSpacing;
    int m_fontSize;
    int m_textlinesCount;
    QString m_fontFamily;

    void setConfigGroup(KConfig* config);
};

#endif
