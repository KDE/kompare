/***************************************************************************
                                komparesaveoptionsbase.h
                                ------------------------
        Converted from komparesaveoptionsbase.ui using uic3, Tue Dec 4 2007
        Copyright 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
        Copyright 2007      Kevin Kofler   <kevin.kofler@chello.at>
****************************************************************************/

/***************************************************************************
**   This program is free software; you can redistribute it and/or
**   modify it under the terms of the GNU General Public
**   License as published by the Free Software Foundation; either
**   version 2 of the License, or (at your option) any later version.
***************************************************************************/

#ifndef KOMPARESAVEOPTIONSBASE_H
#define KOMPARESAVEOPTIONSBASE_H

#include <QtCore/QVariant>

#include "ui_komparesaveoptionsbase.h"

class KompareSaveOptionsBase : public QWidget, public Ui::KompareSaveOptionsBase
{
    Q_OBJECT

public:
    explicit KompareSaveOptionsBase(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = 0);
    ~KompareSaveOptionsBase();

protected slots:
    virtual void languageChange();

};

#endif // KOMPARESAVEOPTIONSBASE_H
