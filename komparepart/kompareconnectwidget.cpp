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
	m_label.setFrameShape( QFrame::StyledPanel );
	m_label.setFrameShadow ( QFrame::Plain );
	m_label.setMargin(3);
	m_layout.setSpacing(0);
	m_layout.setMargin(0);
	m_layout.addWidget(&m_label);
	m_layout.addWidget(&m_wid);
}

KompareConnectWidgetFrame::~KompareConnectWidgetFrame()
{
}

QSize KompareConnectWidgetFrame::sizeHint() const
{
	return QSize(50, style().pixelMetric( QStyle::PM_ScrollBarExtent ) );
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
	connect( m_settings, SIGNAL( settingsChanged() ), this, SLOT( slotDelayedRepaint() ) );
	setBackgroundMode( NoBackground );
	setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Minimum ) );
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
//	kdDebug(8106) << "KompareConnectWidget::paintEvent()" << endl;

	QPixmap pixbuf(size());
	QPainter paint(&pixbuf, this);
	QPainter* p = &paint;

	p->fillRect( 0, 0, pixbuf.width(), pixbuf.height(), white );

	if( m_selectedModel )
	{
		int firstL = m_leftView->firstVisibleDifference();
		int firstR = m_rightView->firstVisibleDifference();
		int lastL = m_leftView->lastVisibleDifference();
		int lastR = m_rightView->lastVisibleDifference();

		int first = firstL < 0 ? firstR : QMIN( firstL, firstR );
		int last = lastL < 0 ? lastR : QMAX( lastL, lastR );

//		kdDebug(8106) << "    left: " << firstL << " - " << lastL << endl;
//		kdDebug(8106) << "   right: " << firstR << " - " << lastR << endl;
//		kdDebug(8106) << " drawing: " << first << " - " << last << endl;
		if( first >= 0 && last >= 0 && first <= last )
		{
			QValueListConstIterator<Difference*> diffIt = m_selectedModel->differences().at( first );
			QValueListConstIterator<Difference*> dEnd   = m_selectedModel->differences().end();

			QRect leftRect, rightRect;

			for( int i = first; diffIt != dEnd && i <= last; ++diffIt, ++i )
			{
				Difference* diff = *diffIt;
				bool selected = (diff == m_selectedDifference);

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

//				kdDebug(8106) << "drawing: " << tl << " " << tr << " " << bl << " " << br << endl;
				QPointArray topBezier = makeTopBezier( tl, tr );
				QPointArray bottomBezier = makeBottomBezier( bl, br );

				p->setPen( m_settings->colorForDifferenceType( diff->type(), selected, diff->applied() ) );
				p->setBrush( m_settings->colorForDifferenceType( diff->type(), selected, diff->applied() ) );
				p->drawPolygon ( makeConnectPoly( topBezier, bottomBezier ) );

				if( selected )
				{
					p->setPen( black );
					p->drawPolyline( topBezier );
					p->drawPolyline( bottomBezier );
				}

			}

		}
	}

	p->setPen( black );
	p->drawLine( 0,0, 0,pixbuf.height() );
	p->drawLine( pixbuf.width()-1,0, pixbuf.width()-1,pixbuf.height() );

	p->flush();
	bitBlt(this, 0, 0, &pixbuf);
}

QPointArray KompareConnectWidget::makeTopBezier( int tl, int tr )
{
	int l = 0;
	int r = width();
	QPointArray controlPoints;

	if ( true )
	{
		controlPoints.setPoints( 4, l,tl, 20,tl, r-20,tr, r,tr );
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
	QPointArray controlPoints;

	if ( true )
	{
		controlPoints.setPoints( 4, r,br, r-20,br, 20,bl, l,bl );
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
