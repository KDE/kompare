/**************************************************************************
**                              kompareview.h
**                              -------------
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

#ifndef _KOMPAREVIEW_H_
#define _KOMPAREVIEW_H_

#include <QtGui/QFrame>

#include "komparemodellist.h"

class ViewSettings;
class KompareSplitter;

class KompareView : public QFrame
{
	Q_OBJECT

public:
	KompareView(ViewSettings *settings, QWidget *parent);
	~KompareView() {}
	KompareSplitter *splitter() { return m_splitter; }

private:
	KompareSplitter *m_splitter;
};
#endif //_KOMPARESPLITTER_H_
