/***************************************************************************
                                komparesaveoptionsbase.cpp  -  description
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
**   This program is free software; you can redistribute it and/or
**   modify it under the terms of the GNU General Public
**   License as published by the Free Software Foundation; either
**   version 2 of the License, or (at your option) any later version.
***************************************************************************/

#include "komparesaveoptionsbase.h"

#include <qvariant.h>
/*
 *  Constructs a KompareSaveOptionsBase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
KompareSaveOptionsBase::KompareSaveOptionsBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : QWidget(parent, name, fl)
{
    setupUi(this);

}

/*
 *  Destroys the object and frees any allocated resources
 */
KompareSaveOptionsBase::~KompareSaveOptionsBase()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void KompareSaveOptionsBase::languageChange()
{
    retranslateUi(this);
}

