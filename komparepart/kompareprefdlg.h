/***************************************************************************
                                kdiffprefdlg.h  -  description
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

#ifndef KDIFFPREFDLG_H
#define KDIFFPREFDLG_H

#include <kdialogbase.h>

#include "diffprefs.h"
#include "generalprefs.h"
#include "miscprefs.h"

class KDiffPrefDlg : public KDialogBase
{
Q_OBJECT
public:
	KDiffPrefDlg( GeneralSettings*, DiffSettings*, MiscSettings* );
	~KDiffPrefDlg();

public:
	GeneralPrefs*	m_generalPage;
	DiffPrefs*	m_diffPage;
	MiscPrefs*	m_miscPage;

protected slots:
	/** No descriptions */
	virtual void slotOk();
	/** No descriptions */
	virtual void slotApply();
	/** No descriptions */
	virtual void slotHelp();
	/** No descriptions */
	virtual void slotDefault();
	/** No descriptions */
	virtual void slotCancel();
};

#endif
