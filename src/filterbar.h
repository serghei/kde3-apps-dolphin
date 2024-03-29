/***************************************************************************
 *   Copyright (C) 2006 by Peter Penz <peter.penz@gmx.at>                  *
 *   Copyright (C) 2006 by Gregor Kališnik <gregor@podnapisi.net>          *
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
#ifndef FILTERBAR_H
#define FILTERBAR_H

#include <qwidget.h>

class QLabel;
class KLineEdit;
class KPushButton;

/**
 * @brief Provides an input field for filtering the currently shown items.
 *
 * @author Gregor Kališnik <gregor@podnapisi.net>
 */
class FilterBar : public QWidget
{
    Q_OBJECT

public:
    FilterBar(QWidget *parent = 0, const char *name = 0);
    virtual ~FilterBar();

signals:
    /**
    * Signal that reports the name filter has been
    * changed to \a nameFilter.
    */
    void signalFilterChanged(const QString& nameFilter);

public slots:
    /** @see QWidget::hide() */
    virtual void hide();

    /** @see QWidget::show() */
    virtual void show();

protected:
    virtual void keyReleaseEvent(QKeyEvent* event);

private:
    QLabel* m_filter;
    KLineEdit* m_filterInput;
    KPushButton* m_close;
};

#endif
