/***************************************************************************
                                kompareprefdlg.h
                                ----------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2009 Otto Bruggeman <bruggie@gmail.com>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
        Copyright 2007      Kevin Kofler   <kevin.kofler@chello.at>
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

signals:
	void configChanged();

private:
	ViewPage* m_viewPage;
	DiffPage* m_diffPage;
};

#endif
