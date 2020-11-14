/*
    SPDX-FileCopyrightText: 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007-2011 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPARESAVEOPTIONSBASE_H
#define KOMPARESAVEOPTIONSBASE_H

#include <QVariant>

#include "ui_komparesaveoptionsbase.h"

class KompareSaveOptionsBase : public QWidget, public Ui::KompareSaveOptionsBase
{
    Q_OBJECT

public:
    explicit KompareSaveOptionsBase(QWidget* parent = nullptr);
    ~KompareSaveOptionsBase() override;

protected Q_SLOTS:
    virtual void languageChange();

};

#endif // KOMPARESAVEOPTIONSBASE_H
