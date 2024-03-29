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

#include "iconsviewsettingspage.h"

#include <qlabel.h>
#include <qslider.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <kiconloader.h>
#include <kfontcombo.h>
#include <kdialog.h>
#include <klocale.h>
#include <assert.h>

#include "dolphiniconsviewsettings.h"
#include "dolphinsettings.h"
#include "pixmapviewer.h"

#define GRID_SPACING_BASE 8
#define GRID_SPACING_INC 12

IconsViewSettingsPage::IconsViewSettingsPage(DolphinIconsView::LayoutMode mode,
                                             QWidget* parent) :
    QVBox(parent),
    m_mode(mode),
    m_iconSizeSlider(0),
    m_previewSizeSlider(0),
    m_textWidthBox(0),
    m_gridSpacingBox(0),
    m_fontFamilyBox(0),
    m_fontSizeBox(0),
    m_textlinesCountBox(0),
    m_arrangementBox(0)
{
    const int spacing = KDialog::spacingHint();
    const int margin = KDialog::marginHint();
    const QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    setSpacing(spacing);
    setMargin(margin);

    DolphinIconsViewSettings* settings = DolphinSettings::instance().iconsView(m_mode);
    assert(settings != 0);

    QHBox* sizesLayout = new QHBox(this);
    sizesLayout->setSpacing(spacing);
    sizesLayout->setSizePolicy(sizePolicy);

    // create 'Icon Size' group including slider and preview
    QGroupBox* iconSizeGroup = new QGroupBox(2, Qt::Vertical, i18n("Icon Size"), sizesLayout);
    iconSizeGroup->setSizePolicy(sizePolicy);
    iconSizeGroup->setMargin(margin);

    const QColor iconBackgroundColor(KGlobalSettings::baseColor());

    QHBox* iconSizeVBox = new QHBox(iconSizeGroup);
    iconSizeVBox->setSpacing(spacing);
    new QLabel(i18n("Small"), iconSizeVBox);
    m_iconSizeSlider = new QSlider(0, 5, 1, 0,  Qt::Horizontal, iconSizeVBox);
    m_iconSizeSlider->setValue(sliderValue(settings->iconSize()));
    m_iconSizeSlider->setTickmarks(QSlider::Below);
    connect(m_iconSizeSlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotIconSizeChanged(int)));
    new QLabel(i18n("Large"), iconSizeVBox);

    m_iconSizeViewer = new PixmapViewer(iconSizeGroup);
    m_iconSizeViewer->setMinimumWidth(KIcon::SizeEnormous);
    m_iconSizeViewer->setFixedHeight(KIcon::SizeEnormous);
    m_iconSizeViewer->setEraseColor(iconBackgroundColor);
    slotIconSizeChanged(m_iconSizeSlider->value());

    if (m_mode == DolphinIconsView::Previews) {
        // create 'Preview Size' group including slider and preview
        QGroupBox* previewSizeGroup = new QGroupBox(2, Qt::Vertical, i18n("Preview Size"), sizesLayout);
        previewSizeGroup->setSizePolicy(sizePolicy);
        previewSizeGroup->setMargin(margin);

        QHBox* previewSizeVBox = new QHBox(previewSizeGroup);
        previewSizeVBox->setSpacing(spacing);
        new QLabel(i18n("Small"), previewSizeVBox);
        m_previewSizeSlider = new QSlider(0, 5, 1, 0,  Qt::Horizontal, previewSizeVBox);
        m_previewSizeSlider->setValue(sliderValue(settings->previewSize()));
        m_previewSizeSlider->setTickmarks(QSlider::Below);
        connect(m_previewSizeSlider, SIGNAL(valueChanged(int)),
                this, SLOT(slotPreviewSizeChanged(int)));
        new QLabel(i18n("Large"), previewSizeVBox);

        m_previewSizeViewer = new PixmapViewer(previewSizeGroup);
        m_previewSizeViewer->setMinimumWidth(KIcon::SizeEnormous);
        m_previewSizeViewer->setFixedHeight(KIcon::SizeEnormous);
        m_previewSizeViewer->setEraseColor(iconBackgroundColor);

        slotPreviewSizeChanged(m_previewSizeSlider->value());
    }

    QGroupBox* textGroup = new QGroupBox(2, Qt::Horizontal, i18n("Text"), this);
    textGroup->setSizePolicy(sizePolicy);
    textGroup->setMargin(margin);

    new QLabel(i18n("Font family:"), textGroup);
    m_fontFamilyBox = new KFontCombo(textGroup);
    m_fontFamilyBox->setCurrentFont(settings->fontFamily());

    new QLabel(i18n("Font size:"), textGroup);
    m_fontSizeBox = new QSpinBox(6, 99, 1, textGroup);
    m_fontSizeBox->setValue(settings->fontSize());

    new QLabel(i18n("Number of lines:"), textGroup);
    m_textlinesCountBox = new QSpinBox(1, 5, 1, textGroup);
    m_textlinesCountBox->setValue(settings->textlinesCount());

    new QLabel(i18n("Text width:"), textGroup);
    m_textWidthBox = new QComboBox(textGroup);
    m_textWidthBox->insertItem(i18n("Small"));
    m_textWidthBox->insertItem(i18n("Medium"));
    m_textWidthBox->insertItem(i18n("Large"));

    QGroupBox* gridGroup = new QGroupBox(2, Qt::Horizontal, i18n("Grid"), this);
    gridGroup->setSizePolicy(sizePolicy);
    gridGroup->setMargin(margin);

    const bool leftToRightArrangement = (settings->arrangement() == QIconView::LeftToRight);
    new QLabel(i18n("Arrangement:"), gridGroup);
    m_arrangementBox = new QComboBox(gridGroup);
    m_arrangementBox->insertItem(i18n("Left to right"));
    m_arrangementBox->insertItem(i18n("Top to bottom"));
    m_arrangementBox->setCurrentItem(leftToRightArrangement ? 0 : 1);

    new QLabel(i18n("Grid spacing:"), gridGroup);
    m_gridSpacingBox = new QComboBox(gridGroup);
    m_gridSpacingBox->insertItem(i18n("Small"));
    m_gridSpacingBox->insertItem(i18n("Medium"));
    m_gridSpacingBox->insertItem(i18n("Large"));
    m_gridSpacingBox->setCurrentItem((settings->gridSpacing() - GRID_SPACING_BASE) / GRID_SPACING_INC);

    // Add a dummy widget with no restriction regarding
    // a vertical resizing. This assures that the dialog layout
    // is not stretched vertically.
    new QWidget(this);

    adjustTextWidthSelection();
}

IconsViewSettingsPage::~IconsViewSettingsPage()
{
}

void IconsViewSettingsPage::applySettings()
{
    DolphinIconsViewSettings* settings = DolphinSettings::instance().iconsView(m_mode);
    assert(settings != 0);

    const int defaultSize = iconSize(m_iconSizeSlider->value());
    settings->setIconSize(defaultSize);

    int previewSize = (m_mode == DolphinIconsView::Previews) ?
                      iconSize(m_previewSizeSlider->value()) :
                      defaultSize;
    if (previewSize < defaultSize) {
        // assure that the preview size is never smaller than the icon size
        previewSize = defaultSize;
    }
    settings->setPreviewSize(previewSize);

    const int fontSize = m_fontSizeBox->value();

    QIconView::Arrangement arrangement = (m_arrangementBox->currentItem() == 0) ?
                                         QIconView::LeftToRight :
                                         QIconView::TopToBottom;
    settings->setArrangement(arrangement);
    settings->calculateGridSize(m_textWidthBox->currentItem());

    settings->setFontFamily(m_fontFamilyBox->currentFont());
    settings->setFontSize(fontSize);
    settings->setTextlinesCount(m_textlinesCountBox->value());

    settings->setGridSpacing(GRID_SPACING_BASE +
                             m_gridSpacingBox->currentItem() * GRID_SPACING_INC);
}

void IconsViewSettingsPage::slotIconSizeChanged(int value)
{
    KIconLoader iconLoader;
    m_iconSizeViewer->setPixmap(iconLoader.loadIcon("folder", KIcon::Desktop, iconSize(value)));

    if (m_previewSizeSlider != 0) {
        int previewSizeValue = m_previewSizeSlider->value();
        if (previewSizeValue < value) {
            // assure that the preview size is never smaller than the icon size
            previewSizeValue = value;
        }
        slotPreviewSizeChanged(previewSizeValue);
    }
}

void IconsViewSettingsPage::slotPreviewSizeChanged(int value)
{
    KIconLoader iconLoader;
    const int iconSizeValue = m_iconSizeSlider->value();
    if (value < iconSizeValue) {
        // assure that the preview size is never smaller than the icon size
        value = iconSizeValue;
    }
    m_previewSizeViewer->setPixmap(iconLoader.loadIcon("preview", KIcon::Desktop, iconSize(value)));
}

int IconsViewSettingsPage::iconSize(int sliderValue) const
{
    int size = KIcon::SizeMedium;
    switch (sliderValue) {
        case 0: size = KIcon::SizeSmall; break;
        case 1: size = KIcon::SizeSmallMedium; break;
        case 2: size = KIcon::SizeMedium; break;
        case 3: size = KIcon::SizeLarge; break;
        case 4: size = KIcon::SizeHuge; break;
        case 5: size = KIcon::SizeEnormous; break;
    }
    return size;
}

int IconsViewSettingsPage::sliderValue(int iconSize) const
{
    int value = 0;
    switch (iconSize) {
        case KIcon::SizeSmall: value = 0; break;
        case KIcon::SizeSmallMedium: value = 1; break;
        case KIcon::SizeMedium: value = 2; break;
        case KIcon::SizeLarge: value = 3; break;
        case KIcon::SizeHuge: value = 4; break;
        case KIcon::SizeEnormous: value = 5; break;
        default: break;
    }
    return value;
}

void IconsViewSettingsPage::adjustTextWidthSelection()
{
    DolphinIconsViewSettings* settings = DolphinSettings::instance().iconsView(m_mode);
    assert(settings != 0);
    m_textWidthBox->setCurrentItem(settings->textWidthHint());
}

#include "iconsviewsettingspage.moc"
