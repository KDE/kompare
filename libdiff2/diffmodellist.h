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

#include <qvaluelist.h> // include for the base class

#include "diffmodel.h"

namespace Diff2
{

typedef QValueListIterator<DiffModel*> DiffModelListIterator;
typedef QValueListConstIterator<DiffModel*> DiffModelListConstIterator;

class DiffModelList : public QValueList<DiffModel*>
{
public:
	DiffModelList() {}
	DiffModelList( const DiffModelList &list ) : QValueList<DiffModel*>( list ) {}
	virtual ~DiffModelList()
	{
		clear();
	}

public:
	virtual void sort();

}; // End of class DiffModelList

} // End of Namespace Diff2

#endif // DIFFMODELLIST_H
