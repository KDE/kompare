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

	DiffModel();
	~DiffModel();

	int parseDiff( enum Kompare::Format format, const QStringList& list );

	int hunkCount() const
		{ return m_hunks.count(); };
	int differenceCount() const
		{ return m_differences.count(); };
	int appliedCount() const
		{ return m_appliedCount; };
	DiffHunk* hunkAt( int i ) const
		{ return const_cast<DiffModel*>(this)->m_hunks.at( i ); };
	Difference* differenceAt( int i ) const
		{ return const_cast<DiffModel*>(this)->m_differences.at( i ); };
	const QPtrList<DiffHunk>& hunks() const
		{ return m_hunks; };
	const QPtrList<Difference>& differences() const
		{ return m_differences; };
	int findDifference( Difference* diff )
	    { return m_differences.find( diff ); };

	QString  sourceFile();
	QString  destinationFile();
	QString srcPath();
	QString destPath();
	QString  sourceTimestamp()       { return m_sourceTimestamp; };
	QString  destinationTimestamp()  { return m_destinationTimestamp; };
	QString  sourceRevision()        { return m_sourceRevision; };
	QString  destinationRevision()   { return m_destinationRevision; };

//	Format getFormat() { return m_format; };
	bool isModified() const { return m_modified; };

public slots:
	void setModified( bool modified );
	void toggleApplied( int diffIndex );

signals:
	void appliedChanged( const Difference* d );

private:
	int parseContextDiff( const QStringList& list );
	int parseEdDiff     ( const QStringList& list );
	int parseNormalDiff ( const QStringList& list );
	int parseRCSDiff    ( const QStringList& list );
	int parseUnifiedDiff( const QStringList& list );

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
};

#endif

