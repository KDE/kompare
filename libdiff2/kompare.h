/***************************************************************************
                                kompare.h  -  description
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

#ifndef KOMPARE_H
#define KOMPARE_H

#include <kurl.h>

namespace Kompare
{
	enum Format {
		Context       = 0,
		Ed            = 1,
		Normal        = 2,
		RCS           = 3,
		Unified       = 4,
		SideBySide    = 5,
		UnknownFormat = -1
	};

	enum Generator {
		CVSDiff          = 0,
		Diff             = 1,
		Perforce         = 2,
		Reserved1        = 3,
		Reserved2        = 4,
		Reserved3        = 5,
		Reserved4        = 6,
		Reserved5        = 7,
		Reserved6        = 8,
		Reserved7        = 9,
		UnknownGenerator = -1
	};

	enum Mode {
		ComparingFiles,
		ComparingDirs,
		ShowingDiff
	};

	enum DiffMode {
		Default,
		Custom
	};

	enum Status {
		RunningDiff,
		Parsing,
		FinishedParsing,
		FinishedWritingDiff,
		ReRunningDiff	// When a change has been detected after diff has run
	};

	enum Target {
		Source,
		Destination
	};

	enum Type {
		SingleFileDiff,
		MultiFileDiff
	};
}; // End of namespace Kompare

/*
** This should be removed and put somewhere else
*/
class KompareFunctions
{
public:
	static QString constructRelativePath( const QString& from, const QString& to )
	{
		KURL fromURL( from );
		KURL toURL( to );
		KURL root;
		int upLevels = 0;

		// Find a common root.
		root = from;
		while( !root.isValid() && !root.isParentOf( toURL ) ) {
			root = root.upURL();
			upLevels++;
		}

		if( root.isValid() ) return to;

		QString relative;
		for( ; upLevels > 0; upLevels-- ) {
			relative += "../";
		}

		relative += QString( to ).replace( 0, root.path(1).length(), "" );

		return relative;
	}
};

#endif
