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

class Q3GroupBox;

class QComboBox;
class KComboBox;
class KConfig;
class KFileDialog;
class KUrlComboBox;
class KUrlRequester;

class FilesSettings;

class FilesPage : public PageBase
{
Q_OBJECT
public:
	FilesPage( QWidget* parent );
	virtual ~FilesPage();

public:
	KUrlRequester* firstURLRequester() const;
	KUrlRequester* secondURLRequester() const;

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
	Q3GroupBox*     m_firstGB;
	Q3GroupBox*     m_secondGB;
	Q3GroupBox*     m_thirdGB;
	KUrlComboBox*  m_firstURLComboBox;
	KUrlComboBox*  m_secondURLComboBox;
	KUrlRequester* m_firstURLRequester;
	KUrlRequester* m_secondURLRequester;
	// Use this bool to lock the connection between both KUrlRequesters.
	// This prevents annoying behaviour
	bool           m_URLChanged;
	QComboBox*     m_encodingComboBox;

	FilesSettings* m_settings;
};

#endif
