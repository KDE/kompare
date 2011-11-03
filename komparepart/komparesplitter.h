/***************************************************************************
                                komparesplitter.h
                                -----------------
        begin                   : Wed Jan 14 2004
        Copyright 2004      Jeff Snyder  <jeff@caffeinated.me.uk>
        Copyright 2007-2011 Kevin Kofler <kevin.kofler@chello.at>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#ifndef _KOMPARESPLITTER_H_
#define _KOMPARESPLITTER_H_

#include <QtGui/QSplitter>

#include "komparemodellist.h"

class QSplitterHandle;
class QTimer;
class QScrollBar;
class QWheelEvent;
class QKeyEvent;

namespace Diff2 {
class DiffModel;
class Difference;
}
class ViewSettings;

class KompareListView;
class KompareConnectWidget;

class KompareSplitter : public QSplitter
{
	Q_OBJECT

public:
	KompareSplitter(ViewSettings *settings, QWidget *parent);
	~KompareSplitter();

signals:
	void configChanged();

	void scrollViewsToId( int id );
	void setXOffset( int x );

	void selectionChanged( const Diff2::Difference* diff );

public slots:
	void slotScrollToId( int id );
	void slotDelayedUpdateScrollBars();
	void slotUpdateScrollBars();
	void slotDelayedUpdateVScrollValue();
	void slotUpdateVScrollValue();
	void keyPressEvent( QKeyEvent* e );

	void slotApplyDifference( bool apply );
	void slotApplyAllDifferences( bool apply );
	void slotApplyDifference( const Diff2::Difference* diff, bool apply );

	void slotSetSelection( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void slotSetSelection( const Diff2::Difference* diff );

	void slotDifferenceClicked( const Diff2::Difference* diff );

	void slotConfigChanged();

protected:
	void moveSplitter( QCOORD pos, int id );
	void wheelEvent( QWheelEvent* e );

	ViewSettings* settings() const { return m_settings; }

protected slots:
	void slotDelayedRepaintHandles();
	void slotRepaintHandles();
	void timerTimeout();

private:
	// override from QSplitter
	QSplitterHandle* createHandle();

	void               setCursor( int id, const QCursor& cursor );
	void               unsetCursor( int id );

protected:
	KompareListView* listView( int index );
	KompareConnectWidget* connectWidget( int index );

private:

	// Scrollbars. all this just for the goddamn scrollbars. i hate them.
	int  scrollId();
	int  lineHeight();
	int  pageSize();
	bool needVScrollBar();
	int  minVScrollId();
	int  maxVScrollId();
	bool needHScrollBar();
	int  maxHScrollId();
	int  maxContentsX();
	int  minVisibleWidth();

	QTimer*            m_scrollTimer;
	bool               m_restartTimer;
	int                m_scrollTo;

	ViewSettings*      m_settings;
	QScrollBar*        m_vScroll;
	QScrollBar*        m_hScroll;

	friend class KompareConnectWidgetFrame;
};
#endif //_KOMPARESPLITTER_H_
