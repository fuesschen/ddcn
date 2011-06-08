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

#include "JobRequest.h"


JobRequest::JobRequest(OnlinePeer sourcePeer, int maxNumberOfAcceptedJobs) {
	this->sourcePeer = sourcePeer;
	this->maxNumberOfAcceptedJobs = 0;
}

OnlinePeer 	JobRequest::getTargetPeer() {
	return this->targetPeer;
}
OnlinePeer 	JobRequest::getSourcePeer() {
	return this->sourcePeer;
}
int	JobRequest::getNumberOfAcceptedJobs() {
	return this->maxNumberOfAcceptedJobs;
}
ToolChain JobRequest::getToolChain() {
	return this->toolChain;
}
QList<Job*>	JobRequest::getJobList() {
	return this->jobList;
}
void JobRequest::setTargetPeet(OnlinePeer targetPeer) {
	this->targetPeer = targetPeer;
}

bool JobRequest::addJobList(QList<Job*> jobList) {
	//TODO geht das?
	QList<Job*> tmpList = jobList;
	while (jobList.count() > this->maxNumberOfAcceptedJobs) {
		tmpList.removeLast();
	}
	return this->jobList.append(tmpList);
}

bool JobRequest::addJob(Job *job) {
	return (this->jobList.count() < this->maxNumberOfAcceptedJobs) ? this->jobList.append(job) : false;
}