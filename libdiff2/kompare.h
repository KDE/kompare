/***************************************************************************
                                kompare.h  -  description
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

#ifndef KOMPARE_H
#define KOMPARE_H

#include <kurl.h>

class Kompare
{
public:
	enum Format {
		Context     = 0,
		Ed          = 1,
		Normal      = 2,
		RCS         = 3,
		Unified     = 4,
		SideBySide  = 5,
		Unknown     = -1
	};
	
	enum Mode {
		ComparingFiles,
		ComparingDirs,
		ShowingDiff
	};

	enum Status {
		RunningDiff,
		Parsing,
		FinishedParsing,
		FinishedWritingDiff
	};

	enum Target {
		Source,
		Destination
	};

	enum Type {
		SingleFileCVSDiff,
		MultiFileCVSDiff,
		SingleFileDiff,
		MultiFileDiff
	};

	static QString constructRelativePath( const QString& from, const QString& to )
	{
		KURL fromURL( from );
		KURL toURL( to );
		KURL root;
		int upLevels = 0;

		// Find a common root.
		root = from;
		while( !root.isMalformed() && !root.isParentOf( toURL ) ) {
			root = root.upURL();
			upLevels++;
		}
		
		if( root.isMalformed() ) return to;
		
		QString relative;
		for( ; upLevels > 0; upLevels-- ) {
			relative += "../";
		}
		
		relative += QString( to ).replace( 0, root.path(1).length(), "" );
		
		return relative;
	}
};

#endif
