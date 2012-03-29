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

#ifndef VIEWPROPERTIES_H
#define VIEWPROPERTIES_H

#include <dolphinview.h>
#include <kurl.h>
#include <qdatetime.h>
class QFile;

/**
 * @short Maintains the view properties like 'view mode' or 'show hidden files' for a directory.
 *
 * The view properties are automatically stored inside
 * the directory as hidden file called '.dolphinview'. To read out the view properties
 * just construct an instance by passing the URL of the directory:
 * \code
 * ViewProperties props(KURL("/home/peter/Documents"));
 * const DolphinView::Mode mode = props.viewMode();
 * const bool showHiddenFiles = props.isShowHiddenFilesEnabled();
 * \endcode
 * When modifying a view property, the '.dolphinview' file is automatically updated
 * inside the destructor.
 *
 * @author Peter Penz
 */
// TODO: provide detailed design description, as mapping the user model to
// the physical modal is not trivial.
class ViewProperties
{
public:
    ViewProperties(KURL url);
    virtual ~ViewProperties();

    void setViewMode(DolphinView::Mode mode);
    DolphinView::Mode viewMode() const;

    void setShowHiddenFilesEnabled(bool show);
    bool isShowHiddenFilesEnabled() const;

    void setSorting(DolphinView::Sorting sorting);
    DolphinView::Sorting sorting() const;

    void setSortOrder(Qt::SortOrder sortOrder);
    Qt::SortOrder sortOrder() const;

    void setValidForSubDirs(bool valid);
    bool isValidForSubDirs() const;

    void setAutoSaveEnabled(bool autoSave);
    bool isAutoSaveEnabled() const;

    void updateTimeStamp();
    void save();

private:
    class Properties
    {
    public:
        Properties();
        ~Properties();  // non virtual

        bool m_showHiddenFiles;
        DolphinView::Mode m_viewMode;
        QDateTime m_timeStamp;
        DolphinView::Sorting m_sorting;
        Qt::SortOrder m_sortOrder;
    };

    class PropertiesNode
    {
    public:
        PropertiesNode(QFile* file = 0);
        ~PropertiesNode();
        PropertiesNode& operator = (const PropertiesNode& node);
        bool isEmpty() const { return m_empty; }

        void setValidForSubDirs(bool valid) { m_isValidForSubDirs = valid; }
        bool isValidForSubDirs() const { return m_isValidForSubDirs; }

        void setLocalProperties(const Properties& props) { m_props = props; }
        const Properties& localProperties() const { return m_props; }

        void setShowHiddenFilesEnabled(bool show) { m_props.m_showHiddenFiles = show; }
        void setViewMode(DolphinView::Mode mode) { m_props.m_viewMode = mode; }
        void setTimeStamp(const QDateTime timeStamp) { m_props.m_timeStamp = timeStamp; }
        const QDateTime& timeStamp() const { return m_props.m_timeStamp; }
        void setSorting(DolphinView::Sorting sorting) { m_props.m_sorting = sorting; }
        void setSortOrder(Qt::SortOrder sortOrder) { m_props.m_sortOrder = sortOrder; }

        void setSubDirProperties(const Properties& props) { m_subDirProps = props; }
        const Properties& subDirProperties() const { return m_subDirProps; }

    private:
        int toInt(const char* buffer, int count) const;
        int readProperties(Properties& props, const char* buffer, int version);

        bool m_empty;
        bool m_isValidForSubDirs;
        Properties m_props;
        Properties m_subDirProps;
    };

    bool m_changedProps;
    bool m_autoSave;
    bool m_subDirValidityHidden;
    QString m_filepath;
    PropertiesNode m_node;
};

#endif
