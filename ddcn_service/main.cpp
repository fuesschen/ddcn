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

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * @mainpage ddcn_service
 *
 * @section about About
 *
 * The ddcn_service program sends and receives compiler jobs in the network and
 * receives local jobs, additionally it executes jobs if the system is idle.
 *
 * @section structure Structure
 *
 * The system is built around two important classes, CompilerService and
 * CompilerNetwork. The manages and executes the local jobs and jobs coming from
 * the network while the latter implements the network protocol and
 * sends/receives compiler jobs. A compiler job is encapsulated in a Job object,
 * with additional information attached to it via OutgoingJob or IncomingJob
 * objects if the job is executed on a different computer.
 *
 * The D-Bus interface is implemented in the classes ending with *Adaptor, that
 * is, CompilerNetworkAdaptor and CompilerServiceAdaptor.
 */

#include "CompilerServiceAdaptor.h"
#include "CompilerNetworkAdaptor.h"
#include "DBusStructs.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include "LogWriter.h"

int main(int argc, char **argv) {
	QCoreApplication app(argc, argv);
	// Create the settings directory if it does not exist
	{
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ddcn", "ddcn");
		QString settingsDir = QFileInfo(settings.fileName()).absolutePath();
		QDir dir;
		if (!dir.exists(settingsDir)) {
			dir.mkpath(settingsDir);
		}
	}
	// This has to be done before initializing the log writer as an existing
	// instance might be writing into the log right now
	if (!QDBusConnection::sessionBus().registerService("org.ddcn.service")) {
		std::cout << "The program is already running, aborting." << std::endl;
		return -1;
	}
	// Initialize an instance of the log writer
	LogWriter logWriter;
	if (!logWriter.init()) {
		qCritical("Could not open the log file!");
	}
	// Register meta types
	registerCustomDBusTypes();
	qRegisterMetaType<ariba::utility::NodeID>();
	qRegisterMetaType<ariba::utility::LinkID>();
	qRegisterMetaType<ariba::DataMessage>();
	// Initialize crypto framework
	TLS::initialize();
	// Create the compiler network
	CompilerNetwork network;
	new CompilerNetworkAdaptor(&network);
	// Create the compiler service
	CompilerService service(&network);
	CompilerServiceAdaptor *serviceAdaptor = new CompilerServiceAdaptor(&service);
	QObject::connect (serviceAdaptor,
			 SIGNAL(onShutdownRequest()),
			 &app,
			 SLOT(quit())
	);
	// Create the D-Bus interface
	QDBusConnection::sessionBus().registerObject("/CompilerService", &service);
	QDBusConnection::sessionBus().registerObject("/CompilerNetwork", &network);
	// Start the event loop
	return app.exec();
}
