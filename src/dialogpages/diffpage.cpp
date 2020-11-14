/*
	SPDX-FileCopyrightText: 2001-2004 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2007-2011 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "diffpage.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QToolTip>
#include <QButtonGroup>

#include <KComboBox>
#include <KEditListWidget>
#include <KLineEdit>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KUrlComboBox>
#include <KUrlRequester>

#include <libkomparediff2/diffsettings.h>

DiffPage::DiffPage() : QFrame(), m_ignoreRegExpDialog(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    m_tabWidget = new QTabWidget(this);

    addDiffTab();

    addFormatTab();

    addOptionsTab();

    addExcludeTab();
    layout->addWidget(m_tabWidget);
}

DiffPage::~DiffPage()
{
    m_settings = nullptr;
}

void DiffPage::setSettings(DiffSettings* setts)
{
    m_settings = setts;

    m_diffURLRequester->setText(m_settings->m_diffProgram);

    m_newFilesCheckBox->setChecked(m_settings->m_newFiles);
    m_smallerCheckBox->setChecked(m_settings->m_createSmallerDiff);
    m_largerCheckBox->setChecked(m_settings->m_largeFiles);
    m_tabsCheckBox->setChecked(m_settings->m_convertTabsToSpaces);
    m_caseCheckBox->setChecked(m_settings->m_ignoreChangesInCase);
    m_linesCheckBox->setChecked(m_settings->m_ignoreEmptyLines);
    m_whitespaceCheckBox->setChecked(m_settings->m_ignoreWhiteSpace);
    m_allWhitespaceCheckBox->setChecked(m_settings->m_ignoreAllWhiteSpace);
    m_ignoreTabExpansionCheckBox->setChecked(m_settings->m_ignoreChangesDueToTabExpansion);

    m_ignoreRegExpCheckBox->setChecked(m_settings->m_ignoreRegExp);
    m_ignoreRegExpEdit->setCompletedItems(m_settings->m_ignoreRegExpTextHistory);
    m_ignoreRegExpEdit->setText(m_settings->m_ignoreRegExpText);

    m_locSpinBox->setValue(m_settings->m_linesOfContext);

    m_modeButtonGroup->button(m_settings->m_format)->setChecked(true);

    m_excludeFilePatternGroupBox->setChecked(m_settings->m_excludeFilePattern);
    slotExcludeFilePatternToggled(m_settings->m_excludeFilePattern);
    m_excludeFilePatternEditListBox->insertStringList(m_settings->m_excludeFilePatternList);

    m_excludeFileNameGroupBox->setChecked(m_settings->m_excludeFilesFile);
    slotExcludeFileToggled(m_settings->m_excludeFilesFile);
    m_excludeFileURLComboBox->setUrls(m_settings->m_excludeFilesFileHistoryList);
    m_excludeFileURLComboBox->setUrl(QUrl::fromUserInput(m_settings->m_excludeFilesFileURL, QDir::currentPath(), QUrl::AssumeLocalFile));
}

DiffSettings* DiffPage::settings()
{
    return m_settings;
}

void DiffPage::restore()
{
    // this shouldn't do a thing...
}

void DiffPage::apply()
{
    m_settings->m_diffProgram                    = m_diffURLRequester->text();

    m_settings->m_newFiles                       = m_newFilesCheckBox->isChecked();
    m_settings->m_largeFiles                     = m_largerCheckBox->isChecked();
    m_settings->m_createSmallerDiff              = m_smallerCheckBox->isChecked();
    m_settings->m_convertTabsToSpaces            = m_tabsCheckBox->isChecked();
    m_settings->m_ignoreChangesInCase            = m_caseCheckBox->isChecked();
    m_settings->m_ignoreEmptyLines               = m_linesCheckBox->isChecked();
    m_settings->m_ignoreWhiteSpace               = m_whitespaceCheckBox->isChecked();
    m_settings->m_ignoreAllWhiteSpace            = m_allWhitespaceCheckBox->isChecked();
    m_settings->m_ignoreChangesDueToTabExpansion = m_ignoreTabExpansionCheckBox->isChecked();

    m_settings->m_ignoreRegExp                   = m_ignoreRegExpCheckBox->isChecked();
    m_settings->m_ignoreRegExpText               = m_ignoreRegExpEdit->text();
    m_settings->m_ignoreRegExpTextHistory        = m_ignoreRegExpEdit->completionObject()->items();

    m_settings->m_linesOfContext                 = m_locSpinBox->value();

    m_settings->m_format = static_cast<Kompare::Format>(m_modeButtonGroup->checkedId());

    m_settings->m_excludeFilePattern             = m_excludeFilePatternGroupBox->isChecked();
    m_settings->m_excludeFilePatternList         = m_excludeFilePatternEditListBox->items();

    m_settings->m_excludeFilesFile               = m_excludeFileNameGroupBox->isChecked();
    m_settings->m_excludeFilesFileURL            = m_excludeFileURLComboBox->currentText();
    m_settings->m_excludeFilesFileHistoryList    = m_excludeFileURLComboBox->urls();

    m_settings->saveSettings(KSharedConfig::openConfig().data());
}

void DiffPage::setDefaults()
{
    m_diffURLRequester->setText(QString());
    m_newFilesCheckBox->setChecked(true);
    m_smallerCheckBox->setChecked(true);
    m_largerCheckBox->setChecked(true);
    m_tabsCheckBox->setChecked(false);
    m_caseCheckBox->setChecked(false);
    m_linesCheckBox->setChecked(false);
    m_whitespaceCheckBox->setChecked(false);
    m_allWhitespaceCheckBox->setChecked(false);
    m_ignoreTabExpansionCheckBox->setChecked(false);
    m_ignoreRegExpCheckBox->setChecked(false);

    m_ignoreRegExpEdit->setText(QString());

    m_locSpinBox->setValue(3);

    m_modeButtonGroup->button(Kompare::Unified)->setChecked(true);

    m_excludeFilePatternGroupBox->setChecked(false);

    m_excludeFileNameGroupBox->setChecked(false);
}

void DiffPage::slotExcludeFilePatternToggled(bool on)
{
    m_excludeFilePatternEditListBox->setEnabled(on);
}

void DiffPage::slotExcludeFileToggled(bool on)
{
    m_excludeFileURLComboBox->setEnabled(on);
    m_excludeFileURLRequester->setEnabled(on);
}

void DiffPage::addDiffTab()
{
    QWidget* page   = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);

    // add diff program selector
    m_diffProgramGroup = new QGroupBox(page);
    layout->addWidget(m_diffProgramGroup);
    QVBoxLayout* bgLayout = new QVBoxLayout(m_diffProgramGroup);
    m_diffProgramGroup->setTitle(i18nc("@title:group", "Diff Program"));

    // using the "text" property over the "url" property, to also allow plain executable names instead of full path
    m_diffURLRequester = new KUrlRequester(m_diffProgramGroup);
    m_diffURLRequester->setObjectName(QStringLiteral("diffURLRequester"));
    m_diffURLRequester->setWhatsThis(i18n("You can select a different diff program here. On Solaris the standard diff program does not support all the options that the GNU version does. This way you can select that version."));
    m_diffURLRequester->setPlaceholderText(QStringLiteral("diff"));
    bgLayout->addWidget(m_diffURLRequester);

    layout->addStretch(1);

    m_tabWidget->addTab(page, i18nc("@title:tab", "Diff"));
}

void DiffPage::addFormatTab()
{
    QWidget* page   = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);

    // add diff modes
    m_modeButtonGroup = new QButtonGroup(page);
    QGroupBox* box = new QGroupBox(page);
    box->setWhatsThis(i18nc("@info:whatsthis", "Select the format of the output generated by diff. Unified is the one that is used most frequently because it is very readable. The KDE developers like this format the best so use it for sending patches."));
    layout->addWidget(box);
    QVBoxLayout* bgLayout = new QVBoxLayout(box);
    box->setTitle(i18nc("@title:group", "Output Format"));

    QRadioButton* radioButton = new QRadioButton(i18nc("@option:radio format type", "Context"), box);
    m_modeButtonGroup->addButton(radioButton, Kompare::Context);
    bgLayout->addWidget(radioButton);
    radioButton = new QRadioButton(i18nc("@option:radio format type", "Normal"),  box);
    m_modeButtonGroup->addButton(radioButton, Kompare::Normal);
    bgLayout->addWidget(radioButton);
    radioButton =  new QRadioButton(i18nc("@option:radio format type", "Unified"), box);
    m_modeButtonGroup->addButton(radioButton, Kompare::Unified);
    bgLayout->addWidget(radioButton);

    // #lines of context (loc)
    QGroupBox* groupBox = new QGroupBox(page);
    layout->addWidget(groupBox);
    QFormLayout* groupLayout = new QFormLayout(groupBox);
    groupBox->setTitle(i18nc("@title:group", "Lines of Context"));
    groupBox->setWhatsThis(i18nc("@info:whatsthis", "The number of context lines is normally 2 or 3. This makes the diff readable and applicable in most cases. More than 3 lines will only bloat the diff unnecessarily."));

    m_locSpinBox = new QSpinBox(groupBox);
    m_locSpinBox->setRange(0, 100);
    groupLayout->addRow(i18nc("@label:spinbox" ,"Number of context lines:"), m_locSpinBox);
    m_locSpinBox->setWhatsThis(i18nc("@info:whatsthis", "The number of context lines is normally 2 or 3. This makes the diff readable and applicable in most cases. More than 3 lines will only bloat the diff unnecessarily."));

    layout->addStretch(1);

    m_tabWidget->addTab(page, i18nc("@title:tab", "Format"));
}

void DiffPage::addOptionsTab()
{
    QWidget* page   = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);

    // add diff options
    QGroupBox* optionButtonGroup = new QGroupBox(page);
    layout->addWidget(optionButtonGroup);
    QVBoxLayout* bgLayout = new QVBoxLayout(optionButtonGroup);
    optionButtonGroup->setTitle(i18nc("@title:group", "General"));

    m_newFilesCheckBox    = new QCheckBox(i18nc("@option:check", "&Treat new files as empty"), optionButtonGroup);
    m_newFilesCheckBox->setToolTip(i18nc("@info:tooltip", "This option corresponds to the -N diff option."));
    m_newFilesCheckBox->setWhatsThis(i18nc("@info:whatsthis", "With this option enabled diff will treat a file that only exists in one of the directories as empty in the other directory. This means that the file is compared with an empty file and because of this will appear as one big insertion or deletion."));
    bgLayout->addWidget(m_newFilesCheckBox);

    m_smallerCheckBox     = new QCheckBox(i18nc("@option:check", "&Look for smaller changes"), optionButtonGroup);
    m_smallerCheckBox->setToolTip(i18nc("@info:tooltip", "This corresponds to the -d diff option."));
    m_smallerCheckBox->setWhatsThis(i18nc("@info:whatsthis", "With this option enabled diff will try a little harder (at the cost of more memory) to find fewer changes."));
    bgLayout->addWidget(m_smallerCheckBox);
    m_largerCheckBox      = new QCheckBox(i18nc("@option:check", "O&ptimize for large files"), optionButtonGroup);
    m_largerCheckBox->setToolTip(i18nc("@info:tooltip", "This corresponds to the -H diff option."));
    m_largerCheckBox->setWhatsThis(i18nc("@info:whatsthis", "This option lets diff makes better diffs when using large files. The definition of large is nowhere to be found though."));
    bgLayout->addWidget(m_largerCheckBox);
    m_caseCheckBox        = new QCheckBox(i18nc("@option:check", "&Ignore changes in case"), optionButtonGroup);
    m_caseCheckBox->setToolTip(i18nc("@info:tooltip", "This corresponds to the -i diff option."));
    m_caseCheckBox->setWhatsThis(i18nc("@info:whatsthis", "With this option to ignore changes in case enabled, diff will not indicate a difference when something in one file is changed into SoMEthing in the other file."));
    bgLayout->addWidget(m_caseCheckBox);

    QHBoxLayout* groupLayout = new QHBoxLayout();
    layout->addLayout(groupLayout);
    groupLayout->setObjectName(QStringLiteral("regexp_horizontal_layout"));
    groupLayout->setSpacing(-1);

    m_ignoreRegExpCheckBox = new QCheckBox(i18nc("@option:check", "Ignore regexp:"), page);
    m_ignoreRegExpCheckBox->setToolTip(i18nc("@info:tooltip", "This option corresponds to the -I diff option."));
    m_ignoreRegExpCheckBox->setWhatsThis(i18nc("@info:whatsthis", "When this checkbox is enabled, an option to diff is given that will make diff ignore lines that match the regular expression."));
    groupLayout->addWidget(m_ignoreRegExpCheckBox);
    m_ignoreRegExpEdit = new KLineEdit(page);
    m_ignoreRegExpEdit->setObjectName(QStringLiteral("regexplineedit"));
    m_ignoreRegExpEdit->setToolTip(i18nc("@info:tooltip", "Add the regular expression here that you want to use\nto ignore lines that match it."));
    groupLayout->addWidget(m_ignoreRegExpEdit);

    QGroupBox* moreOptionButtonGroup = new QGroupBox(page);
    layout->addWidget(moreOptionButtonGroup);
    bgLayout = new QVBoxLayout(moreOptionButtonGroup);
    moreOptionButtonGroup->setTitle(i18nc("@title:group", "Whitespace"));

    m_tabsCheckBox        = new QCheckBox(i18nc("@option:check", "E&xpand tabs to spaces in output"), moreOptionButtonGroup);
    m_tabsCheckBox->setToolTip(i18nc("@info:tooltip", "This option corresponds to the -t diff option."));
    m_tabsCheckBox->setWhatsThis(i18nc("@info:whatsthis", "This option does not always produce the right result. Due to this expansion Kompare may have problems applying the change to the destination file."));
    bgLayout->addWidget(m_tabsCheckBox);
    m_linesCheckBox       = new QCheckBox(i18nc("@option:check", "I&gnore added or removed empty lines"), moreOptionButtonGroup);
    m_linesCheckBox->setToolTip(i18nc("@info:tooltip", "This option corresponds to the -B diff option."));
    m_linesCheckBox->setWhatsThis(i18nc("@info:whatsthis", "This can be very useful in situations where code has been reorganized and empty lines have been added or removed to improve legibility."));
    bgLayout->addWidget(m_linesCheckBox);
    m_whitespaceCheckBox  = new QCheckBox(i18nc("@option:check", "Ig&nore changes in the amount of whitespace"), moreOptionButtonGroup);
    m_whitespaceCheckBox->setToolTip(i18nc("@info:tooltip", "This option corresponds to the -b diff option."));
    m_whitespaceCheckBox->setWhatsThis(i18nc("@info:whatsthis", "If you are uninterested in differences arising due to, for example, changes in indentation, then use this option."));
    bgLayout->addWidget(m_whitespaceCheckBox);
    m_allWhitespaceCheckBox = new QCheckBox(i18nc("@option:check", "Ign&ore all whitespace"), moreOptionButtonGroup);
    m_allWhitespaceCheckBox->setToolTip(i18nc("@info:tooltip", "This option corresponds to the -w diff option."));
    m_allWhitespaceCheckBox->setWhatsThis(i18nc("@info:whatsthis", "This is useful for seeing the significant changes without being overwhelmed by all the white space changes."));
    bgLayout->addWidget(m_allWhitespaceCheckBox);
    m_ignoreTabExpansionCheckBox = new QCheckBox(i18nc("@option:check", "Igno&re changes due to tab expansion"), moreOptionButtonGroup);
    m_ignoreTabExpansionCheckBox->setToolTip(i18nc("@info:tooltip", "This option corresponds to the -E diff option."));
    m_ignoreTabExpansionCheckBox->setWhatsThis(i18nc("@info:whatsthis", "If there is a change because tabs have been expanded into spaces in the other file, then this option will make sure that these do not show up. Kompare currently has some problems applying such changes so be careful when you use this option."));
    bgLayout->addWidget(m_ignoreTabExpansionCheckBox);

    layout->addStretch(1);

    m_tabWidget->addTab(page, i18nc("@title:tab", "Options"));
}

void DiffPage::addExcludeTab()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);

    m_excludeFilePatternGroupBox = new QGroupBox(page);
    m_excludeFilePatternGroupBox->setCheckable(true);
    QHBoxLayout* excludeFileLayout = new QHBoxLayout;
    m_excludeFilePatternGroupBox->setLayout(excludeFileLayout);
    m_excludeFilePatternGroupBox->setTitle(i18nc("@title:group", "File Pattern to Exclude"));
    m_excludeFilePatternGroupBox->setToolTip(i18nc("@info:tooltip", "If this is checked you can enter a shell pattern in the text box on the right or select entries from the list."));
    m_excludeFilePatternEditListBox = new KEditListWidget;
    excludeFileLayout->addWidget(m_excludeFilePatternEditListBox);
    m_excludeFilePatternEditListBox->setObjectName(QStringLiteral("exclude_file_pattern_editlistbox"));
    m_excludeFilePatternEditListBox->setButtons(KEditListWidget::Add | KEditListWidget::Remove);
    m_excludeFilePatternEditListBox->setCheckAtEntering(false);
    m_excludeFilePatternEditListBox->setToolTip(i18nc("@info:tooltip", "Here you can enter or remove a shell pattern or select one or more entries from the list."));
    layout->addWidget(m_excludeFilePatternGroupBox);


    connect(m_excludeFilePatternGroupBox, &QGroupBox::toggled, this, &DiffPage::slotExcludeFilePatternToggled);

    m_excludeFileNameGroupBox = new QGroupBox(page);
    m_excludeFileNameGroupBox->setCheckable(true);
    excludeFileLayout = new QHBoxLayout;
    m_excludeFileNameGroupBox->setLayout(excludeFileLayout);
    m_excludeFileNameGroupBox->setTitle(i18nc("@title:group", "File with Filenames to Exclude"));
    m_excludeFileNameGroupBox->setToolTip(i18nc("@info:tooltip", "If this is checked you can enter a filename in the combo box below."));
    m_excludeFileURLComboBox  = new KUrlComboBox(KUrlComboBox::Files, true);
    excludeFileLayout->addWidget(m_excludeFileURLComboBox);
    m_excludeFileURLComboBox->setObjectName(QStringLiteral("exclude_file_urlcombo"));
    m_excludeFileURLComboBox->setToolTip(i18nc("@info:tooltip", "Here you can enter the URL of a file with shell patterns to ignore during the comparison of the folders."));
    m_excludeFileURLRequester = new KUrlRequester(m_excludeFileURLComboBox, m_excludeFileNameGroupBox);
    excludeFileLayout->addWidget(m_excludeFileURLRequester);
    m_excludeFileURLRequester->setObjectName(QStringLiteral("exclude_file_name_urlrequester"));
    m_excludeFileURLRequester->setToolTip(i18nc("@info:tooltip", "Any file you select in the dialog that pops up when you click it will be put in the dialog to the left of this button."));
    layout->addWidget(m_excludeFileNameGroupBox);

    connect(m_excludeFileNameGroupBox, &QGroupBox::toggled, this, &DiffPage::slotExcludeFileToggled);

    layout->addStretch(1);

    m_tabWidget->addTab(page, i18nc("@title:tab", "Exclude"));
}

//kate: replace-tabs 0; indent-width 4; tab-width 4;
