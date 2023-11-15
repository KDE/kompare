/*
    SPDX-FileCopyrightText: 2001-2002 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001-2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007-2011 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "viewpage.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QLayout>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QFontComboBox>
#include <QTabWidget>

#include <KColorButton>
#include <KLocalizedString>
#include <KSharedConfig>

#include "viewsettings.h"

ViewPage::ViewPage() : QFrame()
{
    QWidget*     page;
    QVBoxLayout* layout;
    QFormLayout* formLayout;
    QGroupBox*   colorGroupBox;
    QGroupBox*   snolGroupBox;
    QGroupBox*   tabGroupBox;

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setDocumentMode(true);
    layout = new QVBoxLayout(this);
    layout->addWidget(m_tabWidget);
    layout->setContentsMargins({});

    page   = new QWidget(m_tabWidget);
    layout = new QVBoxLayout(page);

    // add a groupbox
    colorGroupBox = new QGroupBox(page);
    colorGroupBox->setTitle(i18nc("@title:group", "Colors"));
    layout->addWidget(colorGroupBox);
    formLayout = new QFormLayout(colorGroupBox);

    // add the removeColor
    m_removedColorButton = new KColorButton(colorGroupBox);
    formLayout->addRow(i18nc("@label:chooser", "Removed color:"), m_removedColorButton);

    // add the changeColor
    m_changedColorButton = new KColorButton(colorGroupBox);
    formLayout->addRow(i18nc("@label:chooser", "Changed color:"), m_changedColorButton);

    // add the addColor
    m_addedColorButton = new KColorButton(colorGroupBox);
    formLayout->addRow(i18nc("@label:chooser", "Added color:"), m_addedColorButton);

    // add the appliedColor
    m_appliedColorButton = new KColorButton(colorGroupBox);
    formLayout->addRow(i18nc("@label:chooser", "Applied color:"), m_appliedColorButton);

    // scroll number of lines (snol)
    snolGroupBox = new QGroupBox(page);
    snolGroupBox->setTitle(i18nc("@title:group", "Mouse Wheel"));
    layout->addWidget(snolGroupBox);
    QFormLayout* snolLayout = new QFormLayout(snolGroupBox);

    m_snolSpinBox = new QSpinBox(snolGroupBox);
    m_snolSpinBox->setRange(0, 50);
    snolLayout->addRow(i18nc("@label:spinbox", "Number of lines:"), m_snolSpinBox);

    // Temporarily here for testing...
    // number of spaces for a tab character stuff
    tabGroupBox = new QGroupBox(page);
    tabGroupBox->setTitle(i18nc("@title:group", "Tabs to Spaces"));
    layout->addWidget(tabGroupBox);
    QFormLayout* tabLayout = new QFormLayout(tabGroupBox);

    m_tabSpinBox = new QSpinBox(tabGroupBox);
    m_tabSpinBox->setRange(1, 16);
    tabLayout->addRow(i18nc("@label:spinbox", "Number of spaces to convert a tab character to:"), m_tabSpinBox);

    layout->addStretch(1);

    m_tabWidget->addTab(page, i18nc("@title:tab", "Appearance"));

    page   = new QWidget(m_tabWidget);
    layout = new QVBoxLayout(page);

    QFormLayout* layfont = new QFormLayout(page);
    layout->addLayout(layfont);

    m_fontCombo = new QFontComboBox(page);
    layfont->addRow(i18nc("@label:listbox", "Font:"), m_fontCombo);
    m_fontCombo->setObjectName(QStringLiteral("fontcombo"));

    m_fontSizeSpinBox = new QSpinBox(page);
    m_fontSizeSpinBox->setRange(6, 24);
    layfont->addRow(i18nc("@label:spinbox", "Size:"), m_fontSizeSpinBox);

    layout->addStretch(1);

    m_tabWidget->addTab(page, i18nc("@title:tab", "Fonts"));
}

ViewPage::~ViewPage()
{

}

void ViewPage::setSettings(ViewSettings* setts)
{
    m_settings = setts;

    m_addedColorButton->setColor(m_settings->m_addColor);
    m_changedColorButton->setColor(m_settings->m_changeColor);
    m_removedColorButton->setColor(m_settings->m_removeColor);
    m_appliedColorButton->setColor(m_settings->m_appliedColor);
    m_snolSpinBox->setValue(m_settings->m_scrollNoOfLines);
    m_tabSpinBox->setValue(m_settings->m_tabToNumberOfSpaces);

    m_fontCombo->setCurrentFont(m_settings->m_font.family());
    m_fontSizeSpinBox->setValue(m_settings->m_font.pointSize());
}

ViewSettings* ViewPage::settings()
{
    return m_settings;
}

void ViewPage::restore()
{
}

void ViewPage::apply()
{
    m_settings->m_addColor            = m_addedColorButton->color();
    m_settings->m_changeColor         = m_changedColorButton->color();
    m_settings->m_removeColor         = m_removedColorButton->color();
    m_settings->m_appliedColor        = m_appliedColorButton->color();
    m_settings->m_scrollNoOfLines     = m_snolSpinBox->value();
    m_settings->m_tabToNumberOfSpaces = m_tabSpinBox->value();

    m_settings->m_font                = QFont(m_fontCombo->currentFont());
    m_settings->m_font.setPointSize(m_fontSizeSpinBox->value());

    m_settings->saveSettings(KSharedConfig::openConfig().data());
}

void ViewPage::setDefaults()
{
    m_addedColorButton->setColor(ViewSettings::default_addColor);
    m_changedColorButton->setColor(ViewSettings::default_changeColor);
    m_removedColorButton->setColor(ViewSettings::default_removeColor);
    m_appliedColorButton->setColor(ViewSettings::default_appliedColor);
    m_snolSpinBox->setValue(3);
    m_tabSpinBox->setValue(4);

    // TODO: port
    // m_fontCombo->setCurrentFont   ( KGlobalSettings::fixedFont().family() );
    m_fontSizeSpinBox->setValue(10);
}

#include "moc_viewpage.cpp"
