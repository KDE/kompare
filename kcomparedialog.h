/***************************************************************************
                                kcomparedialog.h  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2002 by Otto Bruggeman
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

#ifndef KCOMPAREDIALOG_H
#define KCOMPAREDIALOG_H

#include <kdialogbase.h>
#include <kurl.h>

class KFileDialog;
class KURLComboBox;
class KURLRequester;

class KCompareDialog : public KDialogBase
{
	Q_OBJECT

public:
	KCompareDialog( const KURL* sourceURL = 0, const KURL* destURL = 0, QWidget* parent = 0, const char* name = 0 );
	~KCompareDialog();

	KURL getSourceURL() const;
	KURL getDestinationURL() const;

protected slots:
	virtual void slotOk();

private slots:
	void slotEnableCompare();

private:
	KURLComboBox*      m_sourceURLComboBox;
	KURLComboBox*      m_destinationURLComboBox;
	KURLRequester*     m_sourceURLRequester;
	KURLRequester*     m_destinationURLRequester;
};

#endif
