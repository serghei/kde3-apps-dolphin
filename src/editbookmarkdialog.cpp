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

#include "editbookmarkdialog.h"
#include <qgrid.h>
#include <klocale.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <kiconloader.h>
#include <qpushbutton.h>
#include <kurl.h>
#include <kfiledialog.h>
#include <kicondialog.h>
#include <qhbox.h>

EditBookmarkDialog::~EditBookmarkDialog()
{
}

KBookmark EditBookmarkDialog::getBookmark(const QString& title,
                                          const QString& name,
                                          const KURL& url,
                                          const QString& icon)
{
    EditBookmarkDialog dialog(title, name, url, icon);
    dialog.exec();
    return dialog.m_bookmark;
}

void EditBookmarkDialog::slotOk()
{
    m_bookmark = KBookmark::standaloneBookmark(m_name->text(),
                                               KURL(m_location->text()),
                                               m_iconName);

    KDialogBase::slotOk();
}

EditBookmarkDialog::EditBookmarkDialog(const QString& title,
                                       const QString& name,
                                       const KURL& url,
                                       const QString& icon) :
    KDialogBase(Plain, title, Ok|Cancel, Ok),
    m_iconButton(0),
    m_name(0),
    m_location(0)
{
    QVBoxLayout* topLayout = new QVBoxLayout(plainPage(), 0, spacingHint());

    QGrid* grid = new QGrid(2, Qt::Horizontal, plainPage());
    grid->setSpacing(spacingHint());

    // create icon widgets
    new QLabel(i18n("Icon:"), grid);
    m_iconName = icon;
    m_iconButton = new QPushButton(SmallIcon(m_iconName), QString::null, grid);
    m_iconButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(m_iconButton, SIGNAL(clicked()),
            this, SLOT(selectIcon()));

    // create name widgets
    new QLabel(i18n("Name:"), grid);
    m_name = new QLineEdit(name, grid);
    m_name->selectAll();
    m_name->setFocus();

    // create location widgets
    new QLabel(i18n("Location:"), grid);

    QHBox* locationBox = new QHBox(grid);
    locationBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    locationBox->setSpacing(spacingHint());
    m_location = new QLineEdit(url.prettyURL(), locationBox);
    m_location->setMinimumWidth(320);

    QPushButton* selectLocationButton = new QPushButton(SmallIcon("folder"), QString::null, locationBox);
    selectLocationButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(selectLocationButton, SIGNAL(clicked()),
            this, SLOT(selectLocation()));

    topLayout->addWidget(grid);
}

void EditBookmarkDialog::selectIcon()
{
    const QString iconName(KIconDialog::getIcon(KIcon::Small, KIcon::FileSystem));
    if (!iconName.isEmpty()) {
        m_iconName = iconName;
        m_iconButton->setIconSet(SmallIcon(iconName));
    }
}

void EditBookmarkDialog::selectLocation()
{
    const QString location(m_location->text());
    KURL url(KFileDialog::getExistingURL(location));
    if (!url.isEmpty()) {
        m_location->setText(url.prettyURL());
    }
}

#include "editbookmarkdialog.moc"
