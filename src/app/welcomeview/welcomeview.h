/*
    SPDX-FileCopyrightText: 2022 Jiří Wolker <woljiri@gmail.com>
    SPDX-FileCopyrightText: 2022 Eugene Popov <popov895@ukr.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WELCOMEVIEW_H
#define WELCOMEVIEW_H

#include "ui_welcomeview.h"

class KompareShell;

class WelcomeView : public QScrollArea, Ui::WelcomeView
{
    Q_OBJECT

public:
    explicit WelcomeView(KompareShell *mainWindow, QWidget *parent = nullptr);

protected:
    bool event(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateButtons();
    void updateFonts();
    bool updateLayout();

    KompareShell *m_mainWindow = nullptr;
};

#endif // WELCOMEVIEW_H
