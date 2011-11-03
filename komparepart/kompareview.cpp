/**************************************************************************
**                              kompareview.cpp
**                              ---------------
**      begin                   : Thu Nov 3 2011
**      Copyright 2011 Kevin Kofler <kevin.kofler@chello.at>
***************************************************************************/

/**************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

// associated header
#include "kompareview.h"

// kompare
#include "komparesplitter.h"

KompareView::KompareView(ViewSettings *settings, QWidget *parent) :
	QFrame( parent )
{
	setObjectName( "scrollFrame" );
	m_splitter = new KompareSplitter( settings, this );
}
