#ifndef DIFFERENCE_H
#define DIFFERENCE_H

#include <qobject.h>
#include <qstring.h>

class Difference : QObject
{
Q_OBJECT
public:
	enum Type { Change, Insert, Delete, Neutral, Unchanged, Separator };

	Difference( int linenoA, int linenoB );
	~Difference();

	QString asString() const;
	int sourceLineCount() const;
	int destinationLineCount() const;
	const QStringList getSourceLines() const { return sourceLines; };
	const QStringList getDestinationLines() const { return destinationLines; };
	void addSourceLine( QString line );
	void addDestinationLine( QString line );

public:
	Type type;
	int linenoA;       // the startline of the hunk in the A file
	int linenoB;       // the startline fo the hunk in the B file

private:
	QStringList sourceLines;
	QStringList destinationLines;

};

#endif

