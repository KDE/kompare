/***************************************************************************
                                komparelistview.h  -  description
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

#ifndef KOMPARELISTVIEW_H
#define KOMPARELISTVIEW_H

#include <qptrlist.h>
#include <qptrdict.h>

#include <klistview.h>

class DiffModel;
class DiffHunk;
class Difference;
class GeneralSettings;
class KompareListViewItem;
class KompareListViewDiffItem;
class KompareModelList;

class KompareListView : public KListView
{
	Q_OBJECT

public:
	KompareListView( bool isSource, GeneralSettings* settings, QWidget* parent, const char* name = 0 );
	virtual ~KompareListView();

	void setSelectedDifference( const Difference* diff, bool scroll = true );
	
	KompareListViewItem* itemAtIndex( int i );
	int                  firstVisibleDifference();
	int                  lastVisibleDifference();
	QRect                itemRect( int i );
	int                  minScrollId();
	int                  maxScrollId();
	
	bool                 isSource() const { return m_isSource; };
	GeneralSettings*     settings() const { return m_settings; };
	
public slots:
	void slotSetSelection( const DiffModel* model, const Difference* diff );
	void slotSetSelection( const Difference* diff );
	void setXOffset( int x );
	void scrollToId( int id );
	int  scrollId();
	void slotApplyDifference( bool apply );
	void slotApplyAllDifferences( bool apply );

signals:
	void selectionChanged( const Difference* diff );

protected:
	void resizeEvent( QResizeEvent* e );
	void wheelEvent( QWheelEvent* e );
	void contentsMousePressEvent ( QMouseEvent * e );
	void contentsMouseDoubleClickEvent ( QMouseEvent* ) {};
	void contentsMouseReleaseEvent ( QMouseEvent * ) {};
	void contentsMouseMoveEvent ( QMouseEvent * ) {};

protected slots:
	void updateMainColumnWidth();

private:
	QPtrList<KompareListViewDiffItem>  m_items;
	QPtrDict<KompareListViewDiffItem>  m_itemDict;
	bool                               m_isSource;
	GeneralSettings*                   m_settings;
	int                                m_maxScrollId;
	int                                m_scrollId;
	int                                m_maxMainWidth;
	const DiffModel*                   m_selectedModel;
	const Difference*                  m_selectedDifference;
};

class KompareListViewItem : public QListViewItem
{
public:
	KompareListViewItem( QListView* parent );
	KompareListViewItem( QListView* parent, KompareListViewItem* after );
	
	void paintFocus( QPainter* p, const QColorGroup& cg, const QRect& r );
	int scrollId() { return m_scrollId; };
	
	virtual int maxHeight() = 0;
	
private:
	int m_scrollId;
};

class KompareListViewDiffItem : public KompareListViewItem
{
public:
	KompareListViewDiffItem( KompareListView* parent, Difference* difference );
	KompareListViewDiffItem( KompareListView* parent, KompareListViewItem* after, Difference* difference );
	
	void setup();
	void paintCell( QPainter* p, const QColorGroup& cg, int column, int width, int align );
	
	Difference* difference() { return m_difference; };
	
	int maxHeight();
	int maxMainWidth() const;
	
	void applyDifference( bool apply );

private:
	bool isSource() const { return m_listView->isSource(); };
	int lineCount() const;

	KompareListView* m_listView;
	Difference*      m_difference;
};

class KompareListViewHunkItem : public KompareListViewItem
{
public:
	KompareListViewHunkItem( KompareListView* parent, DiffHunk* hunk );
	KompareListViewHunkItem( KompareListView* parent, KompareListViewItem* after, DiffHunk* hunk );
	
	void setup();
	void paintCell( QPainter* p, const QColorGroup& cg, int column, int width, int align );
	
	int maxHeight();
	
private:	
	DiffHunk* m_hunk;
};

#endif
