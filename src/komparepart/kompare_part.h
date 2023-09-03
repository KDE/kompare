/*
    SPDX-FileCopyrightText: 2001-2005, 2009 Otto Bruggeman <bruggie@gmail.com>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2004 Jeff Snyder <jeff@caffeinated.me.uk>
    SPDX-FileCopyrightText: 2007-2011 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREPART_H
#define KOMPAREPART_H

#include <komparepartdebug.h>
#include "kompareinterface.h"
// Komparediff2
#include <KompareDiff2/Info>
// KF
#include <KParts/ReadWritePart>
// Qt
#include <QVariantList>

class QAction;
class QPrinter;
class QUrl;
class QWidget;

class KPluginMetaData;

namespace KompareDiff2 {
class Difference;
class DiffModel;
class DiffModelList;
class ModelList;
}
class DiffSettings;
class ViewSettings;
class KompareSplitter;
class KompareView;

/**
 * This is a "Part".  It does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author John Firebaugh <jfirebaugh@kde.org>
 * @author Otto Bruggeman <bruggie@home.nl>
 * @version 0.3
 */
class KomparePart : public KParts::ReadWritePart,
                    public KompareInterface
{
    Q_OBJECT
    Q_INTERFACES(KompareInterface)
public:
    enum Modus {
        ReadOnlyModus = 0,
        ReadWriteModus = 1
    };

    /**
    * Default constructor
    */
    KomparePart(QWidget* parentWidget, QObject* parent, const KPluginMetaData& metaData, Modus modus);

    /**
    * Destructor
    */
    ~KomparePart() override;

    // Sessionmanagement stuff, added to the kompare iface
    // because they are not in the Part class where they belong
    // Should be added when bic changes are allowed again (kde 4.0)
    int readProperties(KConfig* config) override;
    int saveProperties(KConfig* config) override;
    // this one is called when the shell_app is about to close.
    // we need it now to save the properties of the part when apps don't (can't)
    // use the readProperties and saveProperties methods
    bool queryClose() override;
    void setReadWrite(bool readWrite) override;

    // Do we really want to expose this ???
    const KompareDiff2::ModelList* model() const { return m_modelList; };

public:
    // Reimplemented from the KompareInterface
    /**
     * Open and parse the diff file at diffUrl.
     */
    bool openDiff(const QUrl& diffUrl) override;

    /** Added on request of Harald Fernengel */
    bool openDiff(const QString& diffOutput) override;

    /** Open and parse the diff3 file at diff3Url */
    bool openDiff3(const QUrl& diff3URL) override;

    /** Open and parse the file diff3Output with the output of diff3 */
    bool openDiff3(const QString& diff3Output) override;

    /** Compare, with diff, source with destination */
    void compare(const QUrl& sourceFile, const QUrl& destinationFile) override;

    /** Compare a Source file to a custom Destination string */
    void compareFileString(const QUrl& sourceFile, const QString& destination) override;

    /** Compare a custom Source string to a Destination file */
    void compareStringFile(const QString& source, const QUrl& destinationFile) override;

    /** Compare, with diff, source with destination */
    void compareFiles(const QUrl& sourceFile, const QUrl& destinationFile) override;

    /** Compare, with diff, source with destination */
    void compareDirs(const QUrl& sourceDir, const QUrl& destinationDir) override;

    /** Compare, with diff3, originalFile with changedFile1 and changedFile2 */
    void compare3Files(const QUrl& originalFile, const QUrl& changedFile1, const QUrl& changedFile2) override;

    /** This will show the file and the file with the diff applied */
    void openFileAndDiff(const QUrl& file, const QUrl& diffFile) override;

    /** This will show the directory and the directory with the diff applied */
    void openDirAndDiff(const QUrl& dir,  const QUrl& diffFile) override;

    /** Reimplementing this because this one knows more about the real part then the interface */
    void setEncoding(const QString& encoding) override;

    // This is the interpart interface, it is signal and slot based so no "real" interface here
    // All you have to do is connect the parts from your application.
    // These just point to their counterpart in the ModelList or get called from their
    // counterpart in ModelList.
Q_SIGNALS:
    void modelsChanged(const KompareDiff2::DiffModelList* models);

    void setSelection(const KompareDiff2::DiffModel* model, const KompareDiff2::Difference* diff);
    void setSelection(const KompareDiff2::Difference* diff);

    void selectionChanged(const KompareDiff2::DiffModel* model, const KompareDiff2::Difference* diff);
    void selectionChanged(const KompareDiff2::Difference* diff);

    void applyDifference(bool apply);
    void applyAllDifferences(bool apply);
    void applyDifference(const KompareDiff2::Difference*, bool apply);

    void configChanged();

    /*
    ** This is emitted when a difference is clicked in the kompare view. You can connect to
    ** it so you can use it to jump to this particular line in the editor in your app.
    */
    void differenceClicked(int lineNumber);

    // Stuff that can probably be removed by putting it in the part where it belongs in my opinion
public Q_SLOTS:
    /** Save all destinations. */
    bool saveAll();

    /** Save the results of a comparison as a diff file. */
    void saveDiff();

    /** To enable printing, the part has the only interesting printable content so putting it here */
    void slotFilePrint();
    void slotFilePrintPreview();

Q_SIGNALS:
    void appliedChanged();
    void diffURLChanged();
    void kompareInfo(KompareDiff2::Info* info);
    void setStatusBarModelInfo(int modelIndex, int differenceIndex, int modelCount, int differenceCount, int appliedCount);
//     void setStatusBarText( const QString& text );
    void diffString(const QString&);

protected:
    /**
     * This is the method that gets called when the file is opened,
     * when using openURL( const QUrl& ) or in our case also openDiff( const QUrl& );
     * return true when everything went ok, false if there were problems
     */
    bool openFile() override;
    // ... Uhm we return true without saving ???
    bool saveFile() override { return true; };

protected Q_SLOTS:
    void slotSetStatus(KompareDiff2::Status status);
    void slotShowError(const QString& error);

    void slotSwap();
    void slotShowDiffstats();
    void slotRefreshDiff();
    void optionsPreferences();

    void updateActions();
    void updateCaption();
    void updateStatus();
    void compareAndUpdateAll();

    void slotPaintRequested(QPrinter*);

private:
    void cleanUpTemporaryFiles();
    void setupActions(Modus modus);
    bool isDirectory(const QUrl& url);
    // FIXME (like in cpp file not urgent) Replace with enum, cant find a proper
    // name now but it is private anyway so can not be used from outside
    bool fetchURL(const QUrl& url, bool isSource);

private Q_SLOTS:
    void onContextMenuRequested(const QPoint& pos);

private:
    // Uhm why were these static again ???
    // Ah yes, so multiple instances of kompare use the
    // same settings after one of them changes them
    static ViewSettings* m_viewSettings;
    static DiffSettings* m_diffSettings;

    KompareDiff2::ModelList* m_modelList;

    KompareView*             m_view;
    KompareSplitter*         m_splitter;

    QAction*                 m_saveAll;
    QAction*                 m_saveDiff;
    QAction*                 m_swap;
    QAction*                 m_diffStats;
    QAction*                 m_diffRefresh;
    QAction*                 m_print;
    QAction*                 m_printPreview;

    KompareDiff2::Info       m_info;
};

#endif // KOMPAREPART_H
