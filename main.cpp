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

#include <qcstring.h>

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

#include "kdiffapp.h"
#include "kmaindiffview.h"

static const char *description =
	I18N_NOOP("A program to view the differences between files and optionally generate a diff." );

static const char *version = "v2.0";

static KCmdLineOptions options[] =
{
	{ "f format", I18N_NOOP( "The format of the generated diff. It can be one of\nthese: Context, Ed, Normal, RCS, Unified." ), 0 },
	{ "+[URL [URL2]]", I18N_NOOP( "Document to open. If only one url is given\nit is considered a .diff file. If 2 files are given\nkdiff will create a diff of these files and show it." ), 0 },
	{ 0, 0, 0 }
};

int main(int argc, char *argv[])
{
	QCString format;
	KDiffApp* kdiffapp;

	KAboutData aboutData( "kdiff", I18N_NOOP("KDiff"), version, description,
	                      KAboutData::License_GPL,
	                      "(c) 2001, Otto Bruggeman", 0, 0, "otto.bruggeman@home.nl");
	aboutData.addAuthor( "Otto Bruggeman", "Author", "otto.bruggeman@home.nl" );
	aboutData.addAuthor( "John Firebaugh", "Author", "jfirebaugh@mac.com" );
	aboutData.addCredit( "Malte Starostik", "Big help :)", "malte@kde.org" );
//	aboutData.addAuthor( ); // TODO: add cervisia author
	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	kdiffapp = new KDiffApp( args );

	return kdiffapp->exec();
}
