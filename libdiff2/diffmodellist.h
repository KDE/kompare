/*******************************************************************************
**
** Filename   : diffmodellist.h
** Created on : 24 januari, 2004
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

#ifndef DIFFMODELLIST_H
#define DIFFMODELLIST_H

#include <qptrlist.h> // include for the base class

#include <kdebug.h>

#include "diffmodel.h"

namespace Diff2
{

class DiffModelList : public QPtrList<DiffModel>
{
public:
	DiffModelList() {}
	DiffModelList( const DiffModelList &list ) : QPtrList<DiffModel>( list ) {}
	~DiffModelList() { this->clear(); }

public:
	DiffModelList& operator=( const DiffModelList &list )
	{
		return ( DiffModelList& )QPtrList<DiffModel>::operator=( list );
	}

	virtual int compareItems( QPtrCollection::Item s1, QPtrCollection::Item s2 )
	{
		return ( (DiffModel*)s1 )->localeAwareCompareSource( (DiffModel*)s2 );
	}
}; // End of class DiffModelList

} // End of Namespace Diff2

#endif // DIFFMODELLIST_H
