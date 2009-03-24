/*******************************************************************************
**
** Filename   : diffmodellist.cpp
** Created on : 26 march, 2004
** Copyright 2004 Otto Bruggeman <bruggie@gmail.com>
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

#include "diffmodellist.h"

#include <q3tl.h>
#include <kdebug.h>

using namespace Diff2;

void DiffModelList::sort()
{
	// This is not going to be performance critical so implementing a very simple bubblesort based on qbubblesortrcode
	// Goto last element;
	DiffModelListIterator last = end();
	DiffModelListIterator e = end();
	DiffModelListIterator b = begin();

	// empty list
	if ( b == e )
		return;

	--last;
	// only one element ?
	if ( last == b )
		return;

	// So we have at least two elements in here
	while (b != last) {
		bool swapped = false;
		DiffModelListIterator swapPos = b;
		DiffModelListIterator x = e;
		DiffModelListIterator y = x;
		y--;
		do {
			--x;
			--y;
			if ( (*(*x) < *(*y)) ) {
				swapped = true;
				DiffModel* temp = *x;
				*x = *y;
				*y = temp;
				swapPos = y;
			}
		} while (y != b);
		if (!swapped)
			return;
		b = swapPos;
		++b;
	}
}

