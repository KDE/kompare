/***************************************************************************
                                kdiffnavigationtree.h  -  description
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

#ifndef KDIFFNAVIGATIONTREE_H
#define KDIFFNAVIGATIONTREE_H

#include <qlist.h>

#include <klistview.h>

class KDiffPart;

class KDiffNavigationTree : public KListView
{
	Q_OBJECT

public:
	KDiffNavigationTree( KDiffModelList* models, QWidget* parent = 0L, const char* name = 0L );
	virtual ~KDiffNavigationTree();

public slots:
	void slotSetSelection( int model, int diff );

signals:
	void selectionChanged( int model, int diff );

private slots:
	void slotSelectionChanged( QListViewItem* item );
	void slotAddModel( DiffModel * );

private: // Private methods
  /** No descriptions */
  QListViewItem* lastItem();
};

#endif
