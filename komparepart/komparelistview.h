/***************************************************************************
                                komparelistview.h  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2003 by Otto Bruggeman
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

class Diff2::DiffModel;
class Diff2::DiffHunk;
class Diff2::Difference;
class ViewSettings;
class KompareListViewItem;
class KompareListViewDiffItem;
class KompareListViewLineContainerItem;
class Diff2::KompareModelList;

class KompareListView : public KListView
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

	bool                 isSource() const { return m_isSource; };
	ViewSettings*        settings() const { return m_settings; };

	void setSelectedDifference( const Diff2::Difference* diff, bool scroll );

	const QString& spaces() const { return m_spaces; };

public slots:
	void slotSetSelection( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void slotSetSelection( const Diff2::Difference* diff );
	void setXOffset( int x );
	void scrollToId( int id );
	int  scrollId();
	void slotApplyDifference( bool apply );
	void slotApplyAllDifferences( bool apply );
	void slotApplyDifference( const Diff2::Difference* diff, bool apply );
	void slotConfigChanged();

protected slots:
	void slotDelayedScrollToId();

signals:
	void differenceClicked( const Diff2::Difference* diff );
	void applyDifference( bool apply );

protected:
	void wheelEvent( QWheelEvent* e );
	void contentsMousePressEvent ( QMouseEvent * e );
	void contentsMouseDoubleClickEvent ( QMouseEvent* );
	void contentsMouseReleaseEvent ( QMouseEvent * ) {};
	void contentsMouseMoveEvent ( QMouseEvent * ) {};

private:
	QPtrList<KompareListViewDiffItem> m_items;
	QPtrDict<KompareListViewDiffItem> m_itemDict;
	bool                              m_isSource;
	ViewSettings*                     m_settings;
	int                               m_scrollId;
	int                               m_maxMainWidth;
	const Diff2::DiffModel*           m_selectedModel;
	const Diff2::Difference*          m_selectedDifference;
	QString                           m_spaces;
	int                               m_idToScrollTo;
};

class KompareListViewItem : public QListViewItem
{
public:
	KompareListViewItem( KompareListView* parent );
	KompareListViewItem( KompareListView* parent, KompareListViewItem* after );
	KompareListViewItem( KompareListViewItem* parent );
	KompareListViewItem( KompareListViewItem* parent, KompareListViewItem* after );

	void paintFocus( QPainter* p, const QColorGroup& cg, const QRect& r );
	int scrollId() { return m_scrollId; };

	virtual int maxHeight() = 0;

	KompareListView* kompareListView() const;

private:
	int     m_scrollId;
};

class KompareListViewDiffItem : public KompareListViewItem
{
public:
	KompareListViewDiffItem( KompareListView* parent, Diff2::Difference* difference );
	KompareListViewDiffItem( KompareListView* parent, KompareListViewItem* after, Diff2::Difference* difference );

	void setup();
	void setSelected( bool b );
	void applyDifference( bool apply );

	Diff2::Difference* difference() { return m_difference; };

	int maxHeight();

private:
	void init();
	void setVisibility();

	Diff2::Difference* m_difference;
	KompareListViewLineContainerItem* m_sourceItem;
	KompareListViewLineContainerItem* m_destItem;
};

class KompareListViewLineContainerItem : public KompareListViewItem
{
public:
	KompareListViewLineContainerItem( KompareListViewDiffItem* parent, bool isSource );

	void setup();
	int maxHeight() { return 0; }
	KompareListViewDiffItem* diffItemParent() const;

private:
	int lineCount() const;
	int lineNumber() const;
	QString lineAt( int i ) const;

	bool m_isSource;
};

class KompareListViewLineItem : public KompareListViewItem
{
public:
	KompareListViewLineItem( KompareListViewLineContainerItem* parent, int line, const QString& text );

	virtual void setup();
	int maxHeight() { return 0; }

	virtual void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align );
	virtual void paintText( QPainter * p, const QColorGroup & cg, int column, int width, int align );

	KompareListViewDiffItem* diffItemParent() const;
};

class KompareListViewBlankLineItem : public KompareListViewLineItem
{
public:
	KompareListViewBlankLineItem( KompareListViewLineContainerItem* parent );

	void setup();

	void paintText( QPainter * p, const QColorGroup & cg, int column, int width, int align );
};

class KompareListViewHunkItem : public KompareListViewItem
{
public:
	KompareListViewHunkItem( KompareListView* parent, Diff2::DiffHunk* hunk );
	KompareListViewHunkItem( KompareListView* parent, KompareListViewItem* after, Diff2::DiffHunk* hunk );

	void setup();
	void paintCell( QPainter* p, const QColorGroup& cg, int column, int width, int align );

	int maxHeight();

private:
	Diff2::DiffHunk* m_hunk;
};

#endif
