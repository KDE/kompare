/***************************************************************************
                                main.cpp  -  description
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


#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kfile.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kompare_shell.h"
#include "kompareurldialog.h"

static const char description[] =
	I18N_NOOP("A program to view the differences between files and optionally generate a diff." );

static const char version[] = "v3.2";

static KCmdLineOptions options[] =
{
	{ "c", I18N_NOOP( "This will compare URL1 with URL2." ), 0 },
	{ "o", I18N_NOOP( "This will open URL1 and expect it to be diff output. URL1 can also be a '-' and then it will read from standard input. Can be used for instance for cvs diff | kompare -o -. Kompare will do a check to see if it can find the original file(s) and then blend the original file(s) into the diffoutput and show that in the viewer. -n disables the check." ), 0 },
	{ "b", I18N_NOOP( "This will blend URL2 into URL1, URL2 is expected to be diff output and URL1 the file or folder that the diffoutput needs to be blended into. " ), 0 },
	{ "n", I18N_NOOP( "Disables the check for automatically finding the original file(s) when using '-' as URL with the -o option." ), 0 },
	{ "+[URL1 [URL2]]",0 , 0 },
	{ "+-", 0, 0 },
//	{ "", I18N_NOOP( "" ), 0 },
	KCmdLineLastOption
};

int main(int argc, char *argv[])
{
	KAboutData aboutData( "kompare", I18N_NOOP("Kompare"), version, description,
	                      KAboutData::License_GPL,
	                      I18N_NOOP("(c) 2001-2003, John Firebaugh and Otto Bruggeman"), 0, "http://bruggie.dnsalias.org" );
	aboutData.addAuthor( "John Firebaugh", I18N_NOOP("Author"), "jfirebaugh@kde.org" );
	aboutData.addAuthor( "Otto Bruggeman", I18N_NOOP("Author"), "otto.bruggeman@home.nl" );
	aboutData.addCredit( "Malte Starostik", I18N_NOOP("A lot of good advice"), "malte@kde.org" );
	aboutData.addCredit( "Bernd Gehrmann", I18N_NOOP("Cervisia diff viewer"), "bernd@physik.hu-berlin.de" );

	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions( options );
	KApplication app;
	bool difault = false;

	// see if we are starting with session management
	if (app.isRestored())
	{
		RESTORE(KompareShell)
	}
	else
	{
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

		KompareShell* ks;

		kdDebug( 8100 ) << "Arg Count = " << args->count() << endl;
		for ( int i=0; i < args->count(); i++ )
		{
			kdDebug( 8100 ) << "Argument " << (i+1) << ": " << args->arg( i ) << endl;
		}

		if ( args->isSet( "o" ) )
		{
			kdDebug( 8100 ) << "Option -o is set" << endl;
			if ( args->count() != 1 )
			{
				difault = true;
			}
			else
			{
				ks = new KompareShell();
				ks->show();
				kdDebug( 8100 ) << "OpenDiff..." << endl;
				if ( ( strlen( args->arg(0) ) == 1 ) && (  *(args->arg(0)) == '-' ) )
					ks->openStdin();
				else
					ks->openDiff( args->url( 0 ) );
				difault = false;
			}
		}
		else if ( args->isSet( "c" ) )
		{
			kdDebug( 8100 ) << "Option -c is set" << endl;
			if ( args->count() != 2 )
			{
				KCmdLineArgs::usage( "kompare" );
				difault = true;
			}
			else
			{
				ks = new KompareShell();
				ks->show();
				KURL url0 = args->url( 0 );
				kdDebug( 8100 ) << "URL0 = " << url0.url() << endl;
				KURL url1 = args->url( 1 );
				kdDebug( 8100 ) << "URL1 = " << url1.url() << endl;
				ks->compare( url0, url1 );
				difault = false;
			}
		}
		else if ( args->isSet( "b" ) )
		{
			kdDebug( 8100 ) << "Option -b is set" << endl;
			if ( args->count() != 2 )
			{
				KCmdLineArgs::usage( "kompare" );
				difault = true;
			}
			else
			{
				ks = new KompareShell();
				ks->show();
				kdDebug( 8100 ) << "blend..." << endl;
				KURL url0 = args->url( 0 );
				kdDebug( 8100 ) << "URL0 = " << url0.url() << endl;
				KURL url1 = args->url( 1 );
				kdDebug( 8100 ) << "URL1 = " << url1.url() << endl;
				ks->blend( url0, url1 );
				difault = false;
			}
		}
		else if ( args->count() == 1 )
		{
			ks = new KompareShell();
			ks->show();

			kdDebug( 8100 ) << "Single file. so openDiff/openStdin is only possible..." << endl;
			if ( ( strlen( args->arg(0) ) == 1 && *(args->arg(0)) == '-' ) )
				ks->openStdin();
			else
				ks->openDiff( args->url( 0 ) );

			difault = false;
		}
		else if ( args->count() == 2 )
		{
			// In this case we are assuming you want to compare files/dirs
			// and not blending because that is almost impossible to detect
			ks = new KompareShell();
			ks->show();
			kdDebug( 8100 ) << "Dunno, we'll have to figure it out later, trying compare for now..." << endl;
			KURL url0 = args->url( 0 );
			kdDebug( 8100 ) << "URL0 = " << url0.url() << endl;
			KURL url1 = args->url( 1 );
			kdDebug( 8100 ) << "URL1 = " << url1.url() << endl;
			ks->compare( url0, url1 );
			difault = false;
		}
		else if ( args->count() == 0 ) // no options and no args
		{
			difault = true;
		}

		if ( difault )
		{
			KompareURLDialog* dialog = new KompareURLDialog();

			dialog->setCaption( i18n("Compare Files or Folders") );
			dialog->setFirstGroupBoxTitle( i18n( "Source" ) );
			dialog->setSecondGroupBoxTitle( i18n( "Destination" ) );

			KGuiItem compareGuiItem( i18n( "Compare" ), QString::null, i18n( "Compare these files or folder" ), i18n( "If you have entered 2 filenames or 2 folders in the fields in this dialog then this button will be enabled and pressing it will start a comparison of the entered files or folders. " ) );
			dialog->setButtonOK( compareGuiItem );

			dialog->setGroup( "Recent Compare Files" );

			dialog->setFirstURLRequesterMode( KFile::File|KFile::Directory|KFile::ExistingOnly );
			dialog->setSecondURLRequesterMode( KFile::File|KFile::Directory|KFile::ExistingOnly );

			if( dialog->exec() == QDialog::Accepted )
			{
				ks = new KompareShell();
				ks->show();
				ks->compare( dialog->getFirstURL(), dialog->getSecondURL() );
			}
			else
				return -1;

			delete dialog;
		}

		args->clear();
	}

	return kapp->exec();
}

/* vim: set ts=4 sw=4 noet: */

