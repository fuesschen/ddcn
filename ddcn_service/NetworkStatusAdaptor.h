/*
Copyright 2011 Benjamin Fus, Florian Muenchbach, Mathias Gottschlag. All
rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NETWORKSTATUSADAPTOR_H_INCLUDED
#define NETWORKSTATUSADAPTOR_H_INCLUDED

#include "NetworkStatus.h"
#include "DBusStructs.h"

#include <QDBusAbstractAdaptor>

/**
 * Class which exports a CompilerNetwork object.
 */
class NetworkStatusAdaptor : public QDBusAbstractAdaptor {
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.ddcn.NetworkStatus")
	Q_PROPERTY(float networkLoad
	           READ getNetworkLoad
	           NOTIFY networkLoadChanged)
public:
	/**
	 * Creates a dbus adaptor for the network status
	 * @param status Network status object to export.
	 */
	explicit NetworkStatusAdaptor(NetworkStatus *networkStatus);
public slots:
private slots:
signals:
private:
	NetworkStatus *networkStatus;
};

#endif

