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

#ifndef INCOMINGJOB_H_INCLUDED
#define INCOMINGJOB_H_INCLUDED

#include <QString>
#include "NetworkNode.h"
#include "Job.h"
/**
 * Class represents a job that has been accepted from the network.
 * Additionally, it contains informathin about the source peer and an unique id.
 */
class IncomingJob {
public:
	/**Constructs a new incoming job.
	 * @param sourcePeer the peer who delegated the job to the network.
	 * @param job the job that has been delegated.
	 * @param id the unique id used to identify the job.
	 */
	IncomingJob(NetworkNode *sourcePeer, Job *job, unsigned int id) {
		this->sourcePeer = sourcePeer;
		this->job = job;
		this->id = id;
	}

	/**
	 * Returns the peer that delegated the job to the network.
	 * @return the peer that delegated the job to the network.
	 */
	NetworkNode *getSourcePeer() {
		return sourcePeer;
	}

	/**
	 * Returns the job that has been delegated to the network.
	 * @return the job that has been delegated to the network.
	 */
	Job *getJob() {
		return job;
	}

	/**
	 * Returns the id used for the job's identification.
	 * @return the id used for the job's identification.
	 */
	unsigned int getId() {
		return id;
	}
private:
	NetworkNode *sourcePeer;
	Job *job;
	unsigned int id;
};

#endif

