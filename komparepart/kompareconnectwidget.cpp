/***************************************************************************
                          kompareconnectwidget.cpp  -  description
                             -------------------
    begin                : Tue Jun 26 2001
    copyright            : (C) 2001 by John Firebaugh
    email                : jfirebaugh@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qpainter.h>
#include <qpixmap.h>
#include <qstyle.h>

#include <kdebug.h>

#include "generalsettings.h"
#include "komparelistview.h"
#include "kompareview.h"

#include "kompareconnectwidget.h"

#define kdDebug() kdDebug(8106)

KompareConnectWidget::KompareConnectWidget( KompareListView* left, KompareListView* right,
      GeneralSettings* settings, KompareView* parent, const char* name )
	: QWidget(parent, name),
	m_settings( settings ),
	m_diffView( parent ),
	m_leftView( left ),
	m_rightView( right ),
	m_selectedModel( 0 ),
	m_selectedDifference( 0 )
{
	connect( m_settings, SIGNAL( settingsChanged() ), this, SLOT( repaint() ) );
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
		repaint();
		return;
	}

	m_selectedModel = model;
	m_selectedDifference = diff;

	repaint();
}

void KompareConnectWidget::slotSetSelection( const Difference* diff )
{
	if ( m_selectedDifference == diff )
		return;

	m_selectedDifference = diff;

	repaint();
}

QSize KompareConnectWidget::sizeHint() const
{
	return QSize(50, style().pixelMetric(QStyle::PM_ScrollBarExtent));
}

void KompareConnectWidget::paintEvent( QPaintEvent* /* e */ )
{
	kdDebug() << "KompareConnectWidget::paintEvent()" << endl;

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

		kdDebug() << "    left: " << firstL << " - " << lastL << endl;
		kdDebug() << "   right: " << firstR << " - " << lastR << endl;
		kdDebug() << " drawing: " << first << " - " << last << endl;
		if( first >= 0 && last >= 0 && first <= last )
		{
			QPtrListIterator<Difference> diffIt =
			     QPtrListIterator<Difference>( m_selectedModel->differences() );
			diffIt += first;
			for( int i = first; diffIt.current() && i <= last; ++diffIt, ++i )
			{
				const Difference* diff = diffIt.current();
				bool selected = (diff == m_selectedDifference);

				QRect leftRect = m_leftView->itemRect( i );
				QRect rightRect = m_rightView->itemRect( i );
				int tl = leftRect.top();
				int tr = rightRect.top();
				int bl = leftRect.bottom();
				int br = rightRect.bottom();

//				kdDebug() << "drawing: " << tl << " " << tr << " " << bl << " " << br << endl;
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
