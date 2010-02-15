/***************************************************************************
                                komparelistview.h
                                -----------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
        Copyright 2004      Jeff Snyder    <jeff@caffeinated.me.uk>
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

#include <QHash>
#include <QLabel>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QFrame>

#include <k3listview.h>

namespace Diff2 {
class DiffModel;
class DiffHunk;
class Difference;
class DifferenceString;
}
class ViewSettings;
class KompareSplitter;
class KompareListView;
class KompareListViewItem;
class KompareListViewDiffItem;
class KompareListViewLineContainerItem;

class KompareListView : public K3ListView
{
	Q_OBJECT

public:
	KompareListView( bool isSource, ViewSettings* settings, QWidget* parent, const char* name = 0 );
	virtual ~KompareListView();

	KompareListViewItem* itemAtIndex( int i );
	int                  firstVisibleDifference();
	int                  lastVisibleDifference();
	QRect                itemRect( int i );
	int                  minScrollId();
	int                  maxScrollId();
	int                  contentsWidth();

	bool                 isSource() const { return m_isSource; };
	ViewSettings*        settings() const { return m_settings; };

	void setSelectedDifference( const Diff2::Difference* diff, bool scroll );

public slots:
	void slotSetSelection( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void slotSetSelection( const Diff2::Difference* diff );
	void setXOffset( int x );
	void scrollToId( int id );
	int  scrollId();
	void slotApplyDifference( bool apply );
	void slotApplyAllDifferences( bool apply );
	void slotApplyDifference( const Diff2::Difference* diff, bool apply );

signals:
	void differenceClicked( const Diff2::Difference* diff );
	void applyDifference( bool apply );
	void resized();

protected:
	void wheelEvent( QWheelEvent* e );
	void resizeEvent( QResizeEvent* e );
	void contentsMousePressEvent ( QMouseEvent * e );
	void contentsMouseDoubleClickEvent ( QMouseEvent* );
	void contentsMouseReleaseEvent ( QMouseEvent * ) {};
	void contentsMouseMoveEvent ( QMouseEvent * ) {};
	void renumberLines( void );

private:
	QList<KompareListViewDiffItem*> m_items;
	QHash<const Diff2::Difference*, KompareListViewDiffItem*> m_itemDict;
	bool                              m_isSource;
	ViewSettings*                     m_settings;
	int                               m_scrollId;
	int                               m_maxMainWidth;
	const Diff2::DiffModel*           m_selectedModel;
	const Diff2::Difference*          m_selectedDifference;
};

class KompareListViewFrame : public QFrame
{
	Q_OBJECT

public:
	KompareListViewFrame( bool isSource, ViewSettings* settings, KompareSplitter* parent, const char* name = 0 );
	virtual ~KompareListViewFrame() {};
	KompareListView*     view() { return &m_view; };

public slots:
	void slotSetModel( const Diff2::DiffModel* model );

private:
	KompareListView      m_view;
	QLabel               m_label;
	QVBoxLayout          m_layout;
};


class KompareListViewItem : public Q3ListViewItem
{
public:
	KompareListViewItem( KompareListView* parent );
	KompareListViewItem( KompareListView* parent, KompareListViewItem* after );
	KompareListViewItem( KompareListViewItem* parent );
	KompareListViewItem( KompareListViewItem* parent, KompareListViewItem* after );

	void paintFocus( QPainter* p, const QColorGroup& cg, const QRect& r );
	int scrollId() { return m_scrollId; };

	virtual int maxHeight() = 0;
	virtual int rtti(void) const = 0;

	KompareListView* kompareListView() const;

	enum ListViewItemType { Diff = 1001, Container = 1002, Line = 1003, Blank = 1004, Hunk = 1005 };

private:
	int     m_scrollId;
};

class KompareListViewDiffItem : public KompareListViewItem
{
public:
	KompareListViewDiffItem( KompareListView* parent, Diff2::Difference* difference );
	KompareListViewDiffItem( KompareListView* parent, KompareListViewItem* after, Diff2::Difference* difference );
	~KompareListViewDiffItem();

	void setup();
	void setSelected( bool b );
	void applyDifference( bool apply );

	Diff2::Difference* difference() { return m_difference; };

	virtual int maxHeight();
	virtual int rtti(void) const { return Diff; };

private:
	void init();
	void setVisibility();

private:
	Diff2::Difference* m_difference;
	KompareListViewLineContainerItem* m_sourceItem;
	KompareListViewLineContainerItem* m_destItem;
};

class KompareListViewLineItem;
class KompareListViewBlankLineItem;

class KompareListViewLineContainerItem : public KompareListViewItem
{
public:
	KompareListViewLineContainerItem( KompareListViewDiffItem* parent, bool isSource );
	~KompareListViewLineContainerItem();

	void setup();
	virtual int maxHeight() { return 0; }
	virtual int rtti(void) const { return Container; };
	KompareListViewDiffItem* diffItemParent() const;

private:
	int lineCount() const;
	int lineNumber() const;
	Diff2::DifferenceString* lineAt( int i ) const;

private:
	QList<KompareListViewLineItem *> m_lineItemList;
	KompareListViewBlankLineItem* m_blankLineItem;
	bool m_isSource;
};

class KompareListViewLineItem : public KompareListViewItem
{
public:
	KompareListViewLineItem( KompareListViewLineContainerItem* parent, int line, Diff2::DifferenceString* text );
	~KompareListViewLineItem();

	virtual void setup();
	virtual int maxHeight() { return 0; }
	virtual int rtti(void) const { return Line; };

	virtual void paintCell( QPainter* p, const QColorGroup& cg, int column, int width, int align );
	virtual void paintText( QPainter* p, const QColor& bg, int column, int width, int align );

	KompareListViewDiffItem* diffItemParent() const;

private:
	void expandTabs(QString& text, int tabstop, int startPos = 0) const;

private:
	Diff2::DifferenceString* m_text;
};

class KompareListViewBlankLineItem : public KompareListViewLineItem
{
public:
	KompareListViewBlankLineItem( KompareListViewLineContainerItem* parent );

	void setup();
	virtual int rtti(void) const { return Blank; };

	void paintText( QPainter* p, const QColor& bg, int column, int width, int align );
};

class KompareListViewHunkItem : public KompareListViewItem
{
public:
	KompareListViewHunkItem( KompareListView* parent, Diff2::DiffHunk* hunk, bool zeroHeight = false );
	KompareListViewHunkItem( KompareListView* parent, KompareListViewItem* after, Diff2::DiffHunk* hunk, bool zeroHeight= false );
	~KompareListViewHunkItem();

	void setup();
	void paintCell( QPainter* p, const QColorGroup& cg, int column, int width, int align );

	virtual int maxHeight();
	virtual int rtti(void) const { return Hunk; };

private:
	bool             m_zeroHeight;
	Diff2::DiffHunk* m_hunk;
};

#endif
