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
#include "kdiffview.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qscrollbar.h>
#include <klocale.h>
#include <kdebug.h>

#include "diffview.h"
#include "diffmodel.h"
#include "diffhunk.h"
#include "difference.h"
#include "generalsettings.h"

KDiffView::KDiffView( GeneralSettings* settings, QWidget *parent, const char *name )
	: QWidget(parent, name),
	m_models( 0 ),
	m_selectedModel( 0 ),
	m_selectedDifference( 0 ),
	m_settings( settings )
{
	QGridLayout *pairlayout = new QGridLayout(this, 4, 3, 10);
	pairlayout->setSpacing( 0 );
	pairlayout->setMargin( 0 );
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
	revlabel1 = new QLabel("Rev A", Frame1);
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
	revlabel2 = new QLabel("Rev A", Frame2 );
	Frame2Layout->addWidget( revlabel2 );
	pairlayout->addMultiCellWidget( Frame2, 0,0, 2,3 );

	diff1 = new DiffView( m_settings, true, this );
	diff2 = new DiffView( m_settings, true, this );
	pairlayout->addWidget(diff1, 1, 0);
	pairlayout->addWidget(diff2, 1, 2);

	zoom = new DiffConnectWidget( m_settings, this );
	zoom->setDiffViews( diff1, diff2 );
	pairlayout->addWidget( zoom,  1, 1);

	vScroll = new QScrollBar( QScrollBar::Vertical, this );
	pairlayout->addWidget( vScroll, 1, 3 );

	hScroll = new QScrollBar( QScrollBar::Horizontal, this );
	pairlayout->addMultiCellWidget( hScroll, 2,2, 0,2 );

	connect( vScroll, SIGNAL(valueChanged(int)), diff1, SLOT(scrollToId(int)) );
	connect( vScroll, SIGNAL(valueChanged(int)), diff2, SLOT(scrollToId(int)) );
	connect( vScroll, SIGNAL(valueChanged(int)), zoom, SLOT(repaint()) );
	connect( vScroll, SIGNAL(sliderMoved(int)), diff1, SLOT(scrollToId(int)) );
	connect( vScroll, SIGNAL(sliderMoved(int)), diff2, SLOT(scrollToId(int)) );
	connect( vScroll, SIGNAL(sliderMoved(int)), zoom, SLOT(repaint()) );
	connect( hScroll, SIGNAL(valueChanged(int)), diff1, SLOT(setXOffset(int)) );
	connect( hScroll, SIGNAL(valueChanged(int)), diff2, SLOT(setXOffset(int)) );
	connect( hScroll, SIGNAL(sliderMoved(int)), diff1, SLOT(setXOffset(int)) );
	connect( hScroll, SIGNAL(sliderMoved(int)), diff2, SLOT(setXOffset(int)) );

}

KDiffView::~KDiffView()
{
}

void KDiffView::setModels( const QList<DiffModel>* models )
{
	m_models = models;
	updateViews();
}

void KDiffView::slotSetSelection( int model, int diff )
{
	bool modelChanged = model != m_selectedModel;

	if( modelChanged ) {
		m_selectedModel = model;
		updateViews();
	}

	if( !modelChanged ) {
		const Difference* d = modelAt( m_selectedModel )->differenceAt( m_selectedDifference );
		for (int i = d->linenoA; i < d->linenoA+d->sourceLineCount(); ++i)
			diff1->setInverted(i, false);
		for (int i = d->linenoB; i < d->linenoB+d->destinationLineCount(); ++i)
			diff2->setInverted(i, false);
	}

	m_selectedDifference = diff;

	const Difference* d = modelAt( m_selectedModel )->differenceAt( m_selectedDifference );
	for (int i = d->linenoA; i < d->linenoA+d->sourceLineCount(); ++i)
		diff1->setInverted(i, true);
	for (int i = d->linenoB; i < d->linenoB+d->destinationLineCount(); ++i)
		diff2->setInverted(i, true);
	diff1->setCenterLine(d->linenoA);
	diff2->setCenterLine(d->linenoB);
	diff1->repaint();
	diff2->repaint();
	zoom->repaint();
	updateScrollBars();

}

void KDiffView::updateViews() {

	DiffModel* model = modelAt( m_selectedModel );

	// FIXME diff1->clear();
	// FIXME diff2->clear();
	QListIterator<DiffHunk> it = QListIterator<DiffHunk>( model->getHunks() );
	int linenoA = 1;
	int linenoB = 1;
	int id = 0;
	for( ; it.current(); ++it ) {
		const DiffHunk* h = it.current();
		linenoA = h->lineStartA;
		linenoB = h->lineStartB;
		diff1->addLine( i18n( "%1: Line %1" ).arg( model->getSourceFilename() ).arg( linenoA ), Difference::Separator, -1, id );
		diff2->addLine( i18n( "%1: Line %1" ).arg( model->getDestinationFilename() ).arg( linenoB ), Difference::Separator, -1, id );
		id++;
		QListIterator<Difference> differences = QListIterator<Difference>( h->getDifferences() );
		for( ; differences.current(); ++differences ) {
			const Difference* d = differences.current();
			const QStringList linesA = d->getSourceLines();
			const QStringList linesB = d->getDestinationLines();
			QStringList::ConstIterator itA = linesA.begin();
			QStringList::ConstIterator itB = linesB.begin();
			if( d->type == Difference::Unchanged ) {
				for ( ; itA != linesA.end(); ++itA ) {
					diff1->addLine((*itA), Difference::Unchanged,linenoA,id);
					diff2->addLine((*itA), Difference::Unchanged,linenoB,id);
					linenoA++;
					linenoB++;
					id++;
				}
			} else {
				while( itA != linesA.end() || itB != linesB.end() ) {
					if (itA != linesA.end()) {
						diff1->addLine((*itA), d->type,linenoA,id);
						++itA;
						if (itB != linesB.end()) {
							diff2->addLine((*itB), Difference::Change,linenoB++,id);
							++itB;
						}
						linenoA++;
						id++;
					} else {
						diff2->addLine((*itB), Difference::Insert,linenoB++,id);
						id++;
						++itB;
					}
				}
			}
		}
	}
	diff1->addLine( "", Difference::Unchanged, linenoA, id );
	diff2->addLine( "", Difference::Unchanged, linenoB, id );
	revlabel1->setText( model->getSourceFilename() );
	revlabel2->setText( model->getDestinationFilename() );
	zoom->setModel( model );
	updateScrollBars();
}

void KDiffView::updateScrollBars()
{
	if( diff1->totalHeight() <= diff1->height() &&
	    diff2->totalHeight() <= diff2->height() ) {
		if( vScroll->isVisible() )
			vScroll->hide();
	} else {
		if( !vScroll->isVisible() ) {
			vScroll->show();
		}

		vScroll->blockSignals( true );
		vScroll->setRange( QMIN( diff1->minScrollId(), diff2->minScrollId() ),
		                   QMAX( diff1->maxScrollId(), diff2->maxScrollId() ) );
		vScroll->setValue( diff1->getScrollId() );
		vScroll->setSteps( 1, diff1->pageStep() );
		vScroll->blockSignals( false );
	}

	if( diff1->totalWidth() <= diff1->width() &&
	    diff2->totalWidth() <= diff2->width() ) {
		if( hScroll->isVisible() )
			hScroll->hide();
	} else {
		if( !hScroll->isVisible() ) {
			hScroll->show();
		}

		hScroll->blockSignals( true );
		hScroll->setRange( 0, QMAX( diff1->totalWidth() - diff1->width(),
		                            diff2->totalWidth() - diff2->width() ) );
		hScroll->setValue( QMAX( diff1->xOffset(), diff2->xOffset() ) );
		hScroll->setSteps( 10, diff1->width() - 10 );
		hScroll->blockSignals( false );
	}
}

void KDiffView::setFont( const QFont& font )
{
	diff1->setFont(font);
	diff2->setFont(font);
}

void KDiffView::setTabWidth( uint tabWidth )
{
	diff1->setTabWidth(tabWidth);
	diff2->setTabWidth(tabWidth);
}

void KDiffView::resizeEvent( QResizeEvent* e )
{
	QWidget::resizeEvent( e );
	updateScrollBars();
}

#include "kdiffview.moc"
