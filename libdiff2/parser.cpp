/**************************************************************************
**                             parser.cpp
**                              -------------------
**      begin                   : Sun Aug  4 15:05:35 2002
**      copyright               : (C) 2002-2003 by Otto Bruggeman
**      email                   : otto.bruggeman@home.nl
**
***************************************************************************/
/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   ( at your option ) any later version.
**
***************************************************************************/

#include <kdebug.h>

#include "parser.h"
#include "cvsdiffparser.h"
#include "diffparser.h"
#include "perforceparser.h"
#include "diffmodel.h"

using namespace Diff2;

Parser::Parser()
{
}

Parser::~Parser()
{
}

QPtrList<DiffModel>* Parser::parse( const QString& diff )
{
	kdDebug(8101) << diff << endl;
	/* Just split the QString and call the other static parse method */
	return Parser::parse( QStringList::split( diff, "\n" ) );
}

QPtrList<DiffModel>* Parser::parse( const QStringList& diff )
{
	kdDebug(8101) << diff.join("\n") << endl;

	/* Basically determine the generator then call the parse method */
	ParserBase* parser;

	switch( determineGenerator( diff ) )
	{
	case Kompare::CVSDiff :
		kdDebug(8101) << "It is a CVS generated diff..." << endl;
		parser = new CVSDiffParser( diff );
		break;
	case Kompare::Diff :
		kdDebug(8101) << "It is a diff generated diff..." << endl;
		parser = new DiffParser( diff );
		break;
	case Kompare::Perforce :
		kdDebug(8101) << "It is a Perforce generated diff..." << endl;
		parser = new PerforceParser( diff );
		break;
	default:
		// Nothing to delete, just leave...
		return 0L;
	}

	QPtrList<DiffModel>* modelList = parser->parse();
	if ( modelList )
	{
		kdDebug(8101) << "Modelcount: " << modelList->count() << endl;
		DiffModel* model = modelList->first();
		while ( model )
		{
			kdDebug(8101) << "Hunkcount:  " << model->hunkCount() << endl;
			kdDebug(8101) << "Diffcount:  " << model->differenceCount() << endl;
			model = modelList->next();
		}
	}

	delete parser;

	return modelList;
}

enum Kompare::Generator Parser::determineGenerator( const QStringList& diffLines )
{
	// Shit have to duplicate some code with this method and the ParserBase derived classes
	QStringList::ConstIterator it = diffLines.begin();

	QRegExp cvsDiffRE     ( "^Index: " );
	QRegExp perforceDiffRE( "^==== " );
	while (  it != diffLines.end() )
	{
		if ( ( *it ).find( cvsDiffRE, 0 ) == 0 )
		{
			kdDebug(8101) << "Diff is a CVSDiff" << endl;
			return Kompare::CVSDiff;
		}
		else if ( ( *it ).find( perforceDiffRE, 0 ) == 0 )
		{
			kdDebug(8101) << "Diff is a Perforce Diff" << endl;
			return Kompare::Perforce;
		}
		++it;
	}

	kdDebug(8101) << "We'll assume it is a diff Diff" << endl;

	// For now we'll assume it is a diff file diff, later we might
	// try to really determine if it is a diff file diff.
	return Kompare::Diff;
}
