/***************************************************************************
                                KompareNavigationTree.cpp  -  description
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

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmimetype.h>

#include "difference.h"
#include "diffmodel.h"
#include "komparemodellist.h"

#include "komparenavigationtree.h"

#define COL_SOURCE        0
#define COL_DESTINATION   1
#define COL_DIFFERENCE    2

KompareNavigationTree::KompareNavigationTree( KompareModelList* models, QWidget* parent, const char* name ) :
	KListView( parent, name ),
	m_models( models ),
	m_rootItem( 0 )
{
	addColumn( i18n("Source") );
	addColumn( i18n("Destination") );
	addColumn( i18n("Difference") );
	setAllColumnsShowFocus( true );
	setRootIsDecorated( true );
	setSorting( -1 );
	
	connect( this, SIGNAL(selectionChanged( QListViewItem* )),
	               SLOT(slotSelectionChanged( QListViewItem* )) );
	connect( models, SIGNAL(modelsChanged()),
	                 SLOT(buildTree()) );
}

KompareNavigationTree::~KompareNavigationTree()
{
}

void KompareNavigationTree::buildTree()
{
	clear();
	
	m_rootItem = 0;
	
	KURL sourceBaseURL = m_models->sourceBaseURL();
	KURL destinationBaseURL = m_models->destinationBaseURL();
	if( !sourceBaseURL.isEmpty() && !destinationBaseURL.isEmpty() )
	{
		m_rootItem = new QListViewItem( this );
		m_rootItem->setOpen( true );
		m_rootItem->setSelectable( false );
		
		m_rootItem->setText( COL_SOURCE, sourceBaseURL.url() );
		m_rootItem->setPixmap( COL_SOURCE,
		     KMimeType::pixmapForURL( sourceBaseURL, 0, 0, KIcon::SizeSmall ) );
		m_rootItem->setText( COL_DESTINATION, destinationBaseURL.url() );
		m_rootItem->setPixmap( COL_DESTINATION,
		     KMimeType::pixmapForURL( destinationBaseURL, 0, 0, KIcon::SizeSmall ) );
	}
	
	QListViewItem* modelItem;
	
	QPtrListIterator<DiffModel> modelIt( m_models->getModels() );
	
	// Go backwards so the items appear in the correct order when
	// added to their parent.
	for( modelIt.toLast(); modelIt.current(); --modelIt ) {
		DiffModel* model = modelIt.current();
		
		if( m_rootItem )
			modelItem = new QListViewItem( m_rootItem );
		else
			modelItem = new QListViewItem( this );
		
		modelItem->setText( COL_SOURCE, model->sourceFile() );
		modelItem->setText( COL_DESTINATION, model->destinationFile() );
		modelItem->setOpen( true );
		modelItem->setSelectable( false );
		if( !sourceBaseURL.isEmpty() ) {
			modelItem->setPixmap( COL_SOURCE, KMimeType::pixmapForURL(
			     KURL( sourceBaseURL, model->sourceFile() ), 0, 0, KIcon::SizeSmall ) );
		}
		if( !destinationBaseURL.isEmpty() ) {
			modelItem->setPixmap( COL_DESTINATION, KMimeType::pixmapForURL(
			     KURL( destinationBaseURL, model->destinationFile() ), 0, 0, KIcon::SizeSmall ) );
		}
		
		QPtrListIterator<Difference> diffIt(model->getDifferences());
		
		m_itemDict.resize( m_itemDict.size() + model->differenceCount() );
		
		// Go backwards so the items appear in the correct order when
		// added to their parent.
		for( diffIt.toLast(); diffIt.current(); --diffIt ) {
			Difference *diff = diffIt.current();
			
			QListViewItem* item = new QListViewItem( modelItem );
			
			setItemText( item, diff );
//			diffItem->setPixmap( );
			
			m_itemDict.insert( diff, item );
		}
		
		connect( model, SIGNAL( appliedChanged( const Difference* ) ),
		         this, SLOT( slotAppliedChanged( const Difference * ) ) );
	}
}

void KompareNavigationTree::setItemText( QListViewItem* item, const Difference* d )
{
	item->setText( COL_SOURCE, i18n( "Line %1" ).arg( d->sourceLineNumber() ) );
	item->setText( COL_DESTINATION, i18n( "Line %1" ).arg( d->destinationLineNumber() ) );

	QString text = "";
	switch( d->type() ) {
	case Difference::Change:
		text = i18n( "Changed line", "Changed lines", QMAX( d->sourceLineCount(),
		                                                    d->destinationLineCount() ) );
		break;
	case Difference::Insert:
		text = i18n( "Inserted line", "Inserted lines", d->destinationLineCount() );
		break;
	case Difference::Delete:
	default:
		text = i18n( "Deleted line", "Deleted lines", d->sourceLineCount() );
		break;
	}
	if( d->applied() ) {
		text = i18n( "Applied: %1" ).arg( text );
	}
	item->setText( COL_DIFFERENCE, text );
}

QListViewItem* KompareNavigationTree::firstItem()
{
	if( m_rootItem )
		return m_rootItem->firstChild();
	else
		return firstChild();
}

QListViewItem* KompareNavigationTree::lastItem()
{
	QListViewItem* item = firstItem();
	QListViewItem* lastItem = item;
	while( item != 0 ) {
		lastItem = item;
		item = item->nextSibling();
	}
	return lastItem;
}

void KompareNavigationTree::slotSetSelection( int model, int diff )
{
	if( model >= 0 && diff >= 0 ) {
		QListViewItem* current = m_itemDict[ m_models->modelAt( model )->differenceAt( diff ) ];
		setSelected( current, true );
	} else {
		clearSelection();
	}
}

void KompareNavigationTree::slotSelectionChanged( QListViewItem* item )
{
	QListViewItem* parent = item->parent();

	int model = 0;
	for( QListViewItem* current = firstItem(); current && current != parent; model++ ) {
		current = current->nextSibling();
	}

	int diff = 0;
	for( QListViewItem* current = parent->firstChild(); current && current != item; diff++ ) {
		current = current->nextSibling();
	}

	emit selectionChanged( model, diff );
}

void KompareNavigationTree::slotAppliedChanged( const Difference* d )
{
	setItemText( m_itemDict[(void*)d], d );
}

#include "komparenavigationtree.moc"
