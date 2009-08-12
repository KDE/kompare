/***************************************************************************
                                kcompareurldialog.h
                                -------------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2004 Otto Bruggeman <otto.bruggeman@home.nl>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
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

#include <kpagedialog.h>
#include <kurl.h>



class FilesPage;
class FilesSettings;
class DiffPage;
class DiffSettings;
class ViewPage;
class ViewSettings;

/**
 * Definition of class KompareURLDialog.
 * @author Otto Bruggeman
 * @author John Firebaugh
 */
class KompareURLDialog : public KPageDialog
{
	Q_OBJECT

public:
	explicit KompareURLDialog( QWidget *parent= 0, Qt::WFlags flags= 0 );
	~KompareURLDialog();

	KUrl getFirstURL() const;
	KUrl getSecondURL() const;
	QString encoding() const;

	void setFirstGroupBoxTitle ( const QString& title );
	void setSecondGroupBoxTitle( const QString& title );

	void setGroup( const QString& groupName );

	void setFirstURLRequesterMode ( unsigned int mode );
	void setSecondURLRequesterMode( unsigned int mode );

protected slots:
	virtual void slotButtonClicked( int button );

private slots:
	void slotEnableOk();
protected:
    void showEvent ( QShowEvent * event );
private:
	FilesPage*     m_filesPage;
	FilesSettings* m_filesSettings;
	DiffPage*      m_diffPage;
	DiffSettings*  m_diffSettings;
	ViewPage*      m_viewPage;
	ViewSettings*  m_viewSettings;
};

#endif
