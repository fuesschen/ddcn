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

#include "Job.h"
#include <QProcess>
#include <QTemporaryFile>
#include "InputOutputFilePair.h"
#include "TemporaryFile.h"



Job::Job(QStringList inputFiles, QStringList parameters,
		 QString toolChain, bool isRemoteJob) : preProcessListPosition(0),
		 incomingJob(NULL), outgoingJob(NULL) {
	this->inputFiles = inputFiles;
	this->parameters = parameters;
	this->toolChain = toolChain;
	this->remoteJob = isRemoteJob;
}

void Job::preProcess() {
	QStringList preProcessParameter;
	if (this->preProcessListPosition < this->inputFiles.count()) {
		QString inputFile = this->inputFiles[this->preProcessListPosition];
		TemporaryFile tmpFile(
			inputFile.right(inputFile.length() - inputFile.lastIndexOf(".")),
			"ddcn_tmp_",
			inputFile.left(inputFile.indexOf(".")));
		preProcessParameter << "-E " << inputFile << "-o"
									<< tmpFile.getFilename() << this->parameters;

		//create a gcc process and submit the parameters
		gccPreProcess = new QProcess(this);
		connect(gccPreProcess,
			SIGNAL(finished(int, QProcess::ExitStatus)),
			this,
			SLOT(onPreProcessFinished(int,QProcess::ExitStatus))
		);
		connect(gccPreProcess,
			SIGNAL(error(int, QProcess::ExitStatus)),
			this,
			SLOT(onPreProcessExecuteError(QProcess::ExitStatus))
		);
		gccPreProcess->start("gcc", preProcessParameter);
	}
	else {
		emit preProcessFinished(this);
	}
}

void Job::execute() {
	//TODO foreach (QByteArray fileContent, this->inputFiles) {
	//gcc




		/*
		QTemporaryFile file(this);
		file.setFileTemplate();
		file.setAutoRemove(true);
		if (file.open()) {
			file.write(fileContent);
			file.close();
		}
		this->parameters.append(file.fileName());*/
	//}

	//create a gcc process and submit the parameters
	gccProcess = new QProcess(this);
	connect(gccProcess,
		SIGNAL(finished(int, QProcess::ExitStatus)),
		this,
		SLOT(onExecuteFinished(int, QProcess::ExitStatus))
	);
	connect(gccProcess,
		SIGNAL(error(int, QProcess::ExitStatus)),
		this,
		SLOT(onExecuteError(QProcess::ProcessError))
	);
	gccProcess->start("gcc", this->parameters);
}

void Job::onExecuteFinished(int exitCode, QProcess::ExitStatus exitStatus) {
	jobResult.consoleOutput = gccProcess->readAll();
	jobResult.returnValue = exitCode;
	emit finished(this);
}

void Job::onExecuteError(QProcess::ProcessError error) {

	jobResult.consoleOutput = gccProcess->readLine() + "\n"
											+ getQProcessErrorDescription(error);
	jobResult.returnValue = -1; //= Error!

	emit finished(this);
}

void Job::onPreProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	this->preProcessListPosition++;
	this->preProcess();
}

void Job::onPreProcessExecuteError(QProcess::ProcessError error) {
	preProcessResult.consoleOutput = gccProcess->readLine() + "\n"
											+ getQProcessErrorDescription(error);
	preProcessResult.returnValue = -1; //= Error!

	emit finished(this);
}

QString Job::getQProcessErrorDescription(QProcess::ProcessError error) {
	//finished signal: finished(bool executed, int resultValue, QString consoleOutput, QList<QByteArray> outputFiles);
	QString errorString = "Error: An unresolveable error occured.\n"; //TODO
	switch(error) {
		case 0: errorString = "Error: Could not start gcc process.\n"; break;
		case 1: errorString = "Error: gcc process started successfully but crashed.\n"; break;
		case 2: errorString = "Error: gcc process timed out.\n"; break;
		case 3: errorString = "Error: An error occurred when attempting to write to the process.\n"; break;
		case 4: errorString = "Error: An error occurred when attempting to read from the process.\n"; break;
		default: errorString = "Error: An unresolveable unknown error occured.\n";
	}
	return errorString;
}




