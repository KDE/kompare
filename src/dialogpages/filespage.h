/*
    SPDX-FileCopyrightText: 2001-2004 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FILESPAGE_H
#define FILESPAGE_H

#include <QFrame>

#include "dialogpages_export.h"

class QGroupBox;

class KComboBox;
class KUrlComboBox;
class KUrlRequester;
class QFormLayout;
class QLabel;

class FilesSettings;

class DIALOGPAGES_EXPORT FilesPage : public QFrame
{
    Q_OBJECT
public:
    FilesPage();
    ~FilesPage() override;

public:
    KUrlRequester* sourceURLRequester() const;
    KUrlRequester* destinationURLRequester() const;

    QString encoding() const;

    void setSourceTitle(const QString& title);
    void setDestinationTitle(const QString& title);

    void setURLsInComboBoxes();

    void setSourceURLRequesterMode(unsigned int mode);
    void setDestinationURLRequesterMode(unsigned int mode);

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
    QFormLayout *  m_formLayout;
    KUrlComboBox*  m_sourceURLComboBox;
    KUrlComboBox*  m_destinationURLComboBox;
    KUrlRequester* m_sourceURLRequester;
    KUrlRequester* m_destinationURLRequester;
    QLabel*        m_sourceLabel;
    QLabel*        m_destinationLabel;
    KComboBox*     m_encodingComboBox;

    FilesSettings* m_settings;
};

#endif
