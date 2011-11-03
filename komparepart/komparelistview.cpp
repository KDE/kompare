/***************************************************************************
                                komparelistview.h
                                -----------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2009 Otto Bruggeman <bruggie@gmail.com>
        Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
        Copyright 2004      Jeff Snyder    <jeff@caffeinated.me.uk>
        Copyright 2007-2011 Kevin Kofler   <kevin.kofler@chello.at>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#include "komparelistview.h"

#include <q3header.h>
#include <QtGui/QPainter>
#include <QtCore/QRegExp>
#include <QtCore/QTimer>
#include <QtGui/QResizeEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>

#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>

#include "diffmodel.h"
#include "diffhunk.h"
#include "difference.h"
#include "viewsettings.h"
#include "komparemodellist.h"
#include "komparesplitter.h"

#define COL_LINE_NO      0
#define COL_MAIN         1

#define BLANK_LINE_HEIGHT 3
#define HUNK_LINE_HEIGHT  5

using namespace Diff2;

KompareListViewFrame::KompareListViewFrame( bool isSource,
                                            ViewSettings* settings,
                                            KompareSplitter* parent,
                                            const char* name ):
	QFrame ( parent ),
	m_view ( isSource, settings, this, name ),
	m_label ( isSource?"Source":"Dest", this ),
	m_layout ( this )
{
	setSizePolicy ( QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored) );
	m_label.setSizePolicy ( QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed) );
	QFrame *bottomLine = new QFrame(this);
	bottomLine->setFrameShape(QFrame::HLine);
	bottomLine->setFrameShadow ( QFrame::Plain );
	bottomLine->setSizePolicy ( QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed) );
	bottomLine->setFixedHeight(1);
	m_label.setMargin(3);
	m_layout.setSpacing(0);
	m_layout.setMargin(0);
	m_layout.addWidget(&m_label);
	m_layout.addWidget(bottomLine);
	m_layout.addWidget(&m_view);

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
				m_label.setText( model->sourceFile() + " (" + model->sourceRevision() + ')' );
			else
				m_label.setText( model->sourceFile() );
		} else {
			if( !model->destinationRevision().isEmpty() )
				m_label.setText( model->destinationFile() + " (" + model->destinationRevision() + ')' );
			else
				m_label.setText( model->destinationFile() );
		}
	} else {
		m_label.setText( QString::null );	//krazy:exclude=nullstrassign for old broken gcc
	}
}

KompareListView::KompareListView( bool isSource,
                                  ViewSettings* settings,
                                  QWidget* parent, const char* name ) :
	K3ListView( parent ),
	m_isSource( isSource ),
	m_settings( settings ),
	m_scrollId( -1 ),
	m_selectedModel( 0 ),
	m_selectedDifference( 0 )
{
	setObjectName( name );
	header()->hide();
	addColumn( "Line Number", 0 );
	addColumn( "Main" );
	addColumn( "Blank" );
	setColumnAlignment( COL_LINE_NO, Qt::AlignRight );
	setAllColumnsShowFocus( true );
	setRootIsDecorated( false );
	setSorting( -1 );
	setItemMargin( 3 );
	setTreeStepSize( 0 );
	setColumnWidthMode( COL_LINE_NO, Maximum );
	setColumnWidthMode( COL_MAIN, Maximum );
	setResizeMode( LastColumn );
	setFrameStyle( QFrame::NoFrame );
	setVScrollBarMode( Q3ScrollView::AlwaysOff );
	setHScrollBarMode( Q3ScrollView::AlwaysOff );
	setFocusPolicy( Qt::NoFocus );
	setFont( m_settings->m_font );
	setFocusProxy( parent->parentWidget() );
}

KompareListView::~KompareListView()
{
	m_settings = 0;
	m_selectedModel = 0;
	m_selectedDifference = 0;
}

KompareListViewItem* KompareListView::itemAtIndex( int i )
{
	return m_items[ i ];
}

int KompareListView::firstVisibleDifference()
{
	Q3ListViewItem* item = itemAt( QPoint( 0, 0 ) );

	if( item == 0 )
	{
		kDebug(8104) << "no item at viewport coordinates (0,0)" << endl;
	}

	while( item ) {
		KompareListViewLineItem* lineItem = dynamic_cast<KompareListViewLineItem*>(item);
		if( lineItem && lineItem->diffItemParent()->difference()->type() != Difference::Unchanged )
			break;
		item = item->itemBelow();
	}

	if( item )
		return m_items.indexOf( ((KompareListViewLineItem*)item)->diffItemParent() );

	return -1;
}

int KompareListView::lastVisibleDifference()
{
	Q3ListViewItem* item = itemAt( QPoint( 0, visibleHeight() - 1 ) );

	if( item == 0 )
	{
		kDebug(8104) << "no item at viewport coordinates (0," << visibleHeight() - 1 << ")" << endl;
		item = lastItem();
	}

	while( item ) {
		KompareListViewLineItem* lineItem = dynamic_cast<KompareListViewLineItem*>(item);
		if( lineItem && lineItem->diffItemParent()->difference()->type() != Difference::Unchanged )
			break;
		item = item->itemAbove();
	}

	if( item )
		return m_items.indexOf( ((KompareListViewLineItem*)item)->diffItemParent() );

	return -1;
}

QRect KompareListView::itemRect( int i )
{
	Q3ListViewItem* item = itemAtIndex( i );
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
	kDebug(8104) << "Max ID = " << maxId << endl;
	return maxId;
}

int KompareListView::contentsWidth()
{
	return ( columnWidth(COL_LINE_NO) + columnWidth(COL_MAIN) );
}

void KompareListView::setXOffset( int x )
{
	kDebug(8104) << "SetXOffset : Scroll to x position: " << x << endl;
	setContentsPos( x, contentsY() );
}

void KompareListView::scrollToId( int id )
{
//	kDebug(8104) << "ScrollToID : Scroll to id : " << id << endl;
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
//		kDebug(8104) << "scrollToID: " << endl;
//		kDebug(8104) << "            id = " << id << endl;
//		kDebug(8104) << "      id after = " << ( item->nextSibling() ? QString::number( ((KompareListViewItem*)item->nextSibling())->scrollId() ) : "no such sibling..." ) << endl;
//		kDebug(8104) << "           pos = " << pos << endl;
//		kDebug(8104) << "        itemId = " << itemId << endl;
//		kDebug(8104) << "             r = " << r << endl;
//		kDebug(8104) << "        height = " << height << endl;
//		kDebug(8104) << "         minID = " << minScrollId() << endl;
//		kDebug(8104) << "             y = " << y << endl;
//		kDebug(8104) << "contentsHeight = " << contentsHeight() << endl;
//		kDebug(8104) << "         c - y = " << contentsHeight() - y << endl;
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
	kDebug(8104) << "KompareListView::setSelectedDifference(" << diff << ", " << scroll << ")" << endl;

	// When something other than a click causes this function to be called,
	// it'll only get called once, and all is simple.
	//
	// When the user clicks on a diff, this function will get called once when
	// komparesplitter::slotDifferenceClicked runs, and again when the
	// setSelection signal from the modelcontroller arrives.
	//
	// the first call (which will always be from the splitter) will have
	// scroll==false, and the second call will bail out here.
	// Which is why clicking on a difference does not cause the listviews to
	// scroll.
	if ( m_selectedDifference == diff )
		return;

	m_selectedDifference = diff;

	KompareListViewItem* item = m_itemDict[ diff ];
	if( !item ) {
		kDebug(8104) << "KompareListView::slotSetSelection(): couldn't find our selection!" << endl;
		return;
	}

	// why does this not happen when the user clicks on a diff? see the comment above.
	if( scroll )
		scrollToId(item->scrollId());
	setSelected( item, true );
}

void KompareListView::slotSetSelection( const Difference* diff )
{
	kDebug(8104) << "KompareListView::slotSetSelection( const Difference* diff )" << endl;

	setSelectedDifference( diff, true );
}

void KompareListView::slotSetSelection( const DiffModel* model, const Difference* diff )
{
	kDebug(8104) << "KompareListView::slotSetSelection( const DiffModel* model, const Difference* diff )" << endl;

	if( m_selectedModel && m_selectedModel == model ) {
		slotSetSelection( diff );
		return;
	}

	clear();
	m_items.clear();
	m_itemDict.clear();
	m_selectedModel = model;

	DiffHunkListConstIterator hunkIt = model->hunks()->begin();
	DiffHunkListConstIterator hEnd   = model->hunks()->end();

	KompareListViewItem* item = 0;

	for ( ; hunkIt != hEnd; ++hunkIt )
	{
		if( item )
			item = new KompareListViewHunkItem( this, item, *hunkIt, model->isBlended() );
		else
			item = new KompareListViewHunkItem( this, *hunkIt, model->isBlended() );

		DifferenceListConstIterator diffIt = (*hunkIt)->differences().begin();
		DifferenceListConstIterator dEnd   = (*hunkIt)->differences().end();

		for ( ; diffIt != dEnd; ++diffIt )
		{
			item = new KompareListViewDiffItem( this, item, *diffIt );

			int type = (*diffIt)->type();

			if ( type != Difference::Unchanged )
			{
				m_items.append( (KompareListViewDiffItem*)item );
				m_itemDict.insert( *diffIt, (KompareListViewDiffItem*)item );
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

void KompareListView::renumberLines( void )
{
//	kDebug( 8104 ) << "Begin" << endl;
	unsigned int newLineNo = 1;
	KompareListViewItem* item = (KompareListViewItem*)firstChild();
	while( item ) {
//		kDebug( 8104 ) << "rtti: " << item->rtti() << endl;
		if ( item->rtti() != KompareListViewItem::Container 
		     && item->rtti() != KompareListViewItem::Blank 
		     && item->rtti() != KompareListViewItem::Hunk )
		{
//			kDebug( 8104 ) << QString::number( newLineNo ) << endl;
			item->setText( COL_LINE_NO, QString::number( newLineNo++ ) );
		}
		item = (KompareListViewItem*)item->itemBelow();
	}
}

void KompareListView::slotApplyDifference( bool apply )
{
	m_itemDict[ m_selectedDifference ]->applyDifference( apply );
	// now renumber the line column if this is the destination
	if ( !m_isSource )
		renumberLines();
}

void KompareListView::slotApplyAllDifferences( bool apply )
{
	QHash<const Diff2::Difference*, KompareListViewDiffItem*>::ConstIterator it = m_itemDict.constBegin();
	QHash<const Diff2::Difference*, KompareListViewDiffItem*>::ConstIterator end = m_itemDict.constEnd();
	for ( ; it != end; ++it )
		it.value()->applyDifference( apply );

	// now renumber the line column if this is the destination
	if ( !m_isSource )
		renumberLines();
	repaint();
}

void KompareListView::slotApplyDifference( const Difference* diff, bool apply )
{
	m_itemDict[ diff ]->applyDifference( apply );
	// now renumber the line column if this is the destination
	if ( !m_isSource )
		renumberLines();
}

void KompareListView::wheelEvent( QWheelEvent* e )
{
	e->ignore(); // we want the parent to catch wheel events
}

void KompareListView::resizeEvent( QResizeEvent* e )
{
	K3ListView::resizeEvent(e);
	emit resized();
}

KompareListViewItem::KompareListViewItem( KompareListView* parent )
	: Q3ListViewItem( parent ),
	m_scrollId( 0 )
{
//	kDebug(8104) << "Created KompareListViewItem with scroll id " << m_scrollId << endl;
}

KompareListViewItem::KompareListViewItem( KompareListView* parent, KompareListViewItem* after )
	: Q3ListViewItem( parent, after ),
	m_scrollId( after->scrollId() + after->maxHeight() )
{
//	kDebug(8104) << "Created KompareListViewItem with scroll id " << m_scrollId << endl;
}

KompareListViewItem::KompareListViewItem( KompareListViewItem* parent )
	: Q3ListViewItem( parent ),
	m_scrollId( 0 )
{
}

KompareListViewItem::KompareListViewItem( KompareListViewItem* parent, KompareListViewItem* /*after*/ )
	: Q3ListViewItem( parent ),
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

KompareListViewDiffItem::~KompareListViewDiffItem()
{
	m_difference = 0;
	delete m_sourceItem;
	delete m_destItem;
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
	kDebug(8104) << "KompareListViewDiffItem::applyDifference( " << apply << " )" << endl;
	setVisibility();
	setup();
	repaint();
}

int KompareListViewDiffItem::maxHeight()
{
	int lines = qMax( m_difference->sourceLineCount(), m_difference->destinationLineCount() );
	if( lines == 0 )
		return BLANK_LINE_HEIGHT;
	else
		return lines * listView()->fontMetrics().height();
}

void KompareListViewDiffItem::setSelected( bool b )
{
	kDebug(8104) << "KompareListViewDiffItem::setSelected( " << b << " )" << endl;
	KompareListViewItem::setSelected( b );
	Q3ListViewItem* item = m_sourceItem->isVisible() ?
	                      m_sourceItem->firstChild() :
	                      m_destItem->firstChild();
	while( item && item->isVisible() ) {
		item->repaint();
		item = item->nextSibling();
	}
}

KompareListViewLineContainerItem::KompareListViewLineContainerItem( KompareListViewDiffItem* parent, bool isSource )
	: KompareListViewItem( parent ),
	m_blankLineItem( 0 ),
	m_isSource( isSource )
{
//	kDebug(8104) << "isSource ? " << (isSource ? " Yes!" : " No!") << endl;
	setExpandable( true );
	setOpen( true );

	int lines = lineCount();
	int line = lineNumber() + lines - 1;
//	kDebug(8104) << "LineNumber : " << lineNumber() << endl;
	if( lines == 0 ) {
		m_blankLineItem = new KompareListViewBlankLineItem( this );
		return;
	}

	for( int i = lines - 1; i >= 0; i--, line-- ) {
		m_lineItemList.append(new KompareListViewLineItem( this, line, lineAt( i ) ) );
	}
}

KompareListViewLineContainerItem::~KompareListViewLineContainerItem()
{
	delete m_blankLineItem;
	qDeleteAll( m_lineItemList );
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

KompareListViewLineItem::~KompareListViewLineItem()
{
	m_text = 0;
}

void KompareListViewLineItem::setup()
{
	KompareListViewItem::setup();
	setHeight( listView()->fontMetrics().height() );
}

void KompareListViewLineItem::paintCell( QPainter * p, const QColorGroup & /*cg*/, int column, int width, int align )
{
	p->setRenderHint(QPainter::Antialiasing);

	QColor bg( Qt::white ); // Always make the background white when it is not a real difference
	if ( diffItemParent()->difference()->type() == Difference::Unchanged )
	{
		if ( column == COL_LINE_NO )
		{
			bg = QColor( Qt::lightGray );
		}
	}
	else
	{
		bg = kompareListView()->settings()->colorForDifferenceType(
		          diffItemParent()->difference()->type(),
		          diffItemParent()->isSelected(),
		          diffItemParent()->difference()->applied() );
	}

	// Paint background
	p->fillRect( 0, 0, width, height(), bg );

	// Paint foreground
	if ( diffItemParent()->difference()->type() == Difference::Unchanged )
		p->setPen( QColor( Qt::darkGray ) ); // always make normal text gray
	else
		p->setPen( QColor( Qt::black ) ); // make text with changes black

	paintText( p, bg, column, width, align );

	// Paint darker lines around selected item
	if ( diffItemParent()->isSelected() )
	{
		p->translate(0.5,0.5);
		p->setPen( bg.dark(135) );
		if ( this == parent()->firstChild() )
			p->drawLine( 0, 0, width, 0 );
		if ( nextSibling() == 0 )
			p->drawLine( 0, height() - 1, width, height() - 1 );
	}
}

void KompareListViewLineItem::paintText( QPainter* p, const QColor& bg, int column, int width, int align )
{
	if ( column == COL_MAIN )
	{
		QString textChunk;
		int offset = listView()->itemMargin();
		int prevValue = 0;
		int charsDrawn = 0;
		int chunkWidth;
		QBrush changeBrush( bg, Qt::Dense3Pattern );
		QBrush normalBrush( bg, Qt::SolidPattern );
		QBrush brush;

		if ( m_text->string().isEmpty() )
		{
			p->fillRect( 0, 0, width, height(), normalBrush );
			return;
		}

		p->fillRect( 0, 0, offset, height(), normalBrush );

		if ( !m_text->markerList().isEmpty() )
		{
			MarkerListConstIterator markerIt = m_text->markerList().begin();
			MarkerListConstIterator mEnd     = m_text->markerList().end();
			Marker* m = *markerIt;

			for ( ; markerIt != mEnd; ++markerIt )
			{
				m  = *markerIt;
				textChunk = m_text->string().mid( prevValue, m->offset() - prevValue );
//				kDebug(8104) << "TextChunk   = \"" << textChunk << "\"" << endl;
//				kDebug(8104) << "c->offset() = " << c->offset() << endl;
//				kDebug(8104) << "prevValue   = " << prevValue << endl;
				expandTabs(textChunk, kompareListView()->settings()->m_tabToNumberOfSpaces, charsDrawn);
				charsDrawn += textChunk.length();
				prevValue = m->offset();
				if ( m->type() == Marker::End )
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
			textChunk = m_text->string().mid( prevValue, qMax( 1, m_text->string().length() - prevValue ) );
			expandTabs(textChunk, kompareListView()->settings()->m_tabToNumberOfSpaces, charsDrawn);
//			kDebug(8104) << "TextChunk   = \"" << textChunk << "\"" << endl;
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

void KompareListViewLineItem::expandTabs(QString& text, int tabstop, int startPos) const
{
	int index;
	while((index = text.indexOf(QChar(9)))!= -1)
		text.replace(index, 1, QString(tabstop-((startPos+index)%4),' '));
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
		QBrush normalBrush( bg, Qt::SolidPattern );
		p->fillRect( 0, 0, width, height(), normalBrush );
	}
}

KompareListViewHunkItem::KompareListViewHunkItem( KompareListView* parent, DiffHunk* hunk, bool zeroHeight )
	: KompareListViewItem( parent ),
	m_zeroHeight( zeroHeight ),
	m_hunk( hunk )
{
	setSelectable( false );
}

KompareListViewHunkItem::KompareListViewHunkItem( KompareListView* parent, KompareListViewItem* after, DiffHunk* hunk,  bool zeroHeight )
	: KompareListViewItem( parent, after ),
	m_zeroHeight( zeroHeight ),
	m_hunk( hunk )
{
	setSelectable( false );
}

KompareListViewHunkItem::~KompareListViewHunkItem()
{
	m_hunk = 0;
}

int KompareListViewHunkItem::maxHeight()
{
	if( m_zeroHeight ) {
		return 0;
	} else if( m_hunk->function().isEmpty() ) {
		return HUNK_LINE_HEIGHT;
	} else {
		return listView()->fontMetrics().height();
	}
}

void KompareListViewHunkItem::setup()
{
	KompareListViewItem::setup();

	setHeight( maxHeight() );
}

void KompareListViewHunkItem::paintCell( QPainter * p, const QColorGroup & /*cg*/, int column, int width, int align )
{
	p->fillRect( 0, 0, width, height(), QColor( Qt::lightGray ) ); // Hunk headers should be lightgray 
	p->setPen( QColor( Qt::black ) ); // Text color in hunk should be black
	if( column == COL_MAIN ) {
		p->drawText( listView()->itemMargin(), 0, width - listView()->itemMargin(), height(),
		             align, m_hunk->function() );
	}
}

#include "komparelistview.moc"
