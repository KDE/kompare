// blah blah standard LGPL license
// Copyright 2002-2003, Otto Bruggeman <otto.bruggeman@home.nl>

#ifndef _KOMPARE_INTERFACE_H
#define _KOMPARE_INTERFACE_H

class KConfig;
class KURL;

class KompareInterface
{
public:
	KompareInterface();
	virtual ~KompareInterface();

public:
	/**
	 * Open and parse the diff file at url.
	 */
	virtual bool openDiff( const KURL& diffUrl );

	/**
	 * Open and parse the supplied diff output
	 */
	virtual bool openDiff( const QString& diffOutput );

	/**
	 * Open and parse the diff3 file at url.
	 */
	virtual bool openDiff3( const KURL& diff3Url );

	/**
	 * Open and parse the supplied diff3 output
	 */
	virtual bool openDiff3( const QString& diff3Output );

	/**
	 * Compare, with diff, source with destination files
	 */
	virtual void compareFiles( const KURL& sourceFile, const KURL& destinationFile );

	/**
	 * Compare, with diff, source with destination directories
	 */
	virtual void compareDirs ( const KURL& sourceDir, const KURL& destinationDir );

	/**
	 * Compare, with diff3, originalFile with changedFile1 and changedFile2
	 */
	virtual void compare3( const KURL& originalFile, const KURL& changedFile1, const KURL& changedFile2 );

	/**
	 * This will show the file and the file with the diff applied
	 */
	virtual void openFileAndDiff( const KURL& file, const KURL& diffFile );

	/**
	 * This will show the directory and the directory with the diff applied
	 */
	virtual void openDirAndDiff ( const KURL& dir,  const KURL& diffFile );

public:
	/**
	 * Warning this should be in class Part in KDE 4.0, not here !
	 * Around that time the methods will disappear here
	 */
	virtual int readProperties( KConfig* config );
	virtual int saveProperties( KConfig* config );

	/**
	 * Warning this should be in class ReadWritePart in KDE 4.0, not here !
	 * Around that time the method will disappear here
	 */
	virtual bool queryClose();

private:
// Maybe some private variables ???
};

#endif /* _KOMPARE_INTERFACE_H */
