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
#include <qfile.h>
//#include

class DiffHunk;
class Difference;

class DiffModel : QObject
{
Q_OBJECT
public:
	enum DiffFormat { Context, Ed, Normal, RCS, Unified, Unknown };

public:
	DiffModel();
	~DiffModel();

public:
	const QList<DiffHunk>& getHunks() const { return hunks; };
	const QList<Difference>& getDifferences() const { return differences; };

	QString getSourceFilename() { return sourceFilename; };
	QString getDestinationFilename() { return destinationFilename; };
	QString getSourceTimestamp() { return sourceTimestamp; };
	QString getDestinationTimestamp() { return destinationTimestamp; };
	
	int parseDiff( const QStringList& list );
	int parseDiff( const QStringList& list, enum DiffFormat format );

private:
	int parseContextDiff( const QStringList& list );
	int parseEdDiff( const QStringList& list );
	int parseNormalDiff( const QStringList& list );
	int parseRCSDiff( const QStringList& list );
	int parseUnifiedDiff( const QStringList& list );

private:
	QString sourceFilename;
	QString sourceTimestamp;
	QString destinationFilename;
	QString destinationTimestamp;

	QList<DiffHunk>   hunks;
	QList<Difference> differences;
};

#endif

