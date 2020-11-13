/***************************************************************************
                                komparesaveoptionswidget.h
                                --------------------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#ifndef KOMPARESAVEOPTIONSWIDGET_H
#define KOMPARESAVEOPTIONSWIDGET_H

#include <libkomparediff2/kompare.h>

#include "komparesaveoptionsbase.h"

class DiffSettings;
class QButtonGroup;

class KompareSaveOptionsWidget : public KompareSaveOptionsBase
{
    Q_OBJECT
public:
    KompareSaveOptionsWidget(const QString& source, const QString& destination,
                             DiffSettings* settings, QWidget* parent);
    ~KompareSaveOptionsWidget() override;

    void      saveOptions();
    QString   directory() const;

protected Q_SLOTS:
    void      updateCommandLine();

private:
    void      loadOptions();

    DiffSettings*   m_settings;
    QString         m_source;
    QString         m_destination;
    QButtonGroup*   m_FormatBG;
};

#endif
