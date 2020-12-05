/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kompare_partfactory.h"

#include "kompare_part.h"
#if KPARTS_VERSION < QT_VERSION_CHECK(5, 77, 0)
#include "../kompare_version.h"
#endif

// KF
#include <KLocalizedString>
#if KPARTS_VERSION >= QT_VERSION_CHECK(5, 77, 0)
#include <KPluginMetaData>
#endif

KomparePartFactory::KomparePartFactory()
#if KPARTS_VERSION >= QT_VERSION_CHECK(5, 77, 0)
{
#else
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
#endif
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

#if KPARTS_VERSION >= QT_VERSION_CHECK(5, 77, 0)
    KomparePart* part = new KomparePart(parentWidget, parent, metaData(), modus);
#else
    KomparePart* part = new KomparePart(parentWidget, parent, m_aboutData, modus);
#endif

    return part;
}
