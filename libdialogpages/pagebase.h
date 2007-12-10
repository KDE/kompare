/***************************************************************************
                                prefsbase.h  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001 by Otto Bruggeman
                                  and John Firebaugh
        email                   : otto.bruggeman@home.nl
                                  jfirebaugh@kde.org
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

#include <qsize.h>

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
