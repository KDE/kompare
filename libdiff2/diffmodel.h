/***************************************************************************
                                diffhunk.h  -  description
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

#ifndef DIFFMODEL_H
#define DIFFMODEL_H

#include <qobject.h>
#include <qptrlist.h>
#include <qstringlist.h>

#include "kompare.h"

class DiffHunk;
class Difference;
class KompareModelList;

class DiffModel : public QObject, Kompare
{
Q_OBJECT
public:

	DiffModel( KURL srcBaseURL = "", KURL destBaseURL = "" );
	~DiffModel();

	int parseDiff( enum Kompare::Format format, const QStringList& list );

	int hunkCount() const
		{ return m_hunks.count(); };
	int differenceCount() const
		{ return m_differences.count(); };
	int appliedCount() const
		{ return m_appliedCount; };
	DiffHunk* hunkAt( int i )
		{ return m_hunks.at( i ); };
	Difference* differenceAt( int i )
		{ return m_differences.at( i ); };
	const QPtrList<DiffHunk>& hunks() const
		{ return m_hunks; };
	const QPtrList<Difference>& differences() const
		{ return m_differences; };
	int findDifference( const Difference* diff ) const
	    { return const_cast<DiffModel*>(this)->m_differences.find( diff ); };

	const KURL&    srcBaseURL()                { return m_srcBaseURL; };
	const KURL&    destBaseURL()               { return m_destBaseURL; };
	const QString  srcFile() const;
	const QString  destFile() const;
	const QString  srcPath() const;
	const QString  destPath() const;
	const QString& sourceTimestamp()           { return m_sourceTimestamp; };
	const QString& destinationTimestamp()      { return m_destinationTimestamp; };
	const QString& sourceRevision() const      { return m_sourceRevision; };
	const QString& destinationRevision() const { return m_destinationRevision; };

//	Format getFormat() { return m_format; };
	bool isModified() const { return m_modified; };

	const int index( void ) const   { return m_index; };
	void      setIndex( int index ) { m_index = index; };

	void applyDifference( bool apply );
	void applyAllDifferences( bool apply );
	
	void setSelectedDifference( Difference* diff );

public slots:
	void setModified( bool modified );

private:
	int parseContextDiff( const QStringList& list );
	int parseEdDiff     ( const QStringList& list );
	int parseNormalDiff ( const QStringList& list );
	int parseRCSDiff    ( const QStringList& list );
	int parseUnifiedDiff( const QStringList& list );

	KURL                 m_srcBaseURL;
	KURL                 m_destBaseURL;
	QString              m_sourceFile;
	QString              m_sourceTimestamp;
	QString              m_sourceRevision;
	QString              m_destinationFile;
	QString              m_destinationTimestamp;
	QString              m_destinationRevision;
	QPtrList<DiffHunk>   m_hunks;
	QPtrList<Difference> m_differences;
	int                  m_appliedCount;
	bool                 m_modified;
	
	int                  m_index;
	Difference*          m_selectedDifference;
};

#endif

