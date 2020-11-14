/*
    SPDX-FileCopyrightText: 2004 Jeff Snyder <jeff@caffeinated.me.uk>
    SPDX-FileCopyrightText: 2007-2011 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _KOMPARESPLITTER_H_
#define _KOMPARESPLITTER_H_

#include <QSplitter>

#include <libkomparediff2/komparemodellist.h>

class QSplitterHandle;
class QTimer;
class QScrollBar;
class QWheelEvent;
class QKeyEvent;

namespace Diff2 {
class DiffModel;
class Difference;
}
class ViewSettings;

class KompareListView;
class KompareConnectWidget;

class KompareSplitter : public QSplitter
{
    Q_OBJECT

public:
    KompareSplitter(ViewSettings* settings, QWidget* parent);
    ~KompareSplitter() override;

Q_SIGNALS:
    void configChanged();

    void scrollViewsToId(int id);
    void setXOffset(int x);

    void selectionChanged(const Diff2::Difference* diff);

public Q_SLOTS:
    void slotScrollToId(int id);
    void slotDelayedUpdateScrollBars();
    void slotUpdateScrollBars();
    void slotDelayedUpdateVScrollValue();
    void slotUpdateVScrollValue();
    void keyPressEvent(QKeyEvent* e) override;

    void slotApplyDifference(bool apply);
    void slotApplyAllDifferences(bool apply);
    void slotApplyDifference(const Diff2::Difference* diff, bool apply);

    void slotSetSelection(const Diff2::DiffModel* model, const Diff2::Difference* diff);
    void slotSetSelection(const Diff2::Difference* diff);

    void slotDifferenceClicked(const Diff2::Difference* diff);

    void slotConfigChanged();

protected:
    void wheelEvent(QWheelEvent* e) override;

    ViewSettings* settings() const { return m_settings; }

protected Q_SLOTS:
    void slotDelayedRepaintHandles();
    void slotRepaintHandles();
    void timerTimeout();

private:
    // override from QSplitter
    QSplitterHandle* createHandle() override;

    void               setCursor(int id, const QCursor& cursor);
    void               unsetCursor(int id);

protected:
    KompareListView* listView(int index);
    KompareConnectWidget* connectWidget(int index);

private:

    // Scrollbars. all this just for the goddamn scrollbars. i hate them.
    int  scrollId();
    int  lineHeight();
    int  pageSize();
    bool needVScrollBar();
    int  minVScrollId();
    int  maxVScrollId();
    bool needHScrollBar();
    int  minHScrollId();
    int  maxHScrollId();
    int  maxContentsX();
    int  minVisibleWidth();

    QTimer*            m_scrollTimer;
    bool               m_restartTimer;
    int                m_scrollTo;

    ViewSettings*      m_settings;
    QScrollBar*        m_vScroll;
    QScrollBar*        m_hScroll;

    friend class KompareConnectWidgetFrame;
};
#endif //_KOMPARESPLITTER_H_
