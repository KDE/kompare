/***************************************************************************
                          kdiffconnectwidget.cpp  -  description
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

#include <qpixmap.h>
#include <qpainter.h>

#include <kdebug.h>

#include "kdiffview.h"
#include "kdifflistview.h"
#include "generalsettings.h"

#include "kdiffconnectwidget.h"
#include "kdiffconnectwidget.moc"

KDiffConnectWidget::KDiffConnectWidget( KDiffModelList* models, KDiffListView* left, KDiffListView* right,
      GeneralSettings* settings, KDiffView* parent, const char* name )
	: QWidget(parent, name),
	m_models( models ),
	m_settings( settings ),
	m_diffView( parent ),
	m_leftView( left ),
	m_rightView( right ),
	m_selectedModel( -1 ),
	m_selectedDiff( -1 )
{
	connect( m_settings, SIGNAL( settingsChanged() ), this, SLOT( repaint() ) );
	setBackgroundMode( NoBackground );
	setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Minimum ) );
}

KDiffConnectWidget::~KDiffConnectWidget()
{
}

void KDiffConnectWidget::slotSetSelection( int model, int diff ) {
	if( m_selectedModel == model && m_selectedDiff == diff )
		return;
	
	m_selectedModel = model;
	m_selectedDiff = diff;
	repaint();
}

QSize KDiffConnectWidget::sizeHint() const
{
	return QSize(50, style().scrollBarExtent().height());
}

void KDiffConnectWidget::paintEvent( QPaintEvent* /* e */ )
{
	QPixmap pixbuf(size());
	QPainter paint(&pixbuf, this);
	QPainter* p = &paint;

	p->fillRect( 0, 0, pixbuf.width(), pixbuf.height(), white );

	if( m_selectedModel >= 0 ) {

		DiffModel* selectedModel = m_models->modelAt( m_selectedModel );
		const Difference* selectedDiff = selectedModel->differenceAt( m_selectedDiff );
		
		int first = m_leftView->firstVisibleDifference();
		if( first < 0 )
			first = m_rightView->firstVisibleDifference();
		else
			first = QMIN( first, m_rightView->firstVisibleDifference() );
		
		int last =  m_leftView->lastVisibleDifference();
		if( last < 0 )
			last = m_rightView->lastVisibleDifference();
		else
			last = QMAX( last, m_rightView->lastVisibleDifference() );
		
//		kdDebug() << "connect: " << first << " " << last << endl;
		if( first >= 0 && last >= 0 && first <= last ) {
			
			QListIterator<Difference> diffIt =
			     QListIterator<Difference>( selectedModel->getDifferences() );
			diffIt += first;
			for( int i = first; diffIt.current() && i <= last; ++diffIt, ++i ) {

				const Difference* d = diffIt.current();
				bool selected = (d == selectedDiff);

				QRect leftRect = m_leftView->itemRect( i );
				QRect rightRect = m_rightView->itemRect( i );
				int tl = leftRect.top();
				int tr = rightRect.top();
				int bl = leftRect.bottom();
				int br = rightRect.bottom();

//				kdDebug() << "drawing: " << tl << " " << tr << " " << bl << " " << br << endl;
				QPointArray topBezier = makeTopBezier( tl, tr );
				QPointArray bottomBezier = makeBottomBezier( bl, br );

				p->setPen( m_settings->getColorForDifferenceType( d->type(), selected ) );
				p->setBrush( m_settings->getColorForDifferenceType( d->type(), selected ) );
				p->drawPolygon ( makeConnectPoly( topBezier, bottomBezier ) );

				if( selected ) {
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

QPointArray KDiffConnectWidget::makeTopBezier( int tl, int tr )
{
	int l = 0;
	int r = width();
	QPointArray controlPoints;
	controlPoints.setPoints( 4, l,tl, 20,tl, r-20,tr, r,tr );
	return controlPoints.quadBezier();
}

QPointArray KDiffConnectWidget::makeBottomBezier( int bl, int br )
{
	int l = 0;
	int r = width();
	QPointArray controlPoints;
	controlPoints.setPoints( 4, r,br, r-20,br, 20,bl, l,bl );
	return controlPoints.quadBezier();
}

QPointArray KDiffConnectWidget::makeConnectPoly( const QPointArray& topBezier, const QPointArray& bottomBezier )
{
	QPointArray poly( topBezier.size() + bottomBezier.size() );
	for( uint i = 0; i < topBezier.size(); i++ )
		poly.setPoint( i, topBezier.point( i ) );
	for( uint i = 0; i < bottomBezier.size(); i++ )
		poly.setPoint( i + topBezier.size(), bottomBezier.point( i ) );

	return poly;
}
