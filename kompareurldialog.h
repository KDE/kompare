/***************************************************************************
                                kcompareurldialog.h  -  description
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

#ifndef KOMPAREURLDIALOG_H
#define KOMPAREURLDIALOG_H

#include <kdialogbase.h>
#include <kurl.h>

class QGroupBox;

class KFileDialog;
class KURLComboBox;
class KURLRequester;
class KConfig;

class KompareURLDialog : public KDialogBase
{
	Q_OBJECT

public:
	KompareURLDialog( const KURL* firstURL = 0, const KURL* secondURL = 0, QWidget* parent = 0, const char* name = 0 );
	~KompareURLDialog();

	KURL getFirstURL() const;
	KURL getSecondURL() const;

	void setFirstGroupBoxTitle ( const QString& title );
	void setSecondGroupBoxTitle( const QString& title );

	void setGroup( const QString& groupName );

	void setFirstURLRequesterMode ( unsigned int mode );
	void setSecondURLRequesterMode( unsigned int mode );

protected slots:
	virtual void slotOk();

private slots:
	void slotEnableOk();

private:
	QGroupBox*         m_firstGB;
	QGroupBox*         m_secondGB;
	KURLComboBox*      m_firstURLComboBox;
	KURLComboBox*      m_secondURLComboBox;
	KURLRequester*     m_firstURLRequester;
	KURLRequester*     m_secondURLRequester;
	KConfig*           m_config;
	QString            m_configGroupName;
//	QString            m_configFirstKeyName;
//	QString            m_configSecondKeyName;
};

#endif
