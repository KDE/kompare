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

#ifndef PREFSBASE_H
#define PREFSBASE_H

#include <qsize.h>
#include <qwidget.h>

#include <ktabctl.h>
#include <kconfig.h>

#include "settingsbase.h"

class PrefsBase : public KTabCtl
{
Q_OBJECT
public: 
	PrefsBase( QWidget* );
	~PrefsBase();

public:
	/** No descriptions */
	QSize sizeHintForWidget( QWidget* widget );
	/** No descriptions */
	virtual void restore();
	/** No descriptions */
	virtual void apply();
	/** No descriptions */
	virtual void setDefaults();
};

#endif
