/***************************************************************************
                          komparemodellist.h  -  description
                             -------------------
    begin                : Tue Jun 26 2001
    copyright            : (C) 2001 by John Firebaugh
    email                : jfirebaugh@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KOMPAREMODELLIST_H
#define KOMPAREMODELLIST_H

#include <qptrlist.h>
#include <qobject.h>

#include <kurl.h>

#include "diffmodel.h"
#include "kompare.h"

class QFile;

class KTempFile;

class DiffSettings;
class KompareProcess;

class KompareModelList : public QObject, Kompare
{
	Q_OBJECT

public:
	KompareModelList();
	~KompareModelList();

	bool compare( const KURL& source, const KURL& destination );
	bool saveDestination( int index );

	bool openDiff( const KURL& url );
	bool saveDiff( const KURL& url, QString directory, DiffSettings* settings );
	bool saveAll();

	void swap();

	enum Kompare::Mode          mode()
	    { return m_mode; };
	enum Kompare::Format        format()
	    { return m_format; };
	const QPtrList<DiffModel>&  models()
	    { return m_models; };
	int                         modelCount()
	    { return m_models.count(); };
	DiffModel*                  modelAt( int i )
	    { return m_models.at( i ); };
	int                         findModel( DiffModel* model )
	    { return m_models.find( model ); };

	QString                     sourceTemp() const      { return m_sourceTemp; };
	QString                     destinationTemp() const { return m_destinationTemp; };

	KURL                        diffURL() const         { return m_diffURL; };
	KURL                        sourceBaseURL() const;
	KURL                        destinationBaseURL() const;

	bool                        isModified() const;

signals:
	void status( Kompare::Status );
	void error( QString error );
	void modelsChanged();

protected slots:
	void slotDiffProcessFinished( bool success );
	void slotWriteDiffOutput( bool success );

private:
	int parseDiffOutput    ( const QStringList& lines );
	int determineDiffFormat( const QStringList& lines );
	int determineType      ( const QStringList& lines, enum Kompare::Format format );
	QPtrList<DiffModel> splitFiles( const QStringList& lines, bool split );
	void clear();

private:
	KompareProcess*      m_diffProcess;
	QPtrList<DiffModel>  m_models;

	KURL                 m_sourceURL;
	KURL                 m_destinationURL;
	QString              m_sourceTemp;
	QString              m_destinationTemp;

	KURL                 m_diffURL;
	KTempFile*           m_diffTemp;

	enum Kompare::Format m_format;
	enum Kompare::Mode   m_mode; // reading from a DiffFile or Comparing in Kompare
	enum Kompare::Type   m_type; // single, multi and diff or cvsdiff
};

#endif
