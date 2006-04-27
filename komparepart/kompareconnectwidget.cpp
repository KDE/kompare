/***************************************************************************
                          kompareconnectwidget.cpp  -  description
                             -------------------
    begin                : Tue Jun 26 2001
    copyright            : (C) 2001-2003 John Firebaugh
                           (C) 2001-2004 Otto Bruggeman
                           (C) 2004      Jeff Snyder
    email                : jfirebaugh@kde.org
                           otto.bruggeman@home.nl
                           jeff@caffeinated.me.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qapplication.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qstyle.h>
#include <qtimer.h>

#include <kdebug.h>

#include "viewsettings.h"
#include "komparemodellist.h"
#include "komparelistview.h"
#include "komparesplitter.h"

#include "kompareconnectwidget.h"

using namespace Diff2;

KompareConnectWidgetFrame::KompareConnectWidgetFrame( KompareListView* left,
                                                      KompareListView* right,
                                                      ViewSettings* settings,
                                                      KompareSplitter* parent,
                                                      const char* name ) :
	QSplitterHandle(Horizontal, (QSplitter *)parent, name),
	m_wid ( left, right, settings, this, name ),
	m_label ( "", this ),
	m_layout ( this )
{
	setSizePolicy ( QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored) );
	m_wid.setSizePolicy ( QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored) );
	m_label.setSizePolicy ( QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed) );
	m_label.setMargin(3);
	QFrame* bottomLine = new QFrame(this);
	bottomLine->setFrameShape(QFrame::HLine);
	bottomLine->setFrameShadow ( QFrame::Plain );
	bottomLine->setSizePolicy ( QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed) );
	bottomLine->setFixedHeight(1);
	m_layout.setSpacing(0);
	m_layout.setMargin(0);
	m_layout.addWidget(&m_label);
	m_layout.addWidget(bottomLine);
	m_layout.addWidget(&m_wid);
}

KompareConnectWidgetFrame::~KompareConnectWidgetFrame()
{
}

QSize KompareConnectWidgetFrame::sizeHint() const
{
	return QSize(50, style().pixelMetric( QStyle::PM_ScrollBarExtent ) );
}

static int kMouseOffset;

void KompareConnectWidgetFrame::mouseMoveEvent( QMouseEvent *e )
{
	if ( !(e->state()&LeftButton) )
		return;

	QCOORD pos = s->pick( parentWidget()->mapFromGlobal(e->globalPos()) )
		- kMouseOffset;

	((KompareSplitter*)s)->moveSplitter( pos, id() );
}

void KompareConnectWidgetFrame::mousePressEvent( QMouseEvent *e )
{
	if ( e->button() == LeftButton )
		kMouseOffset = s->pick( e->pos() );
	QSplitterHandle::mousePressEvent(e);
}

void KompareConnectWidgetFrame::mouseReleaseEvent( QMouseEvent *e )
{
	if ( !opaque() && e->button() == LeftButton ) {
		QCOORD pos = s->pick( parentWidget()->mapFromGlobal(e->globalPos()) )
			- kMouseOffset;
		((KompareSplitter*)s)->moveSplitter( pos, id() );
    }
}

KompareConnectWidget::KompareConnectWidget( KompareListView* left, KompareListView* right,
      ViewSettings* settings, QWidget* parent, const char* name )
	: QWidget(parent, name),
	m_settings( settings ),
	m_leftView( left ),
	m_rightView( right ),
	m_selectedModel( 0 ),
	m_selectedDifference( 0 )
{
//	connect( m_settings, SIGNAL( settingsChanged() ), this, SLOT( slotDelayedRepaint() ) );
	setBackgroundMode( NoBackground );
	setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Minimum ) );
	setFocusProxy( parent->parentWidget() );
}

KompareConnectWidget::~KompareConnectWidget()
{
}

void KompareConnectWidget::slotSetSelection( const DiffModel* model, const Difference* diff )
{
	if( m_selectedModel == model && m_selectedDifference == diff )
		return;

	if ( m_selectedModel == model && m_selectedDifference != diff )
	{
		m_selectedDifference = diff;
		slotDelayedRepaint();
		return;
	}

	m_selectedModel = model;
	m_selectedDifference = diff;

	slotDelayedRepaint();
}

void KompareConnectWidget::slotDelayedRepaint()
{
	QTimer::singleShot( 0, this, SLOT( repaint() ) );
}

void KompareConnectWidget::slotSetSelection( const Difference* diff )
{
	if ( m_selectedDifference == diff )
		return;

	m_selectedDifference = diff;

	slotDelayedRepaint();
}

void KompareConnectWidget::paintEvent( QPaintEvent* /* e */ )
{
//	kDebug(8106) << "KompareConnectWidget::paintEvent()" << endl;

	QPixmap pixbuf(size());
	QPainter paint(&pixbuf, this);
	QPainter* p = &paint;

	p->fillRect( 0, 0, pixbuf.width(), pixbuf.height(), white.dark(110) );

	if ( m_selectedModel )
	{
		int firstL = m_leftView->firstVisibleDifference();
		int firstR = m_rightView->firstVisibleDifference();
		int lastL = m_leftView->lastVisibleDifference();
		int lastR = m_rightView->lastVisibleDifference();

		int first = firstL < 0 ? firstR : qMin( firstL, firstR );
		int last = lastL < 0 ? lastR : qMax( lastL, lastR );

//		kDebug(8106) << "    left: " << firstL << " - " << lastL << endl;
//		kDebug(8106) << "   right: " << firstR << " - " << lastR << endl;
//		kDebug(8106) << " drawing: " << first << " - " << last << endl;
		if ( first >= 0 && last >= 0 && first <= last )
		{
			const DifferenceList* differences  = const_cast<DiffModel*>(m_selectedModel)->differences();
			DifferenceListConstIterator diffIt = differences->at( first );
			DifferenceListConstIterator dEnd   = differences->at( last + 1 );

			QRect leftRect, rightRect;

			for ( int i = first; i <= last; ++diffIt, ++i )
			{
				Difference* diff = *diffIt;
				bool selected = ( diff == m_selectedDifference );

				if ( QApplication::reverseLayout() )
				{
					leftRect = m_rightView->itemRect( i );
					rightRect = m_leftView->itemRect( i );
				}
				else
				{
					leftRect = m_leftView->itemRect( i );
					rightRect = m_rightView->itemRect( i );
				}

				int tl = leftRect.top();
				int tr = rightRect.top();
				int bl = leftRect.bottom();
				int br = rightRect.bottom();

				// Bah, stupid 16-bit signed shorts in that crappy X stuff...
				tl = tl >= -32768 ? tl : -32768;
				tr = tr >= -32768 ? tr : -32768;
				bl = bl <=  32767 ? bl :  32767;
				br = br <=  32767 ? br :  32767;

//				kDebug(8106) << "drawing: " << tl << " " << tr << " " << bl << " " << br << endl;
				QPointArray topBezier = makeTopBezier( tl, tr );
				QPointArray bottomBezier = makeBottomBezier( bl, br );

				QColor color = m_settings->colorForDifferenceType( diff->type(), selected, diff->applied() ).dark(110);
				p->setPen( color );
				p->setBrush( color );
				p->drawPolygon ( makeConnectPoly( topBezier, bottomBezier ) );

				if ( selected )
				{
					p->setPen( color.dark(135) );
					p->drawPolyline( topBezier );
					p->drawPolyline( bottomBezier );
				}
			}
		}
	}

	p->flush();
	bitBlt(this, 0, 0, &pixbuf);
}

QPointArray KompareConnectWidget::makeTopBezier( int tl, int tr )
{
	int l = 0;
	int r = width();
	int o = (int)((double)r*0.4); // 40% of width
	QPointArray controlPoints;

	if ( tl != tr )
	{
		controlPoints.setPoints( 4, l,tl, o,tl, r-o,tr, r,tr );
		return controlPoints.cubicBezier();
	}
	else
	{
		controlPoints.setPoints( 2, l,tl, r,tr );
		return controlPoints;
	}
}

QPointArray KompareConnectWidget::makeBottomBezier( int bl, int br )
{
	int l = 0;
	int r = width();
	int o = (int)((double)r*0.4); // 40% of width
	QPointArray controlPoints;

	if ( bl != br )
	{
		controlPoints.setPoints( 4, r,br, r-o,br, o,bl, l,bl );
		return controlPoints.cubicBezier();
	}
	else
	{
		controlPoints.setPoints( 2, r,br, l,bl );
		return controlPoints;
	}
}

QPointArray KompareConnectWidget::makeConnectPoly( const QPointArray& topBezier, const QPointArray& bottomBezier )
{
	QPointArray poly( topBezier.size() + bottomBezier.size() );
	for( uint i = 0; i < topBezier.size(); i++ )
		poly.setPoint( i, topBezier.point( i ) );
	for( uint i = 0; i < bottomBezier.size(); i++ )
		poly.setPoint( i + topBezier.size(), bottomBezier.point( i ) );

	return poly;
}

#include "kompareconnectwidget.moc"
