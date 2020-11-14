/*
    SPDX-FileCopyrightText: 2001-2009 Otto Bruggeman <bruggie@gmail.com>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREPREFDLG_H
#define KOMPAREPREFDLG_H

#include <KPageDialog>

class DiffPage;
class DiffSettings;
class ViewPage;
class ViewSettings;

class KomparePrefDlg : public KPageDialog
{
    Q_OBJECT
public:
    KomparePrefDlg(ViewSettings*, DiffSettings*);
    ~KomparePrefDlg() override;

protected Q_SLOTS:
    /** No descriptions */
    virtual void slotOk();
    /** No descriptions */
    virtual void slotApply();
    /** No descriptions */
    virtual void slotHelp();
    /** No descriptions */
    virtual void slotDefault();
    /** No descriptions */
    virtual void slotCancel();

Q_SIGNALS:
    void configChanged();

private:
    ViewPage* m_viewPage;
    DiffPage* m_diffPage;
};

#endif
