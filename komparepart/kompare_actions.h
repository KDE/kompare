#ifndef _KDIFFERENCESACTION_H_
#define _KDIFFERENCESACTION_H_

#include <kaction.h>

class QWidget;
class QPopupMenu;

class DiffModel;

class KDifferencesAction : public KAction
{
  Q_OBJECT
public:
    KDifferencesAction( const QString & text, QObject* parent = 0, const char* name = 0 );

    virtual ~KDifferencesAction() {};

    virtual int plug( QWidget *widget, int index = -1 );
    //virtual void unplug( QWidget *widget );

    void fillDifferenceMenu( DiffModel* model, int current );

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
