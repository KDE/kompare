/*
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2001-2004 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2004 Jeff Snyder <jeff@caffeinated.me.uk>
    SPDX-FileCopyrightText: 2007 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPARECONNECTWIDGET_H
#define KOMPARECONNECTWIDGET_H

#include <QWidget>
#include <QSplitter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QLabel>

#include <KompareDiff2/KompareModelList>

namespace KompareDiff2 {
class DiffModel;
}
class ViewSettings;
class KompareSplitter;

class KompareConnectWidget : public QWidget
{
    Q_OBJECT

public:
    KompareConnectWidget(ViewSettings* settings, QWidget* parent, const char* name = nullptr);
    ~KompareConnectWidget() override;

public Q_SLOTS:
    void slotSetSelection(const KompareDiff2::DiffModel* model, const KompareDiff2::Difference* diff);
    void slotSetSelection(const KompareDiff2::Difference* diff);

    void slotDelayedRepaint();

Q_SIGNALS:
    void selectionChanged(const KompareDiff2::Difference* diff);

protected:
    void paintEvent(QPaintEvent* e) override;
    QPainterPath makeBezier(int l, int r) const;

private:
    ViewSettings*             m_settings;

    const KompareDiff2::DiffModel*   m_selectedModel;
    const KompareDiff2::Difference*  m_selectedDifference;
};

class KompareConnectWidgetFrame : public QSplitterHandle
{
    Q_OBJECT
public:
    KompareConnectWidgetFrame(ViewSettings* settings, KompareSplitter* parent, const char* name = nullptr);
    ~KompareConnectWidgetFrame() override;

    QSize sizeHint() const override;

    KompareConnectWidget*     wid() { return &m_wid; }

protected:
    // stop the parent QSplitterHandle painting
    void paintEvent(QPaintEvent* /* e */) override { }

#if 0
    void mouseMoveEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
#endif

private:
    KompareConnectWidget      m_wid;
    QLabel                    m_label;
    QVBoxLayout               m_layout;
};

#endif
