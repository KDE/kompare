/*
    SPDX-FileCopyrightText: 2004 Otto Bruggeman <otto.bruggeman@home.nl>
    SPDX-FileCopyrightText: 2007-2011 Kevin Kofler <kevin.kofler@chello.at>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filespage.h"

#include <QLabel>
#include <QGroupBox>
#include <QFormLayout>
#include <QHBoxLayout>

#include <KCharsets>
#include <KConfig>
#include <KLocalizedString>
#include <KUrlComboBox>
#include <KUrlRequester>

#include "filessettings.h"

FilesPage::FilesPage()
    : QFrame()
{
    m_formLayout = new QFormLayout(this);

    // Source
    auto sourceLayout = new QHBoxLayout(this);
    m_sourceURLComboBox = new KUrlComboBox(KUrlComboBox::Both, true, this);
    m_sourceURLComboBox->setMaxItems(10);
    m_sourceURLComboBox->setObjectName(QStringLiteral("SourceURLComboBox"));
    m_sourceURLRequester = new KUrlRequester(m_sourceURLComboBox, nullptr);
    sourceLayout->addWidget(m_sourceURLComboBox);
    m_sourceURLComboBox->setFocus();

    auto button = new QPushButton(this);
    button->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    button->setToolTip(i18nc("@info:tooltip", "Select file"));
    button->setProperty("combobox", QStringLiteral("SourceURLComboBox"));
    button->setProperty("folder", false);
    connect(button, &QPushButton::clicked, this, &FilesPage::open);
    sourceLayout->addWidget(button);

    button = new QPushButton(this);
    button->setIcon(QIcon::fromTheme(QStringLiteral("folder-open")));
    QSizePolicy sp = button->sizePolicy();
    sp.setRetainSizeWhenHidden(true);
    button->setSizePolicy(sp);
    button->setObjectName(QStringLiteral("firstURLOpenFolder"));
    button->setToolTip(i18nc("@info:tooltip", "Select folder"));
    button->setProperty("combobox", QStringLiteral("SourceURLComboBox"));
    button->setProperty("folder", true);
    connect(button, &QPushButton::clicked, this, &FilesPage::open);
    sourceLayout->addWidget(button);

    m_formLayout->addRow(i18nc("@label:file", "Source:"), sourceLayout);
    m_sourceLabel = static_cast<QLabel *>(m_formLayout->labelForField(sourceLayout));

    // Destination
    auto destinationLayout = new QHBoxLayout(this);
    m_destinationURLComboBox = new KUrlComboBox(KUrlComboBox::Both, true, this);
    m_destinationURLComboBox->setMaxItems(10);
    m_destinationURLComboBox->setObjectName(QStringLiteral("DestURLComboBox"));
    m_destinationURLRequester = new KUrlRequester(this, nullptr);
    destinationLayout->addWidget(m_destinationURLComboBox);

    button = new QPushButton(this);
    button->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    button->setToolTip(i18nc("@info:tooltip", "Select file"));
    button->setProperty("combobox", QStringLiteral("DestURLComboBox"));
    button->setProperty("folder", false);
    connect(button, &QPushButton::clicked, this, &FilesPage::open);
    destinationLayout->addWidget(button);

    button = new QPushButton(this);
    button->setIcon(QIcon::fromTheme(QStringLiteral("folder-open")));
    button->setObjectName(QStringLiteral("secondURLOpenFolder"));
    sp = button->sizePolicy();
    sp.setRetainSizeWhenHidden(true);
    button->setSizePolicy(sp);
    button->setToolTip(i18nc("@info:tooltip", "Select folder"));
    button->setProperty("combobox", QStringLiteral("DestURLComboBox"));
    button->setProperty("folder", true);
    connect(button, &QPushButton::clicked, this, &FilesPage::open);
    destinationLayout->addWidget(button);

    m_formLayout->addRow(i18nc("@label:file", "Destination:"), destinationLayout);
    m_destinationLabel = static_cast<QLabel *>(m_formLayout->labelForField(destinationLayout));

    // Encoding
    m_encodingComboBox = new KComboBox(false, this);
    m_encodingComboBox->setObjectName(QStringLiteral("encoding_combobox"));
    m_encodingComboBox->insertItem(0, i18nc("@item:inlistbox encoding", "Default"));
    m_encodingComboBox->insertItems(1, KCharsets::charsets()->availableEncodingNames());
    m_formLayout->addRow(i18nc("@label:checkbox", "Encoding:"), m_encodingComboBox);
}

FilesPage::~FilesPage()
{
    delete m_sourceURLRequester;
    m_sourceURLRequester = nullptr;
    delete m_destinationURLRequester;
    m_destinationURLRequester = nullptr;
    m_settings = nullptr;
}


void FilesPage::open()
{
    QPushButton* button = (QPushButton*) sender();
    bool selectFolders = button->property("folder").toBool();
    KUrlComboBox* urlComboBox = findChild<KUrlComboBox*>(button->property("combobox").toString());

    doOpen(urlComboBox, selectFolders);
}

void FilesPage::doOpen(KUrlComboBox* urlComboBox, bool selectFolders)
{
    QUrl currentUrl = QUrl::fromUserInput(urlComboBox->currentText());

    QUrl newUrl = selectFolders ? QFileDialog::getExistingDirectoryUrl(this,
                                                                       i18nc("@title:window", "Select Folder"),
                                                                       currentUrl,
                                                                       QFileDialog::ReadOnly)
                                : QFileDialog::getOpenFileUrl(this,
                                                              i18nc("@title:window", "Select File"),
                                                              currentUrl);
    if (!newUrl.isEmpty())
    {
        // remove newUrl if already exists and add it as the first item
        urlComboBox->setUrl(newUrl);
        Q_EMIT urlChanged();
    }

}

KUrlRequester* FilesPage::sourceURLRequester() const
{
    return m_sourceURLRequester;
}

KUrlRequester* FilesPage::destinationURLRequester() const
{
    return m_destinationURLRequester;
}

QString FilesPage::encoding() const
{
    return m_encodingComboBox->currentText();
}

void FilesPage::setSourceTitle(const QString& title)
{
    m_sourceLabel->setText(title);
}

void FilesPage::setDestinationTitle(const QString& title)
{
    m_destinationLabel->setText(title);
}

void FilesPage::setURLsInComboBoxes()
{
//     qDebug() << "first : " << m_firstURLComboBox->currentText() ;
//     qDebug() << "second: " << m_secondURLComboBox->currentText() ;
    m_sourceURLComboBox->setUrl(QUrl::fromUserInput(m_sourceURLComboBox->currentText(), QDir::currentPath(), QUrl::AssumeLocalFile));
    m_destinationURLComboBox->setUrl(QUrl::fromUserInput(m_destinationURLComboBox->currentText(), QDir::currentPath(), QUrl::AssumeLocalFile));
}


void FilesPage::setSourceURLRequesterMode(unsigned int mode)
{
    m_sourceURLRequester->setMode((KFile::Mode) mode);
    if ((mode & KFile::Directory) == 0)
    {
        QPushButton* button = findChild<QPushButton*>(QStringLiteral("firstURLOpenFolder"));
        button->setVisible(false);
    }
}

void FilesPage::setDestinationURLRequesterMode(unsigned int mode)
{
    m_destinationURLRequester->setMode((KFile::Mode) mode);
    if ((mode & KFile::Directory) == 0)
    {
        QPushButton* button = findChild<QPushButton*>(QStringLiteral("secondURLOpenFolder"));
        button->setVisible(false);
    }
}

void FilesPage::setSettings(FilesSettings* settings)
{
    m_settings = settings;

    m_sourceURLComboBox->setUrls(m_settings->m_recentSources);
    m_sourceURLComboBox->setUrl(QUrl::fromUserInput(m_settings->m_lastChosenSourceURL, QDir::currentPath(), QUrl::AssumeLocalFile));
    m_destinationURLComboBox->setUrls(m_settings->m_recentDestinations);
    m_destinationURLComboBox->setUrl(QUrl::fromUserInput(m_settings->m_lastChosenDestinationURL, QDir::currentPath(), QUrl::AssumeLocalFile));
    m_encodingComboBox->setCurrentIndex(m_encodingComboBox->findText(m_settings->m_encoding, Qt::MatchFixedString));
}

void FilesPage::restore()
{
    // this shouldn't do a thing...
}

void FilesPage::apply()
{
    // set the current items as the first ones
    m_sourceURLComboBox->insertUrl(0, QUrl(m_sourceURLComboBox->currentText()));
    m_destinationURLComboBox->insertUrl(0, QUrl(m_destinationURLComboBox->currentText()));

    m_settings->m_recentSources            = m_sourceURLComboBox->urls();
    m_settings->m_lastChosenSourceURL      = m_sourceURLComboBox->currentText();
    m_settings->m_recentDestinations       = m_destinationURLComboBox->urls();
    m_settings->m_lastChosenDestinationURL = m_destinationURLComboBox->currentText();
    m_settings->m_encoding                 = m_encodingComboBox->currentText();
}

void FilesPage::setDefaults()
{
    m_sourceURLComboBox->setUrls(QStringList());
    m_sourceURLComboBox->setUrl(QUrl());
    m_destinationURLComboBox->setUrls(QStringList());
    m_destinationURLComboBox->setUrl(QUrl());
    m_encodingComboBox->setCurrentIndex(0);   // "Default"
}

#include "moc_filespage.cpp"
