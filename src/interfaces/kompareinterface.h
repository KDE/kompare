/*
    SPDX-FileCopyrightText: 2002-2003 Otto Bruggeman <otto.bruggeman@home.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef _KOMPARE_INTERFACE_H
#define _KOMPARE_INTERFACE_H

#include <QString>
#include <QStringList>
#include <QtPlugin>

#include "kompareinterface_export.h"

class KConfig;
class QUrl;

class KompareInterfacePrivate;

class KOMPAREINTERFACE_EXPORT KompareInterface
{
public:
    KompareInterface();
    virtual ~KompareInterface();

protected:
    KompareInterface(const KompareInterface&);
    KompareInterface& operator=(const KompareInterface&);

public:
    /**
     * Open and parse the diff file at url.
     */
    virtual bool openDiff(const QUrl& diffUrl) = 0;

    /**
     * Open and parse the supplied diff output
     */
    virtual bool openDiff(const QString& diffOutput) = 0;

    /**
     * Open and parse the diff3 file at url.
     */
    virtual bool openDiff3(const QUrl& diff3Url) = 0;

    /**
     * Open and parse the supplied diff3 output
     */
    virtual bool openDiff3(const QString& diff3Output) = 0;

    /**
     * Compare, with diff, source with destination, can also be used if you do not
     * know what source and destination are. The part will try to figure out what
     * they are (directory, file, diff output file) and call the
     * appropriate method(s)
     */
    virtual void compare(const QUrl& sourceFile, const QUrl& destinationFile) = 0;

    /**
     * Compare a Source file to a custom Destination string
     */
    virtual void compareFileString(const QUrl& sourceFile, const QString& destination) = 0;

    /**
     * Compare a custom Source string to a Destination file
     */
    virtual void compareStringFile(const QString& source, const QUrl& destinationFile) = 0;

    /**
     * Compare, with diff, source with destination files
     */
    virtual void compareFiles(const QUrl& sourceFile, const QUrl& destinationFile) = 0;

    /**
     * Compare, with diff, source with destination directories
     */
    virtual void compareDirs(const QUrl& sourceDir, const QUrl& destinationDir) = 0;

    /**
     * Compare, with diff3, originalFile with changedFile1 and changedFile2
     */
    virtual void compare3Files(const QUrl& originalFile, const QUrl& changedFile1, const QUrl& changedFile2) = 0;

    /**
     * This will show the file and the file with the diff applied
     */
    virtual void openFileAndDiff(const QUrl& file, const QUrl& diffFile) = 0;

    /**
     * This will show the directory and the directory with the diff applied
     */
    virtual void openDirAndDiff(const QUrl& dir,  const QUrl& diffFile) = 0;

    /**
     * This will set the encoding to use for all files that are read or for the diffoutput
     */
    virtual void setEncoding(const QString& encoding);

public:
    /**
     * Warning this should be in class Part in KDE 4.0, not here !
     * Around that time the methods will disappear here
     */
    virtual int readProperties(KConfig* config) = 0;
    virtual int saveProperties(KConfig* config) = 0;

    /**
     * Warning this should be in class ReadWritePart in KDE 4.0, not here !
     * Around that time the method will disappear here
     */
    virtual bool queryClose() = 0;

protected:
    // Add all variables to the KompareInterfacePrivate class and access them through the kip pointer
    KompareInterfacePrivate* kip;
    QString                  m_encoding;
};

Q_DECLARE_INTERFACE(KompareInterface, "com.kde.Kompare.KompareInterface/4.0")

#endif /* _KOMPARE_INTERFACE_H */
