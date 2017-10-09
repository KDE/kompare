/***************************************************************************
                                main.cpp
                                --------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2005,2009 Otto Bruggeman <bruggie@gmail.com>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
        Copyright 2007-2012 Kevin Kofler   <kevin.kofler@chello.at>
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

#include <kfile.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>

#include <QApplication>
#include <QPushButton>
#include <QDebug>
#include <QCommandLineParser>

#include "kompareinterface.h"

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
static const char version[] = "4.1.3";

QUrl urlFromArg(const QString& arg)
{
#if QT_VERSION >= 0x050400
    return QUrl::fromUserInput(arg, QDir::currentPath(), QUrl::AssumeLocalFile);
#else
    // Logic from QUrl::fromUserInput(QString, QString, UserInputResolutionOptions)
    return (QUrl(arg, QUrl::TolerantMode).isRelative() && !QDir::isAbsolutePath(arg))
           ? QUrl::fromLocalFile(QDir::current().absoluteFilePath(arg))
           : QUrl::fromUserInput(arg);
#endif
}

/**
 * Setting up the KAboutData structure.
 * Parsing and handling of the given command line arguments.
 * @param argc   the number of arguments
 * @param argv   the array of arguments
 * @return exit status
 */
int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
        KLocalizedString::setApplicationDomain("kompare");

	KAboutData aboutData( "kompare",  i18n("Kompare"), version, i18n(description),
	                      KAboutLicense::GPL,
	                      i18n("(c) 2001-2004 John Firebaugh, (c) 2001-2005,2009 Otto Bruggeman, (c) 2004-2005 Jeff Snyder, (c) 2007-2012 Kevin Kofler") );
	aboutData.addAuthor( i18n("John Firebaugh"), i18n("Author"), "jfirebaugh@kde.org" );
	aboutData.addAuthor( i18n("Otto Bruggeman"), i18n("Author"), "bruggie@gmail.com" );
	aboutData.addAuthor( i18n("Jeff Snyder"), i18n("Developer"), "jeff@caffeinated.me.uk" );
	aboutData.addCredit( i18n("Kevin Kofler"), i18n("Maintainer"), "kevin.kofler@chello.at" );
	aboutData.addCredit( i18n("Chris Luetchford"), i18n("Kompare icon artist"), "chris@os11.com" );
	aboutData.addCredit( i18n("Malte Starostik"), i18n("A lot of good advice"), "malte@kde.org" );
	aboutData.addCredit( i18n("Bernd Gehrmann"), i18n("Cervisia diff viewer"), "bernd@physik.hu-berlin.de" );

	KAboutData::setApplicationData(aboutData);
	app.setWindowIcon(QIcon::fromTheme(QStringLiteral("kompare"), app.windowIcon()));

	QCommandLineParser parser;
	parser.addVersionOption();
	parser.addHelpOption();
	aboutData.setupCommandLine(&parser);
	parser.addOption(QCommandLineOption("c", i18n("This will compare URL1 with URL2")));
	parser.addOption(QCommandLineOption("o", i18n( "This will open URL1 and expect it to be diff output. URL1 can also be a '-' and then it will read from standard input. Can be used for instance for cvs diff | kompare -o -. Kompare will do a check to see if it can find the original file(s) and then blend the original file(s) into the diffoutput and show that in the viewer. -n disables the check." )));
	parser.addOption(QCommandLineOption("b", i18n( "This will blend URL2 into URL1, URL2 is expected to be diff output and URL1 the file or folder that the diffoutput needs to be blended into. " )));
	parser.addOption(QCommandLineOption("n", i18n( "Disables the check for automatically finding the original file(s) when using '-' as URL with the -o option." )));
	parser.addOption(QCommandLineOption("e <encoding>", i18n( "Use this to specify the encoding when calling it from the command line. It will default to the local encoding if not specified." )));

	parser.process(app);
	aboutData.processCommandLine(&parser);

	bool difault = false;

	KompareShell* ks;

	QStringList args = parser.positionalArguments();

	// see if we are starting with session management
	if (app.isSessionRestored())
	{
		RESTORE(KompareShell)
	}
	else
	{

		ks = new KompareShell();
		ks->setObjectName( "FirstKompareShell" );

		qCDebug(KOMPARESHELL) << "Arg Count = " << args.count() ;
		for ( int i=0; i < args.count(); i++ )
		{
			qCDebug(KOMPARESHELL) << "Argument " << (i+1) << ": " << args.at( i ) ;
		}

		if ( parser.isSet( "e <encoding>" ) )
		{
			// Encoding given...
			// FIXME: Need to implement this...
		}

		if ( parser.isSet( "o" ) )
		{
			qCDebug(KOMPARESHELL) << "Option -o is set" ;
			if ( args.count() != 1 )
			{
				difault = true;
			}
			else
			{
				ks->show();
				qCDebug(KOMPARESHELL) << "OpenDiff..." ;
				if ( args.at(0) == QLatin1String("-") )
					ks->openStdin();
				else
					ks->openDiff( urlFromArg( args.at( 0 ) ) );
				difault = false;
			}
		}
		else if ( parser.isSet( "c" ) )
		{
			qCDebug(KOMPARESHELL) << "Option -c is set" ;
			if ( args.count() != 2 )
			{
				parser.showHelp();
				difault = true;
			}
			else
			{
				ks->show();
				QUrl url0 = urlFromArg(args.at( 0 ));
				qCDebug(KOMPARESHELL) << "URL0 = " << url0.url() ;
				QUrl url1 = urlFromArg(args.at( 1 ));
				qCDebug(KOMPARESHELL) << "URL1 = " << url1.url() ;
				ks->compare( url0, url1 );
				difault = false;
			}
		}
		else if ( parser.isSet( "b" ) )
		{
			qCDebug(KOMPARESHELL) << "Option -b is set" ;
			if ( args.count() != 2 )
			{
				parser.showHelp();
				difault = true;
			}
			else
			{
				ks->show();
				qCDebug(KOMPARESHELL) << "blend..." ;
				QUrl url0 = urlFromArg(args.at( 0 ));
				qCDebug(KOMPARESHELL) << "URL0 = " << url0.url() ;
				QUrl url1 = urlFromArg(args.at( 1 ));
				qCDebug(KOMPARESHELL) << "URL1 = " << url1.url() ;
				ks->blend( url0, url1 );
				difault = false;
			}
		}
		else if ( args.count() == 1 )
		{
			ks->show();

			qCDebug(KOMPARESHELL) << "Single file. so openDiff/openStdin is only possible..." ;
			if ( args.at(0) == QLatin1String("-") )
				ks->openStdin();
			else
				ks->openDiff( urlFromArg(args.at( 0 ) ) );

			difault = false;
		}
		else if ( args.count() == 2 )
		{
			// In this case we are assuming you want to compare files/dirs
			// and not blending because that is almost impossible to detect
			ks->show();
			qCDebug(KOMPARESHELL) << "Dunno, we'll have to figure it out later, trying compare for now..." ;
			QUrl url0 = urlFromArg(args.at( 0 ));
			qCDebug(KOMPARESHELL) << "URL0 = " << url0.url() ;
			QUrl url1 = urlFromArg(args.at( 1 ));
			qCDebug(KOMPARESHELL) << "URL1 = " << url1.url() ;
			ks->compare( url0, url1 );
			difault = false;
		}
		else if ( args.count() == 0 ) // no options and no args
		{
			difault = true;
		}

		if ( difault )
		{
			KompareURLDialog dialog( 0 );

			dialog.setWindowTitle( i18n("Compare Files or Folders") );
			dialog.setFirstGroupBoxTitle( i18n( "Source" ) );
			dialog.setSecondGroupBoxTitle( i18n( "Destination" ) );

			QPushButton *okButton = dialog.button( QDialogButtonBox::Ok );
			okButton->setText( i18n( "Compare" ));
			okButton->setToolTip( i18n( "Compare these files or folders" ));
			okButton->setWhatsThis( i18n( "If you have entered 2 filenames or 2 folders in the fields in this dialog then this button will be enabled and pressing it will start a comparison of the entered files or folders. " ));

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


	}

	return app.exec();
}

/* vim: set ts=4 sw=4 noet: */

