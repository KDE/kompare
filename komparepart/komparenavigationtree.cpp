/***************************************************************************
                                KDiffNavigationTree.cpp  -  description
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

#include <klocale.h>
#include <kmimetype.h>
#include <kdebug.h>

#include "kdiff_part.h"
#include "diffmodel.h"
#include "difference.h"

#include "kdiffnavigationtree.h"

#define COL_SOURCE        0
#define COL_DESTINATION   1
#define COL_DIFFERENCE    2

KDiffNavigationTree::KDiffNavigationTree( KDiffPart* part, QWidget* parent, const char* name )
	: KListView( parent, name ),
	m_diffPart( part )
{
	addColumn( i18n("Source") );
	addColumn( i18n("Destination") );
	addColumn( i18n("Difference") );
	setAllColumnsShowFocus( true );
	setRootIsDecorated( true );
	setSorting( -1 );

	connect( this, SIGNAL(selectionChanged( QListViewItem* )),
	               SLOT(slotSelectionChanged( QListViewItem* )) );

}

KDiffNavigationTree::~KDiffNavigationTree()
{

}

void KDiffNavigationTree::setModels( const QList<DiffModel>* models )
{
	clear();

	QListIterator<DiffModel> modelIt(*models);

	// Go backwards so the items appear in the correct order when
	// added to the list view.
	for( modelIt.toLast(); modelIt.current(); --modelIt ) {
		DiffModel *model = modelIt.current();

		QListViewItem* modelItem = new QListViewItem( this );
		modelItem->setText( COL_SOURCE, model->getSourceFilename() );
		modelItem->setText( COL_DESTINATION, model->getDestinationFilename() );

		KURL url = m_diffPart->getSourceURL();
		if( !url.isEmpty() ) {
			modelItem->setPixmap( COL_SOURCE, KMimeType::pixmapForURL( url, 0, 0, KIcon::SizeSmall ) );
		}

		url = m_diffPart->getDestinationURL();
		if( !url.isEmpty() ) {
			modelItem->setPixmap( COL_DESTINATION, KMimeType::pixmapForURL( url, 0, 0, KIcon::SizeSmall ) );
		}

		modelItem->setOpen( true );
		modelItem->setSelectable( false );

		QListIterator<Difference> diffIt(model->getDifferences());

		// Go backwards so the items appear in the correct order when
		// added to their parent.
		for( diffIt.toLast(); diffIt.current(); --diffIt ) {
			Difference *diff = diffIt.current();

			QListViewItem* diffItem = new QListViewItem( modelItem );
			diffItem->setText( COL_SOURCE, i18n( "Line %1" ).arg( diff->linenoA ) );
			diffItem->setText( COL_DESTINATION, i18n( "Line %1" ).arg( diff->linenoB ) );

			QString text = "";
			switch( diff->type ) {
			case Difference::Change:
				text = i18n( "Nonmatching lines" );
				break;
			case Difference::Insert:
				text = i18n( "Lines inserted in destination" );
				break;
			case Difference::Delete:
			default:
				text = i18n( "Lines deleted from destination" );
				break;
			}
			diffItem->setText( COL_DIFFERENCE, text );
//			diffItem->setPixmap( );
		}
	}
}

void KDiffNavigationTree::slotSetSelection( int model, int diff )
{
	QListViewItem* current;

	for( current = firstChild(); model > 0; model-- ) {
		current = current->nextSibling();
	}

	for( current = current->firstChild(); diff > 0; diff-- ) {
		current = current->nextSibling();
	}

	setSelected( current, true );
}

void KDiffNavigationTree::slotSelectionChanged( QListViewItem* item )
{
	if( item->depth() != 1 ) return;

	QListViewItem* parent = item->parent();

	int model = 0;
	for( QListViewItem* current = firstChild(); current && current != parent; model++ ) {
		current = current->nextSibling();
	}

	int diff = 0;
	for( QListViewItem* current = parent->firstChild(); current && current != item; diff++ ) {
		current = current->nextSibling();
	}

	emit selectionChanged( model, diff );
}

#include "kdiffnavigationtree.moc"
