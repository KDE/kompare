/***************************************************************************
                                kcompareurldialog.h  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2004 Otto Bruggeman
                                  (C) 2001-2003 John Firebaugh
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

#ifndef FILESPAGE_H
#define FILESPAGE_H

#include "pagebase.h"

class QGroupBox;

class QComboBox;
class KComboBox;
class KConfig;
class KFileDialog;
class KURLComboBox;
class KURLRequester;

class FilesSettings;

class FilesPage : PageBase
{
Q_OBJECT
public:
	FilesPage( QWidget* parent );
	virtual ~FilesPage();

public:
	KURLRequester* firstURLRequester() const;
	KURLRequester* secondURLRequester() const;

	QString encoding() const;

	void setFirstGroupBoxTitle ( const QString& title );
	void setSecondGroupBoxTitle( const QString& title );

	void setURLsInComboBoxes();

	void setFirstURLRequesterMode( unsigned int mode );
	void setSecondURLRequesterMode( unsigned int mode );

public:
	virtual void setSettings( FilesSettings* settings );
	virtual void restore();
	virtual void apply();
	virtual void setDefaults();

protected slots:
	void setFirstURL( const QString & );
	void setSecondURL( const QString & );

private:
	QGroupBox*     m_firstGB;
	QGroupBox*     m_secondGB;
	QGroupBox*     m_thirdGB;
	KURLComboBox*  m_firstURLComboBox;
	KURLComboBox*  m_secondURLComboBox;
	KURLRequester* m_firstURLRequester;
	KURLRequester* m_secondURLRequester;
	// Use this bool to lock the connection between both KURLRequesters.
	// This prevents annoying behaviour
	bool           m_URLChanged;
	QComboBox*     m_encodingComboBox;

	FilesSettings* m_settings;
};

#endif
