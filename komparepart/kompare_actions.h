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

#ifndef _KOMPAREERENCESACTION_H_
#define _KOMPAREERENCESACTION_H_

#include <kaction.h>

class QPopupMenu;
class QWidget;

class DiffModel;

class KompareerencesAction : public KAction
{
  Q_OBJECT
public:
    KompareerencesAction( const QString & text, QObject* parent = 0, const char* name = 0 );

    virtual ~KompareerencesAction() {};

    virtual int plug( QWidget *widget, int index = -1 );
    //virtual void unplug( QWidget *widget );

    void fillDifferenceMenu( const DiffModel* model, int current );

protected slots:
    void slotActivated( int );

signals:
    void menuAboutToShow();
    // -1 for one step back, 0 for don't move, +1 for one step forward, etc.
    void activated( int );
private:
    uint m_firstIndex; // first index in the Go menu
    int m_startPos;
    int m_currentPos; // == history.at()
    QPopupMenu *m_differenceMenu; // hack
};

#endif
