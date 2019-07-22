/***************************************************************************
                                kcompareurldialog.h
                                -------------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2004 Otto Bruggeman <otto.bruggeman@home.nl>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
        Copyright 2007      Kevin Kofler   <kevin.kofler@chello.at>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#ifndef FILESPAGE_H
#define FILESPAGE_H

#include <QFrame>

#include "dialogpages_export.h"

class QGroupBox;

class KComboBox;
class KUrlComboBox;
class KUrlRequester;

class FilesSettings;

class DIALOGPAGES_EXPORT FilesPage : public QFrame
{
    Q_OBJECT
public:
    FilesPage();
    ~FilesPage() override;

public:
    KUrlRequester* firstURLRequester() const;
    KUrlRequester* secondURLRequester() const;

    QString encoding() const;

    void setFirstGroupBoxTitle(const QString& title);
    void setSecondGroupBoxTitle(const QString& title);

    void setURLsInComboBoxes();

    void setFirstURLRequesterMode(unsigned int mode);
    void setSecondURLRequesterMode(unsigned int mode);

public:
    virtual void setSettings(FilesSettings* settings);
    virtual void restore();
    virtual void apply();
    virtual void setDefaults();

Q_SIGNALS:
    // Signal when we set a url from browse buttons.
    // Since KUrlComboBox::setUrl doesn't emit textChanged we need to notify
    // the main dialog that we set a url programatically so it can enable OK
    // if there's a path in both boxes...
    void urlChanged();

private Q_SLOTS:
    void open();

private:
    void doOpen(KUrlComboBox* urlComboBox, bool selectFolders);

private:
    QGroupBox*     m_firstGB;
    QGroupBox*     m_secondGB;
    QGroupBox*     m_thirdGB;
    KUrlComboBox*  m_firstURLComboBox;
    KUrlComboBox*  m_secondURLComboBox;
    KUrlRequester* m_firstURLRequester;
    KUrlRequester* m_secondURLRequester;
    // Use this bool to lock the connection between both KUrlRequesters.
    // This prevents annoying behaviour
    bool           m_URLChanged;
    KComboBox*     m_encodingComboBox;

    FilesSettings* m_settings;
};

#endif
