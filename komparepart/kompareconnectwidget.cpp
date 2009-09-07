/***************************************************************************
                          kompareconnectwidget.cpp
                          ------------------------
    begin                : Tue Jun 26 2001
    Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    Copyright 2001-2009 Otto Bruggeman <bruggie@gmail.com>
    Copyright 2004      Jeff Snyder    <jeff@caffeinated.me.uk>
    Copyright 2007      Kevin Kofler   <kevin.kofler@chello.at>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kompareconnectwidget.h"

#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QStyle>
#include <QtCore/QTimer>
#include <QtGui/QPaintEvent>
#include <QFrame>
#include <QtGui/QMouseEvent>

#include <kdebug.h>

#include "viewsettings.h"
#include "komparelistview.h"
#include "komparesplitter.h"

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
	return QSize(50, style()->pixelMetric( QStyle::PM_ScrollBarExtent ) );
}

#if 0
static int kMouseOffset;

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
	m_settings = 0;
	m_selectedModel = 0;
	m_selectedDifference = 0;
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
	QPixmap pixbuf(size());
	QPainter paint(&pixbuf);
	QPainter* p = &paint;

	p->setRenderHint(QPainter::Antialiasing);
	p->fillRect( 0, 0, pixbuf.width(), pixbuf.height(), palette().color( QPalette::Window ) );
	p->translate(QPointF(0, 0.5));

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
			QRect leftRect, rightRect;
			for ( int i = first; i <= last; ++i )
			{
                            Difference* diff = differences->at(i );
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

				QPainterPath topBezier = makeBezier( tl, tr );
				QPainterPath bottomBezier = makeBezier( bl, br );

				QPainterPath poly(topBezier);
				poly.connectPath(bottomBezier.toReversed());
				poly.closeSubpath();

				QColor bg = m_settings->colorForDifferenceType( diff->type(), selected, diff->applied() );
				p->setPen( bg );
				p->setBrush( bg );
				p->drawPath(poly);

				if(selected)
				{
					p->setPen( bg.dark( 135 ) );
					p->setBrush( Qt::NoBrush );
					p->drawPath( topBezier );
					p->drawPath( bottomBezier.toReversed() );
				}
			}
		}
	}

//	p->flush();
	bitBlt(this, 0, 0, &pixbuf);
}

QPainterPath KompareConnectWidget::makeBezier( int leftHeight, int rightHeight ) const
{
	int r = width();
	int o = (int)((double)r*0.4); // 40% of width

	QPainterPath p(QPointF(0, leftHeight));
	if(leftHeight==rightHeight) {
		p.lineTo(QPointF(r, rightHeight));
	} else {
		p.cubicTo(QPointF(o, leftHeight), QPointF(r-o, rightHeight), QPointF(r,rightHeight));
	}
	return p;
}

#include "kompareconnectwidget.moc"
