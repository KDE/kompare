/***************************************************************************
                                kompare_actions.cpp  -  description
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

#include <qpopupmenu.h>

#include <kdebug.h>
#include <klocale.h>

#include "difference.h"
#include "diffmodel.h"

#include "kompare_actions.h"

KompareActions::KompareActions( KomparePart* parent, const char* name )
	: QObject( parent, name ),
	m_modelList( 0L ),
	m_selectedModel( 0L ),
	m_selectedDifference( 0L )
{
	m_applyDifference = new KAction( i18n("&Apply Difference"), "1rightarrow", Qt::Key_Space,
	                                 this, SLOT(slotApplyDifference()),
	                                 parent->actionCollection(), "difference_apply" );
	m_applyAll        = new KAction( i18n("App&ly All"), "2rightarrow", Qt::CTRL + Qt::Key_A,
	                                 this, SLOT(slotApplyAllDifferences()),
	                                 parent->actionCollection(), "difference_applyall" );
	m_unapplyAll      = new KAction( i18n("&Unapply All"), "2leftarrow", Qt::CTRL + Qt::Key_U,
	                                 this, SLOT(slotUnapplyAllDifferences()),
	                                 parent->actionCollection(), "difference_unapplyall" );
	m_previousFile    = new KAction( i18n("P&revious File"), "2uparrow", Qt::CTRL + Qt::Key_PageUp,
	                                 this, SLOT(slotPreviousFile()),
	                                 parent->actionCollection(), "difference_previousfile" );
	m_nextFile        = new KAction( i18n("N&ext File"), "2downarrow", Qt::CTRL + Qt::Key_PageDown,
	                                 this, SLOT(slotNextFile()),
	                                 parent->actionCollection(), "difference_nextfile" );
	m_previousDifference = new KAction( i18n("&Previous Difference"), "1uparrow", Qt::CTRL + Qt::Key_Up,
	                                 this, SLOT(slotPreviousDifference()),
	                                 parent->actionCollection(), "difference_previous" );
	m_previousDifference->setEnabled( false );
	m_nextDifference  = new KAction( i18n("&Next Difference"), "1downarrow", Qt::CTRL + Qt::Key_Down,
	                                 this, SLOT(slotNextDifference()),
	                                 parent->actionCollection(), "difference_next" );
	m_nextDifference->setEnabled( false );

	updateActions();
}

KompareActions::~KompareActions()
{
}

void KompareActions::slotModelsChanged( const QPtrList<Diff2::DiffModel>* modelList )
{
	m_modelList = modelList;
	// these will be set by the following signal, setSelected();
	m_selectedModel = 0L;
	m_selectedDifference = 0L;
	// updating the actions
	updateActions();
}

void KompareActions::slotSetSelection( const Diff2::DiffModel* model, const Diff2::Difference* diff )
{
	m_selectedModel = model;
	m_selectedDifference = diff;

	updateActions();
}

void KompareActions::slotSetSelection( const Diff2::Difference* diff )
{
	m_selectedDifference = diff;

	updateActions();
}

void KompareActions::slotActivated( const Diff2::Difference* diff )
{
	emit selectionChanged( diff );
}

void KompareActions::updateActions()
{
	if( m_modelList && m_selectedModel && m_selectedDifference )
	{
		if ( ( ( KomparePart* )parent() )->isReadWrite() )
		{
			m_applyDifference->setEnabled( true );
			m_applyAll->setEnabled( true );
			m_unapplyAll->setEnabled( true );
			if( m_selectedDifference->applied() )
			{
				m_applyDifference->setText( i18n( "Un&apply Difference" ) );
				m_applyDifference->setIcon( "1leftarrow" );
			}
			else
			{
				m_applyDifference->setText( i18n( "&Apply Difference" ) );
				m_applyDifference->setIcon( "1rightarrow" );
			}
		}
		else
		{
			m_applyDifference->setEnabled( false );
			m_applyAll->setEnabled( false );
			m_unapplyAll->setEnabled( false );
		}

		int modelIndex = m_selectedModel->index();
		int diffIndex  = m_selectedModel->findDifference( const_cast<Diff2::Difference*>(m_selectedDifference) );

		m_previousFile->setEnabled      ( modelIndex > 0 );
		m_nextFile->setEnabled          ( modelIndex < m_modelList->count() - 1 );
		m_previousDifference->setEnabled( diffIndex  > 0 || modelIndex > 0 );
		m_nextDifference->setEnabled    ( modelIndex >= 0 &&
		    ( diffIndex < m_selectedModel->differenceCount() - 1 || modelIndex < m_modelList->count() - 1 ) );
	}
	else
	{
		m_applyDifference->setEnabled( false );
		m_applyAll->setEnabled( false );
		m_unapplyAll->setEnabled( false );

		m_previousFile->setEnabled      ( false );
		m_nextFile->setEnabled          ( false );
		m_previousDifference->setEnabled( false );
		m_nextDifference->setEnabled    ( false );
	}

}

void KompareActions::slotDifferenceActivated( const Diff2::Difference* diff )
{
// totally unnecessary
	emit selectionChanged( diff );
}

void KompareActions::slotApplyDifference()
{
	// Warning hasn't been tested thouroughly...
	bool apply = true;
	if ( m_selectedDifference->applied() )
		apply = false;
	emit applyDifference( apply );

	if( m_nextDifference->isEnabled() )
		slotNextDifference();
}

void KompareActions::slotApplyAllDifferences()
{
	emit applyAllDifferences( true );
	updateActions();
}

void KompareActions::slotUnapplyAllDifferences()
{
	emit applyAllDifferences( false );
	updateActions();
}

void KompareActions::slotPreviousFile()
{
	emit previousModel();
}

void KompareActions::slotNextFile()
{
	emit nextModel();
}

void KompareActions::slotPreviousDifference()
{
	emit previousDifference();
}

void KompareActions::slotNextDifference()
{
	emit nextDifference();
}

#include "kompare_actions.moc"
