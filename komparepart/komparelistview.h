/***************************************************************************
                                kdifflistview.h  -  description
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

#ifndef KDIFFLISTVIEW_H
#define KDIFFLISTVIEW_H

#include <qptrlist.h>
#include <qptrdict.h>

#include <klistview.h>

class DiffModel;
class DiffHunk;
class Difference;
class GeneralSettings;
class KDiffListViewItem;
class KDiffListViewDiffItem;
class KDiffModelList;

class KDiffListView : public KListView
{
	Q_OBJECT

public:
	KDiffListView( KDiffModelList* models, bool isSource, GeneralSettings* settings, QWidget* parent, const char* name = 0 );
	virtual ~KDiffListView();

	void setSelectedModel( int model );
	void setSelectedDifference( int diff, bool scroll = true );
	
	KDiffListViewItem* itemAtIndex( int i );
	int                firstVisibleDifference();
	int                lastVisibleDifference();
	QRect              itemRect( int i );
	int                minScrollId();
	int                maxScrollId();
	
	bool               isSource() const { return m_isSource; };
	GeneralSettings*   settings() const { return m_settings; };
	
public slots:
	void slotSetSelection( int model, int diff );
	void setXOffset( int x );
	void scrollToId( int id );
	int  scrollId();
	
signals:
	void selectionChanged( int model, int diff );
	
protected:
	void resizeEvent( QResizeEvent* e );
	void contentsMousePressEvent ( QMouseEvent * e );
	void contentsMouseDoubleClickEvent ( QMouseEvent* ) {};
	void contentsMouseReleaseEvent ( QMouseEvent * ) {};
	void contentsMouseMoveEvent ( QMouseEvent * ) {};
	
protected slots:
	void slotAppliedChanged( const Difference* d );
	
	void updateMainColumnWidth();
	
private:
	KDiffModelList*                  m_models;
	QPtrList<KDiffListViewDiffItem>  m_items;
	QPtrDict<KDiffListViewDiffItem>  m_itemDict;
	bool                             m_isSource;
	int                              m_selectedModel;
	GeneralSettings*                 m_settings;
	int                              m_maxScrollId;
	int                              m_scrollId;
	int                              m_maxMainWidth;
};

class KDiffListViewItem : public QListViewItem
{
public:
	KDiffListViewItem( QListView* parent );
	KDiffListViewItem( QListView* parent, KDiffListViewItem* after );
	
	void paintFocus( QPainter* p, const QColorGroup& cg, const QRect& r );
	int scrollId() { return m_scrollId; };
	
	virtual int maxHeight() = 0;
	
private:
	int                  m_scrollId;
};

class KDiffListViewDiffItem : public KDiffListViewItem
{
public:
	KDiffListViewDiffItem( KDiffListView* parent, Difference* difference );
	KDiffListViewDiffItem( KDiffListView* parent, KDiffListViewItem* after, Difference* difference );
	
	void setup();
	void paintCell( QPainter* p, const QColorGroup& cg, int column, int width, int align );
	
	Difference* difference() { return m_difference; };
	
	int maxHeight();
	int maxMainWidth() const;
	
	void appliedChanged();
	
private:
	bool isSource() const { return m_listView->isSource(); };
	int lineCount() const;
	
	KDiffListView*       m_listView;
	Difference*          m_difference;
};

class KDiffListViewHunkItem : public KDiffListViewItem
{
public:
	KDiffListViewHunkItem( KDiffListView* parent, DiffHunk* hunk );
	KDiffListViewHunkItem( KDiffListView* parent, KDiffListViewItem* after, DiffHunk* hunk );
	
	void setup();
	void paintCell( QPainter* p, const QColorGroup& cg, int column, int width, int align );
	
	int maxHeight();
	
private:	
	DiffHunk*          m_hunk;
};

#endif
