/***************************************************************************
                                komparelistview.h  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2004 Otto Bruggeman
                                  (C) 2001-2003 John Firebaugh
                                  (C) 2004      Jeff Snyder
        email                   : otto.bruggeman@home.nl
                                  jfirebaugh@kde.org
                                  jeff@caffeinated.me.uk
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#include <qheader.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>

#include "diffmodel.h"
#include "diffhunk.h"
#include "difference.h"
#include "viewsettings.h"
#include "komparemodellist.h"
#include "komparesplitter.h"

#include "komparelistview.h"

#define COL_LINE_NO      0
#define COL_MAIN         1

#define BLANK_LINE_HEIGHT 3
#define HUNK_LINE_HEIGHT  5

using namespace Diff2;

KompareListViewFrame::KompareListViewFrame( bool isSource,
                                            ViewSettings* settings,
                                            KompareSplitter* parent,
                                            const char* name ):
	QFrame ( parent, name ),
	m_view ( isSource, settings, this, name ),
	m_label ( isSource?"Source":"Dest", this ),
	m_layout ( this )
{
	setSizePolicy ( QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored) );
	m_label.setSizePolicy ( QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed) );
	QFrame *frame1 = new QFrame(this), *frame2 = new QFrame(this);
	frame1->setFrameShape(QFrame::HLine);
	frame1->setFrameShadow ( QFrame::Plain );
	frame1->setSizePolicy ( QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed) );
	frame1->setFixedHeight(1);
	frame2->setFrameShape(QFrame::HLine);
	frame2->setFrameShadow ( QFrame::Plain );
	frame2->setSizePolicy ( QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed) );
	frame2->setFixedHeight(1);
	m_label.setMargin(3);
	m_layout.setSpacing(0);
	m_layout.setMargin(0);
	m_layout.addWidget(frame1);
	m_layout.addWidget(&m_label);
	m_layout.addWidget(frame2);
	m_layout.addWidget(&m_view);

	connect( parent, SIGNAL(configChanged()), &m_view, SLOT(slotConfigChanged()) );
	connect( &m_view, SIGNAL(differenceClicked(const Diff2::Difference*)),
	         parent, SLOT(slotDifferenceClicked(const Diff2::Difference*)) );

	connect( parent, SIGNAL(scrollViewsToId(int)), &m_view, SLOT(scrollToId(int)) );
	connect( parent, SIGNAL(setXOffset(int)), &m_view, SLOT(setXOffset(int)) );
	connect( &m_view, SIGNAL(resized()), parent, SLOT(slotUpdateScrollBars()) );
}

void KompareListViewFrame::slotSetModel( const DiffModel* model )
{
	if( model )
	{
		if( view()->isSource() ) {
			if( !model->sourceRevision().isEmpty() )
				m_label.setText( model->sourceFile() + " (" + model->sourceRevision() + ")" );
			else
				m_label.setText( model->sourceFile() );
		} else {
			if( !model->destinationRevision().isEmpty() )
				m_label.setText( model->destinationFile() + " (" + model->destinationRevision() + ")" );
			else
				m_label.setText( model->destinationFile() );
		}
	} else {
		m_label.setText( QString::null );
	}
}

KompareListView::KompareListView( bool isSource,
                                  ViewSettings* settings,
                                  QWidget* parent, const char* name ) :
	KListView( parent, name ),
	m_isSource( isSource ),
	m_settings( settings ),
	m_scrollId( -1 ),
	m_selectedModel( 0 ),
	m_selectedDifference( 0 )
{
	header()->hide();
	addColumn( "Line Number", 0 );
	addColumn( "Main" );
	addColumn( "Blank" );
	setColumnAlignment( COL_LINE_NO, AlignRight );
	setAllColumnsShowFocus( true );
	setRootIsDecorated( false );
	setSorting( -1 );
	setItemMargin( 3 );
	setTreeStepSize( 0 );
	setColumnWidthMode( COL_LINE_NO, Maximum );
	setColumnWidthMode( COL_MAIN, Maximum );
	setResizeMode( LastColumn );
	setFrameStyle( QFrame::NoFrame );
	setVScrollBarMode( QScrollView::AlwaysOff );
	setHScrollBarMode( QScrollView::AlwaysOff );
	setFocusPolicy( QWidget::NoFocus );
	setFont( KGlobalSettings::fixedFont() );
}

KompareListView::~KompareListView()
{
}

KompareListViewItem* KompareListView::itemAtIndex( int i )
{
	return m_items[ i ];
}

int KompareListView::firstVisibleDifference()
{
	QListViewItem* item = itemAt( QPoint( 0, 0 ) );

	if( item == 0 )
	{
		kdDebug(8104) << "no item at viewport coordinates (0,0)" << endl;
	}

	while( item ) {
		KompareListViewLineItem* lineItem = dynamic_cast<KompareListViewLineItem*>(item);
		if( lineItem && lineItem->diffItemParent()->difference()->type() != Difference::Unchanged )
			break;
		item = item->itemBelow();
	}

	if( item )
		return m_items.findIndex( ((KompareListViewLineItem*)item)->diffItemParent() );

	return -1;
}

int KompareListView::lastVisibleDifference()
{
	QListViewItem* item = itemAt( QPoint( 0, visibleHeight() - 1 ) );

	if( item == 0 )
	{
		kdDebug(8104) << "no item at viewport coordinates (0," << visibleHeight() - 1 << ")" << endl;
		item = lastItem();
	}

	while( item ) {
		KompareListViewLineItem* lineItem = dynamic_cast<KompareListViewLineItem*>(item);
		if( lineItem && lineItem->diffItemParent()->difference()->type() != Difference::Unchanged )
			break;
		item = item->itemAbove();
	}

	if( item )
		return m_items.findIndex( ((KompareListViewLineItem*)item)->diffItemParent() );

	return -1;
}

QRect KompareListView::itemRect( int i )
{
	QListViewItem* item = itemAtIndex( i );
	int x = 0;
	int y = itemPos( item );
	int vx, vy;
	contentsToViewport( x, y, vx, vy );
	return QRect( vx, vy, 0, item->totalHeight() );
}

int KompareListView::minScrollId()
{
	return visibleHeight() / 2;
}

int KompareListView::maxScrollId()
{
	KompareListViewItem* item = (KompareListViewItem*)firstChild();
	if(!item) return 0;
	while( item && item->nextSibling() ) {
		item = (KompareListViewItem*)item->nextSibling();
	}
	int maxId = item->scrollId() + item->maxHeight() - minScrollId();
	kdDebug(8104) << "Max ID = " << maxId << endl;
	return maxId;
}

int KompareListView::contentsWidth()
{
	return ( columnWidth(COL_LINE_NO) + columnWidth(COL_MAIN) );
}

void KompareListView::setXOffset( int x )
{
	kdDebug(8104) << "SetXOffset : Scroll to x position: " << x << endl;
	setContentsPos( x, contentsY() );
}

void KompareListView::scrollToId( int id )
{
//	kdDebug(8104) << "ScrollToID : Scroll to id : " << id << endl;
	KompareListViewItem* item = (KompareListViewItem*)firstChild();
	while( item && item->nextSibling() ) {
		if( ((KompareListViewItem*)item->nextSibling())->scrollId() > id )
			break;
		item = (KompareListViewItem*)item->nextSibling();
	}

	if( item ) {
		int pos = item->itemPos();
		int itemId = item->scrollId();
		int height = item->totalHeight();
		double r = (double)( id - itemId ) / (double)item->maxHeight();
		int y = pos + (int)( r * (double)height ) - minScrollId();
//		kdDebug(8104) << "scrollToID: " << endl;
//		kdDebug(8104) << "            id = " << id << endl;
//		kdDebug(8104) << "      id after = " << ( item->nextSibling() ? QString::number( ((KompareListViewItem*)item->nextSibling())->scrollId() ) : "no such sibling..." ) << endl;
//		kdDebug(8104) << "           pos = " << pos << endl;
//		kdDebug(8104) << "        itemId = " << itemId << endl;
//		kdDebug(8104) << "             r = " << r << endl;
//		kdDebug(8104) << "        height = " << height << endl;
//		kdDebug(8104) << "         minID = " << minScrollId() << endl;
//		kdDebug(8104) << "             y = " << y << endl;
//		kdDebug(8104) << "contentsHeight = " << contentsHeight() << endl;
//		kdDebug(8104) << "         c - y = " << contentsHeight() - y << endl;
		setContentsPos( contentsX(), y );
	}

	m_scrollId = id;
}

int KompareListView::scrollId()
{
	if( m_scrollId < 0 )
		m_scrollId = minScrollId();
	return m_scrollId;
}

void KompareListView::setSelectedDifference( const Difference* diff, bool scroll )
{
	kdDebug(8104) << "KompareListView::setSelectedDifference(" << diff << ", " << scroll << ")" << endl;

	if ( m_selectedDifference == diff )
		return;

	m_selectedDifference = diff;

	KompareListViewItem* item = m_itemDict[ (void*)diff ];
	if( !item ) {
		kdDebug(8104) << "KompareListView::slotSetSelection(): couldn't find our selection!" << endl;
		return;
	}
	// Only scroll to item if it isn't selected. This is so that
	// clicking an item doesn't scroll to it. KompareView sets the
	// selection manually in that case.
	if( scroll )
	{
		m_idToScrollTo = item->scrollId();
		kdDebug(8104) << "Triggering a singleshot timer... should scroll to ID : " << m_idToScrollTo << endl;
		QTimer::singleShot( 0, this, SLOT( slotDelayedScrollToId() ) );
	}
	setSelected( item, true );
}

void KompareListView::slotDelayedScrollToId()
{
	scrollToId( m_idToScrollTo );
}

void KompareListView::slotSetSelection( const Difference* diff )
{
	kdDebug(8104) << "KompareListView::slotSetSelection( const Difference* diff )" << endl;

	setSelectedDifference( diff, true );
}

void KompareListView::slotSetSelection( const DiffModel* model, const Difference* diff )
{
	kdDebug(8104) << "KompareListView::slotSetSelection( const DiffModel* model, const Difference* diff )" << endl;

	if( m_selectedModel && m_selectedModel == model ) {
		slotSetSelection( diff );
		return;
	}

	clear();
	m_items.clear();
	m_itemDict.clear();
	m_selectedModel = model;

	m_itemDict.resize(model->differenceCount());

	QValueListConstIterator<DiffHunk*> hunkIt = model->hunks().begin();
	QValueListConstIterator<DiffHunk*> hEnd   = model->hunks().end();

	KompareListViewItem* item = 0;
	Difference* tmpdiff = 0;
	DiffHunk* hunk = 0;
	

	for ( ; hunkIt != hEnd; ++hunkIt )
	{
		hunk = *hunkIt;

		if( item )
			item = new KompareListViewHunkItem( this, item, hunk );
		else
			item = new KompareListViewHunkItem( this, hunk );

		QValueListConstIterator<Difference*> diffIt = hunk->differences().begin();
		QValueListConstIterator<Difference*> dEnd   = hunk->differences().end();

		for ( ; diffIt != dEnd; ++diffIt )
		{
			tmpdiff = *diffIt;

			item = new KompareListViewDiffItem( this, item, tmpdiff );

			if ( tmpdiff->type() != Difference::Unchanged )
			{
				m_items.append( (KompareListViewDiffItem*)item );
				m_itemDict.insert( tmpdiff, (KompareListViewDiffItem*)item );
			}
		}
	}

	slotSetSelection( diff );
}

void KompareListView::contentsMousePressEvent( QMouseEvent* e )
{
	QPoint vp = contentsToViewport( e->pos() );
	KompareListViewLineItem* lineItem = dynamic_cast<KompareListViewLineItem*>( itemAt( vp ) );
	if( !lineItem )
		return;
	KompareListViewDiffItem* diffItem = lineItem->diffItemParent();
	if( diffItem->difference()->type() != Difference::Unchanged ) {
		emit differenceClicked( diffItem->difference() );
	}
}

void KompareListView::contentsMouseDoubleClickEvent( QMouseEvent* e )
{
	QPoint vp = contentsToViewport( e->pos() );
	KompareListViewLineItem* lineItem = dynamic_cast<KompareListViewLineItem*>( itemAt( vp ) );
	if ( !lineItem )
		return;
	KompareListViewDiffItem* diffItem = lineItem->diffItemParent();
	if ( diffItem->difference()->type() != Difference::Unchanged ) {
		// FIXME: make a new signal that does both
		emit differenceClicked( diffItem->difference() );
		emit applyDifference( !diffItem->difference()->applied() );
	}
}

void KompareListView::slotApplyDifference( bool apply )
{
	m_itemDict[ (void*)m_selectedDifference ]->applyDifference( apply );
}

void KompareListView::slotApplyAllDifferences( bool apply )
{
	QPtrDictIterator<KompareListViewDiffItem> it ( m_itemDict );
	for( ; it.current(); ++it )
		it.current()->applyDifference( apply );
	repaint();
}

void KompareListView::slotApplyDifference( const Difference* diff, bool apply )
{
	m_itemDict[ (void*)diff ]->applyDifference( apply );
}

void KompareListView::slotConfigChanged()
{
	m_spaces = "";
	kdDebug( 8104 ) << "tabToNumberOfSpaces: " << settings()->m_tabToNumberOfSpaces << endl;
	for ( int i = 0; i < settings()->m_tabToNumberOfSpaces; i++ )
		m_spaces += " ";
	triggerUpdate();
}

void KompareListView::wheelEvent( QWheelEvent* e )
{
	e->ignore(); // we want the parent to catch wheel events
}

void KompareListView::resizeEvent( QResizeEvent* e )
{
	KListView::resizeEvent(e);
	emit resized();
	kdDebug() << "resizeEvent " << endl;
}

KompareListViewItem::KompareListViewItem( KompareListView* parent )
	: QListViewItem( parent ),
	m_scrollId( 0 )
{
//	kdDebug(8104) << "Created KompareListViewItem with scroll id " << m_scrollId << endl;
}

KompareListViewItem::KompareListViewItem( KompareListView* parent, KompareListViewItem* after )
	: QListViewItem( parent, after ),
	m_scrollId( after->scrollId() + after->maxHeight() )
{
//	kdDebug(8104) << "Created KompareListViewItem with scroll id " << m_scrollId << endl;
}

KompareListViewItem::KompareListViewItem( KompareListViewItem* parent )
	: QListViewItem( parent ),
	m_scrollId( 0 )
{
}

KompareListViewItem::KompareListViewItem( KompareListViewItem* parent, KompareListViewItem* /*after*/ )
	: QListViewItem( parent ),
	m_scrollId( 0 )
{
}

KompareListView* KompareListViewItem::kompareListView() const
{
	return (KompareListView*)listView();
}

void KompareListViewItem::paintFocus( QPainter* /* p */, const QColorGroup& /* cg */, const QRect& /* r */ )
{
	// Don't paint anything
}

KompareListViewDiffItem::KompareListViewDiffItem( KompareListView* parent, Difference* difference )
	: KompareListViewItem( parent ),
	m_difference( difference ),
	m_sourceItem( 0L ),
	m_destItem( 0L )
{
	init();
}

KompareListViewDiffItem::KompareListViewDiffItem( KompareListView* parent, KompareListViewItem* after, Difference* difference )
	: KompareListViewItem( parent, after ),
	m_difference( difference ),
	m_sourceItem( 0L ),
	m_destItem( 0L )
{
	init();
}

void KompareListViewDiffItem::init()
{
	setExpandable( true );
	setOpen( true );
	m_destItem = new KompareListViewLineContainerItem( this, false );
	m_sourceItem = new KompareListViewLineContainerItem( this, true );
	setVisibility();
}

void KompareListViewDiffItem::setup()
{
	KompareListViewItem::setup();
	setHeight( 0 );
}

void KompareListViewDiffItem::setVisibility()
{
	m_sourceItem->setVisible( kompareListView()->isSource() || m_difference->applied() );
	m_destItem->setVisible( !m_sourceItem->isVisible() );
}

void KompareListViewDiffItem::applyDifference( bool apply )
{
	kdDebug(8104) << "KompareListViewDiffItem::applyDifference( " << apply << " )" << endl;
	setVisibility();
	setup();
	repaint();
}

int KompareListViewDiffItem::maxHeight()
{
	int lines = QMAX( m_difference->sourceLineCount(), m_difference->destinationLineCount() );
	if( lines == 0 )
		return BLANK_LINE_HEIGHT;
	else
		return lines * listView()->fontMetrics().lineSpacing();
}

void KompareListViewDiffItem::setSelected( bool b )
{
	kdDebug(8104) << "KompareListViewDiffItem::setSelected( " << b << " )" << endl;
	KompareListViewItem::setSelected( b );
	QListViewItem* item = m_sourceItem->isVisible() ?
	                      m_sourceItem->firstChild() :
	                      m_destItem->firstChild();
	while( item && item->isVisible() ) {
		item->repaint();
		item = item->nextSibling();
	}
}

KompareListViewLineContainerItem::KompareListViewLineContainerItem( KompareListViewDiffItem* parent, bool isSource )
	: KompareListViewItem( parent ),
	m_isSource( isSource )
{
//	kdDebug(8104) << "isSource ? " << (isSource ? " Yes!" : " No!") << endl;
	setExpandable( true );
	setOpen( true );

	int lines = lineCount();
	int line = lineNumber() + lines - 1;
//	kdDebug(8104) << "LineNumber : " << lineNumber() << endl;
	if( lines == 0 ) {
		new KompareListViewBlankLineItem( this );
		return;
	}

	for( int i = lines - 1; i >= 0; i--, line-- ) {
		new KompareListViewLineItem( this, line, lineAt( i ) );
	}
}

void KompareListViewLineContainerItem::setup()
{
	KompareListViewItem::setup();
	setHeight( 0 );
}

KompareListViewDiffItem* KompareListViewLineContainerItem::diffItemParent() const
{
	return (KompareListViewDiffItem*)parent();
}

int KompareListViewLineContainerItem::lineCount() const
{
	return m_isSource ? diffItemParent()->difference()->sourceLineCount() :
	                    diffItemParent()->difference()->destinationLineCount();
}

int KompareListViewLineContainerItem::lineNumber() const
{
	return m_isSource ? diffItemParent()->difference()->sourceLineNumber() :
	                    diffItemParent()->difference()->destinationLineNumber();
}

DifferenceString* KompareListViewLineContainerItem::lineAt( int i ) const
{
	return m_isSource ? diffItemParent()->difference()->sourceLineAt( i ) :
	                    diffItemParent()->difference()->destinationLineAt( i );
}

KompareListViewLineItem::KompareListViewLineItem( KompareListViewLineContainerItem* parent, int line, DifferenceString* text )
	: KompareListViewItem( parent )
{
	setText( COL_LINE_NO, QString::number( line ) );
	setText( COL_MAIN, text->string() );
	m_text = text;
}

void KompareListViewLineItem::setup()
{
	KompareListViewItem::setup();
	setHeight( listView()->fontMetrics().lineSpacing() );
}

void KompareListViewLineItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
	QColor bg = cg.base();
	p->fillRect( 0, 0, width, height(), bg );
	if ( diffItemParent()->difference()->type() == Difference::Unchanged )
	{
		if ( column == COL_LINE_NO )
		{
			bg = cg.background();
			p->fillRect( 0, 0, width, height(), bg );
		}
	}
	else
	{
		bg = kompareListView()->settings()->colorForDifferenceType(
		          diffItemParent()->difference()->type(),
		          diffItemParent()->isSelected(),
		          diffItemParent()->difference()->applied() );
		if ( column != COL_MAIN )
			p->fillRect( 0, 0, width, height(), bg );
	}

	p->setPen( cg.foreground() );

	paintText( p, bg, column, width, align );

	if( diffItemParent()->isSelected() ) {
		if( this == parent()->firstChild() )
			p->drawLine( 0, 0, width, 0 );
		if( nextSibling() == 0 )
			p->drawLine( 0, height() - 1, width, height() - 1 );
	}
}

void KompareListViewLineItem::paintText( QPainter* p, const QColor& bg, int column, int width, int align )
{
	if ( column == COL_MAIN )
	{
		QValueListIterator<Command*> commandIt = m_text->commandsList().begin();
		QValueListIterator<Command*> cEnd      = m_text->commandsList().end();
		Command* c = *commandIt;
		QString textChunk;
		int offset = listView()->itemMargin();
		unsigned int prevValue = 0;
		int chunkWidth;
		QBrush changeBrush( bg, Dense3Pattern );
		QBrush normalBrush( bg, SolidPattern );
		QBrush brush;

		if ( m_text->string().isEmpty() )
		{
			p->fillRect( 0, 0, width, height(), normalBrush );
			return;
		}

		p->fillRect( 0, 0, offset, height(), normalBrush );

		if ( ! m_text->commandsList().isEmpty() )
		{
			for ( ; commandIt != cEnd; ++commandIt )
			{
				c  = *commandIt;
				textChunk = m_text->string().mid( prevValue, c->offset() - prevValue );
//				kdDebug(8104) << "TextChunk   = \"" << textChunk << "\"" << endl;
//				kdDebug(8104) << "c->offset() = " << c->offset() << endl;
//				kdDebug(8104) << "prevValue   = " << prevValue << endl;
				textChunk.replace( QRegExp( "\\t" ), kompareListView()->spaces() );
				prevValue = c->offset();
				if ( c->type() == Command::End )
				{
					QFont font( p->font() );
					font.setBold( true );
					p->setFont( font );
//					p->setPen( Qt::blue );
					brush = changeBrush;
				}
				else
				{
					QFont font( p->font() );
					font.setBold( false );
					p->setFont( font );
//					p->setPen( Qt::black );
					brush = normalBrush;
				}
				chunkWidth = p->fontMetrics().width( textChunk );
				p->fillRect( offset, 0, chunkWidth, height(), brush );
				p->drawText( offset, 0,
				             chunkWidth, height(),
				             align, textChunk );
				offset += chunkWidth;
			}
		}
		if ( prevValue < m_text->string().length() )
		{
			// Still have to draw some string without changes
			textChunk = m_text->string().mid( prevValue, kMax( ( unsigned int )1, m_text->string().length() - prevValue ) );
			textChunk.replace( QRegExp( "\\t" ), kompareListView()->spaces() );
//			kdDebug(8104) << "TextChunk   = \"" << textChunk << "\"" << endl;
			QFont font( p->font() );
			font.setBold( false );
			p->setFont( font );
			chunkWidth = p->fontMetrics().width( textChunk );
			p->fillRect( offset, 0, chunkWidth, height(), normalBrush );
			p->drawText( offset, 0,
			             chunkWidth, height(),
			             align, textChunk );
			offset += chunkWidth;
		}
		p->fillRect( offset, 0, width - offset, height(), normalBrush );
	}
	else
	{
		p->fillRect( 0, 0, width, height(), bg );
		p->drawText( listView()->itemMargin(), 0,
		             width - listView()->itemMargin(), height(),
		             align, text( column ) );
	}
}

KompareListViewDiffItem* KompareListViewLineItem::diffItemParent() const
{
	KompareListViewLineContainerItem* p = (KompareListViewLineContainerItem*)parent();
	return p->diffItemParent();
}

KompareListViewBlankLineItem::KompareListViewBlankLineItem( KompareListViewLineContainerItem* parent )
	: KompareListViewLineItem( parent, 0, new DifferenceString() )
{
}

void KompareListViewBlankLineItem::setup()
{
	KompareListViewLineItem::setup();
	setHeight( BLANK_LINE_HEIGHT );
}

void KompareListViewBlankLineItem::paintText( QPainter* p, const QColor& bg, int column, int width, int )
{
	if ( column == COL_MAIN )
	{
		QBrush normalBrush( bg, SolidPattern );
		p->fillRect( 0, 0, width, height(), normalBrush );
	}
}

KompareListViewHunkItem::KompareListViewHunkItem( KompareListView* parent, DiffHunk* hunk )
	: KompareListViewItem( parent ),
	m_hunk( hunk )
{
	setSelectable( false );
}

KompareListViewHunkItem::KompareListViewHunkItem( KompareListView* parent, KompareListViewItem* after, DiffHunk* hunk )
	: KompareListViewItem( parent, after ),
	m_hunk( hunk )
{
	setSelectable( false );
}

int KompareListViewHunkItem::maxHeight()
{
	if( m_hunk->function().isEmpty() ) {
		return HUNK_LINE_HEIGHT;
	} else {
		return listView()->fontMetrics().lineSpacing();
	}
}

void KompareListViewHunkItem::setup()
{
	KompareListViewItem::setup();

	setHeight( maxHeight() );
}

void KompareListViewHunkItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
	p->fillRect( 0, 0, width, height(), cg.mid() );
	if( column == COL_MAIN ) {
		p->drawText( listView()->itemMargin(), 0, width - listView()->itemMargin(), height(),
		             align, m_hunk->function() );
	}
}

#include "komparelistview.moc"
