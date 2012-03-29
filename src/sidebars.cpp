/***************************************************************************
 *   Copyright (C) 2007 by Marcel Juhnke                                   *
 *   marrat@marrat.homelinux.org                                           *
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

#include <qlayout.h>
#include <qpixmap.h>
#include <kiconloader.h>
#include <klocale.h>
#include <qcombobox.h>

#include "dolphinsettings.h"
#include "sidebarssettings.h"
#include "bookmarkssidebarpage.h"
#include "infosidebarpage.h"
#include "sidebars.h"

#include <assert.h>

/**
 * 
 * @param parent 
 */
leftSidebar::leftSidebar(QWidget* parent) :
        QWidget(parent),
        m_pagesSelector(0),
        m_page(0),
        m_layout(0)
{
    m_layout = new QVBoxLayout(this);

    m_pagesSelector = new QComboBox(this);
    m_pagesSelector->insertItem(i18n("Bookmarks"));
    m_pagesSelector->insertItem(i18n("Information"));

    // Assure that the combo box has the same height as the URL navigator for
    // a clean layout.
    // TODO: the following 2 lines have been copied from the URLNavigator
    // constructor (-> provide a shared height setting?)
    //QFontMetrics fontMetrics(font());
    QFontMetrics fontMetrics(font());
    m_pagesSelector->setMinimumHeight(fontMetrics.height() + 8);

    leftSidebarSettings* settings = DolphinSettings::instance().leftsidebar();
    const int selectedIndex = indexForName(settings->selectedPage());
    m_pagesSelector->setCurrentItem(selectedIndex);
    m_layout->addWidget(m_pagesSelector);

    createPage(selectedIndex);

    connect(m_pagesSelector, SIGNAL(activated(int)),
            this, SLOT(createPage(int)));
}

leftSidebar::~leftSidebar()
{
}

QSize leftSidebar::sizeHint() const
{
    QSize size(QWidget::sizeHint());

    leftSidebarSettings* settings = DolphinSettings::instance().leftsidebar();
    size.setWidth(settings->width());
    return size;
}

void leftSidebar::createPage(int index)
{
    if (m_page != 0) {
        m_page->deleteLater();
        m_page = 0;
    }

    switch (index) {
        case 0: m_page = new BookmarksSidebarPage(this); break;
        case 1: m_page = new InfoSidebarPage(this); break;
        default: break;
    }

    m_layout->addWidget(m_page);
    m_page->show();

    leftSidebarSettings* settings = DolphinSettings::instance().leftsidebar();
    settings->setSelectedPage(m_pagesSelector->text(index));
}

int leftSidebar::indexForName(const QString& name) const
{
    const int count = m_pagesSelector->count();
    for (int i = 0; i < count; ++i) {
        if (m_pagesSelector->text(i) == name) {
            return i;
        }
    }

    return 0;
}

rightSidebar::rightSidebar(QWidget* parent) :
        QWidget(parent),
        m_pagesSelector(0),
        m_page(0),
        m_layout(0)
{
    m_layout = new QVBoxLayout(this);

    m_pagesSelector = new QComboBox(this);
    m_pagesSelector->insertItem(i18n("Bookmarks"));
    m_pagesSelector->insertItem(i18n("Information"));

    // Assure that the combo box has the same height as the URL navigator for
    // a clean layout.
    // TODO: the following 2 lines have been copied from the URLNavigator
    // constructor (-> provide a shared height setting?)
    QFontMetrics fontMetrics(font());
    m_pagesSelector->setMinimumHeight(fontMetrics.height() + 8);

    rightSidebarSettings* settings = DolphinSettings::instance().rightsidebar();
    const int selectedIndex = indexForName(settings->selectedPage());
    m_pagesSelector->setCurrentItem(selectedIndex);
    m_layout->addWidget(m_pagesSelector);

    createPage(selectedIndex);

    connect(m_pagesSelector, SIGNAL(activated(int)),
            this, SLOT(createPage(int)));
}

rightSidebar::~rightSidebar()
{
}

QSize rightSidebar::sizeHint() const
{
    QSize size(QWidget::sizeHint());

    rightSidebarSettings* settings = DolphinSettings::instance().rightsidebar();
    size.setWidth(settings->width());
    return size;
}

void rightSidebar::createPage(int index)
{
    if (m_page != 0) {
        m_page->deleteLater();
        m_page = 0;
    }

    switch (index) {
        case 0: m_page = new BookmarksSidebarPage(this); break;
        case 1: m_page = new InfoSidebarPage(this); break;
        default: break;
    }

    m_layout->addWidget(m_page);
    m_page->show();

    rightSidebarSettings* settings = DolphinSettings::instance().rightsidebar();
    settings->setSelectedPage(m_pagesSelector->text(index));
}

int rightSidebar::indexForName(const QString& name) const
{
    const int count = m_pagesSelector->count();
    for (int i = 0; i < count; ++i) {
        if (m_pagesSelector->text(i) == name) {
            return i;
        }
    }

    return 0;
}

#include "sidebars.moc"

