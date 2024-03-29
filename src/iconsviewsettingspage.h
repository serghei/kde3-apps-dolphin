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

#ifndef ICONSVIEWSETTINGSPAGE_H
#define ICONSVIEWSETTINGSPAGE_H

#include <qvbox.h>
#include <dolphiniconsview.h>

class QSlider;
class QComboBox;
class QCheckBox;
class QPushButton;
class QSpinBox;
class KFontCombo;
class PixmapViewer;

/**
 * @brief Tab page for the 'Icons Mode' and 'Previews Mode' settings
 * of the Dolphin settings dialog.
 *
 * Allows to set:
 * - icon size
 * - preview size
 * - text width
 * - grid spacing
 * - font family
 * - font size
 * - number of text lines
 * - arrangement
 *
 * @see DolphinIconsViewSettings
 *  @author Peter Penz <peter.penz@gmx.at>
 */
class IconsViewSettingsPage : public QVBox
{
    Q_OBJECT

public:
    IconsViewSettingsPage(DolphinIconsView::LayoutMode mode,
                          QWidget* parent);
    virtual ~IconsViewSettingsPage();

    /**
     * Applies the settings for the icons view.
     * The settings are persisted automatically when
     * closing Dolphin.
     */
    void applySettings();

private slots:
    void slotIconSizeChanged(int value);
    void slotPreviewSizeChanged(int value);

private:
    DolphinIconsView::LayoutMode m_mode;

    QSlider* m_iconSizeSlider;
    PixmapViewer* m_iconSizeViewer;
    QSlider* m_previewSizeSlider;
    PixmapViewer* m_previewSizeViewer;
    QComboBox* m_textWidthBox;
    QComboBox* m_gridSpacingBox;
    KFontCombo* m_fontFamilyBox;
    QSpinBox* m_fontSizeBox;
    QSpinBox* m_textlinesCountBox;
    QComboBox* m_arrangementBox;

    /** Returns the icon size for the given slider value. */
    int iconSize(int sliderValue) const;

    /** Returns the slider value for the given icon size. */
    int sliderValue(int iconSize) const;

    /**
     * Adjusts the selection of the text width combo box dependant
     * from the grid width and grid height settings.
     */
    void adjustTextWidthSelection();
};

#endif
