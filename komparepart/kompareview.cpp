/***************************************************************************
                                kompareview.cpp  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001 by Otto Bruggeman
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

#include <qlabel.h>
#include <qlayout.h>
#include <qstyle.h>

#include <kdebug.h>
#include <kglobalsettings.h>
#include <klocale.h>

#include "difference.h"
#include "diffhunk.h"
#include "diffmodel.h"
#include "generalsettings.h"
#include "kompareconnectwidget.h"
#include "komparelistview.h"

#include "kompareview.h"

KompareView::KompareView( GeneralSettings* settings, QWidget *parent, const char *name )
	: QFrame(parent, name),
	m_selectedModel( 0 ),
	m_settings( settings )
{
	setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
	setLineWidth( style().defaultFrameWidth() );

	QGridLayout *pairlayout = new QGridLayout(this, 4, 3, 10);
	pairlayout->setSpacing( 0 );
	pairlayout->setMargin( style().defaultFrameWidth() );
	pairlayout->setRowStretch(0, 0);
	pairlayout->setRowStretch(1, 1);
	pairlayout->setColStretch(1, 0);
	pairlayout->addColSpacing(1, 16);
	pairlayout->setColStretch(0, 10);
	pairlayout->setColStretch(2, 10);

	QFrame* Frame1 = new QFrame( this, "Frame1" );
	Frame1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)1, Frame1->sizePolicy().hasHeightForWidth() ) );
	Frame1->setFrameShape( QFrame::StyledPanel );
	Frame1->setFrameShadow( QFrame::Raised );
	QVBoxLayout* Frame1Layout = new QVBoxLayout( Frame1 );
	Frame1Layout->setSpacing( 0 );
	Frame1Layout->setMargin( 3 );
	m_revlabel1 = new QLabel( i18n( "Source" ), Frame1);
	Frame1Layout->addWidget( m_revlabel1 );
	pairlayout->addWidget(Frame1, 0, 0);

	QFrame* Frame3 = new QFrame( this, "Frame3" );
	Frame3->setFrameShape( QFrame::StyledPanel );
	Frame3->setFrameShadow( QFrame::Raised );
	pairlayout->addWidget( Frame3, 0, 1 );

	QFrame* Frame2 = new QFrame( this, "Frame2" );
	Frame2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)3, Frame2->sizePolicy().hasHeightForWidth() ) );
	Frame2->setFrameShape( QFrame::StyledPanel );
	Frame2->setFrameShadow( QFrame::Raised );
	QVBoxLayout* Frame2Layout = new QVBoxLayout( Frame2 );
	Frame2Layout->setSpacing( 0 );
	Frame2Layout->setMargin( 3 );
	m_revlabel2 = new QLabel( i18n( "Destination" ), Frame2 );
	Frame2Layout->addWidget( m_revlabel2 );
	pairlayout->addMultiCellWidget( Frame2, 0,0, 2,3 );

	m_diff1 = new KompareListView( true, m_settings, this );
	m_diff2 = new KompareListView( false, m_settings, this );
	m_diff1->setFrameStyle( QFrame::NoFrame );
	m_diff2->setFrameStyle( QFrame::NoFrame );
	m_diff1->setVScrollBarMode( QScrollView::AlwaysOff );
	m_diff2->setVScrollBarMode( QScrollView::AlwaysOff );
	m_diff1->setHScrollBarMode( QScrollView::AlwaysOff );
	m_diff2->setHScrollBarMode( QScrollView::AlwaysOff );
	m_diff1->setFont( KGlobalSettings::fixedFont() );
	m_diff2->setFont( KGlobalSettings::fixedFont() );
	pairlayout->addWidget(m_diff1, 1, 0);
	pairlayout->addWidget(m_diff2, 1, 2);

	m_zoom = new KompareConnectWidget( m_diff1, m_diff2, m_settings, this );
	pairlayout->addWidget( m_zoom,  1, 1);

	m_vScroll = new QScrollBar( QScrollBar::Vertical, this );
	pairlayout->addWidget( m_vScroll, 1, 3 );

	m_hScroll = new QScrollBar( QScrollBar::Horizontal, this );
	pairlayout->addMultiCellWidget( m_hScroll, 2,2, 0,2 );

	connect( m_vScroll, SIGNAL(valueChanged(int)), SLOT(scrollToId(int)) );
	connect( m_vScroll, SIGNAL(sliderMoved(int)), SLOT(scrollToId(int)) );
	connect( m_hScroll, SIGNAL(valueChanged(int)), m_diff1, SLOT(setXOffset(int)) );
	connect( m_hScroll, SIGNAL(valueChanged(int)), m_diff2, SLOT(setXOffset(int)) );
	connect( m_hScroll, SIGNAL(sliderMoved(int)), m_diff1, SLOT(setXOffset(int)) );
	connect( m_hScroll, SIGNAL(sliderMoved(int)), m_diff2, SLOT(setXOffset(int)) );

	updateScrollBars();
}

KompareView::~KompareView()
{
}

void KompareView::slotSetSelection( const DiffModel* model, const Difference* diff )
{
	kdDebug() << "KompareView::slotSetSelection( model, diff )" << endl;
	if( model )
	{
		m_selectedModel = model;
		m_revlabel1->setText( model->srcFile() );
		m_revlabel2->setText( model->destFile() );
		if( !model->sourceRevision().isEmpty() )
			m_revlabel1->setText( model->srcFile() + " (" + model->sourceRevision() + ")" );
		if( !model->destinationRevision().isEmpty() )
			m_revlabel1->setText( model->destFile() + " (" + model->destinationRevision() + ")" );
	} else {
		m_revlabel1->setText( QString::null );
		m_revlabel2->setText( QString::null );
	}
	m_diff1->slotSetSelection( model, diff );
	m_diff2->slotSetSelection( model, diff );
	m_zoom->slotSetSelection( model, diff );
	updateScrollBars();
}

void KompareView::slotSetSelection( const Difference* diff )
{
	kdDebug() << "KompareView::slotSetSelection( diff )" << endl;
	m_diff1->slotSetSelection( diff );
	m_diff2->slotSetSelection( diff );
	m_zoom->slotSetSelection( diff );
	updateScrollBars();
}

void KompareView::scrollToId( int id )
{
	m_diff1->scrollToId( id );
	m_diff1->repaint();
	m_diff2->scrollToId( id );
	m_diff2->repaint();
	m_zoom->repaint();
}

void KompareView::updateScrollBars()
{
	if( m_diff1->contentsHeight() <= m_diff1->visibleHeight() &&
	    m_diff2->contentsHeight() <= m_diff2->visibleHeight() ) {
		if( m_vScroll->isVisible() )
			m_vScroll->hide();
	} else {
		if( !m_vScroll->isVisible() ) {
			m_vScroll->show();
		}

		m_vScroll->blockSignals( true );
		m_vScroll->setRange( QMIN( m_diff1->minScrollId(), m_diff2->minScrollId() ),
		                   QMAX( m_diff1->maxScrollId(), m_diff2->maxScrollId() ) );
		m_vScroll->setValue( m_diff1->scrollId() );
		m_vScroll->setSteps( 7, m_diff1->visibleHeight() - 14 );
		m_vScroll->blockSignals( false );
	}

	if( m_diff1->contentsWidth() <= m_diff1->visibleWidth() &&
	    m_diff2->contentsWidth() <= m_diff2->visibleWidth() ) {
		if( m_hScroll->isVisible() )
			m_hScroll->hide();
	} else {
		if( !m_hScroll->isVisible() ) {
			m_hScroll->show();
		}

		m_hScroll->blockSignals( true );
		m_hScroll->setRange( 0, QMAX( m_diff1->contentsWidth() - m_diff1->visibleWidth(),
		                            m_diff2->contentsWidth() - m_diff2->visibleWidth() ) );
		m_hScroll->setValue( QMAX( m_diff1->contentsX(), m_diff2->contentsX() ) );
		m_hScroll->setSteps( 10, m_diff1->visibleWidth() - 10 );
		m_hScroll->blockSignals( false );
	}
}

void KompareView::resizeEvent( QResizeEvent* e )
{
	QWidget::resizeEvent( e );
	updateScrollBars();
}

void KompareView::wheelEvent( QWheelEvent* e )
{
	// scroll lines...
	int pos = m_vScroll->value();
	int height = m_diff1->itemRect( 0 ).height();
	if ( e->delta() < 0 ) // scroll back into file
	{
		m_vScroll->setValue( pos + m_settings->m_scrollNoOfLines*height );
	}
	else // scroll forward into file
	{
		m_vScroll->setValue( pos - m_settings->m_scrollNoOfLines*height );
	}
	m_zoom->repaint();
}

#include "kompareview.moc"
