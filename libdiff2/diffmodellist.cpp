/*******************************************************************************
**
** Filename   : diffmodellist.cpp
** Created on : 26 march, 2004
** Copyright  : (c) 2004 Otto Bruggeman
** Email      : bruggie@home.nl
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
*******************************************************************************/

#include <kdebug.h>

#include "diffmodellist.h"

using namespace Diff2;

DiffModel* DiffModelList::operator[]( size_type i )
{
	return (DiffModel*)QValueList<DiffModel*>::operator[]( i );
}

const DiffModel* DiffModelList::operator[]( size_type i ) const
{
	return (DiffModel*)QValueList<DiffModel*>::operator[]( i );
}

DiffModelList& DiffModelList::operator=( const DiffModelList &list )
{
	return ( DiffModelList& )QValueList<DiffModel*>::operator=( list );
}

int DiffModelList::compareItems( QPtrCollection::Item s1, QPtrCollection::Item s2 )
{
	return ( (DiffModel*)s1 )->localeAwareCompareSource( (DiffModel*)s2 );
}

void DiffModelList::debugPrint()
{
	DiffModelListConstIterator modelIt = begin();
	while ( modelIt != end() )
	{
		kdDebug(8101) << "Model source = " << (*modelIt)->source() << endl;
		++modelIt;
	}
}

void DiffModelList::sort()
{
	// Sort with a silly algorithm like bubblesort
	bool toggled = false;
	int size = count();

	if ( size < 2 )
		return;

	do
	{
		DiffModelListIterator modelIt = begin();
		DiffModelListIterator first, second;
		toggled = false;
		do
		{
			first = modelIt;
			second = ++modelIt;

			if ( modelIt == end() )
				break;

			Diff2::DiffModel* model1 = *first;
			Diff2::DiffModel* model2 = *second;

//			kdDebug(8101) << "model1 = " << model1->source() << ", model2 = " << model2->source() << endl;

			if ( *model1 > model2 )
			{
				kdDebug(8101) << "model1 ( " << model1->source() << " ) is bigger than model2 ( " << model2->source() << " )" << endl;
				remove( second );
				modelIt = insert( first, model2 );
				toggled = true;
			}
		}
		while ( modelIt != end() );
	}
	while ( toggled );
}

