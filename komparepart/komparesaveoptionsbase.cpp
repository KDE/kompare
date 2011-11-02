/***************************************************************************
                                komparesaveoptionsbase.cpp
                                --------------------------
        Converted from komparesaveoptionsbase.ui using uic3, Tue Dec 4 2007
        Copyright 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
        Copyright 2007-2011 Kevin Kofler   <kevin.kofler@chello.at>
****************************************************************************/


/***************************************************************************
**   This program is free software; you can redistribute it and/or
**   modify it under the terms of the GNU General Public
**   License as published by the Free Software Foundation; either
**   version 2 of the License, or (at your option) any later version.
***************************************************************************/

#include "komparesaveoptionsbase.h"

#include <QtCore/QVariant>
/*
 *  Constructs a KompareSaveOptionsBase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
KompareSaveOptionsBase::KompareSaveOptionsBase(QWidget* parent, Qt::WindowFlags fl)
    : QWidget(parent, fl)
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

