/*
    SPDX-FileCopyrightText: 2001-2009 Otto Bruggeman <bruggie@gmail.com>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kompareprefdlg.h"

#include <QTabWidget>
#include <QPushButton>

#include <KLocalizedString>
#include <KHelpClient>
#include <KStandardGuiItem>

#include "diffpage.h"
#include "viewpage.h"

// implementation

KomparePrefDlg::KomparePrefDlg(ViewSettings* viewSets, DiffSettings* diffSets) : KPageDialog(nullptr)
{
    setFaceType(KPageDialog::List);
    setWindowTitle(i18nc("@title:window", "Preferences"));
    setStandardButtons(QDialogButtonBox::Help | QDialogButtonBox::Reset | QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
    setModal(true);

    KGuiItem::assign(button(QDialogButtonBox::Reset), KStandardGuiItem::defaults());

    // ok i need some stuff in that pref dlg...
    //setIconListAllVisible(true);

    m_viewPage = new ViewPage();
    KPageWidgetItem* item = addPage(m_viewPage, i18nc("@title:tab", "View"));
    item->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-theme")));
    item->setHeader(i18nc("@title", "View Settings"));
    m_viewPage->setSettings(viewSets);

    m_diffPage = new DiffPage();
    item = addPage(m_diffPage, i18nc("@title:tab", "Diff"));
    item->setIcon(QIcon::fromTheme(QStringLiteral("text-x-patch")));
    item->setHeader(i18nc("@title", "Diff Settings"));
    m_diffPage->setSettings(diffSets);

//     frame = addVBoxPage(i18n(""), i18n(""), UserIcon(""));

    connect(button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &KomparePrefDlg::slotDefault);
    connect(button(QDialogButtonBox::Help), &QPushButton::clicked, this, &KomparePrefDlg::slotHelp);
    connect(button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &KomparePrefDlg::slotApply);
    connect(button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &KomparePrefDlg::slotOk);
    connect(button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &KomparePrefDlg::slotCancel);

    adjustSize();
}

KomparePrefDlg::~KomparePrefDlg()
{

}

/** No descriptions */
void KomparePrefDlg::slotDefault()
{
    // restore all defaults in the options...
    m_viewPage->setDefaults();
    m_diffPage->setDefaults();
}

/** No descriptions */
void KomparePrefDlg::slotHelp()
{
    // figure out the current active page
    QWidget* currentpage = currentPage()->widget();
    if (dynamic_cast<ViewPage*>(currentpage))
    {
        // figure out the active tab
        int currentTab = static_cast<ViewPage*>(currentpage)->m_tabWidget->currentIndex();
        switch (currentTab)
        {
        case 0:
            KHelpClient::invokeHelp(QStringLiteral("appearance"));
            break;
        case 1:
            KHelpClient::invokeHelp(QStringLiteral("fonts"));
            break;
        default:
            KHelpClient::invokeHelp(QStringLiteral("view-settings"));
        }
    }
    else if (dynamic_cast<DiffPage*>(currentpage))
    {
        // figure out the active tab
        int currentTab = static_cast<DiffPage*>(currentpage)->m_tabWidget->currentIndex();
        switch (currentTab)
        {
        case 0:
            KHelpClient::invokeHelp(QStringLiteral("diff"));
            break;
        case 1:
            KHelpClient::invokeHelp(QStringLiteral("diff-format"));
            break;
        case 2:
            KHelpClient::invokeHelp(QStringLiteral("options"));
            break;
        case 3:
            KHelpClient::invokeHelp(QStringLiteral("exclude"));
            break;
        default:
            KHelpClient::invokeHelp(QStringLiteral("diff-settings"));
        }
    }
    else // Fallback since we had not added the code for the page/tab or forgotten about it
        KHelpClient::invokeHelp(QStringLiteral("configure-preferences"));
}

/** No descriptions */
void KomparePrefDlg::slotApply()
{
    // well apply the settings that are currently selected
    m_viewPage->apply();
    m_diffPage->apply();

    Q_EMIT configChanged();
}

/** No descriptions */
void KomparePrefDlg::slotOk()
{
    // Apply the settings that are currently selected
    m_viewPage->apply();
    m_diffPage->apply();

    //accept();
}

/** No descriptions */
void KomparePrefDlg::slotCancel()
{
    // discard the current settings and use the present ones
    m_viewPage->restore();
    m_diffPage->restore();

    //reject();
}
