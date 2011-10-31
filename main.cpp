/***************************************************************************
                                main.cpp
                                --------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2005,2009 Otto Bruggeman <bruggie@gmail.com>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
        Copyright 2007-2011 Kevin Kofler   <kevin.kofler@chello.at>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/
/**
 * @file main.cpp
 * This is the main entry point for Kompare.
 * The command line arguments are handled and the application is started.
 * @author Otto Bruggeman <otto.bruggeman@home.nl>
 * @author John Firebaugh <jfirebaugh@kde.org>
 * @author Kevin Kofler   <kevin.kofler@chello.at>
 */

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kfile.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdialog.h>

#include "kompare_part.h"
#include "kompare_shell.h"
#include "kompareurldialog.h"

/**
 * Program description.
 */
static const char description[] =
	I18N_NOOP("A program to view the differences between files and optionally generate a diff" );

/**
 * Version number.
 */
static const char version[] = "4.1.0";

/**
 * Setting up the KAboutData structure.
 * Parsing and handling of the given command line arguments.
 * @param argc   the number of arguments
 * @param argv   the array of arguments
 * @return exit status
 */
int main(int argc, char *argv[])
{
	KAboutData aboutData( "kompare", 0, ki18n("Kompare"), version, ki18n(description),
	                      KAboutData::License_GPL,
	                      ki18n("(c) 2001-2004 John Firebaugh, (c) 2001-2005,2009 Otto Bruggeman, (c) 2004-2005 Jeff Snyder, (c) 2007-2011 Kevin Kofler") );
	aboutData.addAuthor( ki18n("John Firebaugh"), ki18n("Author"), "jfirebaugh@kde.org" );
	aboutData.addAuthor( ki18n("Otto Bruggeman"), ki18n("Author"), "bruggie@gmail.com" );
	aboutData.addAuthor( ki18n("Jeff Snyder"), ki18n("Developer"), "jeff@caffeinated.me.uk" );
	aboutData.addCredit( ki18n("Kevin Kofler"), ki18n("Maintainer"), "kevin.kofler@chello.at" );
	aboutData.addCredit( ki18n("Chris Luetchford"), ki18n("Kompare icon artist"), "chris@os11.com" );
	aboutData.addCredit( ki18n("Malte Starostik"), ki18n("A lot of good advice"), "malte@kde.org" );
	aboutData.addCredit( ki18n("Bernd Gehrmann"), ki18n("Cervisia diff viewer"), "bernd@physik.hu-berlin.de" );

	KCmdLineArgs::init(argc, argv, &aboutData);

	KCmdLineOptions options;
	options.add("c", ki18n( "This will compare URL1 with URL2" ));
	options.add("o", ki18n( "This will open URL1 and expect it to be diff output. URL1 can also be a '-' and then it will read from standard input. Can be used for instance for cvs diff | kompare -o -. Kompare will do a check to see if it can find the original file(s) and then blend the original file(s) into the diffoutput and show that in the viewer. -n disables the check." ));
	options.add("b", ki18n( "This will blend URL2 into URL1, URL2 is expected to be diff output and URL1 the file or folder that the diffoutput needs to be blended into. " ));
	options.add("n", ki18n( "Disables the check for automatically finding the original file(s) when using '-' as URL with the -o option." ));
	options.add("e <encoding>", ki18n( "Use this to specify the encoding when calling it from the command line. It will default to the local encoding if not specified." ));
	options.add("+[URL1 [URL2]]");
	options.add("+-");
	KCmdLineArgs::addCmdLineOptions( options );
	KApplication kompare;
	bool difault = false;

	KompareShell* ks;

	// see if we are starting with session management
	if (kompare.isSessionRestored())
	{
		RESTORE(KompareShell)
	}
	else
	{
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

		ks = new KompareShell();
		ks->setObjectName( "FirstKompareShell" );

		kDebug( 8100 ) << "Arg Count = " << args->count() << endl;
		for ( int i=0; i < args->count(); i++ )
		{
			kDebug( 8100 ) << "Argument " << (i+1) << ": " << args->arg( i ) << endl;
		}

		if ( args->isSet( "e" ) )
		{
			// Encoding given...
			// FIXME: Need to implement this...
		}

		if ( args->isSet( "o" ) )
		{
			kDebug( 8100 ) << "Option -o is set" << endl;
			if ( args->count() != 1 )
			{
				difault = true;
			}
			else
			{
				ks->show();
				kDebug( 8100 ) << "OpenDiff..." << endl;
				if ( args->arg(0) == QLatin1String("-") )
					ks->openStdin();
				else
					ks->openDiff( args->url( 0 ) );
				difault = false;
			}
		}
		else if ( args->isSet( "c" ) )
		{
			kDebug( 8100 ) << "Option -c is set" << endl;
			if ( args->count() != 2 )
			{
				KCmdLineArgs::usage( "kompare" );
				difault = true;
			}
			else
			{
				ks->show();
				KUrl url0 = args->url( 0 );
				kDebug( 8100 ) << "URL0 = " << url0.url() << endl;
				KUrl url1 = args->url( 1 );
				kDebug( 8100 ) << "URL1 = " << url1.url() << endl;
				ks->compare( url0, url1 );
				difault = false;
			}
		}
		else if ( args->isSet( "b" ) )
		{
			kDebug( 8100 ) << "Option -b is set" << endl;
			if ( args->count() != 2 )
			{
				KCmdLineArgs::usage( "kompare" );
				difault = true;
			}
			else
			{
				ks->show();
				kDebug( 8100 ) << "blend..." << endl;
				KUrl url0 = args->url( 0 );
				kDebug( 8100 ) << "URL0 = " << url0.url() << endl;
				KUrl url1 = args->url( 1 );
				kDebug( 8100 ) << "URL1 = " << url1.url() << endl;
				ks->blend( url0, url1 );
				difault = false;
			}
		}
		else if ( args->count() == 1 )
		{
			ks->show();

			kDebug( 8100 ) << "Single file. so openDiff/openStdin is only possible..." << endl;
			if ( args->arg(0) == QLatin1String("-") )
				ks->openStdin();
			else
				ks->openDiff( args->url( 0 ) );

			difault = false;
		}
		else if ( args->count() == 2 )
		{
			// In this case we are assuming you want to compare files/dirs
			// and not blending because that is almost impossible to detect
			ks->show();
			kDebug( 8100 ) << "Dunno, we'll have to figure it out later, trying compare for now..." << endl;
			KUrl url0 = args->url( 0 );
			kDebug( 8100 ) << "URL0 = " << url0.url() << endl;
			KUrl url1 = args->url( 1 );
			kDebug( 8100 ) << "URL1 = " << url1.url() << endl;
			ks->compare( url0, url1 );
			difault = false;
		}
		else if ( args->count() == 0 ) // no options and no args
		{
			difault = true;
		}

		if ( difault )
		{
			KompareURLDialog dialog( 0 );

			dialog.setCaption( i18n("Compare Files or Folders") );
			dialog.setFirstGroupBoxTitle( i18n( "Source" ) );
			dialog.setSecondGroupBoxTitle( i18n( "Destination" ) );

			KGuiItem compareGuiItem( i18n( "Compare" ), QString(), i18n( "Compare these files or folder" ), i18n( "If you have entered 2 filenames or 2 folders in the fields in this dialog then this button will be enabled and pressing it will start a comparison of the entered files or folders. " ) );
			dialog.setButtonGuiItem( KDialog::Ok, compareGuiItem );

			dialog.setGroup( "Recent Compare Files" );

			dialog.setFirstURLRequesterMode( KFile::File|KFile::Directory|KFile::ExistingOnly );
			dialog.setSecondURLRequesterMode( KFile::File|KFile::Directory|KFile::ExistingOnly );

			if( dialog.exec() == QDialog::Accepted )
			{
				ks->show();
				ks->viewPart()->setEncoding( dialog.encoding() );
				ks->compare( dialog.getFirstURL(), dialog.getSecondURL() );
			}
			else
                        {
				return -1;
                        }
		}

		args->clear();
	}

	return kompare.exec();
}

/* vim: set ts=4 sw=4 noet: */

