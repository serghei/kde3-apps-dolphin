/***************************************************************************
 *   Copyright (C) 2006 by Peter Penz (peter.penz@gmx.at),                 *
 *   Cvetoslav Ludmiloff and Patrice Tremblay                              *
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

#include "dolphinsettings.h"
#include <qdir.h>

#include <kapplication.h>
#include <kbookmark.h>
#include <kbookmarkmanager.h>
#include <kicontheme.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "dolphin.h"
#include "dolphiniconsviewsettings.h"
#include "dolphindetailsviewsettings.h"
#include "sidebarssettings.h"

DolphinSettings& DolphinSettings::instance()
{
    static DolphinSettings* instance = 0;
    if (instance == 0) {
        instance = new DolphinSettings();
    }
    return *instance;
}

void DolphinSettings::setHomeURL(const KURL& url)
{
    m_homeURL = url;
    // TODO: update home bookmark?
}

DolphinSettings::DolphinSettings() :
    m_defaultMode(DolphinView::IconsView),
    m_isViewSplit(false),
    m_isURLEditable(false)
{
    KConfig* config = kapp->config();
    config->setGroup("General");
    m_homeURL = KURL(config->readEntry("Home URL", QDir::homeDirPath()));
    m_defaultMode = static_cast<DolphinView::Mode>(config->readNumEntry("Default View Mode", DolphinView::IconsView));
    m_isViewSplit = config->readBoolEntry("Split View", false);
    m_isSaveView = config->readBoolEntry("Save View", false);
    m_isURLEditable = config->readBoolEntry("Editable URL", false);

    m_iconsView = new DolphinIconsViewSettings(DolphinIconsView::Icons);
    m_previewsView = new DolphinIconsViewSettings(DolphinIconsView::Previews);
    m_detailsView = new DolphinDetailsViewSettings();
    m_leftsidebar = new leftSidebarSettings();
    m_rightsidebar = new rightSidebarSettings();
}

DolphinSettings::~DolphinSettings()
{
    delete m_iconsView;
    m_iconsView = 0;

    delete m_previewsView;
    m_previewsView = 0;

    delete m_detailsView;
    m_detailsView = 0;

    delete m_leftsidebar;
    m_leftsidebar = 0;

    delete m_rightsidebar;
    m_rightsidebar = 0;
}

KBookmark DolphinSettings::bookmark(int index) const
{
    int i = 0;
    KBookmarkGroup root = bookmarkManager()->root();
    KBookmark bookmark = root.first();
    while (!bookmark.isNull()) {
        if (i == index) {
            return bookmark;
        }
        ++i;
        bookmark = root.next(bookmark);
    }

    return KBookmark();
}

DolphinIconsViewSettings* DolphinSettings::iconsView(DolphinIconsView::LayoutMode mode) const
{
    return (mode == DolphinIconsView::Icons) ? m_iconsView : m_previewsView;
}

KBookmarkManager* DolphinSettings::bookmarkManager() const
{
    QString basePath = KGlobal::instance()->instanceName();
    basePath.append("/bookmarks.xml");
    const QString file = locateLocal("data", basePath);

    return KBookmarkManager::managerForFile(file, false);
}

void DolphinSettings::save()
{
    KConfig* config = kapp->config();
    config->setGroup("General");
    config->writeEntry("Version", 1);  // internal version
    config->writeEntry("Home URL", m_homeURL.prettyURL());
    config->writeEntry("Default View Mode", m_defaultMode);
    config->writeEntry("Split View", m_isViewSplit);
    config->writeEntry("Save View", m_isSaveView);
    config->writeEntry("Editable URL", m_isURLEditable);

    m_iconsView->save();
    m_previewsView->save();
    m_detailsView->save();
    m_leftsidebar->save();
    m_rightsidebar->save();

    QString basePath = KGlobal::instance()->instanceName();
    basePath.append("/bookmarks.xml");
    const QString file = locateLocal( "data", basePath);

    KBookmarkManager* manager = KBookmarkManager::managerForFile(file, false);
    manager->save(false);
}
