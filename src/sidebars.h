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

#ifndef _SIDEBARS_H_
#define _SIDEBARS_H_

#include <qwidget.h>


class KURL;
class QComboBox;
class QVBoxLayout;
class SidebarPage;

class leftSidebar : public QWidget
{
    Q_OBJECT

    public:
        leftSidebar(QWidget* parent);
        virtual ~leftSidebar();

        virtual QSize sizeHint() const;

    signals:
    /**
     * The user selected an item on sidebar widget and item has
     * URL property, so inform the parent to go to this URL;
     */
        void urlChanged(const KURL& url);

    private slots:
        void createPage(int index);

    private:
        int indexForName(const QString& name) const;

        QComboBox* m_pagesSelector;
        SidebarPage* m_page;
        QVBoxLayout* m_layout;
};

class rightSidebar : public QWidget
{
    Q_OBJECT

    public:
        rightSidebar(QWidget* parent);
        virtual ~rightSidebar();

        virtual QSize sizeHint() const;

    signals:
    /**
     * The user selected an item on sidebar widget and item has
     * URL property, so inform the parent togo to this URL;
     */
        void urlChanged(const KURL& url);

    private slots:
        void createPage(int index);

    private:
        int indexForName(const QString& name) const;

        QComboBox* m_pagesSelector;
        SidebarPage* m_page;
        QVBoxLayout* m_layout;
};

#endif // _SIDEBARS_H_
