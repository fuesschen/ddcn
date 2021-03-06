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

#include "InputOutputFilePair.h"
#include <QFile>
#include <QIODevice>
#include <cassert>

InputOutputFilePair::InputOutputFilePair(QString inputExtension,
										 QString outputExtension, QString templateName) {
	this->extensions[0] = inputExtension;
	this->extensions[1] = outputExtension;
	if ((QDir().tempPath()).endsWith("/")) {
		this->path = QDir().tempPath();
	} else {
		this->path = QDir().tempPath() + "/";
	}
	this->filename = generateFilename(templateName);
	createTemporaryFiles();
}


void InputOutputFilePair::createTemporaryFiles() {
	for (int i = 0; i < 2; i++) {
		QFile *file = new QFile(this->filename + this->extensions[i]);
		if (file->open(QIODevice::WriteOnly)) {
			file->close();
		} else {
			// We previously checked that the file does not exist, the
			// template is specific to ddcn and there is only one ddcn service
			// supposed to be running, so just crash here
			assert(false);
		}
	}
}

QString InputOutputFilePair::generateFilename(QString templateName) {
	QString random = randomize();
	QString returnValue = templateName;
	while (QFile().exists(this->path + returnValue + random + this->getInputFileExtension()) ||
		QFile().exists(this->path + returnValue + random + this->getOutputFileExtension())) {
		random = randomize();
	}
	return this->path + returnValue + random;
}

QString InputOutputFilePair::randomize() {
	return QString::number(qrand() % 1000000);
}

QString InputOutputFilePair::getInputFileExtension() {
	return this->extensions[0];
}

QString InputOutputFilePair::getOutputFileExtension() {
	return this->extensions[1];
}

QString InputOutputFilePair::getInputFilename() {
	return this->filename + this->getInputFileExtension();
}

QString InputOutputFilePair::getOutputFilename() {
	return this->filename + this->getOutputFileExtension();
}

QFile* InputOutputFilePair::getInputFile() {
	return new QFile(this->getInputFilename());
}

QFile* InputOutputFilePair::getOutputFile() {
	return new QFile(this->getOutputFilename());
}
