/***************************************************************************
                          kompareconnectwidget.h  -  description
                             -------------------
    begin                : Tue Jun 26 2001
    copyright            : (C) 2001-2003 by John Firebaugh
                           and Otto Bruggeman
    email                : jfirebaugh@kde.org
                           otto.bruggeman@home.nl
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

#include <qwidget.h>

class Diff2::DiffModel;
class ViewSettings;
class KompareListView;
class KompareView;

class KompareConnectWidget : public QWidget
{
	Q_OBJECT

public:
	KompareConnectWidget( KompareListView* left, KompareListView* right,
	      ViewSettings* settings, KompareView* parent, const char* name=0);
	~KompareConnectWidget();

	QSize sizeHint() const;

public slots:
	void slotSetSelection( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void slotSetSelection( const Diff2::Difference* diff );

signals:
	void selectionChanged(const Diff2::Difference* diff);

protected:
	void paintEvent( QPaintEvent* e );
	QPointArray makeTopBezier( int tl, int tr );
	QPointArray makeBottomBezier( int bl, int br );
	QPointArray makeConnectPoly( const QPointArray& topBezier, const QPointArray& bottomBezier );

private:
	ViewSettings*             m_settings;

	KompareView*              m_diffView;
	KompareListView*          m_leftView;
	KompareListView*          m_rightView;

	const Diff2::DiffModel*   m_selectedModel;
	const Diff2::Difference*  m_selectedDifference;
};

#endif
