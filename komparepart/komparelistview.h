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
#include <QItemDelegate>

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

class KompareListViewItemDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	KompareListViewItemDelegate( QObject* parent );
	virtual ~KompareListViewItemDelegate();
	virtual void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
	virtual QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const;
	void paintDefault( QPainter* painter, const QStyleOptionViewItem& option, int column, KompareListViewItem* item ) const;
	void drawDisplayDefault( QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect, const QString& text ) const;
protected:
	virtual void drawDisplay( QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect, const QString& text ) const;
	virtual void drawFocus( QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect ) const;

private:
	mutable KompareListViewItem* m_item;
	mutable int m_column;
};

class KompareListViewItem : public QTreeWidgetItem
{
public:
	KompareListViewItem( KompareListView* parent );
	KompareListViewItem( KompareListView* parent, KompareListViewItem* after );
	KompareListViewItem( KompareListViewItem* parent );
	KompareListViewItem( KompareListViewItem* parent, KompareListViewItem* after );

	virtual void setup() {}
	virtual void paintCell( QPainter* p, const QStyleOptionViewItem& option, int column, const KompareListViewItemDelegate* delegate );
	virtual void paintText( QPainter* p, const QStyleOptionViewItem& option, const QRect& rect, const QString& text, int column, const KompareListViewItemDelegate* delegate );

	void repaint();
	int height() const;
	void setHeight( int h );
	bool isCurrent() const;
	int scrollId() const { return m_scrollId; };

	virtual int maxHeight() = 0;
	virtual int rtti(void) const = 0;

	KompareListView* kompareListView() const;

	enum ListViewItemType { Diff = 1001, Container = 1002, Line = 1003, Blank = 1004, Hunk = 1005 };

private:
	int     m_scrollId;
	int     m_height;
};

class KompareListViewDiffItem : public KompareListViewItem
{
public:
	KompareListViewDiffItem( KompareListView* parent, Diff2::Difference* difference );
	KompareListViewDiffItem( KompareListView* parent, KompareListViewItem* after, Diff2::Difference* difference );
	~KompareListViewDiffItem();

	virtual void setup();
	void applyDifference( bool apply );

	Diff2::Difference* difference() { return m_difference; };

	virtual int maxHeight();
	virtual int rtti(void) const { return Diff; };

	virtual void paintCell( QPainter* p, const QStyleOptionViewItem& option, int column, const KompareListViewItemDelegate* delegate );

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

	virtual void setup();
	virtual int maxHeight() { return 1; }
	virtual int rtti(void) const { return Container; };

	virtual void paintCell( QPainter* p, const QStyleOptionViewItem& option, int column, const KompareListViewItemDelegate* delegate );

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

	virtual void paintCell( QPainter* p, const QStyleOptionViewItem& option, int column, const KompareListViewItemDelegate* delegate );

	KompareListViewDiffItem* diffItemParent() const;

private:
	void paintTextP( QPainter* p, const QColor& bg, int column, int width, int align );
	void expandTabs(QString& text, int tabstop, int startPos = 0) const;

private:
	Diff2::DifferenceString* m_text;
};

class KompareListViewBlankLineItem : public KompareListViewLineItem
{
public:
	KompareListViewBlankLineItem( KompareListViewLineContainerItem* parent );

	virtual void setup();
	virtual int rtti(void) const { return Blank; };

	virtual void paintText( QPainter* p, const QStyleOptionViewItem& option, const QRect& rect, const QString& text, int column, const KompareListViewItemDelegate* delegate );
};

class KompareListViewHunkItem : public KompareListViewItem
{
public:
	KompareListViewHunkItem( KompareListView* parent, Diff2::DiffHunk* hunk, bool zeroHeight = false );
	KompareListViewHunkItem( KompareListView* parent, KompareListViewItem* after, Diff2::DiffHunk* hunk, bool zeroHeight= false );
	~KompareListViewHunkItem();

	virtual void setup();
	virtual void paintCell( QPainter* p, const QStyleOptionViewItem& option, int column, const KompareListViewItemDelegate* delegate );

	virtual int maxHeight();
	virtual int rtti(void) const { return Hunk; };

private:
	bool             m_zeroHeight;
	Diff2::DiffHunk* m_hunk;
};

#endif
