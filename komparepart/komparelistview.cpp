/***************************************************************************
                                kdifflistview.h  -  description
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

#include <klocale.h>
#include <kdebug.h>

#include "kdiffmodellist.h"
#include "diffmodel.h"
#include "diffhunk.h"
#include "difference.h"
#include "generalsettings.h"

#include "kdifflistview.h"
#include "kdifflistview.moc"

#define COL_LINE_NO      0
#define COL_MAIN         1

KDiffListView::KDiffListView( KDiffModelList* models, bool isSource, GeneralSettings* settings, QWidget* parent, const char* name )
	: KListView( parent, name ),
	m_models( models ),
	m_isSource( isSource ),
	m_selectedModel( -1 ),
	m_settings( settings ),
	m_maxScrollId( 0 ),
	m_scrollId( -1 ),
	m_maxMainWidth( 0 )
{
	header()->hide();
	addColumn( "Line Number", 40 );
	addColumn( "Main" );
	setColumnAlignment( COL_LINE_NO, AlignRight );
	setAllColumnsShowFocus( true );
	setRootIsDecorated( false );
	setSorting( -1 );
	setItemMargin( 3 );
	setTreeStepSize( 0 );
}

KDiffListView::~KDiffListView()
{
}

KDiffListViewItem* KDiffListView::itemAtIndex( int i )
{
	return m_items.at( i );
}

int KDiffListView::firstVisibleDifference()
{
	QListViewItem* item = itemAt( QPoint( 0, 0 ) );
	
	while( item ) {
		KDiffListViewDiffItem* dvlitem = dynamic_cast<KDiffListViewDiffItem*>(item);
		if( dvlitem && dvlitem->difference()->type() != Difference::Unchanged )
			break;
		item = item->itemBelow();
	}
	
	if( item ) {
		return m_items.findRef( (KDiffListViewDiffItem*)item );
	}
	
	return -1;
}

int KDiffListView::lastVisibleDifference()
{
	QListViewItem* item = itemAt( QPoint( 0, height() ) );
	if( item == 0 )
		item = m_items.last();
	
	while( item ) {
		KDiffListViewDiffItem* dvlitem = dynamic_cast<KDiffListViewDiffItem*>(item);
		if( dvlitem && dvlitem->difference()->type() != Difference::Unchanged )
			break;
		item = item->itemAbove();
	}
	
	if( item ) {
		return m_items.findRef( (KDiffListViewDiffItem*)item );
	}
	
	return -1;
}

QRect KDiffListView::itemRect( int i )
{
	QListViewItem* item = itemAtIndex( i );
	int x = 0;
	int y = itemPos( itemAtIndex( i ) );
	int vx, vy;
	contentsToViewport( x, y, vx, vy );
	return QRect( vx, vy, 0, item->height() );
}

int KDiffListView::minScrollId()
{
	return visibleHeight() / 2;
}

int KDiffListView::maxScrollId()
{
	return m_maxScrollId;
}

void KDiffListView::setXOffset( int x )
{
	setContentsPos( x, contentsY() );
}

void KDiffListView::scrollToId( int id )
{
	KDiffListViewItem* item = (KDiffListViewItem*)firstChild();
	while( item && item->nextSibling() ) {
		if( ((KDiffListViewItem*)item->nextSibling())->scrollId() > id )
			break;
		item = (KDiffListViewItem*)item->nextSibling();
	}
	
	if( item ) {
		int pos = item->itemPos();
		int itemId = item->scrollId();
		int height = item->height();
		double r = (double)( id - itemId ) / (double) item->maxHeight();
		int y = pos + (int)(r * (double)height) - minScrollId();
//		kdDebug() << "scrollToID: " << endl;
//		kdDebug() << "     id = " << id << endl;
//		kdDebug() << "    pos = " << pos << endl;
//		kdDebug() << " itemId = " << itemId << endl;
//		kdDebug() << "      r = " << r << endl;
//		kdDebug() << " height = " << item->height() << endl;
//		kdDebug() << "  minID = " << minScrollId() << endl;
//		kdDebug() << "      y = " << y << endl;
		setContentsPos( contentsX(), y );
	}
	
	m_scrollId = id;
}

int KDiffListView::scrollId()
{
	if( m_scrollId < 0 )
		m_scrollId = minScrollId();
	return m_scrollId;
}

void KDiffListView::slotSetSelection( int model, int diff )
{
	setSelectedModel( model );
	setSelectedDifference( diff );
}

void KDiffListView::setSelectedModel( int index )
{
	DiffModel* model = 0;
	if( index >= 0 )
		model = m_models->modelAt( index );
	
	if( index == m_selectedModel ) return;
	
	clear();
	m_items.clear();
	m_itemDict.clear();
	m_maxScrollId = 0;
	m_maxMainWidth = 0;
	disconnect( m_models->modelAt( index ), SIGNAL(appliedChanged( const Difference* )),
	            this, SLOT(slotAppliedChanged( const Difference* )) );
	
	m_selectedModel = index;
	
	if( model == 0 ) return;
	
	m_itemDict.resize(model->differenceCount());
	
	QListIterator<DiffHunk> hunkIt(model->getHunks());
	
	KDiffListViewItem* item = 0;
	for( ; hunkIt.current(); ++hunkIt ) {
		DiffHunk* hunk = hunkIt.current();
		
		if( item )
			item = new KDiffListViewHunkItem( this, item, hunk );
		else
			item = new KDiffListViewHunkItem( this, hunk );
		
		QListIterator<Difference> diffIt(hunk->getDifferences());
		
		for( ; diffIt.current(); ++diffIt ) {
			Difference* diff = diffIt.current();
			
			item = new KDiffListViewDiffItem( this, item, diff );
			
			if( diff->type() != Difference::Unchanged ) {
				m_items.append( (KDiffListViewDiffItem*)item );
				m_itemDict.insert( diff, (KDiffListViewDiffItem*)item );
			}
			
			m_maxScrollId = item->scrollId() + item->height() - 1;
			m_maxMainWidth = QMAX( m_maxMainWidth, ((KDiffListViewDiffItem*)item)->maxMainWidth() );
		}
	}
	
	connect( model, SIGNAL(appliedChanged( const Difference* )),
	         this, SLOT(slotAppliedChanged( const Difference* )) );
	updateMainColumnWidth();
}

void KDiffListView::setSelectedDifference( int index )
{
	KDiffListViewItem* item = itemAtIndex( index );
	setSelected( item, true );
	scrollToId( item->scrollId() );
}

void KDiffListView::updateMainColumnWidth()
{
	int width = viewport()->width();
	int x;
	for (x = 0; x < header()->count() - 1; x++)
		width -= header()->sectionSize(x);
	header()->resizeSection(x, QMAX( width, m_maxMainWidth ) );
}

void KDiffListView::resizeEvent( QResizeEvent* e )
{
	KListView::resizeEvent( e );
	updateMainColumnWidth();
}

void KDiffListView::contentsMousePressEvent( QMouseEvent* e )
{
	KDiffListViewDiffItem* item = dynamic_cast<KDiffListViewDiffItem*>( itemAt( e->pos() ) );
	if( item && item->difference()->type() != Difference::Unchanged ) {
		setSelected( item, true );
		emit selectionChanged( m_selectedModel, m_items.findRef( item ) );
	}
}

void KDiffListView::slotAppliedChanged( const Difference* d )
{
	m_itemDict[(void*)d]->appliedChanged();
}

KDiffListViewItem::KDiffListViewItem( QListView* parent )
	: QListViewItem( parent ),
	m_scrollId( 0 )
{
}

KDiffListViewItem::KDiffListViewItem( QListView* parent, KDiffListViewItem* after )
	: QListViewItem( parent, after ),
	m_scrollId( after->scrollId() + after->maxHeight() )
{
}

void KDiffListViewItem::paintFocus( QPainter* /* p */, const QColorGroup& /* cg */, const QRect& /* r */ )
{
	// Don't paint anything
}

KDiffListViewDiffItem::KDiffListViewDiffItem( KDiffListView* parent, Difference* difference )
	: KDiffListViewItem( parent ),
	m_listView( parent ),
	m_difference( difference )
{
	setSelectable( m_difference->type() != Difference::Unchanged );
}

KDiffListViewDiffItem::KDiffListViewDiffItem( KDiffListView* parent, KDiffListViewItem* after, Difference* difference )
	: KDiffListViewItem( parent, after ),
	m_listView( parent ),
	m_difference( difference )
{
	setSelectable( m_difference->type() != Difference::Unchanged );
}

int KDiffListViewDiffItem::maxHeight()
{
	int lines = QMAX( m_difference->sourceLineCount(),
	                  m_difference->destinationLineCount() );
	
	int height = 0;
	if( lines == 0 ) {
		height = 3;
	} else {
		height = m_listView->fontMetrics().lineSpacing() * lines;
	}

	return height;
}

int KDiffListViewDiffItem::lineCount() const
{
	if( isSource() || m_difference->applied() ) {
		return m_difference->sourceLineCount();
	} else {
		return m_difference->destinationLineCount();
	}
}

int KDiffListViewDiffItem::maxMainWidth() const
{
	int maxWidth = 0;
	QFontMetrics fm = listView()->fontMetrics();
	
	QStringList list;
	if( isSource() || m_difference->applied() )
		list = m_difference->sourceLines();
	else
		list = m_difference->destinationLines();
	
	QStringList::Iterator it = list.begin();
	for( ; it != list.end(); ++it ) {
		maxWidth = QMAX( maxWidth, fm.width( (*it) ) );
	}
	
	return maxWidth + listView()->itemMargin() * 2;
}

void KDiffListViewDiffItem::setup()
{
	QListViewItem::setup();
	
	int lines = lineCount();
	
	int height = 0;
	if( lines == 0 ) {
		height = 3;
	} else {
		height = m_listView->fontMetrics().lineSpacing() * lines;
	}
	
	setHeight( height );
}

void KDiffListViewDiffItem::paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int  align )
{
	QColor bg = cg.base();
	if( m_difference->type() != Difference::Unchanged ) {
		bg = m_listView->settings()->getColorForDifferenceType( m_difference->type(), isSelected() );
	} else if( column == COL_LINE_NO ) {
		bg = cg.background();
	}

	p->fillRect( 0, 0, width, height(), bg );
	
	p->setPen( cg.foreground() );
	
	if( isSelected() ) {
		p->drawLine( 0, 0, width, 0 );
		p->drawLine( 0, height() - 1, width, height() - 1 );
	}
	
	int lines = lineCount();
	
	int line = 0;
	if( isSource() || m_difference->applied() ) {
		line = m_difference->sourceLineNumber();
	} else {
		line = m_difference->destinationLineNumber();
	}

	int y = 0;
	for( int i = 0; i < lines; i++, line++ ) {
		
		QString text;
		
		switch( column ) {
		case COL_LINE_NO:
			text = QString::number( line );
			break;
		case COL_MAIN:
		default:
			if( isSource() || m_difference->applied() )
				text = m_difference->sourceLineAt( i );
			else
				text = m_difference->destinationLineAt( i );
			break;
		}
		
		p->drawText( m_listView->itemMargin(), y, width - m_listView->itemMargin(), height(),
		             align, text );
		y += m_listView->fontMetrics().lineSpacing();
		
	}
}

void KDiffListViewDiffItem::appliedChanged()
{
	setup();
}

KDiffListViewHunkItem::KDiffListViewHunkItem( KDiffListView* parent, DiffHunk* hunk )
	: KDiffListViewItem( parent ),
	m_hunk( hunk )
{
	setSelectable( false );
}

KDiffListViewHunkItem::KDiffListViewHunkItem( KDiffListView* parent, KDiffListViewItem* after, DiffHunk* hunk )
	: KDiffListViewItem( parent, after ),
	m_hunk( hunk )
{
	setSelectable( false );
}

int KDiffListViewHunkItem::maxHeight()
{
	return 5;
}

void KDiffListViewHunkItem::setup()
{
	QListViewItem::setup();
	
	setHeight( 5 );
}

void KDiffListViewHunkItem::paintCell( QPainter * p, const QColorGroup & cg, int /* column */, int width, int /* align */ )
{
	p->fillRect( 0, 0, width, height(), cg.mid() );
}
