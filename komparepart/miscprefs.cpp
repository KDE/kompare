/***************************************************************************
                                miscprefs.cpp  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001 by Otto Bruggeman
                                  and John Firebaugh
        email                   : otto.bruggeman@home.nl
                                  jfirebaugh@kde.org
****************************************************************************/
 
/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#include <qlayout.h>

#include <kdialog.h>
#include <klocale.h>

#include "miscprefs.h"

MiscPrefs::MiscPrefs( QWidget* parent ) : PrefsBase( parent )
{
	QWidget* page;
	QVBoxLayout* layout;
//	QGroupBox* blaGroupBox;
//	QLabel* label;

	page = new QWidget( this );
	layout = new QVBoxLayout( page );
	layout->setSpacing( KDialog::spacingHint() );
	layout->setMargin( KDialog::marginHint() );

	layout->addStretch( 1 );
	page->setMinimumSize( sizeHintForWidget( page ) );

	addTab( page, i18n( "&Misc" ) );
};

MiscPrefs::~MiscPrefs()
{

};

void MiscPrefs::setSettings( MiscSettings* setts )
{
	m_settings = setts;
};

MiscSettings* MiscPrefs::getSettings( void )
{
	return m_settings;
};

void MiscPrefs::restore()
{
};

void MiscPrefs::apply()
{
};

void MiscPrefs::setDefaults()
{
};

#include "miscprefs.moc"
