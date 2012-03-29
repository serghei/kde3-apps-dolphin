/***************************************************************************
*   Copyright (C) 2006 by Peter Penz   *
*   peter.penz@gmx.at   *
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

#ifndef URLNAVIGATOR_H
#define URLNAVIGATOR_H

#include <qhbox.h>
#include <kurl.h>
#include <qstring.h>

class DolphinView;
class QPopupMenu;
class QPushButton;
class QComboBox;
class BookmarkSelector;
class KURLComboBox;
class KFileItem;

/**
 * @brief Navigation bar which contains the current shown URL.
 *
 * The URL navigator offers two modes:
 * - Editable:     Represents the 'classic' mode, where the current URL
 *                 is editable inside a line editor.
 * - Non editable: The URL is represented by a number of buttons, where
 *                 clicking on a button results in activating the URL
 *                 the button represents. This mode also supports drag
 *                 and drop of items.
 *
 * The mode can be changed by a toggle button located on the left side of
 * the navigator.
 *
 * The URL navigator also remembers the URL history and allows to go
 * back and forward within this history.
 *
 * @author Peter Penz
*/
class URLNavigator : public QHBox
{
    Q_OBJECT

public:
    /**
     * @brief Represents the history element of an URL.
     *
     * A history element contains the URL, the name of the current file
     * (the 'current file' is the file where the cursor is located) and
     * the x- and y-position of the content.
     */
    class HistoryElem {
    public:
        HistoryElem();
        HistoryElem(const KURL& url);
        ~HistoryElem(); // non virtual

        const KURL& url() const { return m_url; }

        void setCurrentFileName(const QString& name) { m_currentFileName = name; }
        const QString& currentFileName() const { return m_currentFileName; }

        void setContentsX(int x) { m_contentsX = x; }
        int contentsX() const { return m_contentsX; }

        void setContentsY(int y) { m_contentsY = y; }
        int contentsY() const { return m_contentsY; }

    private:
        KURL m_url;
        QString m_currentFileName;
        int m_contentsX;
        int m_contentsY;
    };

    URLNavigator(const KURL& url, DolphinView* dolphinView);;
    virtual ~URLNavigator();

    /**
     * Sets the current active URL.
     * The signals URLNavigator::urlChanged and URLNavigator::historyChanged
     * are submitted.
     */
    void setURL(const KURL& url);

    /** Returns the current active URL. */
    const KURL& url() const;

    /** Returns the portion of the current active URL up to the button at index. */
    KURL url(int index) const;

    /**
     * Returns the complete URL history. The index 0 indicates the oldest
     * history element.
     * @param index     Output parameter which indicates the current
     *                  index of the location.
     */
    const QValueList<HistoryElem>& history(int& index) const;

    /**
     * Goes back one step in the URL history. The signals
     * URLNavigator::urlChanged and URLNavigator::historyChanged
     * are submitted.
     */
    void goBack();

    /**
     * Goes forward one step in the URL history. The signals
     * URLNavigator::urlChanged and URLNavigator::historyChanged
     * are submitted.
     */
    void goForward();

    /**
     * Goes up one step of the URL path. The signals
     * URLNavigator::urlChanged and URLNavigator::historyChanged
     * are submitted.
     */
    void goUp();

    /**
     * Goes to the home URL. The signals URLNavigator::urlChanged
     * and URLNavigator::historyChanged are submitted.
     */
    void goHome();

    /**
     * Allows to edit the URL of the navigation bar if \a editable
     * is true. If \a editable is false, each part of
     * the URL is presented by a button for a fast navigation.
     */
    void setURLEditable(bool editable);

    /**
     * @return True, if the URL is editable by the user within a line editor.
     *         If false is returned, each part of the URL is presented by a button
     *         for fast navigation.
     */
    bool isURLEditable() const;

    /**
     * Switches to the edit mode and assures that the keyboard focus
     * is assigned.
     */
    void editURL();

    DolphinView* dolphinView() const;

signals:
    void urlChanged(const KURL& url);
    void historyChanged();

protected:
    /** If the Escape key is pressed, the navigation bar should switch
        to the browse mode. */
    virtual void keyReleaseEvent(QKeyEvent* event);

private slots:
    void slotReturnPressed(const QString& text);
    void slotURLActivated(const KURL& url);

    void slotRequestActivation();
    void slotBookmarkActivated(int index);

    /**
     * Stores the coordinates of the moved content into
     * the current history element. Is usually triggered
     * by the signal 'contentsMoved' emitted by DolphinView.
     */
    void slotContentsMoved(int x, int y);

    /**
     * Switches the navigation bar between the editable and noneditable
     * state (see setURLEditable()) and is connected to the clicked signal
     * of the navigation bar button.
     */
    void slotClicked();

private:
    int m_historyIndex;
    DolphinView* m_dolphinView;
    QValueList<HistoryElem> m_history;
    QPushButton* m_toggleButton;
    BookmarkSelector* m_bookmarkSelector;
    KURLComboBox* m_pathBox;

    /**
     * Updates the history element with the current file item
     * and the contents position.
     */
    void updateHistoryElem();
    void updateContent();
};

#endif
