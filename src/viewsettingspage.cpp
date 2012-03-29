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

#include "viewsettingspage.h"
#include <qtabwidget.h>
#include <qlayout.h>
#include <qlabel.h>
#include <kdialogbase.h>
#include <klocale.h>
#include "iconsviewsettingspage.h"
#include "detailsviewsettingspage.h"

ViewSettingsPage::ViewSettingsPage(QWidget *parent) :
    SettingsPageBase(parent),
    m_iconsPage(0),
    m_detailsPage(0),
    m_previewsPage(0)
{
    QVBoxLayout* topLayout = new QVBoxLayout(parent, 0, KDialog::spacingHint());

    QTabWidget* tabWidget = new QTabWidget(parent);

    // initialize 'Icons' tab
    m_iconsPage = new IconsViewSettingsPage(DolphinIconsView::Icons, tabWidget);
    tabWidget->addTab(m_iconsPage, SmallIcon("view_icon"), i18n("Icons"));

    // initialize 'Details' tab
    m_detailsPage = new DetailsViewSettingsPage(tabWidget);
    tabWidget->addTab(m_detailsPage, SmallIcon("view_text"), i18n("Details"));

    // initialize 'Previews' tab
    m_previewsPage = new IconsViewSettingsPage(DolphinIconsView::Previews, tabWidget);
    tabWidget->addTab(m_previewsPage, SmallIcon("gvdirpart"), i18n("Previews"));

    topLayout->addWidget(tabWidget, 0, 0 );
}

ViewSettingsPage::~ViewSettingsPage()
{
}

void ViewSettingsPage::applySettings()
{
    m_iconsPage->applySettings();
    m_detailsPage->applySettings();
    m_previewsPage->applySettings();
}

#include "viewsettingspage.moc"
