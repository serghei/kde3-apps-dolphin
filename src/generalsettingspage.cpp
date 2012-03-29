/***************************************************************************
 *   Copyright (C) 2006 by Peter Penz (peter.penz@gmx.at) and              *
 *   and Patrice Tremblay                                                  *
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

#include "generalsettingspage.h"

#include <qlayout.h>
#include <kdialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qvbox.h>
#include <qgrid.h>
#include <qgroupbox.h>
#include <klocale.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <kfiledialog.h>
#include <qradiobutton.h>

#include "dolphinsettings.h"
#include "dolphin.h"
#include "dolphinview.h"

GeneralSettingsPage::GeneralSettingsPage(QWidget* parent) :
    SettingsPageBase(parent),
    m_homeURL(0),
    m_startSplit(0),
    m_startEditable(0)
{
    QVBoxLayout* topLayout = new QVBoxLayout(parent, 2, KDialog::spacingHint());

    const int spacing = KDialog::spacingHint();
    const int margin = KDialog::marginHint();
    const QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    DolphinSettings& settings = DolphinSettings::instance();

    QVBox* vBox = new QVBox(parent);
    vBox->setSizePolicy(sizePolicy);
    vBox->setSpacing(spacing);
    vBox->setMargin(margin);
    vBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);

    // create 'Home URL' editor
    QGroupBox* homeGroup = new QGroupBox(1, Qt::Horizontal, i18n("Home URL"), vBox);
    homeGroup->setSizePolicy(sizePolicy);
    homeGroup->setMargin(margin);

    QHBox* homeURLBox = new QHBox(homeGroup);
    homeURLBox->setSizePolicy(sizePolicy);
    homeURLBox->setSpacing(spacing);

    new QLabel(i18n("Location:"), homeURLBox);
    m_homeURL = new QLineEdit(settings.homeURL().prettyURL(), homeURLBox);

    QPushButton* selectHomeURLButton = new QPushButton(SmallIcon("folder"), QString::null, homeURLBox);
    connect(selectHomeURLButton, SIGNAL(clicked()),
            this, SLOT(selectHomeURL()));

    QHBox* buttonBox = new QHBox(homeGroup);
    buttonBox->setSizePolicy(sizePolicy);
    buttonBox->setSpacing(spacing);
    QPushButton* useCurrentButton = new QPushButton(i18n("Use current location"), buttonBox);
    connect(useCurrentButton, SIGNAL(clicked()),
            this, SLOT(useCurrentLocation()));
    QPushButton* useDefaultButton = new QPushButton(i18n("Use default location"), buttonBox);
    connect(useDefaultButton, SIGNAL(clicked()),
            this, SLOT(useDefaulLocation()));

    // create 'Default View Mode' group
    QButtonGroup* buttonGroup = new QButtonGroup(3, Qt::Vertical, i18n("Default View Mode"), vBox);
    buttonGroup->setSizePolicy(sizePolicy);
    buttonGroup->setMargin(margin);

    m_iconsView = new QRadioButton(i18n("Icons"), buttonGroup);
    m_detailsView = new QRadioButton(i18n("Details"), buttonGroup);
    m_previewsView = new QRadioButton(i18n("Previews"), buttonGroup);

    switch (settings.defaultViewMode()) {
        case DolphinView::IconsView:    m_iconsView->setChecked(true); break;
        case DolphinView::DetailsView:  m_detailsView->setChecked(true); break;
        case DolphinView::PreviewsView: m_previewsView->setChecked(true); break;
    }

    // create 'Start with split view' checkbox
    m_startSplit = new QCheckBox(i18n("Start with split view"), vBox);
    m_startSplit->setChecked(settings.isViewSplit());

    // create 'Start with editable navigation bar' checkbox
    m_startEditable = new QCheckBox(i18n("Start with editable navigation bar"), vBox);
    m_startEditable->setChecked(settings.isURLEditable());

    // create 'Save view properties for each folder' checkbox
    m_saveView = new QCheckBox(i18n("Save view properties for each folder"), vBox);
    m_saveView->setChecked(settings.isSaveView());

    // Add a dummy widget with no restriction regarding
    // a vertical resizing. This assures that the dialog layout
    // is not stretched vertically.
    new QWidget(vBox);

    topLayout->addWidget(vBox);
}


GeneralSettingsPage::~GeneralSettingsPage()
{
}

void GeneralSettingsPage::applySettings()
{
    DolphinSettings& settings = DolphinSettings::instance();

    const KURL url(m_homeURL->text());
    KFileItem fileItem(S_IFDIR, KFileItem::Unknown, url);
    if (url.isValid() && fileItem.isDir()) {
        settings.setHomeURL(url);
    }

    DolphinView::Mode viewMode = DolphinView::IconsView;
    if (m_detailsView->isChecked()) {
        viewMode = DolphinView::DetailsView;
    }
    else if (m_previewsView->isChecked()) {
        viewMode = DolphinView::PreviewsView;
    }
    settings.setDefaultViewMode(viewMode);

    settings.setViewSplit(m_startSplit->isChecked());
    settings.setSaveView(m_saveView->isChecked());
    settings.setURLEditable(m_startEditable->isChecked());
}

void GeneralSettingsPage::selectHomeURL()
{
    const QString homeURL(m_homeURL->text());
    KURL url(KFileDialog::getExistingURL(homeURL));
    if (!url.isEmpty()) {
        m_homeURL->setText(url.prettyURL());
    }
}

void GeneralSettingsPage::useCurrentLocation()
{
    const DolphinView* view = Dolphin::mainWin().activeView();
    m_homeURL->setText(view->url().prettyURL());
}

void GeneralSettingsPage::useDefaulLocation()
{
    m_homeURL->setText("file://" + QDir::homeDirPath());
}

#include "generalsettingspage.moc"
