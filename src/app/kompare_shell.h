/*
    SPDX-FileCopyrightText: 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007-2011 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPARESHELL_H
#define KOMPARESHELL_H

#include <KParts/MainWindow>

#include <libkomparediff2/kompare.h>

#include <kompareshelldebug.h>

class KompareInterface;
namespace KParts {
class ReadOnlyPart;
class ReadWritePart;
}
class KToggleAction;

class KSqueezedTextLabel;
class KomparePart;
class KompareNavTreePart;
class QLabel;
class QEventLoopLocker;

namespace KTextEditor {
class Document;
class EditInterface;
class View;
}

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * Adapted the shell a bit so it now handles separate view and navigation parts
 *
 * @short Application Shell
 * @author John Firebaugh <jfirebaugh@kde.org>
 * @author Otto Bruggeman <bruggie@home.nl>
 * @version 3.2.90
 */
class KompareShell : public KParts::MainWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    KompareShell();

    /**
     * Default Destructor
     */
    ~KompareShell() override;

    /**
     * Use this method to load whatever file/URL you have
     */
    void openDiff(const QUrl& url);

    /**
     * Use this method to load the diff from stdin
     */
    void openStdin();

    /**
     * Use this method to compare 2 URLs (files or directories)
     */
    void compare(const QUrl& source, const QUrl& destination);

    /**
     * Use this method to blend diff into url1 (file or directory)
     */
    void blend(const QUrl& url1, const QUrl& diff);

public Q_SLOTS:
    void slotUpdateStatusBar(int modelIndex, int differenceIndex, int modelCount, int differenceCount, int appliedCount);

    KompareInterface* viewPart() const;

protected:
    bool queryClose() override;

    /**
     * This method is called when it is time for the app to save its
     * properties for session management purposes.
     */
    void saveProperties(KConfigGroup&) override;

    /**
     * This method is called when this app is restored.  The KConfig
     * object points to the session management config file that was saved
     * with @ref saveProperties
     */
    void readProperties(const KConfigGroup&) override;

private Q_SLOTS:
    void slotSetStatusBarText(const QString& text) override;
    void slotFileOpen();
    void slotFileCompareFiles();
    void slotFileBlendURLAndDiff();
    void slotShowTextView();
    void slotFileClose();
    void optionsConfigureKeys();
    void optionsConfigureToolbars();
    void slotSetDiffString(const QString& diffString);
    void newToolbarConfig();
    void slotVisibilityChanged(bool visible);

private:
    void setupAccel();
    void setupActions();
    void setupStatusBar();

private:
    QUrl                        m_sourceURL;
    QUrl                        m_destinationURL;
    QUrl                        m_diffURL;

    KParts::ReadWritePart*      m_viewPart;
    KParts::ReadOnlyPart*         m_navTreePart;
    KTextEditor::Document*      m_textViewPart;
    KTextEditor::View*          m_textView;
//     KTextEditor::EditInterface* m_textEditIface;

    QDockWidget*                m_textViewWidget;
    QDockWidget*                m_navTreeDock;

    KToggleAction*              m_showTextView;

    enum Kompare::Mode          m_mode;
    // This is the statusbarwidget for displaying the general stuff
    KSqueezedTextLabel*         m_generalLabel;

    QString                     m_diffString;
    QLabel*                     m_filesLabel;
    QLabel*                     m_differencesLabel;
    QEventLoopLocker*           m_eventLoopLocker;
};

#endif // KOMPARE_H
