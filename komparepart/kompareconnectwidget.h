/***************************************************************************
                          kompareconnectwidget.h  -  description
                             -------------------
    begin                : Tue Jun 26 2001
    copyright            : (C) 2001-2003 John Firebaugh
                           (C) 2001-2004 Otto Bruggeman
                           (C) 2004      Jeff Snyder
                           (C) 2007      Kevin Kofler
    email                : jfirebaugh@kde.org
                           otto.bruggeman@home.nl
                           jeff@caffeinated.me.uk
                           kevin.kofler@chello.at
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KOMPARECONNECTWIDGET_H
#define KOMPARECONNECTWIDGET_H

#include <QWidget>
#include <QSplitter>
#include <Q3PointArray>
//Added by qt3to4:
#include <QPaintEvent>
#include <QMouseEvent>
#include <Q3VBoxLayout>
#include <QLabel>

namespace Diff2 {
class DiffModel;
}
class ViewSettings;
class KompareListView;
class KompareSplitter;

class KompareConnectWidget : public QWidget
{
	Q_OBJECT

public:
	KompareConnectWidget( ViewSettings* settings, QWidget* parent, const char* name = 0 );
	~KompareConnectWidget();

public slots:
	void slotSetSelection( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void slotSetSelection( const Diff2::Difference* diff );

	void slotDelayedRepaint();

signals:
	void selectionChanged(const Diff2::Difference* diff);

protected:
	void paintEvent( QPaintEvent* e );
	Q3PointArray makeTopBezier( int tl, int tr );
	Q3PointArray makeBottomBezier( int bl, int br );
	Q3PointArray makeConnectPoly( const Q3PointArray& topBezier, const Q3PointArray& bottomBezier );

private:
	ViewSettings*             m_settings;

	const Diff2::DiffModel*   m_selectedModel;
	const Diff2::Difference*  m_selectedDifference;
};

class KompareConnectWidgetFrame : public QSplitterHandle
{
	Q_OBJECT
public:
	KompareConnectWidgetFrame( ViewSettings* settings, KompareSplitter* parent, const char* name = 0 );
	~KompareConnectWidgetFrame();
	
	QSize sizeHint() const;

	KompareConnectWidget*     wid() { return &m_wid; }

protected:
	// stop the parent QSplitterHandle painting
	void paintEvent( QPaintEvent* /* e */ ) { }

#if 0
	void mouseMoveEvent( QMouseEvent * );
	void mousePressEvent( QMouseEvent * );
	void mouseReleaseEvent( QMouseEvent * );
#endif

private:
	KompareConnectWidget      m_wid;
	QLabel                    m_label;
	Q3VBoxLayout               m_layout;
};

#endif
