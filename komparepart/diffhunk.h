#ifndef DIFFHUNK_H
#define DIFFHUNK_H

#include <qlist.h>

class Difference;

class DiffHunk
{
public:
	// We need a constructor without arguments for contextdiff
	DiffHunk();
	// This constructor can be used for the other formats
	DiffHunk( int lineStartA, int lineStartB );
	~DiffHunk();

	QString asString() const;
	const QList<Difference>& getDifferences() const { return differences; };

	void add( Difference* diff );
	
	int lineStartA;
	int lineStartB;

//private:
	QList<Difference> differences;
};

#endif
