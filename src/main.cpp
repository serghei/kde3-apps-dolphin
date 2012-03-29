/***************************************************************************
 *   Copyright (C) 2007 by Marcel Juhnke <marrat@marrat.homelinux.org>     *
 *   Copyright (C) 2006 by Peter Penz <peter.penz@gmx.at>                  *
 *   Copyright (C) 2006 by Stefan Monov <logixoul@gmail.com>               *
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

#include "dolphin.h"
#include <kapplication.h>
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <krun.h>

static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP( "Document to open" ), 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KAboutData about("d3lphin",
                     I18N_NOOP("Dolphin"),
                     "0.9.2",
                     I18N_NOOP("File Manager"),
                     KAboutData::License_GPL,
                     "(C) 2007 Marcel Juhnke");
    about.setHomepage("https://marrat.homelinux.org/D3lphin");
    about.setBugAddress("marrat@marrat.homelinux.org");
    about.addAuthor("Marcel Juhnke", I18N_NOOP("Maintainer and developer"), "marrat@marrat.homelinux.org");
    about.addAuthor("Michael Austin", I18N_NOOP("Documentation"), "tuxedup@users.sourceforge.net");
    about.addAuthor("Orville Bennett", I18N_NOOP("Documentation"), "obennett@hartford.edu");
    about.addCredit("Peter Penz", I18N_NOOP("... for the great original Dolphin"));
    about.addCredit("Cvetoslav Ludmiloff, Stefan Monov", I18N_NOOP("... for their development on the original Dolphin"));
    about.addCredit("Aaron J. Seigo", I18N_NOOP("... for the great support and the amazing patches for the orignal Dolphin"));
    about.addCredit("Patrice Tremblay, Gregor Kalisnik, Filip Brcic, Igor Stepin and Jan Mette", I18N_NOOP("... for their patches"));
    about.addCredit("Ain, Itai, Ivan, Jannick, Stephane, Patrice, Piotr, Stefano and Power On",
                    I18N_NOOP("... for their translations"));

    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;
    Dolphin& mainWin = Dolphin::mainWin();
    mainWin.show();

    if (app.isRestored()) {
        int n = 1;
        while (KMainWindow::canBeRestored(n)){
            Dolphin::mainWin().restore(n);
            ++n;
        }
    } else {
        KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
        if (args->count() > 0) {
            mainWin.activeView()->setURL(args->url(0));

            for (int i = 1; i < args->count(); ++i) {
                KRun::run("d3lphin", args->url(i));
            }
        }
        args->clear();
    }

    return app.exec();
}
