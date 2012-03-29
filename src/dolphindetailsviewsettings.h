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

#ifndef DOLPHINDETAILSVIEWSETTINGS_H
#define DOLPHINDETAILSVIEWSETTINGS_H

#include <qstring.h>
#include <qnamespace.h>
#include <dolphinsettingsbase.h>

/**
 * @brief Contains the settings for the details view.
 *
 * The following properties are stored:
 * - enabled columns
 * - sorted column
 * - sort order for the sorted column
 * - icon size
 * - font family
 * - font size
 *
 * @see DolphinDetailsView
 * @author Peter Penz <peter.penz@gmx.at>
 */
class DolphinDetailsViewSettings : public DolphinSettingsBase
{
public:
    DolphinDetailsViewSettings();

    virtual ~DolphinDetailsViewSettings();

    void setColumnEnabled(int column, bool enable);
    bool isColumnEnabled(int column) const;

    void setIconSize(int size) { m_iconSize = size; }
    int iconSize() const { return m_iconSize; }

    void setFontFamily(const QString& family) { m_fontFamily = family; }
    const QString& fontFamily() const { return m_fontFamily; }

    void setFontSize(int size) { m_fontSize = size; }
    int fontSize() const { return m_fontSize; }

    /** @see DolphinSettingsBase::save() */
    virtual void save();

private:
    int m_columnEnabled;
    int m_iconSize;
    int m_fontSize;
    QString m_fontFamily;
};

#endif
