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

#include "kdiffmodellist.h"

class DiffModel;
class KDiffListView;
class KDiffConnectWidget;
class GeneralSettings;

class KDiffView : public QWidget
{
	Q_OBJECT
	
public:
	KDiffView( KDiffModelList* models, GeneralSettings* settings, QWidget *parent=0, const char *name=0 );
	~KDiffView();
	
public slots:
	void slotSetSelection( int model, int diff );
	
signals:
	void selectionChanged( int model, int diff );
	
protected:
	void resizeEvent( QResizeEvent* e );
	void updateScrollBars();
	
private:
	KDiffModelList*           m_models;
	int                       m_selectedModel;
	int                       m_selectedDifference;
	GeneralSettings*          m_settings;
	QLabel*                   revlabel1;
	QLabel*                   revlabel2;
	KDiffListView*            diff1;
	KDiffListView*            diff2;
	KDiffConnectWidget*       zoom;
	QScrollBar*               vScroll;
	QScrollBar*               hScroll;
};

#endif

