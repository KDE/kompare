// Blah blah standard LGPL license
// Copyright 2002-2003, Otto Bruggeman <otto.bruggeman@home.nl>

#include "kompareinterface.h"

class KompareInterfacePrivate
{
public:
	KompareInterfacePrivate();
	~KompareInterfacePrivate();
	KompareInterfacePrivate( const KompareInterfacePrivate& );
	KompareInterfacePrivate& operator=( const KompareInterfacePrivate& );
private:
	// Add all variables for the KompareInterface class here and access them through the d pointer
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
}

KompareInterface::KompareInterface()
{
	d = new KompareInterfacePrivate();
}

KompareInterface::~KompareInterface()
{
	delete d;
}

KompareInterface::KompareInterface( const KompareInterface& ki )
{
	d = new KompareInterfacePrivate( *(ki.d) );
}

KompareInterface& KompareInterface::operator=( const KompareInterface& ki )
{
	d = ki.d;
}
