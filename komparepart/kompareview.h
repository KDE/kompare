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


#ifndef _KOMPAREVIEW_H_
#define _KOMPAREVIEW_H_

#include <qframe.h>

#include "komparemodellist.h"

class QComboBox;
class QFont;
class QFile;
class QLabel;
class QScrollBar;

class DiffModel;
class GeneralSettings;
class KompareConnectWidget;
class KompareListView;

class KompareView : public QFrame
{
	Q_OBJECT
	
public:
	KompareView( KompareModelList* models, GeneralSettings* settings, QWidget *parent=0, const char *name=0 );
	~KompareView();
	
public slots:
	void slotSetSelection( int model, int diff );
	
signals:
	void selectionChanged( int model, int diff );
	
protected:
	void resizeEvent( QResizeEvent* e );
	void wheelEvent( QWheelEvent* e );
	void updateScrollBars();

protected slots:
	void scrollToId( int id );
	void slotSelectionChanged( int model, int diff );

private:
	KompareModelList*         m_models;
	int                       m_selectedModel;
	int                       m_selectedDifference;
	GeneralSettings*          m_settings;
	QLabel*                   revlabel1;
	QLabel*                   revlabel2;
	KompareListView*          diff1;
	KompareListView*          diff2;
	KompareConnectWidget*     zoom;
	QScrollBar*               vScroll;
	QScrollBar*               hScroll;
};

#endif

