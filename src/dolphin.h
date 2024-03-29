/***************************************************************************
 *   Copyright (C) 2006 by Peter Penz <peter.penz@gmx.at>                  *
 *   Copyright (C) 2006 by Stefan Monov <logixoul@gmail.com>               *
 *   Copyright (C) 2006 by Cvetoslav Ludmiloff <ludmiloff@gmail.com>       *
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

#ifndef _DOLPHIN_H_
#define _DOLPHIN_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kmainwindow.h>
#include <qvaluelist.h>
#include <qptrlist.h>
#include <qstring.h>
#include <ksortablevaluelist.h>

#include "dolphinview.h"
#include "undomanager.h"

class KPrinter;
class KURL;
class QLineEdit;
class KFileIconView;
class QHBox;
class QIconViewItem;
class QSplitter;
class KAction;
class URLNavigator;
class leftSidebar;
class rightSidebar;

/**
 * @short Main window for Dolphin.
 *
 * Handles the menus, toolbars and Dolphin views.
 *
 * @author Peter Penz <peter.penz@gmx.at>
*/
class Dolphin : public KMainWindow
{
    Q_OBJECT

public:
    /**
     * Returns the instance for the Dolphin main window.
     */
    // KXMLGUIClient::instance() already in use :-(
    static Dolphin& mainWin();

    virtual ~Dolphin();

	/**
     * Activates the given view, which means that
     * all menu actions are applied to this view. When
     * having a split view setup the nonactive view
     * is usually shown in darker colors.
     */
    void setActiveView(DolphinView* view);

    /**
     * Returns the currently active view. See
     * Dolphin::setActiveView() for more details.
     */
    DolphinView* activeView() const { return m_activeView; }

    /**
     * Handles the dropping of URLs to the given
     * destination. A context menu with the options
     * 'Move Here', 'Copy Here', 'Link Here' and
     * 'Cancel' is offered to the user.
     * @param urls        List of URLs which have been
     *                    dropped.
     * @param destination Destination URL, where the
     *                    list or URLs should be moved,
     *                    copied or linked to.
     */
    void dropURLs(const KURL::List& urls,
                  const KURL& destination);

    /**
     * Returns 'true', if the clipboard contains data
     * which has been cutted by the Cut action (Ctrl + X).
     */
    bool clipboardContainsCutData() const { return m_clipboardContainsCutData; }

    /**
     * Returns the list of actions which are part of the file group
     * of the 'Create New...' sub menu. Usually the list contains actions
     * for creating folders, text files, HTML files etc.
     */
    const QPtrList<KAction>& fileGroupActions() const { return m_fileGroupActions; }
    //const QPtrList<KAction>& linkGroupActions() const { return m_linkGroupActions; }
    //const QPtrList<KAction>& linkToDeviceActions() const { return m_linkToDeviceActions; }

    /**
     * Refreshs the views of the main window by recreating them dependent from
     * the given Dolphin settings.
     */
    void refreshViews();

signals:
    /**
     * Is send if the active view has been changed in
     * the split view mode.
     */
    void activeViewChanged();

    /**
     * Is send if the selection of the currently active view has
     * been changed.
     */
    void selectionChanged();

public slots:
    /**
     * Updates the state of the 'Back' and 'Forward' menu
     * actions corresponding the the current history.
     */
    void slotHistoryChanged();

    /**
     * Updates the caption of the main window and the state
     * of all menu actions which depend from a changed URL.
     */
    void slotURLChanged(const KURL& url);

    /**
     * Go to the given URL.
     */
    void slotURLChangeRequest(const KURL& url);

    /** Updates the state of all 'View' menu actions. */
    void slotViewModeChanged();

    /** Updates the state of the 'Show hidden files' menu action. */
    void slotShowHiddenFilesChanged();

    /** Updates the state of the 'Show filter bar' menu action. */
    void slotShowFilterBarChanged();

    /** Updates the state of the 'Sort by' actions. */
    void slotSortingChanged(DolphinView::Sorting sorting);

    /** Updates the state of the 'Sort Ascending/Descending' action. */
    void slotSortOrderChanged(Qt::SortOrder order);

    /** Updates the state of the 'Edit' menu actions. */
    void slotSelectionChanged();

protected:
    /** @see QMainWindow::closeEvent */
    virtual void closeEvent(QCloseEvent* event);

    /**
     * This method is called when it is time for the app to save its
     * properties for session management purposes.
     */
    void saveProperties(KConfig*);

    /**
     * This method is called when this app is restored.  The KConfig
     * object points to the session management config file that was saved
     * with @ref saveProperties
     */
    void readProperties(KConfig*);

private slots:
    /** Opens an input dialog for creating a new folder. */
    void createFolder();

    /** Creates a file with the MIME type given by the sender. */
    void createFile();

    /** Renames the selected item of the active view. */
    void rename();

    /** Moves the selected items of the active view to the trash. */
    void moveToTrash();

    /** Deletes the selected items of the active view. */
    void deleteItems();

    /**
     * Opens the properties window for the selected items of the
     * active view. The properties windows shows informations
     * like name, size and permissions.
     */
    void properties();

    /** Stores all settings and quits Dolphin. */
    void quit();

    /**
     * Shows the error information of the job \a job
     * in the status bar.
     */
    void slotHandleJobError(KIO::Job* job);

    /**
     * Indicates in the status bar that the delete operation
     * of the job \a job has been finished.
     */
    void slotDeleteFileFinished(KIO::Job* job);

    /**
     * Updates the state of the 'Undo' menu action dependent
     * from the parameter \a available.
     */
    void slotUndoAvailable(bool available);

    /** Sets the text of the 'Undo' menu action to \a text. */
    void slotUndoTextChanged(const QString& text);

    /**
     * Updates the state of the 'Redo' menu action dependent
     * from the parameter \a available.
     */
    void slotRedoAvailable(bool available);

    /** Sets the text of the 'Redo' menu action to \a text. */
    void slotRedoTextChanged(const QString& text);

    /**
     * Copies all selected items to the clipboard and marks
     * the items as cutted.
     */
    void cut();

    /** Copies all selected items to the clipboard. */
    void copy();

    /** Pastes the clipboard data to the active view. */
    void paste();

    /**
     * Updates the text of the paste action dependent from
     * the number of items which are in the clipboard.
     */
    void updatePasteAction();

    /** Selects all items from the active view. */
    void selectAll();

    /**
     * Inverts the selection of all items of the active view:
     * Selected items get nonselected and nonselected items get
     * selected.
     */
    void invertSelection();

    /** The current active view is switched to the icons mode. */
    void setIconsView();

    /** The current active view is switched to the details mode. */
    void setDetailsView();

    /** The current active view is switched to the previews mode. */
    void setPreviewsView();

    /** The sorting of the current view should be done by the name. */
    void sortByName();

    /** The sorting of the current view should be done by the size. */
    void sortBySize();

    /** The sorting of the current view should be done by the date. */
    void sortByDate();

    /** Switches between an ascending and descending sorting order. */
    void toggleSortOrder();

    /**
     * Switches between one and two views:
     * If one view is visible, it will get split into two views.
     * If already two views are visible, the nonactivated view gets closed.
     */
    void toggleSplitView();

    /** Reloads the current active view. */
    void reloadView();

    /** Stops the loading process for the current active view. */
    void stopLoading();

    /**
     * Switches between showing and hiding of hidden marked files dependent
     * from the current state of the 'Show Hidden Files' menu toggle action.
     */
    void showHiddenFiles();

    /**
     * Switches between showing and hiding of the filter bar dependent
     * from the current state of the 'Show Filter Bar' menu toggle action.
     */
    void showFilterBar();

    /** Increases the size of the current set view mode. */
    void zoomIn();

    /** Decreases the size of the current set view mode. */
    void zoomOut();

    /**
     * Switches to the edit mode of the navigation bar. If the edit mode is
     * already active, it is assured that the navigation bar get focused.
     */
    void editLocation();

    /** Switches to the browse mode of the navigation bar. */
    void browse();

    /**
     * Opens the view properties dialog, which allows to modify the properties
     * of the currently active view.
     */
    void adjustViewProperties();

    /** Goes back on step of the URL history. */
    void goBack();

    /** Goes forward one step of the URL history. */
    void goForward();

    /** Goes up one hierarchy of the current URL. */
    void goUp();

    /** Goes to the home URL. */
    void goHome();

    /** Opens a terminal for the current shown directory. */
    void openTerminal();

    /** Opens KFind for the current shown directory. */
    void findFile();

    /** Opens Kompare for 2 selected files. */
    void compareFiles();

    /** Opens the settings dialog for Dolphin. */
    void editSettings();

    /**
     * Adds the undo operation given by \a job
     * to the UndoManager.
     */
    void addUndoOperation(KIO::Job* job);


    void toggleleftSidebar();
    void togglerightSidebar();

    /**
     * Stores the current sidebar width and closes
     * the sidebar.
     */
    void closeleftSidebar();
    void closerightSidebar();

private:
    Dolphin();
    void init();
    void loadSettings();

    void setupAccel();
    void setupActions();
    void setupCreateNewMenuActions();
    void updateHistory();
    void updateEditActions();
    void updateViewActions();
    void updateGoActions();
    void updateViewProperties(const KURL::List& urls);
    void copyURLs(const KURL::List& source, const KURL& dest);
    void moveURLs(const KURL::List& source, const KURL& dest);
    void addPendingUndoJob(KIO::Job* job,
                           DolphinCommand::Type commandType,
                           const KURL::List& source,
                           const KURL& dest);
    void clearStatusBar();
    void openleftSidebar();
    void openrightSidebar();

    QSplitter* m_splitter;
	leftSidebar* m_leftsidebar;
	rightSidebar* m_rightsidebar;
    DolphinView* m_activeView;

    /**
     * Dolphin supports only one or two views, which
     * are handled internally as primary and secondary view.
     */
    enum ViewIndex
    {
        PrimaryIdx = 0,
        SecondaryIdx = 1
    };
    DolphinView* m_view[SecondaryIdx + 1];

    /// If set to true, the clipboard contains data which should be cutted after pasting.
    bool m_clipboardContainsCutData;

    /**
     * Asynchronous operations like 'Move' and 'Copy' may only be added as undo
     * operation after they have been finished successfully. When an asynchronous
     * operation is started, it is added to a pending undo jobs list in the meantime.
     * As soon as the job has been finished, the operation is added to the undo mangager.
     * @see UndoManager
     * @see Dolphin::addPendingUndoJob
     * @see Dolphin::addUndoOperation
     */
    struct UndoInfo
    {
        int id;
        DolphinCommand command;
    };
    QValueList<UndoInfo> m_pendingUndoJobs;

    /** Contains meta information for creating files. */
    struct CreateFileEntry
    {
        QString name;
        QString filePath;
        QString templatePath;
        QString icon;
        QString comment;
    };

    QPtrList<KAction> m_fileGroupActions;
    KSortableValueList<CreateFileEntry,QString> m_createFileTemplates;

    // TODO: not used yet. See documentation of Dolphin::linkGroupActions()
    // and Dolphin::linkToDeviceActions() in for details.
    //QPtrList<KAction> m_linkGroupActions;
    //QPtrList<KAction> m_linkToDeviceActions;
};

#endif // _DOLPHIN_H_

