/*
 *  Copyright (C) 1999-2001 Bernd Gehrmann
 *                          bernd@physik.hu-berlin.de
 *                2001-2003 Otto Bruggeman
 *                          otto.bruggeman@home.nl
 *                2001-2002 John Firebaugh
 *                          jfire@kde.org
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

namespace Diff2 {
class DiffModel;
}
class ViewSettings;
class KompareConnectWidget;
class KompareListView;

class KompareView : public QFrame
{
	Q_OBJECT

public:
	KompareView( ViewSettings* settings, QWidget *parent=0, const char *name=0 );
	~KompareView();

public:
	const KompareListView*      srcLV()      { return m_diff1; };
	const KompareListView*      destLV()     { return m_diff2; };
	const KompareConnectWidget* zoomWidget() { return m_zoom; };

public slots:
	void slotSetSelection( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void slotSetSelection( const Diff2::Difference* diff );

	void slotApplyDifference( const Diff2::Difference* diff, bool apply );
	void slotApplyDifference( bool apply );
	void slotApplyAllDifferences( bool apply );

	void slotConfigChanged();

	void slotFontChanged( void );

signals:
	void selectionChanged( const Diff2::Difference* diff );

protected:
	void resizeEvent( QResizeEvent* e );
	void wheelEvent( QWheelEvent* e );
	void updateScrollBars();

protected slots:
	void scrollToId( int id );
	void slotDifferenceClicked( const Diff2::Difference* diff );
	void slotDelayedUpdateScrollBars();

private:
	const Diff2::DiffModel*   m_selectedModel;
	const Diff2::Difference*  m_selectedDifference;

	ViewSettings*             m_settings;

	QLabel*                   m_revlabel1;
	QLabel*                   m_revlabel2;
	KompareListView*          m_diff1;
	KompareListView*          m_diff2;
	KompareConnectWidget*     m_zoom;
	QScrollBar*               m_vScroll;
	QScrollBar*               m_hScroll;

	int                       m_scrollDistance;
	int                       m_pageSize;
};

#endif

