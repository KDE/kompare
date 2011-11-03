/**************************************************************************
**                              komparesplitter.cpp
**                              -------------------
**      begin                   : Wed Jan 14 2004
**      Copyright 2004-2005 Jeff Snyder  <jeff-webcvsspam@caffeinated.me.uk>
**      Copyright 2007-2011 Kevin Kofler <kevin.kofler@chello.at>
***************************************************************************/

/**************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

// associated header
#include "komparesplitter.h"

// qt
#include <QtGui/QStyle>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtGui/QScrollBar>
#include <QtCore/QMap>
#include <QtGui/QSplitter>
#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QKeyEvent>
#include <QtGui/QGridLayout>
#include <QtGui/QResizeEvent>
#include <QtCore/QChildEvent>
#include <QtCore/QEvent>
#include <QtGui/QWheelEvent>

// kde
#include <kdebug.h>
#include <kapplication.h>
#include <kglobalsettings.h>

// kompare
#include "komparelistview.h"
#include "viewsettings.h"
#include "kompareconnectwidget.h"
#include "diffmodel.h"
#include "difference.h"

using namespace Diff2;

KompareSplitter::KompareSplitter( ViewSettings *settings, QWidget *parent ) :
	QSplitter( Qt::Horizontal, parent ),
	m_settings( settings )
{
	QFrame *scrollFrame = static_cast<QFrame *>(parent);

	// Set up the scrollFrame
	scrollFrame->setFrameStyle( QFrame::NoFrame | QFrame::Plain );
	scrollFrame->setLineWidth(scrollFrame->style()->pixelMetric(QStyle::PM_DefaultFrameWidth));
	QGridLayout *pairlayout = new QGridLayout(scrollFrame);
	pairlayout->setSpacing(0);
	pairlayout->setContentsMargins( 0, 0, 0, 0 );
	m_vScroll = new QScrollBar( Qt::Vertical, scrollFrame );
	pairlayout->addWidget( m_vScroll, 0, 1 );
	m_hScroll = new QScrollBar( Qt::Horizontal, scrollFrame );
	pairlayout->addWidget( m_hScroll, 1, 0 );

	new KompareListViewFrame(true, m_settings, this, "source");
	new KompareListViewFrame(false, m_settings, this, "destination");
	pairlayout->addWidget( this, 0, 0 );

	// set up our looks
	setLineWidth( style()->pixelMetric( QStyle::PM_DefaultFrameWidth ) );

	setHandleWidth(50);
	setChildrenCollapsible( false );
	setFrameStyle( QFrame::NoFrame );
	setSizePolicy( QSizePolicy (QSizePolicy::Ignored, QSizePolicy::Ignored ));
	setOpaqueResize( true );
	setFocusPolicy( Qt::WheelFocus );

	connect( this, SIGNAL(configChanged()), SLOT(slotConfigChanged()) );
	connect( this, SIGNAL(configChanged()), SLOT(slotDelayedRepaintHandles()) );
	connect( this, SIGNAL(configChanged()), SLOT(slotDelayedUpdateScrollBars()) );
	
	// scrolling
	connect( m_vScroll, SIGNAL(valueChanged(int)), SLOT(slotScrollToId(int)) );
	connect( m_vScroll, SIGNAL(sliderMoved(int)),  SLOT(slotScrollToId(int)) );
	connect( m_hScroll, SIGNAL(valueChanged(int)), SIGNAL(setXOffset(int)) );
	connect( m_hScroll, SIGNAL(sliderMoved(int)),  SIGNAL(setXOffset(int)) );

	m_scrollTimer=new QTimer(this);
	m_restartTimer = false;
	connect (m_scrollTimer, SIGNAL(timeout()), SLOT(timerTimeout()) );

	// we need to receive childEvents now so that d->list is ready for when
	// slotSetSelection(...) arrives
	kapp->sendPostedEvents(this, QEvent::ChildInserted);

	// init stuff
	slotUpdateScrollBars();
}

KompareSplitter::~KompareSplitter()
{
}

QSplitterHandle* KompareSplitter::createHandle()
{
	return new KompareConnectWidgetFrame(m_settings, this);
}

void KompareSplitter::slotDelayedRepaintHandles()
{
	QTimer::singleShot(0, this, SLOT(slotRepaintHandles()));
}

void KompareSplitter::slotRepaintHandles()
{
	const int end = count();
	for ( int i = 1; i < end; ++i )
		handle(i)->update();
}

void KompareSplitter::timerTimeout()
{
	if ( m_restartTimer )
		m_restartTimer = false;
	else
		m_scrollTimer->stop();

	slotDelayedRepaintHandles();

	emit scrollViewsToId( m_scrollTo );
	slotRepaintHandles();
	m_vScroll->setValue( m_scrollTo );
}

void KompareSplitter::slotScrollToId( int id )
{
	m_scrollTo = id;

	if( m_restartTimer )
		return;

	if( m_scrollTimer->isActive() )
	{
		m_restartTimer = true;
	}
	else
	{
		emit scrollViewsToId( id );
		slotRepaintHandles();
		m_vScroll->setValue( id );
		m_scrollTimer->start(30, false);
	}
}

void KompareSplitter::slotDelayedUpdateScrollBars()
{
	QTimer::singleShot( 0, this, SLOT( slotUpdateScrollBars() ) );
}

void KompareSplitter::slotUpdateScrollBars()
{
	int m_scrollDistance = m_settings->m_scrollNoOfLines * lineHeight();
	int m_pageSize = pageSize();

	if( needVScrollBar() )
	{
		m_vScroll->show();

		m_vScroll->blockSignals( true );
		m_vScroll->setRange( minVScrollId(), 
		                     maxVScrollId() );
		m_vScroll->setValue( scrollId() );
		m_vScroll->setSteps( m_scrollDistance, m_pageSize );
		m_vScroll->blockSignals( false );
	}
	else
	{
		m_vScroll->hide();
	}

	if( needHScrollBar() )
	{
		m_hScroll->show();
		m_hScroll->blockSignals( true );
		m_hScroll->setRange( 0, maxHScrollId() );
		m_hScroll->setValue( maxContentsX() );
		m_hScroll->setSteps( 10, minVisibleWidth() - 10 );
		m_hScroll->blockSignals( false );
	}
	else
	{
		m_hScroll->hide();
	}
}

void KompareSplitter::slotDelayedUpdateVScrollValue()
{
	QTimer::singleShot( 0, this, SLOT( slotUpdateVScrollValue() ) );
}

void KompareSplitter::slotUpdateVScrollValue()
{
	m_vScroll->setValue( scrollId() );
}

void KompareSplitter::keyPressEvent( QKeyEvent* e )
{
	//keyboard scrolling
	switch ( e->key() ) {
	case Qt::Key_Right:
	case Qt::Key_L:
		m_hScroll->addLine();
		break;
	case Qt::Key_Left:
	case Qt::Key_H:
		m_hScroll->subtractLine();
		break;
	case Qt::Key_Up:
	case Qt::Key_K:
		m_vScroll->subtractLine();
		break;
	case Qt::Key_Down:
	case Qt::Key_J:
		m_vScroll->addLine();
		break;
	case Qt::Key_PageDown:
		m_vScroll->addPage();
		break;
	case Qt::Key_PageUp:
		m_vScroll->subtractPage();
		break;
	}
	e->accept();
	slotRepaintHandles();
}

void KompareSplitter::wheelEvent( QWheelEvent* e )
{
	if ( e->orientation() == Qt::Vertical )
	{
		if ( e->state() & Qt::ControlButton ) {
			if ( e->delta() < 0 ) // scroll down one page
				m_vScroll->addPage();
			else // scroll up one page
				m_vScroll->subtractPage();
		} else {
			if ( e->delta() < 0 ) // scroll down
				m_vScroll->addLine();
			else // scroll up
				m_vScroll->subtractLine();
		}
	}
	else
	{
		if ( e->state() & Qt::ControlButton ) {
			if ( e->delta() < 0 ) // scroll right one page
				m_hScroll->addPage();
			else // scroll left one page
				m_hScroll->subtractPage();
		} else {
			if ( e->delta() < 0 ) // scroll to the right
				m_hScroll->addLine();
			else // scroll to the left
				m_hScroll->subtractLine();
		}
	}
	e->accept();
	slotDelayedRepaintHandles();
}

/* FIXME: this should return/the scrollId() from the listview containing the
 * /base/ of the diff. but there's bigger issues with that atm.
 */
 
int KompareSplitter::scrollId()
{
	if(widget(0))
		return listView(0)->scrollId();
	return minVScrollId();
}

int KompareSplitter::lineHeight()
{
	if(widget(0))
		return listView(0)->fontMetrics().height();
	return 1;
}

int KompareSplitter::pageSize()
{
	if(widget(0)) {
		KompareListView *view = listView(0);
		return view->visibleHeight() - QStyle::PM_ScrollBarExtent;
	}
	return 1;
}

bool KompareSplitter::needVScrollBar()
{
	int pagesize = pageSize();
	const int end = count();
	for ( int i = 0; i < end; ++i ) {
		KompareListView *view = listView(i);
		if( view ->contentsHeight() > pagesize)
			return true;
	}
	return false;
}

int KompareSplitter::minVScrollId()
{

	int min = -1;
	int mSId;
	const int end = count();
	for ( int i = 0; i < end; ++i ) {
		mSId = listView(i)->minScrollId();
		if (mSId < min || min == -1)
			min = mSId;
	}
	return ( min == -1 ) ? 0 : min;
}

int KompareSplitter::maxVScrollId()
{
	int max = 0;
	int mSId;
	const int end = count();
	for ( int i = 0; i < end; ++i ) {
		mSId = listView(i)->maxScrollId();
		if ( mSId > max )
			max = mSId;
	}
	return max;
}

bool KompareSplitter::needHScrollBar()
{
	const int end = count();
	for ( int i = 0; i < end; ++i ) {
		KompareListView *view = listView(i);
		if ( view->contentsWidth() > view->visibleWidth() )
			return true;
	}
	return false;
}

int KompareSplitter::maxHScrollId()
{
	int max = 0;
	int mHSId;
	const int end = count();
	for ( int i = 0; i < end; ++i ) {
		KompareListView *view = listView(i);
		mHSId = view->contentsWidth() - view->visibleWidth();
		if ( mHSId > max )
			max = mHSId;
	}
	return max;
}

int KompareSplitter::maxContentsX()
{
	int max = 0;
	int mCX;
	const int end = count();
	for ( int i = 0; i < end; ++i ) {
		mCX = listView(i)->contentsX();
		if ( mCX > max )
			max = mCX;
	}
	return max;
}

int KompareSplitter::minVisibleWidth()
{
	// Why the hell do we want to know this?
	// ah yes, it is because we use it to set the "page size" for horiz. scrolling.
	// despite the fact that *none* has a pgright and pgleft key :P
	// But we do have mousewheels with horizontal scrolling functionality,
	// pressing shift and scrolling then goes left and right one page at the time
	int min = -1;
	int vW;
	const int end = count();
	for ( int i = 0; i < end; ++i ) {
		vW = listView(i)->visibleWidth();
		if ( vW < min || min == -1 )
			min = vW;
	}
	return ( min == -1 ) ? 0 : min;
}

KompareListView* KompareSplitter::listView( int index )
{
	return static_cast<KompareListViewFrame*>(widget(index))->view();
}

KompareConnectWidget* KompareSplitter::connectWidget( int index )
{
	return static_cast<KompareConnectWidgetFrame*>(handle(index))->wid();
}

void KompareSplitter::slotSetSelection( const DiffModel* model, const Difference* diff )
{
	const int end = count();
	for ( int i = 0; i < end; ++i ) {
		connectWidget(i)->slotSetSelection( model, diff );
		listView(i)->slotSetSelection( model, diff );
		static_cast<KompareListViewFrame*>(widget(i))->slotSetModel( model );
	}

	slotDelayedRepaintHandles();
	slotDelayedUpdateScrollBars();
}

void KompareSplitter::slotSetSelection( const Difference* diff )
{
	const int end = count();
	for ( int i = 0; i < end; ++i ) {
		connectWidget(i)->slotSetSelection( diff );
		listView(i)->slotSetSelection( diff );
	}

	slotDelayedRepaintHandles();
	slotDelayedUpdateScrollBars();
}

void KompareSplitter::slotApplyDifference( bool apply )
{
	const int end = count();
	for ( int i = 0; i < end; ++i )
		listView(i)->slotApplyDifference( apply );
	slotDelayedRepaintHandles();
}

void KompareSplitter::slotApplyAllDifferences( bool apply )
{
	const int end = count();
	for ( int i = 0; i < end; ++i )
		listView(i)->slotApplyAllDifferences( apply );
	slotDelayedRepaintHandles();
	slotScrollToId( m_scrollTo ); // FIXME!
}

void KompareSplitter::slotApplyDifference( const Difference* diff, bool apply )
{
	const int end = count();
	for ( int i = 0; i < end; ++i )
		listView(i)->slotApplyDifference( diff, apply );
	slotDelayedRepaintHandles();
}

void KompareSplitter::slotDifferenceClicked( const Difference* diff )
{
	const int end = count();
	for ( int i = 0; i < end; ++i )
		listView(i)->setSelectedDifference( diff, false );
	emit selectionChanged( diff );
}

void KompareSplitter::slotConfigChanged()
{
	const int end = count();
	for ( int i = 0; i < end; ++i ) {
		KompareListView *view = listView(i);
		view->setFont( m_settings->m_font );
		view->update();
	}
}
#include "komparesplitter.moc"
