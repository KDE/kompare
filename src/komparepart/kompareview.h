/*
    SPDX-FileCopyrightText: 2011 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _KOMPAREVIEW_H_
#define _KOMPAREVIEW_H_

#include <QFrame>

#include <KompareDiff2/KompareModelList>

class ViewSettings;
class KompareSplitter;

class KompareView : public QFrame
{
    Q_OBJECT

public:
    KompareView(ViewSettings* settings, QWidget* parent);
    ~KompareView() override {}
    KompareSplitter* splitter() { return m_splitter; }

private:
    KompareSplitter* m_splitter;
};
#endif //_KOMPARESPLITTER_H_
