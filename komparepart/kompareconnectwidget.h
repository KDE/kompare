/***************************************************************************
                          kompareconnectwidget.h  -  description
                             -------------------
    begin                : Tue Jun 26 2001
    copyright            : (C) 2001 by John Firebaugh
    email                : jfirebaugh@kde.org
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

class DiffModel;
class GeneralSettings;
class KompareListView;
class KompareView;

class KompareConnectWidget : public QWidget
{
	Q_OBJECT

public:
	KompareConnectWidget( KompareModelList* models, KompareListView* left, KompareListView* right,
	      GeneralSettings* settings, KompareView* parent, const char* name=0);
	~KompareConnectWidget();

	QSize sizeHint() const;

public slots:
	void slotSetSelection( int model, int diff );
	
protected:
	void paintEvent( QPaintEvent* e );
	QPointArray makeTopBezier( int tl, int tr );
	QPointArray makeBottomBezier( int bl, int br );
	QPointArray makeConnectPoly( const QPointArray& topBezier, const QPointArray& bottomBezier );

private:
	KompareModelList*  m_models;
	GeneralSettings*   m_settings;
	KompareView*       m_diffView;
	KompareListView*   m_leftView;
	KompareListView*   m_rightView;
	int                m_selectedModel;
	int                m_selectedDiff;

};

#endif
