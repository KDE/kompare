/***************************************************************************
                                diffmodel.h  -  description
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

#ifndef DIFFMODEL_H
#define DIFFMODEL_H

#include <qobject.h>
#include <qptrlist.h>
#include <qstringlist.h>

#include "kompare.h"

namespace Diff2
{

class DiffHunk;
class Difference;

class DiffModel : public QObject
{
Q_OBJECT
public:

	DiffModel( const QString& srcBaseURL = "", const QString& destBaseURL = "" );
	~DiffModel();

	int parseDiff( enum Kompare::Format format, const QStringList& list );

	int hunkCount() const       { return m_hunks.count(); };
	int differenceCount() const { return m_differences.count(); };
	int appliedCount() const    { return m_appliedCount; };

	DiffHunk* hunkAt( int i )         { return m_hunks.at( i ); };
	Difference* differenceAt( int i ) { return m_differences.at( i ); };

	const QPtrList<DiffHunk>& hunks() const         { return m_hunks; };
	const QPtrList<Difference>& differences() const { return m_differences; };

	const QPtrList<Difference>& allDifferences();

	int findDifference( const Difference* diff ) const { return const_cast<DiffModel*>(this)->m_differences.findRef( diff ); };

	Difference* firstDifference();
	Difference* lastDifference();
	Difference* prevDifference();
	Difference* nextDifference();

	const QString  source() const              { return m_source; };
	const QString  destination() const         { return m_destination; };
	const QString  sourceFile() const;
	const QString  destinationFile() const;
	const QString  sourcePath() const;
	const QString  destinationPath() const;
	const QString& sourceTimestamp()           { return m_sourceTimestamp; };
	const QString& destinationTimestamp()      { return m_destinationTimestamp; };
	const QString& sourceRevision() const      { return m_sourceRevision; };
	const QString& destinationRevision() const { return m_destinationRevision; };

	void setSourceFile( QString path );
	void setDestinationFile( QString path );
	void setSourceTimestamp( QString timestamp );
	void setDestinationTimestamp( QString timestamp );
	void setSourceRevision( QString revision );
	void setDestinationRevision( QString revision );

	void addHunk( DiffHunk* hunk );
	void addDiff( Difference* diff );
	bool isModified() const { return m_modified; };

	const int diffIndex( void ) const       { return m_diffIndex; };
	void      setDiffIndex( int diffIndex ) { m_diffIndex = diffIndex; };

	void applyDifference( bool apply );
	void applyAllDifferences( bool apply );

	bool setSelectedDifference( Difference* diff );

	DiffModel& operator=( const DiffModel& model );

signals:
	void setModified( bool modified );

public slots:
	void slotSetModified( bool modified );

private:
	void splitSourceInPathAndFileName();
	void splitDestinationInPathAndFileName();

private:
	QString              m_source;
	QString              m_destination;

	QString              m_sourcePath;
	QString              m_destinationPath;

	QString              m_sourceFile;
	QString              m_destinationFile;

	QString              m_sourceTimestamp;
	QString              m_destinationTimestamp;

	QString              m_sourceRevision;
	QString              m_destinationRevision;

	QPtrList<DiffHunk>   m_hunks;
	QPtrList<Difference> m_differences;
	QPtrList<Difference> m_allDifferences;
	int                  m_appliedCount;
	bool                 m_modified;

	unsigned int         m_diffIndex;
	Difference*          m_selectedDifference;
};

} // End of namespace Diff2

#endif

