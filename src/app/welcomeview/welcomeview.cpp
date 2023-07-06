/*
    SPDX-FileCopyrightText: 2022 Jiří Wolker <woljiri@gmail.com>
    SPDX-FileCopyrightText: 2022 Eugene Popov <popov895@ukr.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "welcomeview.h"

#include "kompare_shell.h"

#include <KAboutData>
#include <KConfigGroup>
#include <KIconLoader>
#include <KSharedConfig>
#include <KIO/OpenFileManagerWindowJob>

#include <QClipboard>
#include <QDesktopServices>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QMenu>
#include <QTimer>

WelcomeView::WelcomeView(KompareShell *mainWindow, QWidget *parent)
    : QScrollArea(parent)
    , m_mainWindow(mainWindow)
{
    setupUi(this);

    const KAboutData aboutData = KAboutData::applicationData();
    labelTitle->setText(i18n("Welcome to %1", aboutData.displayName()));
    labelDescription->setText(aboutData.shortDescription());
    labelIcon->setPixmap(aboutData.programLogo().value<QIcon>().pixmap(KIconLoader::SizeEnormous));

    connect(buttonCompareFiles, SIGNAL(clicked()), m_mainWindow, SLOT(slotFileCompareFiles()));
    connect(buttonOpenDiff, SIGNAL(clicked()), m_mainWindow, SLOT(slotFileOpen()));

    connect(labelHomepage, qOverload<>(&KUrlLabel::leftClickedUrl), this, [aboutData]() {
        QDesktopServices::openUrl(QUrl(aboutData.homepage()));
    });
    connect(labelHandbook, qOverload<>(&KUrlLabel::leftClickedUrl), m_mainWindow, &KompareShell::appHelpActivated);
    connect(buttonClose, &QPushButton::clicked, m_mainWindow, &KompareShell::hideWelcomeScreen);

    static const char showOnStartupKey[] = "ShowWelcomeScreenOnStartup";
    KConfigGroup configGroup = KSharedConfig::openConfig()->group("General");
    checkBoxShowOnStartup->setChecked(configGroup.readEntry(showOnStartupKey, true));
    connect(checkBoxShowOnStartup, &QCheckBox::toggled, this, [configGroup](bool checked) mutable {
        configGroup.writeEntry(showOnStartupKey, checked);
    });

    updateFonts();
    updateButtons();
}

bool WelcomeView::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::FontChange:
        updateFonts();
        updateButtons();
        break;
    case QEvent::Resize:
        if (updateLayout()) {
            return true;
        }
        break;
    default:
        break;
    }

    return QScrollArea::event(event);
}

void WelcomeView::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);

    updateLayout();
}

void WelcomeView::updateButtons()
{
    QVector<QPushButton*> buttons {
        buttonOpenDiff,
        buttonCompareFiles
    };
    const int maxWidth = std::accumulate(buttons.cbegin(), buttons.cend(), 0, [](int maxWidth, const QPushButton *button) {
        return std::max(maxWidth, button->sizeHint().width());
    });
    for (QPushButton *button : std::as_const(buttons)) {
        button->setFixedWidth(maxWidth);
    }
}

void WelcomeView::updateFonts()
{
    QFont titleFont = font();
    titleFont.setPointSize(titleFont.pointSize() + 6);
    titleFont.setWeight(QFont::Bold);
    labelTitle->setFont(titleFont);

    QFont panelTitleFont = font();
    panelTitleFont.setPointSize(panelTitleFont.pointSize() + 2);
    labelHelp->setFont(panelTitleFont);

    QFont placeholderFont = font();
    placeholderFont.setPointSize(qRound(placeholderFont.pointSize() * 1.3));
}

bool WelcomeView::updateLayout()
{
    bool result = false;

    // show/hide widgetHeader depending on the view height
    if (widgetHeader->isVisible()) {
        if (height() <= frameContent->height() + widgetClose->height()) {
            widgetHeader->hide();
            result = true;
        }
    } else {
        const int implicitHeight = frameContent->height()
                                   + widgetHeader->height()
                                   + layoutContent->spacing()
                                   + widgetClose->height();
        if (height() > implicitHeight) {
            widgetHeader->show();
            result = true;
        }
    }

    // show/hide widgetHelp depending on the view width
    if (widgetHelp->isVisible()) {
        if (width() <= frameContent->width() + widgetClose->width()) {
            widgetHelp->hide();
            result = true;
        }
    } else {
        const int implicitWidth = frameContent->width()
                                  + widgetHelp->width()
                                  + layoutPanels->spacing()
                                  + widgetClose->width();
        if (width() > implicitWidth) {
            widgetHelp->show();
            return true;
        }
    }

    return result;
}
