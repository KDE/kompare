/***************************************************************************
                                kompareprefdlg.h  -  description
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

#ifndef KOMPAREPREFDLG_H
#define KOMPAREPREFDLG_H

#include <kdialogbase.h>

class DiffPrefs;
class DiffSettings;
class ViewPrefs;
class ViewSettings;

class KomparePrefDlg : public KDialogBase
{
Q_OBJECT
public:
	KomparePrefDlg( ViewSettings*, DiffSettings* );
	~KomparePrefDlg();

public:
	ViewPrefs* m_viewPage;
	DiffPrefs* m_diffPage;

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
