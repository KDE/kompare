/***************************************************************************
                                main.cpp  -  description
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


#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "kompare_shell.h"
#include "kcomparedialog.h"

#define kdDebug() kdDebug(8100)

static const char *description =
	I18N_NOOP("A program to view the differences between files and optionally generate a diff." );

static const char *version = "v2.0";

static KCmdLineOptions options[] =
{
	{ "+[URL [URL2]]", I18N_NOOP( "Document to open. If only one URL is given, it is\nconsidered to be a .diff file. If the file is a -\nthen Kompare will read from stdin, this can be used\nfor cvs diff | kompare -. If 2 files are given,\nKompare will compare them." ), 0 },
	{ 0, 0, 0 }
};

int main(int argc, char *argv[])
{
	KAboutData aboutData( "kompare", I18N_NOOP("Kompare"), version, description,
	                      KAboutData::License_GPL,
	                      "(c) 2001, John Firebaugh and Otto Bruggeman", 0, 0, "jfirebaugh@kde.org");
	aboutData.addAuthor( "John Firebaugh", "Author", "jfirebaugh@kde.org" );
	aboutData.addAuthor( "Otto Bruggeman", "Author", "otto.bruggeman@home.nl" );
	aboutData.addCredit( "Malte Starostik", "Big help :)", "malte@kde.org" );
	aboutData.addCredit( "Bernd Gehrmann", "Cervisia diff viewer", "bernd@physik.hu-berlin.de" );
	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
	KApplication app;

	// see if we are starting with session management
	if (app.isRestored())
	{
		RESTORE(KompareShell)
	}
	else
	{
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

		KompareShell* widget;

		switch ( args->count() )
		{
		case 0:  // no files, show compare dialog
			{
				KCompareDialog* dialog = new KCompareDialog();
				if( dialog->exec() == QDialog::Accepted ) {
					KURL source = dialog->getSourceURL();
					KURL destination = dialog->getDestinationURL();
					widget = new KompareShell();
					widget->show();
					widget->compare( source, destination );
				} else {
					return 0;
				}
				delete dialog;
			}
			break;
		case 1:  // 1 file -> it is a diff, use load()
			kdDebug() << "Url is : " << args->arg(0) << endl;
			widget = new KompareShell();
			widget->show();
			widget->load( args->url( 0 ) );
			break;
		case 2:  // 2 files -> diff them with compare
			widget = new KompareShell();
			widget->show();
			widget->compare( args->url( 0 ), args->url( 1 ) );
			break;
		default: // error
			KMessageBox::error( NULL, i18n( "Sorry, you gave too many arguments, I don't know what to do with them..." ) );
			return 0;
			break;
		}
		args->clear();
	}

	return kapp->exec();
}

/* vim: set ts=4 sw=4 noet: */

