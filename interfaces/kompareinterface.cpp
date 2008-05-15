/*
  Copyright 2002-2004  Otto Bruggeman <otto.bruggeman@home.nl>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of
  the License or (at your option) version 3 or any later version
  accepted by the membership of KDE e.V. (or its successor approved
  by the membership of KDE e.V.), which shall act as a proxy 
  defined in Section 14 of version 3 of the license.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "kompareinterface.h"

class KompareInterfacePrivate
{
public:
	KompareInterfacePrivate();
	~KompareInterfacePrivate();
	KompareInterfacePrivate( const KompareInterfacePrivate& );
	KompareInterfacePrivate& operator=( const KompareInterfacePrivate& );

protected:
	// Add all variables for the KompareInterface class here and access them through the kip pointer
};

KompareInterfacePrivate::KompareInterfacePrivate()
{
}

KompareInterfacePrivate::~KompareInterfacePrivate()
{
}

KompareInterfacePrivate::KompareInterfacePrivate( const KompareInterfacePrivate& /*kip*/ )
{
}

KompareInterfacePrivate& KompareInterfacePrivate::operator=(const KompareInterfacePrivate& /*kip*/ )
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

KompareInterface::KompareInterface( const KompareInterface& ki )
{
	kip = new KompareInterfacePrivate( *(ki.kip) );
}

KompareInterface& KompareInterface::operator=( const KompareInterface& ki )
{
	kip = ki.kip;
	return *this;
}

void KompareInterface::setEncoding( const QString& encoding )
{
	m_encoding = encoding;
}

