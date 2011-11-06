/***************************************************************************
                                komparelistview.h
                                -----------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2004,2009 Otto Bruggeman <bruggie@gmail.com>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
        Copyright 2004      Jeff Snyder    <jeff@caffeinated.me.uk>
        Copyright 2011      Kevin Kofler   <kevin.kofler@chello.at>
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
#include <QTreeWidget>
#include <QStyledItemDelegate>

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
class KompareListViewItemDelegate;
class KompareListViewDiffItem;
class KompareListViewLineContainerItem;

class KompareListView : public QTreeWidget
{
	Q_OBJECT

	friend class KompareListViewItemDelegate;

public:
	KompareListView( bool isSource, ViewSettings* settings, QWidget* parent, const char* name = 0 );
	virtual ~KompareListView();

	KompareListViewItem* itemAtIndex( int i );
	int                  firstVisibleDifference();
	int                  lastVisibleDifference();
	QRect                itemRect( int i );
	int                  minScrollId();
	int                  maxScrollId();
	int                  contentsHeight();
	int                  contentsWidth();
	int                  visibleHeight();
	int                  visibleWidth();
	int                  contentsX();
	int                  contentsY();
	int                  nextPaintOffset() const;
	void                 setNextPaintOffset(int offset);

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
	virtual void wheelEvent( QWheelEvent* e );
	virtual void resizeEvent( QResizeEvent* e );
	virtual void mousePressEvent ( QMouseEvent * e );
	virtual void mouseDoubleClickEvent ( QMouseEvent* );
	virtual void mouseReleaseEvent ( QMouseEvent * ) {};
	virtual void mouseMoveEvent ( QMouseEvent * ) {};

private:
	QRect totalVisualItemRect( QTreeWidgetItem* item );
	void renumberLines( void );

	QList<KompareListViewDiffItem*> m_items;
	QHash<const Diff2::Difference*, KompareListViewDiffItem*> m_itemDict;
	bool                              m_isSource;
	ViewSettings*                     m_settings;
	int                               m_scrollId;
	int                               m_maxMainWidth;
	const Diff2::DiffModel*           m_selectedModel;
	const Diff2::Difference*          m_selectedDifference;
	int                               m_nextPaintOffset;
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

class KompareListViewItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	KompareListViewItemDelegate( QObject* parent );
	virtual ~KompareListViewItemDelegate();
	virtual void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
	virtual QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const;
};

class KompareListViewItem : public QTreeWidgetItem
{
public:
	KompareListViewItem( KompareListView* parent, int type );
	KompareListViewItem( KompareListView* parent, KompareListViewItem* after, int type );
	KompareListViewItem( KompareListViewItem* parent, int type );
	KompareListViewItem( KompareListViewItem* parent, KompareListViewItem* after, int type );

	virtual void paintCell( QPainter* p, const QStyleOptionViewItem& option, int column );

	void repaint();
	int height() const;
	void setHeight( int h );
	int paintHeight() const;
	int paintOffset() const;
	bool isCurrent() const;
	int scrollId() const { return m_scrollId; };

	virtual int maxHeight() = 0;

	KompareListView* kompareListView() const;

	enum ListViewItemType { Diff = QTreeWidgetItem::UserType + 1, Container = QTreeWidgetItem::UserType + 2, Line = QTreeWidgetItem::UserType + 3, Blank = QTreeWidgetItem::UserType + 4, Hunk = QTreeWidgetItem::UserType + 5 };

private:
	int     m_scrollId;
	int     m_height;
	int     m_paintHeight;
	int     m_paintOffset;
};

class KompareListViewDiffItem : public KompareListViewItem
{
public:
	KompareListViewDiffItem( KompareListView* parent, Diff2::Difference* difference );
	KompareListViewDiffItem( KompareListView* parent, KompareListViewItem* after, Diff2::Difference* difference );
	~KompareListViewDiffItem();

	void applyDifference( bool apply );

	Diff2::Difference* difference() { return m_difference; };

	virtual int maxHeight();

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

	virtual int maxHeight() { return 0; }

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
	KompareListViewLineItem( KompareListViewLineContainerItem* parent, int line, Diff2::DifferenceString* text, int type );
	~KompareListViewLineItem();

	virtual int maxHeight() { return 0; }

	virtual void paintCell( QPainter* p, const QStyleOptionViewItem& option, int column );

	KompareListViewDiffItem* diffItemParent() const;

protected:
	virtual void paintText( QPainter* p, const QColor& bg, int column, int width, int align );

private:
	void init( int line, Diff2::DifferenceString* text );
	void expandTabs(QString& text, int tabstop, int startPos = 0) const;

private:
	Diff2::DifferenceString* m_text;
};

class KompareListViewBlankLineItem : public KompareListViewLineItem
{
public:
	KompareListViewBlankLineItem( KompareListViewLineContainerItem* parent );

protected:
	virtual void paintText( QPainter* p, const QColor& bg, int column, int width, int align );
};

class KompareListViewHunkItem : public KompareListViewItem
{
public:
	KompareListViewHunkItem( KompareListView* parent, Diff2::DiffHunk* hunk, bool zeroHeight = false );
	KompareListViewHunkItem( KompareListView* parent, KompareListViewItem* after, Diff2::DiffHunk* hunk, bool zeroHeight= false );
	~KompareListViewHunkItem();

	virtual void paintCell( QPainter* p, const QStyleOptionViewItem& option, int column );

	virtual int maxHeight();

private:
	bool             m_zeroHeight;
	Diff2::DiffHunk* m_hunk;
};

#endif
