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
	KompareView( GeneralSettings* settings, QWidget *parent=0, const char *name=0 );
	~KompareView();
	
public:
	const KompareListView*      srcLV()      { return m_diff1; };
	const KompareListView*      destLV()     { return m_diff2; };
	const KompareConnectWidget* zoomWidget() { return m_zoom; };

public slots:
	void slotSetSelection( const DiffModel* model, const Difference* diff );
	void slotSetSelection( const Difference* diff );

signals:
	void selectionChanged( const Difference* diff );

protected:
	void resizeEvent( QResizeEvent* e );
	void wheelEvent( QWheelEvent* e );
	void updateScrollBars();

protected slots:
	void scrollToId( int id );
	void slotDifferenceClicked( const Difference* diff );

private:
	const DiffModel*          m_selectedModel;
	const Difference*         m_selectedDifference;
	
	GeneralSettings*          m_settings;
	
	QLabel*                   m_revlabel1;
	QLabel*                   m_revlabel2;
	KompareListView*          m_diff1;
	KompareListView*          m_diff2;
	KompareConnectWidget*     m_zoom;
	QScrollBar*               m_vScroll;
	QScrollBar*               m_hScroll;
};

#endif

