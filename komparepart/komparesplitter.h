/***************************************************************************
                                kompareview.cpp  -  description
                                -------------------
        begin                   : Wed Jan 14 2004
        copyright               : (C) 2004 by Jeff Snyder
        email                   : jeff@caffeinated.me.uk
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#ifndef _KOMPARESPLITTER_H_
#define _KOMPARESPLITTER_H_

#include "kompare_qsplitter.h"

#include <qtimer.h>

#include "komparelistview.h"
#include "komparemodellist.h"

class QSplitterLayoutStruct;
class QTextStream;
class QSplitterHandle;

namespace Diff2 {
class DiffModel;
class Difference;
}
class ViewSettings;

class KompareSplitter : public QSplitter
{
	Q_OBJECT

public:
	KompareSplitter(ViewSettings *settings, QWidget *parent=0, const char *name = 0);
	~KompareSplitter();

signals:
	void selectionChanged( const Diff2::Difference* diff );

	void configChanged();

	void scrollViewsToId( int id );
	void setXOffset( int x );

public slots:
	void slotApplyDifference( bool apply );
	void slotApplyAllDifferences( bool apply );
	void slotApplyDifference( const Diff2::Difference* diff, bool apply );

	// to update the list views
	void slotSetSelection( const Diff2::DiffModel* model, const Diff2::Difference* diff );
	void slotSetSelection( const Diff2::Difference* diff );

	void slotDifferenceClicked( const Diff2::Difference* diff );

	void slotConfigChanged();

	void scrollToId( int id );
	void slotDelayedUpdateScrollBars();
	void slotUpdateScrollBars();
	void slotDelayedUpdateVScrollValue();
	void slotUpdateVScrollValue();

protected:
	void childEvent( QChildEvent * );
	void wheelEvent( QWheelEvent* e );

private slots:
	void slotDelayedRepaintHandles();
	void timerTimeout();

private:
	QSplitterLayoutStruct *addWidget(KompareListViewFrame *w, bool prepend = FALSE );

	void repaintHandles();

	QTimer*            m_scrollTimer;
	bool               restartTimer;
	int                scrollTo;

	// Scrollbars. all this just for the goddamn scrollbars. i hate them.
	// update t+5h: SCROLLBARS ARE THE BANE OF MY LIFE
	int  scrollId();
	int  lineSpacing();
	int  pageSize();
	bool needVScrollBar();
	int  minVScrollId();
	int  maxVScrollId();
	bool needHScrollBar();
	int  maxHScrollId();
	int  maxContentsX();
	int  minVisibleWidth();

	ViewSettings*      m_settings;
	QScrollBar*        m_vScroll;
	QScrollBar*        m_hScroll;
};
#endif //_KOMPARESPLITTER_H_
