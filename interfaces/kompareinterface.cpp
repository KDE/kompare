// Blah blah standard LGPL license
// Copyright 2002-2003, Otto Bruggeman <otto.bruggeman@home.nl>

#include <kdebug.h>

#include "kompareinterface.h"

KompareInterface::KompareInterface()
{
}

KompareInterface::~KompareInterface()
{
}

bool KompareInterface::openDiff( const KURL& /*diffUrl*/ )
{
	kdDebug(8101) << "OpenDiff( const KURL& /*diffUrl*/ ): Danger Will Robinson Danger..." << endl;
	return false;
}

bool KompareInterface::openDiff( const QString& /*diffOutput*/ )
{
	kdDebug(8101) << "OpenDiff( const QString& diffOutput ): Danger Will Robinson Danger..." << endl;
	return false;
}

bool KompareInterface::openDiff3( const KURL& /*diff3Url*/ )
{
	kdDebug(8101) << "OpenDiff3( const KURL& /*diff3Url*/ ): Danger Will Robinson Danger..." << endl;
	return false;
}

bool KompareInterface::openDiff3( const QString& /*diffOutput*/ )
{
	kdDebug(8101) << "OpenDiff3( const QString& diff3Output ): Danger Will Robinson Danger..." << endl;
	return false;
}

void KompareInterface::compareFiles( const KURL& /*sourceFile*/, const KURL& /*destinationFile*/ )
{
	kdDebug(8101) << "CompareFiles( const KURL& /*sourceFile*/, const KURL& /*destinationFile*/ ): Danger Will Robinson Danger..." << endl;
}

void KompareInterface::compareDirs ( const KURL& /*sourceDir*/, const KURL& /*destinationDir*/ )
{
	kdDebug(8101) << "CompareDirs( const KURL& /*sourceDir*/, const KURL& /*destinationDir*/ ): Danger Will Robinson Danger..." << endl;
}

void KompareInterface::compare3( const KURL& /*originalFile*/, const KURL& /*changedFile1*/, const KURL& /*changedFile2*/ )
{
	kdDebug(8101) << "Compare3( const KURL& /*originalFile*/, const KURL& /*changedFile1*/, const KURL& /*changedFile2*/ ): Danger Will Robinson Danger..." << endl;
}

void KompareInterface::openFileAndDiff( const KURL& /*file*/, const KURL& /*diffFile*/ )
{
	kdDebug(8101) << "OpenFileAndDiff( const KURL& /*file*/, const KURL& /*diffFile*/ ): Danger Will Robinson Danger..." << endl;
}

void KompareInterface::openDirAndDiff ( const KURL& /*dir*/,  const KURL& /*diffFile*/ )
{
	kdDebug(8101) << "OpenDirAndDiff( const KURL& /*dir*/,  const KURL& /*diffFile*/ ): Danger Will Robinson Danger..." << endl;
}

int KompareInterface::readProperties( KConfig* config )
{
	return -1;
}

int KompareInterface::saveProperties( KConfig* config )
{
	return -1;
}

bool KompareInterface::queryClose()
{
	return true;
}
