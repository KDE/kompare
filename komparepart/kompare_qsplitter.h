/****************************************************************************
**
**
** Definition of QSplitter class
**
**  Created : 980105
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
**********************************************************************/

#ifndef QSPLITTER_H
#define QSPLITTER_H

#ifndef QT_H
#include "qframe.h"
#include "qvaluelist.h"
#endif // QT_H

#ifndef QT_NO_SPLITTER


//class QSplitterHandle;
class QSplitterLayoutStruct;
class QTextStream;

class QSplitterPrivate
{
public:
    QSplitterPrivate()
	: opaque( FALSE ), firstShow( TRUE ), childrenCollapsible( TRUE ),
	  handleWidth( 0 ) { }

    QPtrList<QSplitterLayoutStruct> list;
    bool opaque : 8;
    bool firstShow : 8;
    bool childrenCollapsible : 8;
    int handleWidth;
};

class Q_EXPORT QSplitter : public QFrame
{
    Q_OBJECT
    Q_PROPERTY( Orientation orientation READ orientation WRITE setOrientation )
    Q_PROPERTY( bool opaqueResize READ opaqueResize WRITE setOpaqueResize )
    Q_PROPERTY( int handleWidth READ handleWidth WRITE setHandleWidth )
    Q_PROPERTY( bool childrenCollapsible READ childrenCollapsible WRITE setChildrenCollapsible )

public:
    // ### Qt 4.0: remove Auto from public API
    enum ResizeMode { Stretch, KeepSize, FollowSizeHint, Auto };

    QSplitter( QWidget* parent = 0, const char* name = 0 );
    QSplitter( Orientation, QWidget* parent = 0, const char* name = 0 );
    ~QSplitter();

    virtual void setOrientation( Orientation );
    Orientation orientation() const { return orient; }

    // ### Qt 4.0: make setChildrenCollapsible() and setCollapsible() virtual

    void setChildrenCollapsible( bool );
    bool childrenCollapsible() const;

    void setCollapsible( QWidget *w, bool );
    virtual void setResizeMode( QWidget *w, ResizeMode );
    virtual void setOpaqueResize( bool = TRUE );
    bool opaqueResize() const;

    void moveToFirst( QWidget * );
    void moveToLast( QWidget * );

    void refresh() { recalc( TRUE ); }
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    QValueList<int> sizes() const;
    void setSizes( QValueList<int> );

    int handleWidth() const;
    void setHandleWidth( int );

protected:
    void childEvent( QChildEvent * );

    bool event( QEvent * );
    void resizeEvent( QResizeEvent * );

    int idAfter( QWidget* ) const;

    void moveSplitter( QCOORD pos, int id );
    virtual void drawSplitter( QPainter*, QCOORD x, QCOORD y,
			       QCOORD w, QCOORD h );
    void styleChange( QStyle& );
    int adjustPos( int, int );
    virtual void setRubberband( int );
    void getRange( int id, int *, int * );

public: // private (:
    enum { DefaultResizeMode = 3 };

    void init();
    void recalc( bool update = FALSE );
    void doResize();
    void storeSizes();
    void getRange( int id, int *, int *, int *, int * );
    void addContribution( int, int *, int *, bool );
    int adjustPos( int, int, int *, int *, int *, int * );
    bool collapsible( QSplitterLayoutStruct * );
    void processChildEvents();
    QSplitterLayoutStruct *findWidget( QWidget * );
    QSplitterLayoutStruct *addWidget( QWidget *, bool prepend = FALSE );
    void recalcId();
    void doMove( bool backwards, int pos, int id, int delta, bool upLeft,
		 bool mayCollapse );
    void setGeo( QWidget *w, int pos, int size, bool splitterMoved );
    int findWidgetJustBeforeOrJustAfter( int id, int delta, int &collapsibleSize );
    void updateHandles();

    inline QCOORD pick( const QPoint &p ) const
    { return orient == Horizontal ? p.x() : p.y(); }
    inline QCOORD pick( const QSize &s ) const
    { return orient == Horizontal ? s.width() : s.height(); }

    inline QCOORD trans( const QPoint &p ) const
    { return orient == Vertical ? p.x() : p.y(); }
    inline QCOORD trans( const QSize &s ) const
    { return orient == Vertical ? s.width() : s.height(); }

    QSplitterPrivate *d;

    Orientation orient;
    friend class QSplitterHandle;

#ifndef QT_NO_TEXTSTREAM
// moc doesn't like these.
//    friend Q_EXPORT QTextStream& operator<<( QTextStream&, const QSplitter& );
//    friend Q_EXPORT QTextStream& operator>>( QTextStream&, QSplitter& );
#endif

public:
#if defined(Q_DISABLE_COPY)
    QSplitter( const QSplitter & );
    QSplitter& operator=( const QSplitter & );
#endif
};

#ifndef QT_NO_TEXTSTREAM
Q_EXPORT QTextStream& operator<<( QTextStream&, const QSplitter& );
Q_EXPORT QTextStream& operator>>( QTextStream&, QSplitter& );
#endif

class QSplitterHandle : public QWidget
{
    Q_OBJECT
public:
    QSplitterHandle( Orientation o,
		     QSplitter *parent, const char* name=0 );
    void setOrientation( Orientation o );
    Orientation orientation() const { return orient; }

    bool opaque() const { return s->opaqueResize(); }

    QSize sizeHint() const;

    int id() const { return myId; } // d->list.at(id())->wid == this
    void setId( int i ) { myId = i; }

protected:
    void paintEvent( QPaintEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );

private:
    Orientation orient;
    bool opaq;
    int myId;

    QSplitter *s;
};

const uint Default = 2;

class QSplitterLayoutStruct : public Qt
{
public:
    QCOORD sizer;
    uint isHandle : 1;
    uint collapsible : 2;
    uint resizeMode : 2;
    QWidget *wid;

    QSplitterLayoutStruct()
	: sizer( -1 ), collapsible( Default ) { }
    QCOORD getSizer( Orientation orient );
};

#endif // QT_NO_SPLITTER

#endif // QSPLITTER_H
