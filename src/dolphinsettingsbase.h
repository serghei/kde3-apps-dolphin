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
#ifndef DOLPHINSETTINGSBASE_H
#define DOLPHINSETTINGSBASE_H

/**
 * @brief Base class for all Dolphin settings.
 *
 * Derived classes must implement the method
 * DolphinSettingsBase::save().
 *
 * @author Peter Penz <peter.penz@gmx.at>
 */
// TODO: design this base class as Composite pattern,
// as settings might contain other settings
class DolphinSettingsBase
{
public:
    DolphinSettingsBase();

    virtual ~DolphinSettingsBase();

    virtual void save() = 0;
};

#endif
