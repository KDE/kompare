/*
 *  Copyright (C) 1999-2001 Bernd Gehrmann
 *                          bernd@physik.hu-berlin.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef _KDIFFVIEW_H_
#define _KDIFFVIEW_H_

#include <qwidget.h>

class QLabel;
class QComboBox;
class QFont;
class QFile;

class DiffModel;
class DiffView;
class DiffConnectWidget;
class GeneralSettings;

class KDiffView : public QWidget
{
	Q_OBJECT

public:
	KDiffView( GeneralSettings* settings, QWidget *parent=0, const char *name=0 );
	~KDiffView();

	DiffModel* getModel();
	void setModel( DiffModel* model, bool ownsModel=false );
	void setFont( const QFont& font );
	void setTabWidth( uint tabWidth );

	int getSelectedItem();
	void setSelectedItem( int item );

signals:
	void selectionChanged();
	void itemsChanged();

protected:
	void resizeEvent( QResizeEvent* e );
	void updateScrollBars();

private:
	GeneralSettings*          m_settings;
	QLabel*                   revlabel1;
	QLabel*                   revlabel2;
	DiffView*                 diff1;
	DiffView*                 diff2;
	DiffConnectWidget*        zoom;
	QScrollBar*               vScroll;
	QScrollBar*               hScroll;
	DiffModel*                model;
	int                       markeditem;
	bool                      ownsModel;
};

#endif

