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
#include <qlist.h>
#include <qstringlist.h>

class DiffHunk;
class Difference;
class KDiffModelList;

class DiffModel : public QObject
{
Q_OBJECT
public:
	enum DiffFormat { Context, Ed, Normal, RCS, Unified, Unknown };
	
	DiffModel();
	~DiffModel();

	static DiffFormat determineDiffFormat( QString line );

	int parseDiff( enum DiffFormat format, const QStringList& list, QStringList::ConstIterator& it);

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
	const QList<DiffHunk>& getHunks() const
		{ return m_hunks; };
	const QList<Difference>& getDifferences() const
		{ return m_differences; };

	QString sourceFile() { return m_sourceFile; };
	QString destinationFile() { return m_destinationFile; };
	QString sourceTimestamp() { return m_sourceTimestamp; };
	QString destinationTimestamp() { return m_destinationTimestamp; };

	enum DiffFormat getFormat() { return m_format; };
	bool isModified() const { return m_modified; };

public slots:
	void setModified( bool modified );
	void toggleApplied( int diffIndex );

signals:
	void appliedChanged( const Difference* d );

private:
	int parseContextDiff( const QStringList& list, QStringList::ConstIterator& it );
	int parseEdDiff( const QStringList& list, QStringList::ConstIterator& it );
	int parseNormalDiff( const QStringList& list, QStringList::ConstIterator& it );
	int parseRCSDiff( const QStringList& list, QStringList::ConstIterator& it );
	int parseUnifiedDiff( const QStringList& list, QStringList::ConstIterator& it );

	QString             m_sourceFile;
	QString             m_sourceTimestamp;
	QString             m_destinationFile;
	QString             m_destinationTimestamp;
	QList<DiffHunk>     m_hunks;
	QList<Difference>   m_differences;
	int                 m_appliedCount;
	enum DiffFormat     m_format;
	bool                m_modified;
};

#endif

