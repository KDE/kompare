/*
    SPDX-FileCopyrightText: 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007-2011 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "komparesaveoptionsbase.h"

#include <QVariant>
/*
 *  Constructs a KompareSaveOptionsBase as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
KompareSaveOptionsBase::KompareSaveOptionsBase(QWidget* parent)
    : QWidget(parent)
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

