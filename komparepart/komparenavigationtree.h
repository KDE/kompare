/***************************************************************************
                                komparenavigationtree.h  -  description
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

#ifndef KOMPARENAVIGATIONTREE_H
#define KOMPARENAVIGATIONTREE_H

#include <qptrlist.h>
#include <qptrdict.h>

#include <klistview.h>

class KompareModelList;
class KomparePart;

class KompareNavigationTree : public KListView
{
	Q_OBJECT

public:
	KompareNavigationTree( KompareModelList* models, QWidget* parent = 0L, const char* name = 0L );
	virtual ~KompareNavigationTree();

public slots:
	void slotSetSelection( int model, int diff );

signals:
	void selectionChanged( int model, int diff );

private slots:
	void slotSelectionChanged( QListViewItem* item );
	void buildTree();
	void slotAppliedChanged( const Difference* d );
	
private:
	void           setItemText( QListViewItem* item, const Difference* d );
	QListViewItem* firstItem();
	QListViewItem* lastItem();

	KompareModelList*          m_models;
	QListViewItem*           m_rootItem;
	QPtrDict<QListViewItem>  m_itemDict;
};

#endif
