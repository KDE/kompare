/***************************************************************************
                                kompare_action.h  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2003 by Otto Bruggeman
                                  and John Firebaugh
        email                   : otto.bruggeman@home.nl
                                  jfirebaugh@kde.org
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#ifndef _KOMPARE_ACTIONS_H
#define _KOMPARE_ACTIONS_H

#include <qptrlist.h>

#include <kaction.h>
#include "kompare_part.h"

class QPopupMenu;
class QWidget;

class Diff2::DiffModel;
class Diff2::Difference;

class DifferencesAction;

class KompareActions : public QObject
{
	Q_OBJECT
public:
	KompareActions( KomparePart* parent, const char* name );
	~KompareActions();

public slots:
	void slotModelsChanged( const QPtrList<Diff2::DiffModel>* modelList );
	void slotSetSelection( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void slotSetSelection( const Diff2::Difference* diff );

	void slotActivated( const Diff2::Difference* diff );

protected slots:
	void slotApplyDifference();
	void slotApplyAllDifferences();
	void slotUnapplyAllDifferences();
	void slotPreviousFile();
	void slotNextFile();
	void slotPreviousDifference();
	void slotNextDifference();
	void slotDifferenceActivated( const Diff2::Difference* diff );

private:
	void updateActions();

signals:
	void selectionChanged( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void selectionChanged( const Diff2::Difference* diff );
	// apply: true is apply, false is undo...
	void applyDifference( bool apply );
	void applyAllDifferences( bool apply );

	void nextModel();
	void previousModel();
	void nextDifference();
	void previousDifference();

private:
	const QPtrList<Diff2::DiffModel>*  m_modelList;
	const Diff2::DiffModel*            m_selectedModel;
	const Diff2::Difference*           m_selectedDifference;
	KAction*                           m_applyDifference;
	KAction*                           m_applyAll;
	KAction*                           m_unapplyAll;
	KAction*                           m_previousFile;
	KAction*                           m_nextFile;
	KAction*                           m_previousDifference;
	KAction*                           m_nextDifference;
};

#endif
