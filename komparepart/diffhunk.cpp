
#include <klocale.h>

#include "diffhunk.h"
#include "difference.h"

DiffHunk::DiffHunk()
{
}

DiffHunk::DiffHunk( int lineStartA, int lineStartB )
{
	this->lineStartA = lineStartA;
	this->lineStartB = lineStartB;
}

DiffHunk::~DiffHunk()
{
}

QString DiffHunk::asString() const
{
	return i18n("Source line %1, Destination line %2").arg(lineStartA).arg(lineStartB);
}

void DiffHunk::add( Difference* diff )
{
	differences.append( diff );
}
