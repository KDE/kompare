/*
    SPDX-FileCopyrightText: 2011 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// associated header
#include "kompareview.h"

// kompare
#include "komparesplitter.h"

KompareView::KompareView(ViewSettings* settings, QWidget* parent) :
    QFrame(parent)
{
    setObjectName(QStringLiteral("scrollFrame"));
    m_splitter = new KompareSplitter(settings, this);
}

#include "moc_kompareview.cpp"
