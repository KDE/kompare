/***************************************************************************
                          kompareconnectwidget.h  -  description
                             -------------------
    begin                : Tue Jun 26 2001
    copyright            : (C) 2001-2003 John Firebaugh
                           (C) 2001-2004 Otto Bruggeman
                           (C) 2004      Jeff Snyder
    email                : jfirebaugh@kde.org
                           otto.bruggeman@home.nl
                           jeff@caffeinated.me.uk
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

#include "kompare_qsplitter.h"
#include <qwidget.h>

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
	KompareConnectWidget( KompareListView* left, KompareListView* right,
	      ViewSettings* settings, QWidget* parent, const char* name=0);
	~KompareConnectWidget();

public slots:
	void slotSetSelection( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void slotSetSelection( const Diff2::Difference* diff );

	void slotDelayedRepaint();

signals:
	void selectionChanged(const Diff2::Difference* diff);

protected:
	void paintEvent( QPaintEvent* e );
	QPointArray makeTopBezier( int tl, int tr );
	QPointArray makeBottomBezier( int bl, int br );
	QPointArray makeConnectPoly( const QPointArray& topBezier, const QPointArray& bottomBezier );

private:
	ViewSettings*             m_settings;

	KompareListView*          m_leftView;
	KompareListView*          m_rightView;

	const Diff2::DiffModel*   m_selectedModel;
	const Diff2::Difference*  m_selectedDifference;
};

class KompareConnectWidgetFrame : public QSplitterHandle
{
	Q_OBJECT
public:
	KompareConnectWidgetFrame( KompareListView* left, KompareListView* right,
	      ViewSettings* settings, KompareSplitter* parent, const char* name=0);
	~KompareConnectWidgetFrame();
	
	QSize sizeHint() const;

	KompareConnectWidget*     wid() { return &m_wid; }

protected:
	// stop the parent QSplitterHandle painting
	void paintEvent( QPaintEvent* e ) { }

private:
	KompareConnectWidget      m_wid;
	QLabel                    m_label;
	QVBoxLayout               m_layout;
};

#endif
