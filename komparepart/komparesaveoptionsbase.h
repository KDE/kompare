/***************************************************************************
                                komparesaveoptionsbase.h  -  description
                                -------------------
        Converted from komparesaveoptionsbase.ui using uic3, Tue Dec 4 2007
        copyright               : (C) 2001-2003 by Otto Bruggeman
                                  and John Firebaugh
                                  (C) 2007      Kevin Kofler
        email                   : otto.bruggeman@home.nl
                                  jfirebaugh@kde.org
                                  kevin.kofler@chello.at
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#ifndef KOMPARESAVEOPTIONSBASE_H
#define KOMPARESAVEOPTIONSBASE_H

#include <qvariant.h>

#include "ui_komparesaveoptionsbase.h"

class KompareSaveOptionsBase : public QWidget, public Ui::KompareSaveOptionsBase
{
    Q_OBJECT

public:
    KompareSaveOptionsBase(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = 0);
    ~KompareSaveOptionsBase();

protected slots:
    virtual void languageChange();

};

#endif // KOMPARESAVEOPTIONSBASE_H
