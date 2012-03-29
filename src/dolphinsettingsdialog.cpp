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

#include "dolphinsettingsdialog.h"
#include <klocale.h>
#include <kiconloader.h>
#include "generalsettingspage.h"
#include "viewsettingspage.h"
#include "bookmarkssettingspage.h"
#include "dolphin.h"

DolphinSettingsDialog::DolphinSettingsDialog() :
    KDialogBase(IconList, i18n("D3lphin Preferences"),
                Ok|Apply|Cancel, Ok)
{
    KIconLoader iconLoader;
    QFrame* generalSettingsFrame = addPage(i18n("General"), 0,
                                                iconLoader.loadIcon("exec",
                                                                    KIcon::NoGroup,
                                                                    KIcon::SizeMedium));
    m_generalSettingsPage = new GeneralSettingsPage(generalSettingsFrame);

    QFrame* viewSettingsFrame = addPage(i18n("View Modes"), 0,
                                        iconLoader.loadIcon("view_choose",
                                                            KIcon::NoGroup,
                                                            KIcon::SizeMedium));
    m_viewSettingsPage = new ViewSettingsPage(viewSettingsFrame);

    QFrame* bookmarksSettingsFrame = addPage(i18n("Bookmarks"), 0,
                                             iconLoader.loadIcon("bookmark",
                                                                 KIcon::NoGroup,
                                                                 KIcon::SizeMedium));
    m_bookmarksSettingsPage = new BookmarksSettingsPage(bookmarksSettingsFrame);
}

DolphinSettingsDialog::~DolphinSettingsDialog()
{
}

void DolphinSettingsDialog::slotOk()
{
    applySettings();
    KDialogBase::slotOk();
}

void DolphinSettingsDialog::slotApply()
{
    applySettings();
    KDialogBase::slotApply();
}

void DolphinSettingsDialog::applySettings()
{
    m_generalSettingsPage->applySettings();
    m_viewSettingsPage->applySettings();
    m_bookmarksSettingsPage->applySettings();
    Dolphin::mainWin().refreshViews();
}

#include "dolphinsettingsdialog.moc"
