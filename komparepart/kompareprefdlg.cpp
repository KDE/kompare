/***************************************************************************
                                kompareprefdlg.cpp  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2003 by Otto Bruggeman
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

#include <qvbox.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>

#include "kompareprefdlg.h"

// implementation

KomparePrefDlg::KomparePrefDlg( ViewSettings* viewSets, DiffSettings* diffSets ) : KDialogBase( IconList, i18n( "Preferences" ), Help|Default|Ok|Apply|Cancel, Ok, 0, 0, true, true )
{
	// ok i need some stuff in that pref dlg...
	setIconListAllVisible(true);

	QVBox* frame;
	frame = addVBoxPage( i18n( "View" ), i18n( "View Settings" ), UserIcon( "diff_view" ) );
	m_viewPage = new ViewPrefs( frame );
	m_viewPage->setSettings( viewSets );

	frame = addVBoxPage( i18n( "Diff" ), i18n( "Diff Settings" ), UserIcon( "diff_specific" ) );
	m_diffPage = new DiffPrefs( frame );
	m_diffPage->setSettings( diffSets );

//	frame = addVBoxPage( i18n( "" ), i18n( "" ), UserIcon( "" ) );

	adjustSize();
}

KomparePrefDlg::~KomparePrefDlg()
{

}

/** No descriptions */
void KomparePrefDlg::slotDefault()
{
	kdDebug(8103) << "SlotDefault called -> Settings should be restored to defaults..." << endl;
	// restore all defaults in the options...
	m_viewPage->setDefaults();
	m_diffPage->setDefaults();
}

/** No descriptions */
void KomparePrefDlg::slotHelp()
{
	// show some help...
	// figure out the current active page
	// and give help for that page
}

/** No descriptions */
void KomparePrefDlg::slotApply()
{
	kdDebug(8103) << "SlotApply called -> Settings should be applied..." << endl;
	// well apply the settings that are currently selected
	m_viewPage->apply();
	m_diffPage->apply();
}

/** No descriptions */
void KomparePrefDlg::slotOk()
{
	kdDebug(8103) << "SlotOk called -> Settings should be applied..." << endl;
	// Apply the settings that are currently selected
	m_viewPage->apply();
	m_diffPage->apply();

	KDialogBase::slotOk();
}

/** No descriptions */
void KomparePrefDlg::slotCancel()
{
	// discard the current settings and use the present ones
	m_viewPage->restore();
	m_diffPage->restore();

	KDialogBase::slotCancel();
}

#include "kompareprefdlg.moc"
