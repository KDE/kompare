/***************************************************************************
                          kdiffconnectwidget.h  -  description
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

#ifndef KDIFFCONNECTWIDGET_H
#define KDIFFCONNECTWIDGET_H

#include <qwidget.h>

class DiffModel;
class GeneralSettings;
class KDiffListView;
class KDiffView;

class KDiffConnectWidget : public QWidget
{
	Q_OBJECT

public:
	KDiffConnectWidget( KDiffModelList* models, KDiffListView* left, KDiffListView* right,
	      GeneralSettings* settings, KDiffView* parent, const char* name=0);
	~KDiffConnectWidget();

	QSize sizeHint() const;

public slots:
	void slotSetSelection( int model, int diff );
	
protected:
	void paintEvent( QPaintEvent* e );
	QPointArray makeTopBezier( int tl, int tr );
	QPointArray makeBottomBezier( int bl, int br );
	QPointArray makeConnectPoly( const QPointArray& topBezier, const QPointArray& bottomBezier );

private:
	KDiffModelList*      m_models;
	GeneralSettings*     m_settings;
	KDiffView*           m_diffView;
	KDiffListView*       m_leftView;
	KDiffListView*       m_rightView;
	int                  m_selectedModel;
	int                  m_selectedDiff;

};

#endif
