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
	void setSynchronizeScrollBars( bool sync );
	void setFont( const QFont& font );
	void setTabWidth( uint tabWidth );

	int getSelectedItem();
	void setSelectedItem( int item );

signals:
	void selectionChanged();
	void itemsChanged();

private:
	GeneralSettings*		m_settings;

	QLabel *revlabel1, *revlabel2;
	DiffView *diff1, *diff2;
	DiffConnectWidget* zoom;
	DiffModel* model;
	int markeditem;
	bool ownsModel;
};

#endif

