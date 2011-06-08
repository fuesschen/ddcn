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

#include "CompilerServiceAdaptor.h"

CompilerServiceAdaptor::CompilerServiceAdaptor(CompilerService *service)
		: QDBusAbstractAdaptor(service), service(service) {
	connect(service,
			SIGNAL(threadCountChanged(int)),
			this,
			SLOT(onThreadCountChanged(int)));
}

void CompilerServiceAdaptor::setThreadCount(int threadCount) {
	service->setThreadCount(threadCount);
}
int CompilerServiceAdaptor::getThreadCount() {
	return service->getThreadCount();
}

void CompilerServiceAdaptor::onThreadCountChanged(int threadCount) {
	emit threadCountChanged(threadCount);
}

QString CompilerServiceAdaptor::addJob(QList<QByteArray> inputFiles, QStringList parameters, QString toolChain) {
	Job *job = new Job(QList<QByteArray> inputFiles, QStringList parameters, QString toolChain, false);
}

void CompilerServiceAdaptor::shutdown() {

}

