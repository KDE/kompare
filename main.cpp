/***************************************************************************
			main.cpp  -  description
			-------------------
	begin			: Sun Mar 4 2001
	copyright			: (C) 2001 by Otto Bruggeman
	email			: otto.bruggeman@home.nl
***************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
****************************************************************************/

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

#include "kdiffapp.h"
#include "kmaindiffview.h"

static const char *description =
	I18N_NOOP( "KDiff" );
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE
	
	
static KCmdLineOptions options[] =
{
	{ 0, 0, 0 }
	// INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{

	KAboutData aboutData( "kdiff", I18N_NOOP("KDiff"),
	"2.0", I18N_NOOP( "A program to view the differences between files with.\n\n"
	"Many thanks go to Malte Starostik for all the help he has given me." ), KAboutData::License_GPL,
	"(c) 2001, Otto Bruggeman", 0, 0, "otto.bruggeman@home.nl");
	aboutData.addAuthor( "Otto Bruggeman", 0, "otto.bruggeman@home.nl" );
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

	KDiffApp* kdiffapp = new KDiffApp();

	return kdiffapp->exec();
}
