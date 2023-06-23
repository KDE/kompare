/*
    SPDX-FileCopyrightText: 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2001-2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2007-2011 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "komparesaveoptionswidget.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>

#include <KIO/Global>
#include <KUrlRequester>

#include <KompareDiff2/DiffSettings>

KompareSaveOptionsWidget::KompareSaveOptionsWidget(const QString& source, const QString& destination,
                                                   DiffSettings* settings, QWidget* parent)
    : KompareSaveOptionsBase(parent)
    , m_source(source)
    , m_destination(destination)
    , m_FormatBG(new QButtonGroup(this))
{
    setObjectName(QStringLiteral("save options"));

    m_settings = settings;

    m_directoryRequester->setMode(
        KFile::ExistingOnly | KFile::Directory | KFile::LocalOnly);

    QUrl sourceURL;
    QUrl destinationURL;
    sourceURL.setPath(source);
    destinationURL.setPath(destination);

    // Find a common root.
    QUrl root(sourceURL);
    while (root.isValid() && !root.isParentOf(destinationURL) && root != KIO::upUrl(root)) {
        root = KIO::upUrl(root);
    }

    // If we found a common root, change to that directory and
    // strip the common part from source and destination.
    if (root.isValid() && root != KIO::upUrl(root)) {
        m_directoryRequester->setUrl(root);
    }

    connect(m_SmallerChangesCB,   &QCheckBox::toggled, this, &KompareSaveOptionsWidget::updateCommandLine);
    connect(m_LargeFilesCB,       &QCheckBox::toggled, this, &KompareSaveOptionsWidget::updateCommandLine);
    connect(m_IgnoreCaseCB,       &QCheckBox::toggled, this, &KompareSaveOptionsWidget::updateCommandLine);
    connect(m_ExpandTabsCB,       &QCheckBox::toggled, this, &KompareSaveOptionsWidget::updateCommandLine);
    connect(m_IgnoreEmptyLinesCB, &QCheckBox::toggled, this, &KompareSaveOptionsWidget::updateCommandLine);
    connect(m_IgnoreWhiteSpaceCB, &QCheckBox::toggled, this, &KompareSaveOptionsWidget::updateCommandLine);
    connect(m_FunctionNamesCB,    &QCheckBox::toggled, this, &KompareSaveOptionsWidget::updateCommandLine);
    connect(m_RecursiveCB,        &QCheckBox::toggled, this, &KompareSaveOptionsWidget::updateCommandLine);
    connect(m_NewFilesCB,         &QCheckBox::toggled, this, &KompareSaveOptionsWidget::updateCommandLine);
    connect(m_ContextRB,          &QCheckBox::toggled, this, &KompareSaveOptionsWidget::updateCommandLine);
    connect(m_EdRB,               &QCheckBox::toggled, this, &KompareSaveOptionsWidget::updateCommandLine);
    connect(m_NormalRB,           &QCheckBox::toggled, this, &KompareSaveOptionsWidget::updateCommandLine);
    connect(m_RCSRB,              &QCheckBox::toggled, this, &KompareSaveOptionsWidget::updateCommandLine);
    connect(m_UnifiedRB,          &QCheckBox::toggled, this, &KompareSaveOptionsWidget::updateCommandLine);
    connect(m_SideBySideRB,       &QCheckBox::toggled, this, &KompareSaveOptionsWidget::updateCommandLine);
    connect(m_ContextLinesSB,     &QSpinBox::textChanged,
            this, &KompareSaveOptionsWidget::updateCommandLine);
    connect(m_directoryRequester, &KUrlRequester::textChanged, this, &KompareSaveOptionsWidget::updateCommandLine);

    m_FormatBG->setExclusive(true);
    m_FormatBG->addButton(m_ContextRB, Kompare::Context);
    m_FormatBG->addButton(m_EdRB, Kompare::Ed);
    m_FormatBG->addButton(m_NormalRB, Kompare::Normal);
    m_FormatBG->addButton(m_UnifiedRB, Kompare::Unified);
    m_FormatBG->addButton(m_RCSRB, Kompare::RCS);
    m_FormatBG->addButton(m_SideBySideRB, Kompare::SideBySide);

    loadOptions();
}

KompareSaveOptionsWidget::~KompareSaveOptionsWidget()
{

}

QString KompareSaveOptionsWidget::directory() const
{
    return QUrl(m_directoryRequester->url()).toLocalFile();
}

void KompareSaveOptionsWidget::updateCommandLine()
{
    QString cmdLine = QStringLiteral("diff");

    QString options;

    switch (static_cast<Kompare::Format>(m_FormatBG->checkedId())) {
    case Kompare::Unified :
        cmdLine += QLatin1String(" -U ") + QString::number(m_ContextLinesSB->value());
        break;
    case Kompare::Context :
        cmdLine += QLatin1String(" -C ") + QString::number(m_ContextLinesSB->value());
        break;
    case Kompare::RCS :
        options += QLatin1Char('n');
        break;
    case Kompare::Ed :
        options += QLatin1Char('e');
        break;
    case Kompare::SideBySide:
        options += QLatin1Char('y');
        break;
    case Kompare::Normal :
    case Kompare::UnknownFormat :
    default:
        break;
    }

    if (m_SmallerChangesCB->isChecked()) {
        options += QLatin1Char('d');
    }

    if (m_LargeFilesCB->isChecked()) {
        options += QLatin1Char('H');
    }

    if (m_IgnoreCaseCB->isChecked()) {
        options += QLatin1Char('i');
    }

    if (m_ExpandTabsCB->isChecked()) {
        options += QLatin1Char('t');
    }

    if (m_IgnoreEmptyLinesCB->isChecked()) {
        options += QLatin1Char('B');
    }

    if (m_IgnoreWhiteSpaceCB->isChecked()) {
        options += QLatin1Char('b');
    }

    if (m_FunctionNamesCB->isChecked()) {
        options += QLatin1Char('p');
    }

//     if (  ) {
//         cmdLine += " -w";
//     }

    if (m_RecursiveCB->isChecked()) {
        options += QLatin1Char('r');
    }

    if (m_NewFilesCB->isChecked()) {
        options += QLatin1Char('N');
    }

//     if (m_AllTextCB->isChecked()) {
//         options += QLatin1Char('a');
//     }

    if (options.length() > 0) {
        cmdLine += QStringLiteral(" -") + options;
    }

    QDir dir(directory());
    cmdLine += QStringLiteral(" -- ");
    cmdLine += dir.relativeFilePath(m_source);
    cmdLine += QLatin1Char(' ');
    cmdLine += dir.relativeFilePath(m_destination);

    m_CommandLineLabel->setText(cmdLine);
}

void KompareSaveOptionsWidget::loadOptions()
{
    m_SmallerChangesCB->setChecked(m_settings->m_createSmallerDiff);
    m_LargeFilesCB->setChecked(m_settings->m_largeFiles);
    m_IgnoreCaseCB->setChecked(m_settings->m_ignoreChangesInCase);
    m_ExpandTabsCB->setChecked(m_settings->m_convertTabsToSpaces);
    m_IgnoreEmptyLinesCB->setChecked(m_settings->m_ignoreEmptyLines);
    m_IgnoreWhiteSpaceCB->setChecked(m_settings->m_ignoreWhiteSpace);
    m_FunctionNamesCB->setChecked(m_settings->m_showCFunctionChange);
    m_RecursiveCB->setChecked(m_settings->m_recursive);
    m_NewFilesCB->setChecked(m_settings->m_newFiles);
//     m_AllTextCB->setChecked( m_settings->m_allText );

    m_ContextLinesSB->setValue(m_settings->m_linesOfContext);

    m_FormatBG->button(m_settings->m_format)->setChecked(true);

    updateCommandLine();
}

void KompareSaveOptionsWidget::saveOptions()
{
    m_settings->m_createSmallerDiff   = m_SmallerChangesCB->isChecked();
    m_settings->m_largeFiles          = m_LargeFilesCB->isChecked();
    m_settings->m_ignoreChangesInCase = m_IgnoreCaseCB->isChecked();
    m_settings->m_convertTabsToSpaces = m_ExpandTabsCB->isChecked();
    m_settings->m_ignoreEmptyLines    = m_IgnoreEmptyLinesCB->isChecked();
    m_settings->m_ignoreWhiteSpace    = m_IgnoreWhiteSpaceCB->isChecked();
    m_settings->m_showCFunctionChange = m_FunctionNamesCB->isChecked();
    m_settings->m_recursive           = m_RecursiveCB->isChecked();
    m_settings->m_newFiles            = m_NewFilesCB->isChecked();
//     m_settings->m_allText             = m_AllTextCB->isChecked();

    m_settings->m_linesOfContext      = m_ContextLinesSB->value();

    m_settings->m_format = static_cast<Kompare::Format>(m_FormatBG->checkedId());

}

#include "moc_komparesaveoptionswidget.cpp"
