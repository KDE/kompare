/**************************************************************************
**                              parser.h
**                              --------
**      begin                   : Tue Jul 30 23:53:52 2002
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

#ifndef _DIFF2_PARSER_H
#define _DIFF2_PARSER_H

#include <qptrlist.h>

#include "kompare.h"

namespace Diff2
{

class DiffModel;

class Parser
{
public:
	Parser();
	~Parser();

public:
	QPtrList<DiffModel>* parse( const QString& diff );
	QPtrList<DiffModel>* parse( const QStringList& diff );

private:
	/** Which program was used to generate the output */
	enum Kompare::Generator determineGenerator( const QStringList& diffLines );
};

} // End of namespace Diff2

#endif

