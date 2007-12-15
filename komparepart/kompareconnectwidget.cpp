/***************************************************************************
                          kompareconnectwidget.cpp  -  description
                             -------------------
    begin                : Tue Jun 26 2001
    copyright            : (C) 2001-2003 John Firebaugh
                           (C) 2001-2004 Otto Bruggeman
                           (C) 2004      Jeff Snyder
                           (C) 2007      Kevin Kofler
    email                : jfirebaugh@kde.org
                           otto.bruggeman@home.nl
                           jeff@caffeinated.me.uk
                           kevin.kofler@chello.at
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
#include <Q3Painter>
#include <qpixmap.h>
#include <qstyle.h>
#include <qtimer.h>
//Added by qt3to4:
#include <Q3PointArray>
#include <QPaintEvent>
#include <Q3Frame>
#include <QMouseEvent>

#include <kdebug.h>

#include "viewsettings.h"
#include "komparemodellist.h"
#include "komparelistview.h"
#include "komparesplitter.h"

#include "kompareconnectwidget.h"

using namespace Diff2;

KompareConnectWidgetFrame::KompareConnectWidgetFrame( ViewSettings* settings,
                                                      KompareSplitter* parent,
                                                      const char* name ) :
	QSplitterHandle(Qt::Horizontal, (QSplitter *)parent),
	m_wid ( settings, this, name ),
	m_label ( " ", this ), // putting a space here because Qt 4 computes different size hints for empty labels
	m_layout ( this )
{
	setObjectName( name );
	setSizePolicy ( QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored) );
	m_wid.setSizePolicy ( QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored) );
	m_label.setSizePolicy ( QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed) );
	m_label.setMargin(3);
	Q3Frame* bottomLine = new Q3Frame(this);
	bottomLine->setFrameShape(Q3Frame::HLine);
	bottomLine->setFrameShadow ( Q3Frame::Plain );
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
	return QSize(50, style()->pixelMetric( QStyle::PM_ScrollBarExtent ) );
}

static int kMouseOffset;

#if 0
void KompareConnectWidgetFrame::mouseMoveEvent( QMouseEvent *e )
{

	if ( !(e->state()&Qt::LeftButton) )
		return;

	QCOORD pos = s->pick( parentWidget()->mapFromGlobal(e->globalPos()) )
		- kMouseOffset;

	((KompareSplitter*)s)->moveSplitter( pos, id() );
}

void KompareConnectWidgetFrame::mousePressEvent( QMouseEvent *e )
{
	if ( e->button() == Qt::LeftButton )
		kMouseOffset = s->pick( e->pos() );
	QSplitterHandle::mousePressEvent(e);
}

void KompareConnectWidgetFrame::mouseReleaseEvent( QMouseEvent *e )
{
	if ( !opaque() && e->button() == Qt::LeftButton ) {
		QCOORD pos = s->pick( parentWidget()->mapFromGlobal(e->globalPos()) )
			- kMouseOffset;
		((KompareSplitter*)s)->moveSplitter( pos, id() );
    }
}
#endif

KompareConnectWidget::KompareConnectWidget( ViewSettings* settings, QWidget* parent, const char* name )
	: QWidget(parent, name),
	m_settings( settings ),
	m_selectedModel( 0 ),
	m_selectedDifference( 0 )
{
//	connect( m_settings, SIGNAL( settingsChanged() ), this, SLOT( slotDelayedRepaint() ) );
	setBackgroundMode( Qt::NoBackground );
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
	Q3Painter paint(&pixbuf);
	Q3Painter* p = &paint;

	p->fillRect( 0, 0, pixbuf.width(), pixbuf.height(), QColor(Qt::white).dark(110) );

	KompareSplitter* splitter = static_cast<KompareSplitter*>( parent()->parent() );
	int count = splitter->count();
	KompareListView *leftView = count >= 2 ? static_cast<KompareListViewFrame*>( splitter->widget(0) )->view() : 0;
	KompareListView *rightView = count >= 2 ? static_cast<KompareListViewFrame*>( splitter->widget(1) )->view() : 0;

	if ( m_selectedModel && leftView && rightView )
	{
		int firstL = leftView->firstVisibleDifference();
		int firstR = rightView->firstVisibleDifference();
		int lastL = leftView->lastVisibleDifference();
		int lastR = rightView->lastVisibleDifference();

		int first = firstL < 0 ? firstR : qMin( firstL, firstR );
		int last = lastL < 0 ? lastR : qMax( lastL, lastR );

//		kDebug(8106) << "    left: " << firstL << " - " << lastL << endl;
//		kDebug(8106) << "   right: " << firstR << " - " << lastR << endl;
//		kDebug(8106) << " drawing: " << first << " - " << last << endl;
		if ( first >= 0 && last >= 0 && first <= last )
		{
			const DifferenceList* differences  = const_cast<DiffModel*>(m_selectedModel)->differences();
			DifferenceListConstIterator diffIt = differences->at( first );
			DifferenceListConstIterator dEnd   = last + 1 < differences->size() ? differences->at( last + 1 ) : differences->end();

			QRect leftRect, rightRect;

			for ( int i = first; i <= last; ++diffIt, ++i )
			{
				Difference* diff = *diffIt;
				bool selected = ( diff == m_selectedDifference );

				if ( QApplication::reverseLayout() )
				{
					leftRect = rightView->itemRect( i );
					rightRect = leftView->itemRect( i );
				}
				else
				{
					leftRect = leftView->itemRect( i );
					rightRect = rightView->itemRect( i );
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
				Q3PointArray topBezier = makeTopBezier( tl, tr );
				Q3PointArray bottomBezier = makeBottomBezier( bl, br );

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

//	p->flush();
	bitBlt(this, 0, 0, &pixbuf);
}

Q3PointArray KompareConnectWidget::makeTopBezier( int tl, int tr )
{
	int l = 0;
	int r = width();
	int o = (int)((double)r*0.4); // 40% of width
	Q3PointArray controlPoints;

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

Q3PointArray KompareConnectWidget::makeBottomBezier( int bl, int br )
{
	int l = 0;
	int r = width();
	int o = (int)((double)r*0.4); // 40% of width
	Q3PointArray controlPoints;

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

Q3PointArray KompareConnectWidget::makeConnectPoly( const Q3PointArray& topBezier, const Q3PointArray& bottomBezier )
{
	Q3PointArray poly( topBezier.size() + bottomBezier.size() );
	for( uint i = 0; i < topBezier.size(); i++ )
		poly.setPoint( i, topBezier.point( i ) );
	for( uint i = 0; i < bottomBezier.size(); i++ )
		poly.setPoint( i + topBezier.size(), bottomBezier.point( i ) );

	return poly;
}

#include "kompareconnectwidget.moc"
