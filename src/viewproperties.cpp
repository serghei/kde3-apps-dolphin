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

#include <assert.h>

#include <qdatetime.h>
#include <qdir.h>
#include <qfile.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kurl.h>

#include "viewproperties.h"
#include "dolphinsettings.h"

#define FILE_NAME "/.d3lphinview"

ViewProperties::ViewProperties(KURL url) :
      m_changedProps(false),
      m_autoSave(true),
      m_subDirValidityHidden(false)
{
    url.cleanPath(true);
    m_filepath = url.path();

    if ((m_filepath.length() < 1) || (m_filepath.at(0) != QChar('/'))) {
        return;
    }

    // we try and save it to a file in the directory being viewed
    // if the directory is not writable by the user or the directory is not local
    // we store the properties information in a local file
    DolphinSettings& settings = DolphinSettings::instance();
    if (settings.isSaveView()) {
    QString rootDir("/"); // TODO: should this be set to the root of the bookmark, if any?
    if (url.isLocalFile()) {
        QFileInfo info(m_filepath);

        if (!info.isWritable()) {
            QString basePath = KGlobal::instance()->instanceName();
            basePath.append("/view_properties/local");
            rootDir = locateLocal("data", basePath);
            m_filepath = rootDir + m_filepath;
        }
    }
    else { 
        QString basePath = KGlobal::instance()->instanceName();
        basePath.append("/view_properties/remote/").append(url.host());
        rootDir = locateLocal("data", basePath);
        m_filepath = rootDir + m_filepath;
    }

    QDir dir(m_filepath);
    QFile file(m_filepath + FILE_NAME);

    
        PropertiesNode node(&file);
    
        const bool isValidForSubDirs = !node.isEmpty() && node.isValidForSubDirs();
        while ((dir.path() != rootDir) && dir.cdUp()) {
            QFile file(dir.path() + FILE_NAME);
            PropertiesNode parentNode(&file);
            if (!parentNode.isEmpty()) {
                const bool inheritProps = parentNode.isValidForSubDirs() &&
                                        (parentNode.subDirProperties().m_timeStamp >
                                        node.localProperties().m_timeStamp);
    
                if (inheritProps) {
                    node.setLocalProperties(parentNode.subDirProperties());
				    break;
                }
            }
        } 
    
        m_node = node;
    
        if (isValidForSubDirs) {
            m_subDirValidityHidden = true;
        }
        m_node.setValidForSubDirs(false);
    }
}

ViewProperties::~ViewProperties()
{
    if (m_changedProps && m_autoSave) {
        save();
    }
}

void ViewProperties::setViewMode(DolphinView::Mode mode)
{
    if (m_node.localProperties().m_viewMode != mode) {
        m_node.setViewMode(mode);
        updateTimeStamp();
    }
}

DolphinView::Mode ViewProperties::viewMode() const
{
    return m_node.localProperties().m_viewMode;
}

void ViewProperties::setShowHiddenFilesEnabled(bool show)
{
    if (m_node.localProperties().m_showHiddenFiles != show) {
        m_node.setShowHiddenFilesEnabled(show);
        updateTimeStamp();
    }
}

bool ViewProperties::isShowHiddenFilesEnabled() const
{
    return m_node.localProperties().m_showHiddenFiles;
}

void ViewProperties::setSorting(DolphinView::Sorting sorting)
{
    if (m_node.localProperties().m_sorting != sorting) {
        m_node.setSorting(sorting);
        updateTimeStamp();
    }
}

DolphinView::Sorting ViewProperties::sorting() const
{
    return m_node.localProperties().m_sorting;
}

void ViewProperties::setSortOrder(Qt::SortOrder sortOrder)
{
    if (m_node.localProperties().m_sortOrder != sortOrder) {
        m_node.setSortOrder(sortOrder);
        updateTimeStamp();
    }
}

Qt::SortOrder ViewProperties::sortOrder() const
{
    return m_node.localProperties().m_sortOrder;
}

void ViewProperties::setValidForSubDirs(bool valid)
{
    if (m_node.isValidForSubDirs() != valid) {
        m_node.setValidForSubDirs(valid);
        updateTimeStamp();
    }
}

bool ViewProperties::isValidForSubDirs() const
{
    return m_node.isValidForSubDirs();
}

void ViewProperties::setAutoSaveEnabled(bool autoSave)
{
    m_autoSave = autoSave;
}

bool ViewProperties::isAutoSaveEnabled() const
{
    return m_autoSave;
}

void ViewProperties::save()
{
    DolphinSettings& settings = DolphinSettings::instance();
    if (settings.isSaveView()) {
        QFile file(m_filepath + FILE_NAME);
        KStandardDirs::makeDir(m_filepath);
        if (!file.open(IO_WriteOnly)) {
            return;
        }
    
        const Properties& props = m_node.localProperties();
        char viewMode = static_cast<char>(props.m_viewMode) + '0';
        char sorting = static_cast<char>(props.m_sorting) + '0';
        const bool isValidForSubDirs = m_node.isValidForSubDirs() || m_subDirValidityHidden;
    
        QTextStream stream(&file);
        stream << "V01"
            << viewMode
            << (props.m_showHiddenFiles ? '1' : '0')
            << props.m_timeStamp.toString("yyyyMMddhhmmss")
            << sorting
            << ((props.m_sortOrder == Qt::Ascending) ? 'A' : 'D')
            << (isValidForSubDirs ? '1' : '0');
    
        if (m_node.isValidForSubDirs()) {
            m_node.setSubDirProperties(props);
        }
    
        if (isValidForSubDirs) {
            const Properties& subDirProps = m_node.subDirProperties();
            viewMode = static_cast<char>(subDirProps.m_viewMode) + '0';
            sorting = static_cast<char>(subDirProps.m_sorting) + '0';
            stream << viewMode
                << (subDirProps.m_showHiddenFiles ? '1' : '0')
                << subDirProps.m_timeStamp.toString("yyyyMMddhhmmss")
                << sorting
                << ((subDirProps.m_sortOrder == Qt::Ascending) ? 'A' : 'D');
        }
        file.flush();
        file.close();
    
        m_changedProps = false;
    }
}

void ViewProperties::updateTimeStamp()
{
    m_changedProps = true;
    m_node.setTimeStamp(QDateTime::currentDateTime());
}

ViewProperties::Properties::Properties() :
    m_showHiddenFiles(false),
    m_viewMode(DolphinView::IconsView),
    m_sorting(DolphinView::SortByName),
    m_sortOrder(Qt::Ascending)
{
    m_timeStamp.setDate(QDate(1999, 12, 31));
    m_timeStamp.setTime(QTime(23, 59, 59));

    m_viewMode = DolphinSettings::instance().defaultViewMode();
}

ViewProperties::Properties::~Properties()
{
}

ViewProperties::PropertiesNode::PropertiesNode(QFile* file) :
    m_empty(true)
{
    m_isValidForSubDirs = false;

    if ((file != 0) && file->open(IO_ReadOnly)) {
        m_empty = false;

        const int max_len = 41;
        static char buffer[max_len];

        // TODO: use memset
        for (int i = 0; i < max_len; ++i) {
            buffer[i] = 0;
        }

        file->readLine(buffer, max_len);

        // Check version of viewproperties file. The initial format
        // sadly had no version numbering, which is indicated by a missing 'V'
        // as first letter. The current scheme uses V + 2 digits.
        int version = 0;
        int startInc = 0;
        if (buffer[0] == 'V') {
            startInc = 3;        // skip version info (e. g. V01)
            version = 1;
            // currently no further versions are available:
            assert(buffer[1] == '0');
            assert(buffer[2] == '1');
        }

        int readBytes = readProperties(m_props, &buffer[startInc], version);
        assert(readBytes >= 0);

        // check whether sub directory properties are available
        m_isValidForSubDirs = (buffer[startInc + readBytes] != '0');
        if (m_isValidForSubDirs) {
            readBytes = readProperties(m_subDirProps,
                                       &buffer[startInc + readBytes + 1],
                                       version);
        }
        file->close();

        m_empty = (readBytes <= 0);
    }
}

ViewProperties::PropertiesNode::~PropertiesNode()
{
}

ViewProperties::PropertiesNode&
    ViewProperties::PropertiesNode::operator = (const PropertiesNode& node)
{
    if (&node != this) {
        m_empty = node.m_empty;
        m_isValidForSubDirs = node.m_isValidForSubDirs;
        m_props = node.m_props;
        m_subDirProps = node.m_subDirProps;
    }

    return *this;
}

int ViewProperties::PropertiesNode::toInt(const char* buffer, int count) const
{
    assert(buffer != 0);

    int value = 0;
    for (int i = 0; i < count; ++i) {
        value = value * 10 + static_cast<int>(buffer[i] - '0');
    }

    return value;
}

int ViewProperties::PropertiesNode::readProperties(Properties& props,
                                                   const char* buffer,
                                                   int version)
{
    props.m_viewMode = static_cast<DolphinView::Mode>(buffer[0] - '0');
    props.m_showHiddenFiles = (buffer[1] != '0');

    // read date
    QDateTime timeStamp;
    const int year  = toInt(&(buffer[2]), 4);
    const int month = toInt(&(buffer[6]), 2);
    const int day   = toInt(&(buffer[8]), 2);
    QDate date(year, month, day);
    timeStamp.setDate(date);

    // read time
    const int hour   = toInt(&(buffer[10]), 2);
    const int minute = toInt(&(buffer[12]), 2);
    const int second = toInt(&(buffer[14]), 2);
    QTime time(hour, minute, second);
    timeStamp.setTime(time);

    props.m_timeStamp = timeStamp;

    int readCount = 16;
    if (version >= 1) {
        // read sorting type and sorting order
        props.m_sorting = static_cast<DolphinView::Sorting>(buffer[16] - '0');
        props.m_sortOrder = (buffer[17] == 'A') ? Qt::Ascending : Qt::Descending;
        readCount = 18;
    }

    return (date.isValid() && time.isValid()) ? readCount : -1;
}


