/***************************************************************************
                                kompare_partfactory.h
                                --------------
        begin                   : Wed Oct 11 2017
        Copyright 2017      Friedrich W. H. Kossebau <kossebau@kde.org>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#ifndef KOMPAREPARTFACTORY_H
#define KOMPAREPARTFACTORY_H

#include <KPluginFactory>
#include <KAboutData>


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
                    const QVariantList& args, const QString& keyword) override;

private:
    KAboutData m_aboutData;
};

#endif
