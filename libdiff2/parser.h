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

#include "diffmodellist.h"
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
	DiffModelList* parse( const QString& diff );
	DiffModelList* parse( const QStringList& diff );

	enum Kompare::Generator generator() const { return m_generator; };
	enum Kompare::Format    format() const    { return m_format; };

private:
	/** Which program was used to generate the output */
	enum Kompare::Generator determineGenerator( const QStringList& diffLines );

private:
	enum Kompare::Generator m_generator;
	enum Kompare::Format    m_format;
};

} // End of namespace Diff2

#endif

