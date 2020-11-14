/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kompare_partfactory.h"

#include "kompare_part.h"
#include "../kompare_version.h"

// KF
#include <KLocalizedString>

KomparePartFactory::KomparePartFactory()
    : m_aboutData(QStringLiteral("komparepart"),
                  i18n("KomparePart"),
                  QStringLiteral(KOMPARE_VERSION_STRING),
                  QString(),
                  KAboutLicense::GPL
                 )
{
    m_aboutData.addAuthor(i18n("John Firebaugh"), i18n("Author"), QStringLiteral("jfirebaugh@kde.org"));
    m_aboutData.addAuthor(i18n("Otto Bruggeman"), i18n("Author"), QStringLiteral("bruggie@gmail.com"));
    m_aboutData.addAuthor(i18n("Kevin Kofler"), i18n("Author"), QStringLiteral("kevin.kofler@chello.at"));
}

KomparePartFactory::~KomparePartFactory() = default;

QObject* KomparePartFactory::create(const char* iface,
                                    QWidget* parentWidget, QObject* parent,
                                    const QVariantList& args, const QString& keyword)
{
    Q_UNUSED(args);
    Q_UNUSED(keyword);

    const bool wantReadOnly = (strcmp(iface, "KParts::ReadOnlyPart") == 0);
    const KomparePart::Modus modus =
        wantReadOnly ? KomparePart::ReadOnlyModus :
        /* else */     KomparePart::ReadWriteModus;

    KomparePart* part = new KomparePart(parentWidget, parent, m_aboutData, modus);

    return part;
}
