/*
    SPDX-FileCopyrightText: 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "kompareinterface.h"

class KompareInterfacePrivate
{
public:
    KompareInterfacePrivate();
    ~KompareInterfacePrivate();
    KompareInterfacePrivate(const KompareInterfacePrivate&);
    KompareInterfacePrivate& operator=(const KompareInterfacePrivate&);

protected:
    // Add all variables for the KompareInterface class here and access them through the kip pointer
};

KompareInterfacePrivate::KompareInterfacePrivate()
{
}

KompareInterfacePrivate::~KompareInterfacePrivate()
{
}

KompareInterfacePrivate::KompareInterfacePrivate(const KompareInterfacePrivate& /*kip*/)
{
}

KompareInterfacePrivate& KompareInterfacePrivate::operator=(const KompareInterfacePrivate& /*kip*/)
{
    return *this;
}

KompareInterface::KompareInterface()
{
    kip = new KompareInterfacePrivate();
}

KompareInterface::~KompareInterface()
{
    delete kip;
}

KompareInterface::KompareInterface(const KompareInterface& ki)
{
    kip = new KompareInterfacePrivate(*(ki.kip));
}

KompareInterface& KompareInterface::operator=(const KompareInterface& ki)
{
    kip = ki.kip;
    return *this;
}

void KompareInterface::setEncoding(const QString& encoding)
{
    m_encoding = encoding;
}

