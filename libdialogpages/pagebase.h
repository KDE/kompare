/***************************************************************************
                                prefsbase.h
                                -----------
        begin                   : Sun Mar 4 2001
        Copyright 2001 Otto Bruggeman <otto.bruggeman@home.nl>
        Copyright 2001 John Firebaugh <jfirebaugh@kde.org>
        Copyright 2007 Kevin Kofler   <kevin.kofler@chello.at>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#ifndef PAGEBASE_H
#define PAGEBASE_H

#include <QtCore/QSize>

#include <kvbox.h>

#include "kompare.h"

class PageBase : public KVBox
{
Q_OBJECT
public:
	PageBase();
	~PageBase();

public:
	/** No descriptions */
	QSize        sizeHintForWidget( QWidget* widget );
	/** No descriptions */
	virtual void restore();
	/** No descriptions */
	virtual void apply();
	/** No descriptions */
	virtual void setDefaults();
};

#endif
