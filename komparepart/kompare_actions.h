/***************************************************************************
                                kompare_action.h  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001 by Otto Bruggeman
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
#include <kparts/part.h>

class QPopupMenu;
class QWidget;

class DiffModel;
class Difference;

class DifferencesAction;

class KompareActions : public QObject
{
	Q_OBJECT
public:
	KompareActions( KParts::ReadOnlyPart* parent, const char* name );
	~KompareActions();

public slots:
	void slotModelsChanged( const QPtrList<DiffModel>* modelList );
	void slotSetSelection( const DiffModel* model, const Difference* diff );
	void slotSetSelection( const Difference* diff );
	
	void slotActivated( const Difference* diff );

protected slots:
	void slotApplyDifference();
	void slotApplyAllDifferences();
	void slotUnapplyAllDifferences();
	void slotPreviousFile();
	void slotNextFile();
	void slotPreviousDifference();
	void slotNextDifference();
	void slotDifferenceActivated( const Difference* diff );

private:
	void updateActions();

signals:
	void selectionChanged( const DiffModel* model, const Difference* diff );
	void selectionChanged( const Difference* diff );
	// apply: true is apply, false is undo...
	void applyDifference( bool apply );
	void applyAllDifferences( bool apply );

	void nextModel();
	void previousModel();
	void nextDifference();
	void previousDifference();

private:
	const QPtrList<DiffModel>*  m_modelList;
	const DiffModel*            m_selectedModel;
	const Difference*           m_selectedDifference;
	KAction*                    m_applyDifference;
	KAction*                    m_applyAll;
	KAction*                    m_unapplyAll;
	KAction*                    m_previousFile;
	KAction*                    m_nextFile;
	KAction*                    m_previousDifference;
	KAction*                    m_nextDifference;
};

#endif
