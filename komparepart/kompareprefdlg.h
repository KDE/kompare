/***************************************************************************
                                kompareprefdlg.h  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
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

#ifndef KOMPAREPREFDLG_H
#define KOMPAREPREFDLG_H

#include <kpagedialog.h>

class DiffPage;
class DiffSettings;
class ViewPage;
class ViewSettings;

class KomparePrefDlg : public KPageDialog
{
Q_OBJECT
public:
	KomparePrefDlg( ViewSettings*, DiffSettings* );
	~KomparePrefDlg();

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

private:
	ViewPage* m_viewPage;
	DiffPage* m_diffPage;
};

#endif
