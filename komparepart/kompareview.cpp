/***************************************************************************
                                kdiffview.cpp  -  description
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

#include <qlayout.h>
#include <qlabel.h>
#include <qscrollbar.h>

#include <klocale.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <kglobalsettings.h>

#include "kdifflistview.h"
#include "kdiffconnectwidget.h"
#include "diffmodel.h"
#include "diffhunk.h"
#include "difference.h"
#include "generalsettings.h"

#include "kdiffview.h"
#include "kdiffview.moc"

KDiffView::KDiffView( KDiffModelList* models, GeneralSettings* settings, QWidget *parent, const char *name )
	: QFrame(parent, name),
	m_models( models ),
	m_selectedModel( -1 ),
	m_selectedDifference( -1 ),
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
	revlabel1 = new QLabel( i18n( "Source" ), Frame1);
	Frame1Layout->addWidget( revlabel1 );
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
	revlabel2 = new QLabel( i18n( "Destination" ), Frame2 );
	Frame2Layout->addWidget( revlabel2 );
	pairlayout->addMultiCellWidget( Frame2, 0,0, 2,3 );

	diff1 = new KDiffListView( models, true, m_settings, this );
	diff2 = new KDiffListView( models, false, m_settings, this );
	diff1->setFrameStyle( QFrame::NoFrame );
	diff2->setFrameStyle( QFrame::NoFrame );
	diff1->setVScrollBarMode( QScrollView::AlwaysOff );
	diff2->setVScrollBarMode( QScrollView::AlwaysOff );
	diff1->setHScrollBarMode( QScrollView::AlwaysOff );
	diff2->setHScrollBarMode( QScrollView::AlwaysOff );
	diff1->setFont( KGlobalSettings::fixedFont() );
	diff2->setFont( KGlobalSettings::fixedFont() );
	pairlayout->addWidget(diff1, 1, 0);
	pairlayout->addWidget(diff2, 1, 2);

	zoom = new KDiffConnectWidget( models, diff1, diff2, m_settings, this );
	pairlayout->addWidget( zoom,  1, 1);

	vScroll = new QScrollBar( QScrollBar::Vertical, this );
	pairlayout->addWidget( vScroll, 1, 3 );

	hScroll = new QScrollBar( QScrollBar::Horizontal, this );
	pairlayout->addMultiCellWidget( hScroll, 2,2, 0,2 );

	connect( diff1, SIGNAL(selectionChanged(int,int)), SLOT(slotSelectionChanged(int,int)) );
	connect( diff2, SIGNAL(selectionChanged(int,int)), SLOT(slotSelectionChanged(int,int)) );
	
	connect( vScroll, SIGNAL(valueChanged(int)), SLOT(scrollToId(int)) );
	connect( vScroll, SIGNAL(sliderMoved(int)), SLOT(scrollToId(int)) );
	connect( hScroll, SIGNAL(valueChanged(int)), diff1, SLOT(setXOffset(int)) );
	connect( hScroll, SIGNAL(valueChanged(int)), diff2, SLOT(setXOffset(int)) );
	connect( hScroll, SIGNAL(sliderMoved(int)), diff1, SLOT(setXOffset(int)) );
	connect( hScroll, SIGNAL(sliderMoved(int)), diff2, SLOT(setXOffset(int)) );

	updateScrollBars();
}

KDiffView::~KDiffView()
{
}

void KDiffView::slotSelectionChanged( int model, int diff )
{
	diff1->setSelectedDifference( diff, false /* don't scroll */ );
	diff2->setSelectedDifference( diff, false /* don't scroll */ );
	emit selectionChanged( model, diff );
}

void KDiffView::slotSetSelection( int model, int diff )
{
	if( model >= 0 ) {
		revlabel1->setText( m_models->modelAt( model )->sourceFile() );
		revlabel2->setText( m_models->modelAt( model )->destinationFile() );
	} else {
		revlabel1->setText( QString::null );
		revlabel2->setText( QString::null );
	}
	diff1->slotSetSelection( model, diff );
	diff2->slotSetSelection( model, diff );
	zoom->slotSetSelection( model, diff );
	updateScrollBars();
}

void KDiffView::scrollToId( int id )
{
	diff1->scrollToId( id );
	diff2->scrollToId( id );
	zoom->repaint();
}

void KDiffView::updateScrollBars()
{
	if( diff1->contentsHeight() <= diff1->visibleHeight() &&
	    diff2->contentsHeight() <= diff2->visibleHeight() ) {
		if( vScroll->isVisible() )
			vScroll->hide();
	} else {
		if( !vScroll->isVisible() ) {
			vScroll->show();
		}

		vScroll->blockSignals( true );
		vScroll->setRange( QMIN( diff1->minScrollId(), diff2->minScrollId() ),
		                   QMAX( diff1->maxScrollId(), diff2->maxScrollId() ) );
		vScroll->setValue( diff1->scrollId() );
		vScroll->setSteps( 7, diff1->visibleHeight() - 14 );
		vScroll->blockSignals( false );
	}

	if( diff1->contentsWidth() <= diff1->visibleWidth() &&
	    diff2->contentsWidth() <= diff2->visibleWidth() ) {
		if( hScroll->isVisible() )
			hScroll->hide();
	} else {
		if( !hScroll->isVisible() ) {
			hScroll->show();
		}

		hScroll->blockSignals( true );
		hScroll->setRange( 0, QMAX( diff1->contentsWidth() - diff1->visibleWidth(),
		                            diff2->contentsWidth() - diff2->visibleWidth() ) );
		hScroll->setValue( QMAX( diff1->contentsX(), diff2->contentsX() ) );
		hScroll->setSteps( 10, diff1->visibleWidth() - 10 );
		hScroll->blockSignals( false );
	}
}

void KDiffView::resizeEvent( QResizeEvent* e )
{
	QWidget::resizeEvent( e );
	updateScrollBars();
}
