/***************************************************************************
                                komparelistview.h  -  description
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

#include <qheader.h>
#include <qpainter.h>

#include <kdebug.h>

#include "diffmodel.h"
#include "diffhunk.h"
#include "difference.h"
#include "generalsettings.h"
#include "komparemodellist.h"

#include "komparelistview.h"

#define COL_LINE_NO      0
#define COL_MAIN         1

#define kdDebug() kdDebug(8104)

KompareListView::KompareListView( bool isSource,
                                  GeneralSettings* settings,
                                  QWidget* parent, const char* name ) :
	KListView( parent, name ),
	m_isSource( isSource ),
	m_settings( settings ),
	m_maxScrollId( 0 ),
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
}

KompareListView::~KompareListView()
{
}

KompareListViewItem* KompareListView::itemAtIndex( int i )
{
	return m_items.at( i );
}

int KompareListView::firstVisibleDifference()
{
	QListViewItem* item = itemAt( QPoint( 0, 0 ) );
	if( item == 0 ) {
		kdDebug() << "no item at (0,0)" << endl;
	}

	while( item ) {
		KompareListViewLineItem* lineItem = dynamic_cast<KompareListViewLineItem*>(item);
		if( lineItem && lineItem->diffItemParent()->difference()->type() != Difference::Unchanged )
			break;
		item = item->itemBelow();
	}

	if( item ) {
		return m_items.findRef( ((KompareListViewLineItem*)item)->diffItemParent() );
	}

	return -1;
}

int KompareListView::lastVisibleDifference()
{
	QListViewItem* item = itemAt( QPoint( 0, visibleHeight() - 1 ) );
	if( item == 0 ) {
		kdDebug() << "no item at (0," << visibleHeight() - 1 << ")" << endl;
		item = lastItem();
	}

	while( item ) {
		KompareListViewLineItem* lineItem = dynamic_cast<KompareListViewLineItem*>(item);
		if( lineItem && lineItem->diffItemParent()->difference()->type() != Difference::Unchanged )
			break;
		item = item->itemAbove();
	}

	if( item ) {
		return m_items.findRef( ((KompareListViewLineItem*)item)->diffItemParent() );
	}

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
	return m_maxScrollId;
}

void KompareListView::setXOffset( int x )
{
	setContentsPos( x, contentsY() );
}

void KompareListView::scrollToId( int id )
{
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
		double r = (double)( id - itemId ) / (double) item->maxHeight();
		int y = pos + (int)(r * (double)height) - minScrollId();
		kdDebug() << "scrollToID: " << endl;
		kdDebug() << "     id = " << id << endl;
		kdDebug() << "    pos = " << pos << endl;
		kdDebug() << " itemId = " << itemId << endl;
		kdDebug() << "      r = " << r << endl;
		kdDebug() << " height = " << height << endl;
		kdDebug() << "  minID = " << minScrollId() << endl;
		kdDebug() << "      y = " << y << endl;
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

void KompareListView::slotSetSelection( const Difference* diff )
{
	if ( m_selectedDifference == diff )
		return;

	m_selectedDifference = diff;
	QListViewItem* item = static_cast<QListViewItem*>(m_itemDict[ (void*)diff ]);
	setSelected( item, true );
	ensureItemVisible( item );
	repaint();
	return;
}

void KompareListView::slotSetSelection( const DiffModel* model, const Difference* diff )
{
	if( m_selectedModel && m_selectedModel == model )
	{
		if ( m_selectedDifference == diff )
			return;

		m_selectedDifference = diff;
		QListViewItem* item = static_cast<QListViewItem*>(m_itemDict[ (void*)diff ]);
		setSelected( item, true );
		ensureItemVisible( item );
		viewport()->repaint();
		return;
	}

	// new model, so disconnect from old one...
	if ( m_selectedModel )
	{
		disconnect( m_selectedModel, SIGNAL(appliedChanged( const Difference* )),
		            this, SLOT(slotAppliedChanged( const Difference* )) );
	}

	clear();
	m_items.clear();
	m_itemDict.clear();
	m_maxScrollId = 0;
	m_selectedModel = model;

	m_itemDict.resize(model->differenceCount());

	QPtrListIterator<DiffHunk> hunkIt(model->hunks());

	KompareListViewItem* item = 0;
	for( ; hunkIt.current(); ++hunkIt ) {
		DiffHunk* hunk = hunkIt.current();

		if( item )
			item = new KompareListViewHunkItem( this, item, hunk );
		else
			item = new KompareListViewHunkItem( this, hunk );

		QPtrListIterator<Difference> diffIt(hunk->differences());

		for( ; diffIt.current(); ++diffIt ) {
			Difference* diff = diffIt.current();

			item = new KompareListViewDiffItem( this, item, diff );

			if( diff->type() != Difference::Unchanged ) {
				m_items.append( (KompareListViewDiffItem*)item );
				m_itemDict.insert( diff, (KompareListViewDiffItem*)item );
			}

			m_maxScrollId = item->scrollId() + item->height() - 1;
		}
	}
	
	connect( model, SIGNAL(appliedChanged( const Difference* )),
	         this, SLOT(slotAppliedChanged( const Difference* )) );
}

void KompareListView::setSelectedDifference( const Difference* diff, bool scroll )
{
	KompareListViewItem* item = m_itemDict[ (void*)diff ];
	// Only scroll to item if it isn't selected. This is so that
	// clicking an item doesn't scroll to to it. KompareView sets the
	// selection manually in that case.
	if( item != selectedItem() && scroll ) {
//		scrollToId( item->scrollId() );
		ensureItemVisible( static_cast<QListViewItem*>(item) );
	}

	setSelected( item, true );
}

void KompareListView::contentsMousePressEvent( QMouseEvent* e )
{
	QPoint vp = contentsToViewport( e->pos() );
	KompareListViewLineItem* lineItem = dynamic_cast<KompareListViewLineItem*>( itemAt( vp ) );
	if( !lineItem )
		return;
	KompareListViewDiffItem* diffItem = lineItem->diffItemParent();
	if( diffItem->difference()->type() != Difference::Unchanged ) {
//		setSelected( diffItem, true ); // let the slot handle this
		emit selectionChanged( diffItem->difference() );
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

void KompareListView::wheelEvent( QWheelEvent* e )
{
	e->ignore(); // we want the parent to catch wheel events
}

KompareListViewItem::KompareListViewItem( KompareListView* parent )
	: QListViewItem( parent )
	, m_scrollId( 0 )
{
}

KompareListViewItem::KompareListViewItem( KompareListView* parent, KompareListViewItem* after )
	: QListViewItem( parent, after )
	, m_scrollId( after->scrollId() + after->maxHeight() )
{
}

KompareListViewItem::KompareListViewItem( KompareListViewItem* parent )
	: QListViewItem( parent )
	, m_scrollId( 0 )
{
}

KompareListViewItem::KompareListViewItem( KompareListViewItem* parent, KompareListViewItem* after )
	: QListViewItem( parent )
	, m_scrollId( 0 )
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
	: KompareListViewItem( parent )
	, m_difference( difference )
	, m_sourceItem( 0L )
	, m_destItem( 0L )
{
	init();
}

KompareListViewDiffItem::KompareListViewDiffItem( KompareListView* parent, KompareListViewItem* after, Difference* difference )
	: KompareListViewItem( parent, after )
	, m_difference( difference )
	, m_sourceItem( 0L )
	, m_destItem( 0L )
{
	init();
}

void KompareListViewDiffItem::init()
{
	setExpandable( true );
	setOpen( true );
	if( !kompareListView()->isSource() && m_difference->type() != Difference::Unchanged )
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
	m_sourceItem->setVisible( m_difference->type() == Difference::Unchanged ||
	                          kompareListView()->isSource() || m_difference->applied() );
	if( m_destItem )
		m_destItem->setVisible( !m_sourceItem->isVisible() );
}

void KompareListViewDiffItem::applyDifference( bool apply )
{
	kdDebug() << "KompareListViewDiffItem::applyDifference( " << apply << " )" << endl;
	setVisibility();
	setup();
	repaint();
}

int KompareListViewDiffItem::maxHeight()
{
	return QMAX( m_sourceItem->totalHeight(), m_destItem ? m_destItem->totalHeight() : 0 );
}

void KompareListViewDiffItem::setSelected( bool b )
{
	kdDebug() << "KompareListViewDiffItem::setSelected( " << b << " )" << endl;
	KompareListViewItem::setSelected( b );
	QListViewItem* item = m_sourceItem->isVisible() ?
	                      m_sourceItem->firstChild() :
	                      m_destItem->firstChild();
	while( item ) {
		item->repaint();
		item = item->nextSibling();
	}
}

KompareListViewLineContainerItem::KompareListViewLineContainerItem( KompareListViewDiffItem* parent, bool isSource )
	: KompareListViewItem( parent )
	, m_isSource( isSource )
{
	setExpandable( true );
	setOpen( true );
	
	int lines = lineCount();
	int line = lineNumber() + lines - 1;
	
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

QString KompareListViewLineContainerItem::lineAt( int i ) const
{
	return m_isSource ? diffItemParent()->difference()->sourceLineAt( i ) :
	                    diffItemParent()->difference()->destinationLineAt( i );
}

KompareListViewLineItem::KompareListViewLineItem( KompareListViewLineContainerItem* parent, int line, const QString& text )
	: KompareListViewItem( parent )
{
	setText( COL_LINE_NO, QString::number( line ) );
	setText( COL_MAIN, text );
}

void KompareListViewLineItem::setup()
{
	KompareListViewItem::setup();
	setHeight( listView()->fontMetrics().lineSpacing() );
}

void KompareListViewLineItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
	QColor bg = cg.base();
	if( diffItemParent()->difference()->type() != Difference::Unchanged ) {
		bg = kompareListView()->settings()->colorForDifferenceType(
		          diffItemParent()->difference()->type(),
		          diffItemParent()->isSelected(),
		          diffItemParent()->difference()->applied() );
	} else if( column == COL_LINE_NO ) {
		bg = cg.background();
	}

	p->fillRect( 0, 0, width, height(), bg );

	p->setPen( cg.foreground() );

	if( diffItemParent()->isSelected() ) {
		if( this == parent()->firstChild() )
			p->drawLine( 0, 0, width, 0 );
		if( nextSibling() == 0 )
			p->drawLine( 0, height() - 1, width, height() - 1 );
	}
	
	paintText( p, cg, column, width, align );
}

void KompareListViewLineItem::paintText( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
	p->drawText( listView()->itemMargin(), 0,
	             width - listView()->itemMargin(), height(),
	             align, text( column ) );
}

KompareListViewDiffItem* KompareListViewLineItem::diffItemParent() const
{
	KompareListViewLineContainerItem* p = (KompareListViewLineContainerItem*)parent();
	return p->diffItemParent();
}

KompareListViewBlankLineItem::KompareListViewBlankLineItem( KompareListViewLineContainerItem* parent )
	: KompareListViewLineItem( parent, 0, QString::null )
{
}

void KompareListViewBlankLineItem::setup()
{
	KompareListViewLineItem::setup();
	setHeight( 3 );
}

void KompareListViewBlankLineItem::paintText( QPainter*, const QColorGroup&, int, int, int )
{
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
		return 5;
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
