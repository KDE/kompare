/***************************************************************************
                          kompareconnectwidget.h
                          ----------------------
    begin                : Tue Jun 26 2001
    Copyright 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    Copyright 2001-2004 Otto Bruggeman <otto.bruggeman@home.nl>
    Copyright 2004      Jeff Snyder    <jeff@caffeinated.me.uk>
    Copyright 2007      Kevin Kofler   <kevin.kofler@chello.at>
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

#include <QtGui/QWidget>
#include <QtGui/QSplitter>
#include <QtGui/QPaintEvent>
#include <QtGui/QMouseEvent>
#include <QVBoxLayout>
#include <QtGui/QLabel>

#include "komparemodellist.h"

namespace Diff2 {
class DiffModel;
}
class ViewSettings;
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
	QPainterPath makeBezier( int l, int r ) const;

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
	QVBoxLayout               m_layout;
};

#endif
