/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KOMPAREPARTFACTORY_H
#define KOMPAREPARTFACTORY_H

// KF
#include <KPluginFactory>


class KomparePartFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "komparepart.json")
    Q_INTERFACES(KPluginFactory)

public:
    KomparePartFactory();
    ~KomparePartFactory() override;

    QObject* create(const char* iface,
                    QWidget* parentWidget, QObject* parent,
                    const QVariantList& args) override;
};

#endif
