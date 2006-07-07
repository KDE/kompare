/***************************************************************************
                                prefsbase.cpp  -  description
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
#include <qobject.h>

#include "pagebase.h"

PageBase::PageBase( QWidget* parent ) : QWidget( parent )
{

}

PageBase::~PageBase()
{

}

/** No descriptions */
QSize PageBase::sizeHintForWidget( QWidget* widget )
{
	//
	// The size is computed by adding the sizeHint().height() of all
	// widget children and taking the width of the widest child and adding
	// layout()->margin() and layout()->spacing()
	//

	// this code in this method has been ripped out of a file in kbabel
	// so copyright goes to the kbabel authors.

	QSize size;

	int numChild = 0;
	QList<QObject*> l = widget->children();



	for( int i=0; i < l.count(); i++ )
	{
		QObject *o = l.at(i);
		if( o->isWidgetType() )
		{
			numChild += 1;
			QWidget *w=((QWidget*)o);

			QSize s = w->sizeHint();
			if( s.isEmpty() == true )
			{
				s = QSize( 50, 100 ); // Default size
			}
			size.setHeight( size.height() + s.height() );
			if( s.width() > size.width() )
			{
				size.setWidth( s.width() );
			}
		}
	}

	if( numChild > 0 )
	{
		size.setHeight( size.height() + widget->layout()->spacing()*(numChild-1) );
		size += QSize( widget->layout()->margin()*2, widget->layout()->margin()*2 + 1 );
	}
	else
	{
		size = QSize( 1, 1 );
	}

	return( size );
}

/** No descriptions */
void PageBase::apply()
{

}

/** No descriptions */
void PageBase::restore()
{

}

/** No descriptions */
void PageBase::setDefaults()
{

}

#include "pagebase.moc"
