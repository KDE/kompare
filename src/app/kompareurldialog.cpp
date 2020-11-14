/*
    SPDX-FileCopyrightText: 2001-2005, 2009 Otto Bruggeman <bruggie@gmail.com>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kompareurldialog.h"
#include <QShowEvent>

#include <QGroupBox>

#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KUrlComboBox>
#include <KUrlRequester>

#include <libkomparediff2/diffsettings.h>

#include <kompareshelldebug.h>
#include "diffpage.h"
#include "filespage.h"
#include "filessettings.h"
#include "viewpage.h"
#include "viewsettings.h"


KompareURLDialog::KompareURLDialog(QWidget* parent)
    : KPageDialog(parent)
{
    setFaceType(List);
    KSharedConfig::Ptr cfg = KSharedConfig::openConfig();

    m_filesPage = new FilesPage();
    KPageWidgetItem* filesItem = addPage(m_filesPage, i18nc("@title:tab", "Files"));
    filesItem->setIcon(QIcon::fromTheme(QStringLiteral("text-plain")));
    filesItem->setHeader(i18n("Here you can enter the files you want to compare."));
    m_filesSettings = new FilesSettings(this);
    m_filesSettings->loadSettings(cfg.data());
    m_filesPage->setSettings(m_filesSettings);

    m_diffPage = new DiffPage();
    KPageWidgetItem* diffItem = addPage(m_diffPage, i18nc("@title:tab", "Diff"));
    diffItem->setIcon(QIcon::fromTheme(QStringLiteral("text-x-patch")));
    diffItem->setHeader(i18n("Here you can change the options for comparing the files."));
    m_diffSettings = new DiffSettings(this);
    m_diffSettings->loadSettings(cfg.data());
    m_diffPage->setSettings(m_diffSettings);

    m_viewPage = new ViewPage();
    KPageWidgetItem* viewItem = addPage(m_viewPage, i18nc("@title:tab", "Appearance"));
    viewItem->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-theme")));
    viewItem->setHeader(i18n("Here you can change the options for the view."));
    m_viewSettings = new ViewSettings(this);
    m_viewSettings->loadSettings(cfg.data());
    m_viewPage->setSettings(m_viewSettings);

    adjustSize();

    connect(m_filesPage->firstURLRequester(), &KUrlRequester::textChanged,
            this, &KompareURLDialog::slotEnableOk);
    connect(m_filesPage->secondURLRequester(), &KUrlRequester::textChanged,
            this, &KompareURLDialog::slotEnableOk);
    connect(m_filesPage, &FilesPage::urlChanged,
            this, &KompareURLDialog::slotEnableOk);
}

KompareURLDialog::~KompareURLDialog()
{
}

void KompareURLDialog::showEvent(QShowEvent* event)
{
    if (!event->spontaneous())
    {
        slotEnableOk();
    }
}

void KompareURLDialog::reject()
{
    m_filesPage->restore();
    m_diffPage->restore();
    m_viewPage->restore();

    QDialog::reject();
}

void KompareURLDialog::accept()
{
    // BUG: 124121 File with filenames to be excluded does not exist so diff complains and no diffs are generated
    qCDebug(KOMPARESHELL) << "Test to see if the file is an actual file that is given in the file with filenames to exclude field" ;
    if (m_diffPage->m_excludeFileNameGroupBox->isChecked())
    {
        qCDebug(KOMPARESHELL) << "Ok the checkbox is active..." ;
        if (QFileInfo(m_diffPage->m_excludeFileURLComboBox->currentText()).isDir())
        {
            qCDebug(KOMPARESHELL) << "Don't enter directory names where filenames are expected..." ;
            KMessageBox::sorry(this, i18n("File used for excluding files cannot be found, please specify another file."));
            reject();
            return;
        }
    }
    // Room for more checks for invalid input

    m_filesPage->setURLsInComboBoxes();

    KSharedConfig::Ptr cfg = KSharedConfig::openConfig();

    m_filesPage->apply();
    m_diffPage->apply();
    m_viewPage->apply();

    m_filesSettings->saveSettings(cfg.data());
    m_diffSettings->saveSettings(cfg.data());
    m_viewSettings->saveSettings(cfg.data());

    cfg->sync();

    QDialog::accept();
}

void KompareURLDialog::slotEnableOk()
{
    button(QDialogButtonBox::Ok)->setEnabled(!m_filesPage->firstURLRequester()->url().isEmpty() &&
            !m_filesPage->secondURLRequester()->url().isEmpty());
}

/**
 * Returns the first URL, which was entered.
 * @return first URL
 */
QUrl KompareURLDialog::getFirstURL() const
{
    return m_filesPage->firstURLRequester()->url();
}

/**
 * Returns the second URL, which was entered.
 * @return second URL
 */
QUrl KompareURLDialog::getSecondURL() const
{
    return m_filesPage->secondURLRequester()->url();
}

/**
 * Returns the encoding.
 * @return encoding
 */
QString KompareURLDialog::encoding() const
{
    return m_filesPage->encoding();
}

void KompareURLDialog::setFirstGroupBoxTitle(const QString& title)
{
    m_filesPage->setFirstGroupBoxTitle(title);
}

void KompareURLDialog::setSecondGroupBoxTitle(const QString& title)
{
    m_filesPage->setSecondGroupBoxTitle(title);
}

void KompareURLDialog::setGroup(const QString& groupName)
{
    m_filesSettings->setGroup(groupName);
    m_filesSettings->loadSettings(KSharedConfig::openConfig().data());
    m_filesPage->setSettings(m_filesSettings);
}

void KompareURLDialog::setFirstURLRequesterMode(unsigned int mode)
{
    m_filesPage->setFirstURLRequesterMode(mode);
}

void KompareURLDialog::setSecondURLRequesterMode(unsigned int mode)
{
    m_filesPage->setSecondURLRequesterMode(mode);
}
