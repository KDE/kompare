/***************************************************************************
                          kdiffmodellist.cpp  -  description
                             -------------------
    begin                : Tue Jun 26 2001
    copyright            : (C) 2001 by John Firebaugh
    email                : jfirebaugh@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kdiffmodellist.h"

KDiffModelList::KDiffModelList()
	: QObject()
{
}

KDiffModelList::~KDiffModelList()
{
}

void KDiffModelList::addModel( DiffModel* model )
{
	m_models.append( model );
	emit modelAdded( model );
}
