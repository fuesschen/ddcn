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

#ifndef OUTGOINGJOB_H_INCLUDED
#define OUTGOINGJOB_H_INCLUDED

#include "NetworkNode.h"
#include "Job.h"

#include <QTimer>

/**
 * Class contains information about an outgoing Job
 */
class OutgoingJob {
public:
	OutgoingJob(NetworkNode* targetPeer, Job* job, unsigned int id) {
	  this->targetPeer = targetPeer;
	  this->job = job;
	  this->id = id;
	}

	NetworkNode *getTargetPeer() {
		return targetPeer;
	}
	Job *getJob() {
		return job;
	}
	unsigned int getId() {
		return id;
	}

	QTimer &getTimer() {
		return timer;
	}
private:
	NetworkNode *targetPeer;
	Job *job;
	unsigned int id;
	QTimer timer;
};

#endif
