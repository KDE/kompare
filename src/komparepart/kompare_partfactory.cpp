/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kompare_partfactory.h"

#include "kompare_part.h"
// KF
#include <KLocalizedString>
#include <KPluginMetaData>

KomparePartFactory::KomparePartFactory() = default;

KomparePartFactory::~KomparePartFactory() = default;

QObject* KomparePartFactory::create(const char* iface,
                                    QWidget* parentWidget, QObject* parent,
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                                     const QVariantList& args)
{
#else
                                    const QVariantList& args, const QString& keyword)
{
    Q_UNUSED(keyword);
#endif
    Q_UNUSED(args);

    const bool wantReadOnly = (strcmp(iface, "KParts::ReadOnlyPart") == 0);
    const KomparePart::Modus modus =
        wantReadOnly ? KomparePart::ReadOnlyModus :
        /* else */     KomparePart::ReadWriteModus;

    KomparePart* part = new KomparePart(parentWidget, parent, metaData(), modus);

    return part;
}

#include "moc_kompare_partfactory.cpp"
