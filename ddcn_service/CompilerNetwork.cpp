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

#include "CompilerNetwork.h"

void FreeCompilerSlotList::append(const FreeCompilerSlots &freeSlots) {
	if (freeSlotCount > 200) {
		// Limit the slot count so that other peers cannot make this peer
		// run low on memory
		return;
	}
	if (freeSlots.slotCount == 0) {
		return;
	}
	slotList.append(freeSlots);
	freeSlotCount += freeSlots.slotCount;
	// As soon as the remote slot count grows, set the new maximum
	maxFreeSlotCount = freeSlotCount;
}
NetworkNode *FreeCompilerSlotList::removeFirst(QString toolChain) {
	while (!slotList.empty()) {
		unsigned int chosenIndex = qrand() % slotList.count();
		FreeCompilerSlots &freeSlots = slotList[chosenIndex];
		if (isCompatible(toolChain, freeSlots)) {
			freeSlots.slotCount--;
			freeSlotCount--;
			// Take a copy as the reference is invalidated in removeLast()
			NetworkNode *node = freeSlots.node;
			if (freeSlots.slotCount == 0) {
				slotList.removeAt(chosenIndex);
			}
			return node;
		} else {
			freeSlotCount -= freeSlots.slotCount;
			slotList.removeAt(chosenIndex);
		}
	}
	return NULL;
}

void FreeCompilerSlotList::removeAll(NetworkNode *node) {
	for (int i = 0; i < slotList.size(); i++) {
		if (slotList[i].node == node) {
			freeSlotCount -= slotList[i].slotCount;
			slotList.removeAt(i);
			i--;
		}
	}
}

bool FreeCompilerSlotList::isCompatible(QString toolChain,
                                        FreeCompilerSlots &freeSlots) {
	if (freeSlots.toolChainVersions.contains(toolChain)) {
		return true;
	}
	foreach (QString version, freeSlots.toolChainVersions) {
		if (ToolChain::isCompatible(toolChain, version)) {
			return true;
		}
	}
	return false;
}

CompilerNetwork::CompilerNetwork() : encryptionEnabled(true),
		compressionEnabled(true), freeLocalSlots(0), lastJobId(0),
		settings(QSettings::IniFormat, QSettings::UserScope, "ddcn", "ddcn"),
		maxThreads(0), currentThreads(0) {
	// Load peer name and public key from configuration
	if (!settings.value("name").isValid()) {
		settings.setValue("name", "ddcn_node");
	}
	QString name = settings.value("name").toString();
	setPeerName(name);
	// Load key from file in the settings directory
	QString keyFile = QFileInfo(settings.fileName()).absolutePath() + "/privkey.pem";
	localKey = PrivateKey::load(keyFile);
	if (!localKey.isValid()) {
		qWarning("Warning: Could not read local private key, generating new key.");
		localKey = PrivateKey::generate(2048);
		if (!localKey.save(keyFile)) {
			qWarning("Warning: Could not save local private key!");
		}
	}
	// Initialize ariba
	network = new NetworkInterface(name, localKey);
	connect(network,
	        SIGNAL(peerConnected(NetworkNode*)),
	        this,
	        SLOT(onPeerConnected(NetworkNode*)));
	connect(network,
	        SIGNAL(peerDisconnected(NetworkNode*)),
	        this,
	        SLOT(onPeerDisconnected(NetworkNode*)));
	connect(network,
	        SIGNAL(messageReceived(NetworkNode*, Packet)),
	        this,
	        SLOT(onMessageReceived(NetworkNode*, Packet)));
	connect(network,
	        SIGNAL(groupMessageReceived(McpoGroup*, NetworkNode*, Packet)),
	        this,
	        SLOT(onGroupMessageReceived(McpoGroup*, NetworkNode*, Packet)));
	loadSettings();
}
CompilerNetwork::~CompilerNetwork() {
	for (int i = 0; i < trustedPeers.size(); i++) {
		delete trustedPeers[i];
	}
	for (int i = 0; i < trustedGroups.size(); i++) {
		delete trustedGroups[i];
	}
	for (int i = 0; i < groupMemberships.size(); i++) {
		delete groupMemberships[i];
	}
	foreach (OutgoingJobRequest *request, outgoingJobRequests) {
		delete request;
	}
	foreach (IncomingJobRequest *request, incomingJobRequests) {
		delete request;
	}
	delete network;
}

void CompilerNetwork::setPeerName(QString peerName) {
	this->peerName = peerName;
	settings.setValue("name", peerName);
	emit peerNameChanged(peerName);
}
QString CompilerNetwork::getPeerName() {
	return peerName;
}

void CompilerNetwork::setCompression(bool compressionEnabled) {
	this->compressionEnabled = compressionEnabled;
	emit compressionChanged(compressionEnabled);
}
bool CompilerNetwork::getCompression() {
	return compressionEnabled;
}

void CompilerNetwork::setLocalKey(const PrivateKey &privateKey) {
	localKey = privateKey;
	// Change the key in NetworkInterface
	network->changeIdentity(peerName, privateKey);
	network->restart();
	// Save key
	QString keyFile = QFileInfo(settings.fileName()).absolutePath() + "/privkey.pem";
	if (!privateKey.save(keyFile)) {
		qWarning("Warning: Could not save local private key!");
	}
	emit localKeyChanged(privateKey);
}
void CompilerNetwork::generateLocalKey(int keyLength) {
	PrivateKey privateKey = PrivateKey::generate(keyLength);
	setLocalKey(privateKey);
}
PrivateKey CompilerNetwork::getLocalKey() {
	return localKey;
}

void CompilerNetwork::setBootstrapHints(const QString &bootstrapHints) {
	bootstrapConfig.setBootstrapHints(bootstrapHints);
	network->restart();
	emit bootstrapHintsChanged(bootstrapHints);
}
QString CompilerNetwork::getBootstrapHints() {
	return bootstrapConfig.getBootstrapHints();
}

void CompilerNetwork::setEndpoints(const QString &endpoints) {
	bootstrapConfig.setEndPoints(endpoints);
	network->restart();
	emit endpointsChanged(endpoints);
}
QString CompilerNetwork::getEndpoints() {
	return bootstrapConfig.getEndPoints();
}

void CompilerNetwork::addTrustedPeer(QString name, const PublicKey &publicKey) {
	TrustedPeer *existing = getTrustedPeer(publicKey);
	if (existing) {
		existing->setName(name);
		return;
	}
	TrustedPeer *trustedPeer = new TrustedPeer(name, publicKey);
	trustedPeers.append(trustedPeer);
	NetworkNode *node = network->getNetworkNode(publicKey);
	if (node) {
		node->setTrustedPeer(trustedPeer);
		trustedPeer->setNetworkNode(node);
	}
	saveSettings();
	emit trustedPeersChanged(trustedPeers);
}
void CompilerNetwork::removeTrustedPeer(QString name, const PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (TrustedPeer *trustedPeer, trustedPeers) {
		if (trustedPeer->getPublicKey() == publicKey) {
			if (trustedPeer->getNetworkNode() != NULL) {
				freeRemoteSlots.removeAll(trustedPeer->getNetworkNode());
				trustedPeer->getNetworkNode()->setTrustedPeer(NULL);
			}
			trustedPeers.removeOne(trustedPeer);
			delete trustedPeer;
			return;
		}
	}
	saveSettings();
	emit trustedPeersChanged(trustedPeers);
}

void CompilerNetwork::addTrustedGroup(QString name, const PublicKey &publicKey) {
	TrustedGroup *existing = getTrustedGroup(publicKey);
	if (existing) {
		existing->setName(name);
		return;
	}
	TrustedGroup *trustedGroup = new TrustedGroup(name, publicKey);
	trustedGroups.append(trustedGroup);
	// Join the group
	ariba::ServiceID serviceId(publicKey.hash());
	trustedGroup->setMcpoGroup(network->joinGroup(serviceId));
	saveSettings();
	emit trustedGroupsChanged(trustedGroups);
}
void CompilerNetwork::removeTrustedGroup(QString name, const PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (TrustedGroup *trustedGroup, trustedGroups) {
		if (trustedGroup->getPublicKey() == publicKey) {
			network->leaveGroup(trustedGroup->getMcpoGroup());
			trustedGroups.removeOne(trustedGroup);
			delete trustedGroup;
			return;
		}
	}
	saveSettings();
	emit trustedGroupsChanged(trustedGroups);
}

void CompilerNetwork::addGroupMembership(QString name, const PrivateKey &privateKey) {
	GroupMembership *existing = getGroupMembership(privateKey);
	if (existing) {
		existing->setName(name);
		return;
	}
	GroupMembership *groupMembership = new GroupMembership(name, privateKey);
	groupMemberships.append(groupMembership);
	// Join the group
	ariba::ServiceID serviceId(PublicKey(privateKey).hash());
	groupMembership->setMcpoGroup(network->joinGroup(serviceId));
	saveSettings();
	emit groupMembershipsChanged(groupMemberships);
}
void CompilerNetwork::removeGroupMembership(QString name, const PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (GroupMembership *groupMembership, groupMemberships) {
		if (groupMembership->getPrivateKey() == publicKey) {
			network->leaveGroup(groupMembership->getMcpoGroup());
			groupMemberships.removeOne(groupMembership);
			delete groupMembership;
			return;
		}
	}
	saveSettings();
	emit groupMembershipsChanged(groupMemberships);
}

void CompilerNetwork::delegateOutgoingJob(Job *job) {
	addWaitingJob(job);
	// Create a job request for the job
	createJobRequests();
}
Job *CompilerNetwork::cancelOutgoingJob() {
	// Cancel a job which has not yet been delegated
	Job *job = removeWaitingJob();
	if (job) {
		return job;
	}
	return NULL;
}
void CompilerNetwork::rejectIncomingJob(Job *job) {
	IncomingJob *incoming = job->getIncomingJob();
	assert(incoming != NULL);
	QByteArray packetData;
	QDataStream stream(&packetData, QIODevice::WriteOnly);
	stream << qToBigEndian(incoming->getId());
	// The job was not executed
	stream << false;
	Packet packet = Packet::fromData(PacketType::JobFinished, packetData);
	network->send(incoming->getSourcePeer(), packet);
}

void CompilerNetwork::setFreeLocalSlots(unsigned int localSlots) {
	freeLocalSlots = localSlots;
}
unsigned int CompilerNetwork::getFreeLocalSlots() {
	return freeLocalSlots;
}

void CompilerNetwork::queryNetworkStatus() {
	qDebug("queryNetworkStatus");
	// Send network status requests to all connected peers
	Packet packet(PacketType::QueryNodeStatus);
	network->sendToAll(packet);
}

void CompilerNetwork::onDelegatedJobFinished(Job *job) {
	IncomingJob *incoming = job->getIncomingJob();
	assert(incoming != NULL);
	// Fetch output data
	JobResult result = job->getJobResult();
	QList<QByteArray> fileContent;
	if (result.returnValue == 0) {
		QStringList outputFiles = job->getOutputFiles();
		foreach (QString fileName, outputFiles) {
			QFile file(fileName);
			if (!file.open(QIODevice::ReadOnly)) {
				qFatal("onDelegatedJobFinished(): Could not read previously created temporary file.");
			}
			fileContent.append(file.readAll());
			file.remove();
		}
		QStringList inputFiles = job->getInputFiles();
		foreach (QString fileName, inputFiles) {
			QFile file(fileName);
			file.remove();
		}
	}
	incomingJobs.removeOne(incoming);
	// Send job result
	qDebug("Remote job finished (id: %d), %d", incoming->getId(), result.returnValue);
	QByteArray packetData;
	QDataStream stream(&packetData, QIODevice::WriteOnly);
	stream << qToBigEndian(incoming->getId());
	// The job was executed
	stream << true;
	stream << qToBigEndian(result.returnValue);
	stream << result.stdout;
	stream << result.stderr;
	stream << fileContent;
	Packet packet = Packet::fromData(PacketType::JobFinished, packetData);
	network->send(incoming->getSourcePeer(), packet);
	delete incoming;
	delete job;
}

void CompilerNetwork::onPeerConnected(NetworkNode *node) {
	TrustedPeer *trustedPeer = getTrustedPeer(node->getPublicKey());
	if (trustedPeer) {
		trustedPeer->setNetworkNode(node);
	}
	// The new peer might accept job requests
	createJobRequests();
}
void CompilerNetwork::onPeerDisconnected(NetworkNode *node) {
	if (node->getTrustedPeer()) {
		node->getTrustedPeer()->setNetworkNode(NULL);
	}
	// Local jobs delegated to this node have been rejected
	for (int i = delegatedJobs.size() - 1; i >= 0; i--) {
		if (delegatedJobs[i]->getTargetPeer() == node) {
			// Move job to waiting list
			delegatedJobs[i]->getJob()->setOutgoingJob(NULL);
			waitingPreprocessedJobs.append(delegatedJobs[i]->getJob());
			delete delegatedJobs[i];
			delegatedJobs.removeAt(i);
			// We might have to create more job requests
			createJobRequests();
		}
	}
	// Abort remote jobs from this node
	for (int i = incomingJobs.size() - 1; i >= 0; i--) {
		if (incomingJobs[i]->getSourcePeer() == node) {
			// Kill the job
			emit incomingJobAborted(incomingJobs[i]->getJob());
			// Delete the job
			delete incomingJobs[i]->getJob();
			delete incomingJobs[i];
			incomingJobs.removeAt(i);
		}
	}
	for (int i = incomingJobRequests.size() - 1; i >= 0; i--) {
		if (incomingJobRequests[i]->source == node) {
			delete incomingJobRequests[i];
			incomingJobRequests.removeAt(i);
		}
	}
	// Abort job requests directed to this node
	bool requestsRemoved = false;
	for (int i = outgoingJobRequests.size() - 1; i >= 0; i--) {
		if (outgoingJobRequests[i]->target == node) {
			delete outgoingJobRequests[i];
			outgoingJobRequests.removeAt(i);
			requestsRemoved = true;
		}
	}
	if (requestsRemoved) {
		// Create new requests if necessary
		createJobRequests();
	}
	// Remove free remote slots on this peer
	freeRemoteSlots.removeAll(node);
}
void CompilerNetwork::onMessageReceived(NetworkNode *node, const Packet &packet) {
	qDebug("Message received: %d, size %d", packet.getType(), packet.getPayloadSize());
	switch (packet.getType()) {
		case PacketType::JobRequest:
			onIncomingJobRequest(node, packet);
			break;
		case PacketType::JobRequestAccepted:
			onJobRequestAccepted(node, packet);
			break;
		case PacketType::JobRequestRejected:
			onJobRequestRejected(node, packet);
			break;
		case PacketType::JobData:
			onJobData(node, packet);
			break;
		case PacketType::JobDataReceived:
			onJobDataReceived(node, packet);
			break;
		case PacketType::JobFinished:
			onJobFinished(node, packet);
			break;
		case PacketType::AbortJob:
			onAbortJob(node, packet);
			break;
		case PacketType::QueryNetworkResources:
			qDebug("Node asking for available resources.");
			reportNetworkResources(node);
			break;
		case PacketType::QueryGroupNetworkResources:
			onQueryGroupNetworkResources(node, packet);
			break;
		case PacketType::NetworkResourcesAvailable:
			onNetworkResourcesAvailable(node, packet);
			break;
		case PacketType::GroupNetworkResourcesAvailable:
			onGroupNetworkResourcesAvailable(node, packet);
			break;
		case PacketType::QueryNodeStatus:
			qDebug("Node asking for node status.");
			reportNodeStatus(node);
			break;
		case PacketType::NodeStatus:
			onNodeStatusChanged(node, packet);
			break;
		default:
			qWarning("Warning: Unknown package type received: %d.", packet.getType());
			break;
	}
}
void CompilerNetwork::onGroupMessageReceived(McpoGroup *group, NetworkNode *node,
		const Packet &packet) {
	switch (packet.getType()) {
		case PacketType::QueryGroupNetworkResources:
			onQueryGroupNetworkResources(node, packet);
			break;
		default:
			qWarning("Warning: Unknown group package type received.");
			break;
	}
}

void CompilerNetwork::onPreprocessingFinished(Job *job) {
	qDebug("onPreprocessingFinished");
	int waitingJobIndex = -1;
	for (int i = 0; i < waitingPreprocessingJobs.count(); i++) {
		if (waitingPreprocessingJobs[i] == job) {
			waitingJobIndex = i;
			break;
		}
	}
	if (waitingJobIndex == -1) {
		// Nothing to do here, the job was cancelled by CompilerService
		qCritical("onPreprocessingFinished: Job already removed?");
		return;
	}
	waitingPreprocessingJobs.removeAt(waitingJobIndex);
	// If an error occurred, the job is finished
	JobResult result = job->getPreprocessingResult();
	if (result.returnValue != 0) {
		job->setFinished(result.returnValue, result.stdout, result.stderr);
		delete job;
		qWarning("Preprocessing finished with error (%d, \"%s\").", result.returnValue, result.stderr.data());
		return;
	}
	// Insert job into waitingPreprocessedJobs if it is in waitingPreprocessingJobs
	// or delegate the job if possible
	if (acceptedJobRequests.size() > 0) {
		OutgoingJobRequest *request = acceptedJobRequests.back();
		delegateJob(job, request);
		acceptedJobRequests.removeLast();
		delete request;
	} else {
		waitingPreprocessedJobs.append(job);
	}
}
void CompilerNetwork::onOutgoingJobRequestTimeout() {
	qWarning("OutgoingJobRequest had a timeout, check your network!");
	// Get the request which triggered the timeout
	int requestIndex = -1;
	for (int i = 0; i < outgoingJobRequests.count(); i++) {
		if (sender() == &outgoingJobRequests[i]->timeout) {
			requestIndex = i;
			break;
		}
	}
	assert(requestIndex >= 0);
	// Remove the request, as if it had been rejected
	NetworkNode *node = outgoingJobRequests[requestIndex]->target;
	delete outgoingJobRequests[requestIndex];
	outgoingJobRequests.removeAt(requestIndex);
	// Remove all free slots from this node as we probably keep running into timeouts
	freeRemoteSlots.removeAll(node);
	// Send new requests to other peers if necessary
	createJobRequests();
}
void CompilerNetwork::onOutgoingJobTimeout() {
	qWarning("OutgoingJob had a timeout, are you working with a slow network connection?");
	// Get the job which triggered the timeout
	int jobIndex = -1;
	for (int i = 0; i < delegatedJobs.count(); i++) {
		if (sender() == &delegatedJobs[i]->getTimer()) {
			jobIndex = i;
			break;
		}
	}
	assert(jobIndex >= 0);
	// Mark the job as cancelled
	OutgoingJob *outgoing = delegatedJobs[jobIndex];
	Job *job = outgoing->getJob();
	emit outgoingJobCancelled(job);
	// Delete the job
	job->setOutgoingJob(NULL);
	delete outgoing;
	delegatedJobs.removeAt(jobIndex);
}
void CompilerNetwork::onIncomingJobRequestTimeout() {
	qWarning("IncomingJobRequest had a timeout, are you working with a slow network connection?");
	// Get the request which triggered the timeout
	int requestIndex = -1;
	for (int i = 0; i < incomingJobRequests.count(); i++) {
		if (sender() == &incomingJobRequests[i]->timeout) {
			requestIndex = i;
			break;
		}
	}
	assert(requestIndex >= 0);
	// We did not execute the job
	IncomingJobRequest *incoming = incomingJobRequests[requestIndex];
	QByteArray packetData;
	QDataStream stream(&packetData, QIODevice::WriteOnly);
	stream << qToBigEndian(incoming->id);
	// The job was not executed
	stream << false;
	Packet packet = Packet::fromData(PacketType::JobFinished, packetData);
	network->send(incoming->source, packet);
	// Remove the request
	delete incomingJobRequests[requestIndex];
	incomingJobRequests.removeAt(requestIndex);
}

TrustedPeer *CompilerNetwork::getTrustedPeer(const PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (TrustedPeer *trustedPeer, trustedPeers) {
		if (trustedPeer->getPublicKey() == publicKey) {
			return trustedPeer;
		}
	}
	return NULL;
}
TrustedGroup *CompilerNetwork::getTrustedGroup(const PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (TrustedGroup *trustedGroup, trustedGroups) {
		if (trustedGroup->getPublicKey() == publicKey) {
			return trustedGroup;
		}
	}
	return NULL;
}
GroupMembership *CompilerNetwork::getGroupMembership(const PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (GroupMembership *groupMembership, groupMemberships) {
		if (groupMembership->getPrivateKey() == publicKey) {
			return groupMembership;
		}
	}
	return NULL;
}

void CompilerNetwork::loadSettings() {
	// Load trusted peers
	{
		int size = this->settings.beginReadArray("trustedPeers");
		QStringList trustedPeerNames;
		QList<PublicKey> trustedPeerKeys;
		for (int i = 0; i < size; ++i) {
			this->settings.setArrayIndex(i);
			QString name = this->settings.value("name").toString();
			PublicKey key = PublicKey::fromPEM(this->settings.value("key").toString());
			if (!key.isValid()) {
				qCritical("Invalid trusted peer key");
				continue;
			}
			qDebug("Adding trusted peer: %s", key.fingerprint().toAscii().data());
			// We have to store these values and cannot call addTrustedPeer()
			// directly because it modifies the settings which we are currently
			// reading
			trustedPeerNames.append(name);
			trustedPeerKeys.append(key);
		}
		this->settings.endArray();
		for (int i = 0; i < trustedPeerNames.size(); ++i) {
			addTrustedPeer(trustedPeerNames[i], trustedPeerKeys[i]);
		}
	}
	// Load trusted groups
	{
		int size = this->settings.beginReadArray("trustedGroups");
		QStringList trustedGroupNames;
		QList<PublicKey> trustedGroupKeys;
		for (int i = 0; i < size; ++i) {
			this->settings.setArrayIndex(i);
			QString name = this->settings.value("name").toString();
			PublicKey key = PublicKey::fromPEM(this->settings.value("key").toString());
			if (!key.isValid()) {
				continue;
			}
			trustedGroupNames.append(name);
			trustedGroupKeys.append(key);
		}
		this->settings.endArray();
		for (int i = 0; i < trustedGroupNames.size(); ++i) {
			addTrustedGroup(trustedGroupNames[i], trustedGroupKeys[i]);
		}
	}
	// Load group memberships
	{
		int size = this->settings.beginReadArray("groupMemberships");
		qDebug("The settings file contains %d sets of group membership data.",
				size);
		QStringList groupMembershipNames;
		QList<PrivateKey> groupMembershipKeys;
		for (int i = 0; i < size; ++i) {
			this->settings.setArrayIndex(i);
			QString name = this->settings.value("name").toString();
			PrivateKey key = PrivateKey::fromPEM(this->settings.value("key").toString());
			if (!key.isValid()) {
				qWarning("Error: Corrupt private group key for group \"%s\", removing the group.",
						name.toAscii().data());
				continue;
			}
			groupMembershipNames.append(name);
			groupMembershipKeys.append(key);
		}
		this->settings.endArray();
		for (int i = 0; i < groupMembershipNames.size(); ++i) {
			qDebug("Adding a group: %s", groupMembershipNames[i].toAscii().data());
			addGroupMembership(groupMembershipNames[i], groupMembershipKeys[i]);
		}
	}
}
void CompilerNetwork::saveSettings() {
	// Save trusted peers
	this->settings.beginWriteArray("trustedPeers");
	for (int i = 0; i < this->trustedPeers.size(); ++i) {
		this->settings.setArrayIndex(i);
		this->settings.setValue("name", trustedPeers[i]->getName());
		this->settings.setValue("key", trustedPeers[i]->getPublicKey().toPEM());
	}
	this->settings.endArray();
	// Save trusted groups
	this->settings.beginWriteArray("trustedGroups");
	for (int i = 0; i < this->trustedGroups.size(); ++i) {
		this->settings.setArrayIndex(i);
		this->settings.setValue("name", trustedGroups[i]->getName());
		this->settings.setValue("key", trustedGroups[i]->getPublicKey().toPEM());
	}
	this->settings.endArray();
	// Save trusted groups
	this->settings.beginWriteArray("groupMemberships");
	for (int i = 0; i < this->groupMemberships.size(); ++i) {
		this->settings.setArrayIndex(i);
		this->settings.setValue("name", groupMemberships[i]->getName());
		this->settings.setValue("key", groupMemberships[i]->getPrivateKey().toPEM());
	}
	this->settings.endArray();
}

void CompilerNetwork::askForFreeSlots() {
	// Send a message to all trusted peers
	// TODO: Maybe include the toolchain version here? Might reduce overall
	// traffic generated
	Packet packet(PacketType::QueryNetworkResources);
	foreach (TrustedPeer *trustedPeer, trustedPeers) {
		if (trustedPeer->getNetworkNode() != NULL) {
			network->send(trustedPeer->getNetworkNode(), packet);
		}
	}
	// The packet sent to the groups has to look different as we have to
	// include the group key
	// TODO: MCPO group functionality not implemented yet!
	// This means that for now we have to send the packet to ALL peers as soon#
	// as we trust a group
	/*foreach (TrustedGroup *trustedGroup, trustedGroups) {
		// Create a new packet for each group containing the group key
		QByteArray payload = trustedGroup->getPublicKey().toDER();
		packet = Packet::fromData(PacketType::QueryGroupNetworkResources, payload);
		network->send(trustedGroup->getMcpoGroup(), packet);
	}*/
	if (trustedGroups.empty()) {
		return;
	}
	QByteArray payload;
	QDataStream stream(&payload, QIODevice::WriteOnly);
	stream << (unsigned short)trustedGroups.size();
	foreach (TrustedGroup *trustedGroup, trustedGroups) {
		stream << trustedGroup->getPublicKey().toDER();
	}
	packet = Packet::fromData(PacketType::QueryGroupNetworkResources, payload);
	network->sendToAll(packet);
}

void CompilerNetwork::reportNodeStatus(NetworkNode *node) {
	NodeStatusPacket nodeStatus;
	nodeStatus.maxThreads = qToBigEndian((unsigned short)maxThreads);
	nodeStatus.currentThreads = qToBigEndian((unsigned short)currentThreads);
	// TODO: This is not displayed anywhere in the GUI yet anyways
	nodeStatus.localJobs = 0;
	nodeStatus.delegatedJobs = qToBigEndian((unsigned short)delegatedJobs.count());
	nodeStatus.remoteJobs = qToBigEndian((unsigned short)incomingJobs.count());
	nodeStatus.groupCount = qToBigEndian((unsigned short)groupMemberships.count());
	QByteArray packetData;
	packetData.resize(sizeof(nodeStatus));
	memcpy(packetData.data(), &nodeStatus, sizeof(nodeStatus));
	QDataStream stream(&packetData, QIODevice::Append);
	stream << getPeerName();
	foreach (GroupMembership *groupMembership, groupMemberships) {
		stream << groupMembership->getName();
		QByteArray key = PublicKey(groupMembership->getPrivateKey()).toDER();
		stream << key;
	}
	Packet packet = Packet::fromData(PacketType::NodeStatus, packetData);
	network->send(node, packet);
}
void CompilerNetwork::reportNetworkResources(NetworkNode *node) {
	if (freeLocalSlots <= 0) {
		return;
	}
	QByteArray packetData;
	QDataStream stream(&packetData, QIODevice::WriteOnly);
	stream << freeLocalSlots;
	// Send toolchain info so that other peers only ask peers who have the correct toolchains
	QStringList toolChainVersions;
	foreach (ToolChain toolChain, toolChains) {
		toolChainVersions.append(toolChain.getVersion());
	}
	stream << toolChainVersions;
	Packet packet = Packet::fromData(PacketType::NetworkResourcesAvailable, packetData);
	network->send(node, packet);
}
void CompilerNetwork::onQueryGroupNetworkResources(NetworkNode *node, const Packet &packet) {
	if (freeLocalSlots <= 0) {
		return;
	}
	QByteArray payload((char*)packet.getPayloadData(), packet.getPayloadSize());
	QDataStream stream(payload);
	// We only use a short here to prevend DoS attacks
	unsigned short groupCount;
	stream >> groupCount;
	for (unsigned short i = 0; i < groupCount; i++) {
		QByteArray derGroupKey;
		stream >> derGroupKey;
		PublicKey key = PublicKey::fromDER(derGroupKey);
		if (!key.isValid()) {
			return;
		}
		// We only want to send a response if we are a member of one of the
		// listed groups
		GroupMembership *group = getGroupMembership(key);
		if (group != NULL) {
			reportGroupNetworkResources(node, group);
			return;
		}
	}
}
void CompilerNetwork::reportGroupNetworkResources(NetworkNode *node, GroupMembership *group) {
	QByteArray packetData;
	QDataStream stream(&packetData, QIODevice::WriteOnly);
	// We have to prove that we are a member of the given group, so we sign our
	// public key and the other peer's public key with the private group key
	QByteArray signedText;
	signedText.append(PublicKey(localKey).toDER());
	signedText.append(node->getPublicKey().toDER());
	stream << PublicKey(group->getPrivateKey()).toDER();
	stream << signedText;
	stream << group->getPrivateKey().sign(signedText);
	// Send number of available slots
	stream << freeLocalSlots;
	// Send toolchain info so that other peers only ask peers who have the correct toolchains
	QStringList toolChainVersions;
	foreach (ToolChain toolChain, toolChains) {
		toolChainVersions.append(toolChain.getVersion());
	}
	stream << toolChainVersions;
	Packet packet = Packet::fromData(PacketType::GroupNetworkResourcesAvailable, packetData);
	network->send(node, packet);
}

void CompilerNetwork::onNodeStatusChanged(NetworkNode *node, const Packet &packet) {
	qDebug("onNodeStatusChanged");
	// Parse node info
	const NodeStatusPacket *nodeStatus = packet.getPayload<NodeStatusPacket>();
	if (!nodeStatus) {
		qWarning("Warning: Received too small packet (%d instead of %d).",
				packet.getPayloadSize(), (int)sizeof(*nodeStatus));
		return;
	}
	NodeStatus status;
	status.currentThreads = qFromBigEndian(nodeStatus->currentThreads);
	status.maxThreads = qFromBigEndian(nodeStatus->maxThreads);
	status.delegatedJobs = qFromBigEndian(nodeStatus->delegatedJobs);
	status.localJobs = qFromBigEndian(nodeStatus->localJobs);
	status.remoteJobs = qFromBigEndian(nodeStatus->remoteJobs);
	// Get the remaining data after the struct
	unsigned int groupCount = qFromBigEndian(nodeStatus->groupCount);
	char *groupData = (char*)packet.getPayloadData() + sizeof(NodeStatusPacket);
	QByteArray remaining(groupData, packet.getPayloadSize() - sizeof(NodeStatusPacket));
	QDataStream stream(remaining);
	QString name;
	stream >> name;
	// Parse group info
	QStringList groupKeys;
	QStringList groupNames;
	for (unsigned int i = 0; i < groupCount; i++) {
		if (stream.atEnd()) {
			qWarning("onNodeStatusChanged: Too few group keys received.");
			break;
		}
		QString groupName;
		stream >> groupName;
		QByteArray keyData;
		stream >> keyData;
		PublicKey key = PublicKey::fromDER(keyData);
		if (!key.isValid()) {
			qWarning("onNodeStatusChanged: Received an invalid key.");
			break;
		}
		groupKeys.append(key.toPEM());
		groupNames.append(groupName);
	}
	QString fingerprint = node->getPublicKey().fingerprint();
	emit nodeStatusChanged(name, node->getPublicKey().toPEM(), fingerprint,
			status, groupNames, groupKeys);
}

void CompilerNetwork::onNetworkResourcesAvailable(NetworkNode *node, const Packet &packet) {
	qDebug("onNetworkResourcesAvailable");
	QByteArray payload((char*)packet.getPayloadData(), packet.getPayloadSize());
	QDataStream stream(payload);
	onGeneralNetworkResourcesAvailable(node, stream);
}
void CompilerNetwork::onGroupNetworkResourcesAvailable(NetworkNode *node, const Packet &packet) {
	QByteArray payload((char*)packet.getPayloadData(), packet.getPayloadSize());
	QDataStream stream(payload);
	// Read group signature
	QByteArray derGroupKey;
	stream >> derGroupKey;
	QByteArray signedText;
	stream >> signedText;
	QByteArray signature;
	stream >> signature;
	// Verify signature
	PublicKey groupKey = PublicKey::fromDER(derGroupKey);
	if (!groupKey.isValid()) {
		return;
	}
	if (!getTrustedGroup(groupKey)) {
		return;
	}
	// We expect a specific and unique text so that nobody can replay this part
	QByteArray expectedText;
	expectedText.append(node->getPublicKey().toDER());
	expectedText.append(PublicKey(localKey).toDER());
	if (signedText != expectedText) {
		return;
	}
	if (!groupKey.verify(signedText, signature)) {
		return;
	}
	// Add network resources
	onGeneralNetworkResourcesAvailable(node, stream);
}
void CompilerNetwork::onGeneralNetworkResourcesAvailable(NetworkNode *node,
		QDataStream &stream) {
	unsigned int availableCount;
	stream >> availableCount;
	if (availableCount == 0) {
		return;
	}
	// This is not a proper fix as the peer can just send the paoutgoingJobRequestscket repeatedly
	if (availableCount > 16) {
		availableCount = 16;
	}
	QStringList toolChainVersions;
	stream >> toolChainVersions;
	qDebug("onNetworkResourcesAvailable: toolchains(%d): %s",
		toolChainVersions.count(), toolChainVersions.join("/").toAscii().data());
	// Register free remote slots for later use
	FreeCompilerSlots freeSlots;
	freeSlots.node = node;
	freeSlots.slotCount = availableCount;
	freeSlots.toolChainVersions = toolChainVersions;
	freeRemoteSlots.append(freeSlots);
	// If we have waiting jobs which can now be sent out, create a request
	createJobRequests();
}

void CompilerNetwork::createJobRequests() {
	qDebug("createJobRequests()");
	// Ask for more remote slots as soon as we have spent 75% of the previous slots
	if (freeRemoteSlots.getFreeSlotCount() <= freeRemoteSlots.getMaxFreeSlotCount() / 4) {
		askForFreeSlots();
	}
	// Send job requests for waiting jobs as long as there are free slots available
	while (freeRemoteSlots.getFreeSlotCount() > 0 && outgoingJobRequests.size() < (int)getWaitingJobCount()) {
		// NOTE: We always create job requests for the toolchain version of the
		// first waiting job, this is okay as we usually compile lots of jobs
		// for the same target architecture
		Job *lastWaiting = NULL;
		if (!waitingPreprocessedJobs.empty()) {
			waitingPreprocessedJobs.last();
		} else if (!waitingPreprocessingJobs.empty()) {
			lastWaiting = waitingPreprocessingJobs.last();
		} else if (!waitingJobs.empty()) {
			lastWaiting = waitingJobs.last();
		} else {
			qFatal("createJobRequests(): waitingJob lists all empty!");
		}
		//assert(lastWaiting != NULL);
		if (lastWaiting == NULL) {
			// TODO: Why does this happen?
			break;
		}
		NetworkNode *target = freeRemoteSlots.removeFirst(lastWaiting->getToolchain().getVersion());
		if (!target) {
			qDebug("createJobRequests: Cannot send job request, no target available.");
			continue;
		}
		qDebug("createJobRequests: Sending job request.");
		// Create request
		OutgoingJobRequest *request = new OutgoingJobRequest;
		request->target = target;
		request->id = generateJobId();
		// A timeout is installed so that we do not wait forever
		connect(&request->timeout, SIGNAL(timeout()), this, SLOT(onOutgoingJobRequestTimeout()));
		request->timeout.setSingleShot(true);
		request->timeout.start(15000);
		outgoingJobRequests.append(request);
		Packet packet(PacketType::JobRequest, qToBigEndian(request->id));
		network->send(request->target, packet);
		// If there are not enough preprocessed waiting jobs, start preprocessing
		// for one of the waiting jobs
		int preprocessed = getPreprocessedWaitingJobCount() + getPreprocessingWaitingJobCount();
		if (outgoingJobRequests.size() > preprocessed) {
			preprocessWaitingJob();
		}
	}
}

void CompilerNetwork::onIncomingJobRequest(NetworkNode *node, const Packet &packet) {
	qDebug("onIncomingJobRequest");
	// Get job id
	bool parsingError = false;
	unsigned int id = 0;
	const unsigned int *idPtr = packet.getPayload<unsigned int>();
	if (idPtr) {
		id = qFromBigEndian(*idPtr);
	} else {
		parsingError = true;
	}
	// Reject the request if necessary
	if (freeLocalSlots == 0 || parsingError) {
		// Request request
		Packet reply(PacketType::JobRequestRejected);
		network->send(node, reply);
		return;
	}
	// Create an incoming job request
	IncomingJobRequest *request = new IncomingJobRequest;
	request->source = node;
	request->id = id;
	connect(&request->timeout, SIGNAL(timeout()), this, SLOT(onIncomingJobRequestTimeout()));
	request->timeout.setSingleShot(true);
	// Fairly high timeout interval as we have to wait for the job data
	request->timeout.start(60000);
	incomingJobRequests.append(request);
	// Send a positive reply
	Packet reply(PacketType::JobRequestAccepted, qToBigEndian(id));
	network->send(node, reply);
}

void CompilerNetwork::onJobRequestAccepted(NetworkNode *node, const Packet &packet) {
	qDebug("onJobRequestAccepted");
	// Get job id
	unsigned int id = 0;
	const unsigned int *idPtr = packet.getPayload<unsigned int>();
	if (idPtr) {
		id = qFromBigEndian(*idPtr);
	} else {
		qWarning("onJobRequestAccepted: Invalid packet received.");
		return;
	}
	// We have to check whether we have sent a job request to this peer
	// to ensure that we do not accept offers from untrusted peers
	// Also, remove the request from the list
	bool requestFound = false;
	for (int i = 0; i < outgoingJobRequests.size(); i++) {
		OutgoingJobRequest *request = outgoingJobRequests[i];
		if (request->target == node && request->id == id) {
			requestFound = true;
			// Really delegate the first job in the queue now
			qDebug("Job request accepted, queue size: %d/%d/%d", waitingJobs.size(),
				waitingPreprocessingJobs.size(), waitingPreprocessedJobs.size());
			Job *job = removePreprocessedWaitingJob();
			if (!job) {
				// No job is ready, so wait until a job has been preprocessed
				acceptedJobRequests.append(request);
				outgoingJobRequests.removeAt(i);
				request->timeout.stop();
				return;
			}
			delegateJob(job, request);
			// Remove the request from the list
			delete request;
			outgoingJobRequests.removeAt(i);
			break;
		}
	}
	if (!requestFound) {
		// TODO: This is happening too often at the moment, better send less request
		//qWarning("onJobRequestAccepted: Received unknown job id (removed because of a timeout?).");
	}
}
void CompilerNetwork::onJobRequestRejected(NetworkNode *node, const Packet &packet) {
	qDebug("onJobRequestRejected");
	// Get job id
	unsigned int id = 0;
	const unsigned int *idPtr = packet.getPayload<unsigned int>();
	if (idPtr) {
		id = qFromBigEndian(*idPtr);
	} else {
		qWarning("onJobRequestRejected: Invalid packet received.");
		return;
	}
	// Remove outgoing job request
	bool requestFound = false;
	for (int i = 0; i < outgoingJobRequests.size(); i++) {
		OutgoingJobRequest *request = outgoingJobRequests[i];
		if (request->target == node && request->id == id) {
			requestFound = true;
			delete request;
			outgoingJobRequests.removeAt(i);
			break;
		}
	}
	if (!requestFound) {
		qWarning("onJobRequestRejected: Received unknown job id.");
	}
	// Remove all free slots from this node as it probably will not accept any later job now
	freeRemoteSlots.removeAll(node);
	// Send new requests to other peers if necessary
	createJobRequests();
}

void CompilerNetwork::onJobData(NetworkNode *node, const Packet &packet) {
	qDebug("onJobData");
	QByteArray packetData((const char*)packet.getPayloadData(), packet.getPayloadSize());
	QDataStream stream(packetData);
	unsigned int id;
	stream >> id;
	id = qFromBigEndian(id);
	bool requestFound = false;
	for (int i = 0; i < incomingJobRequests.size(); i++) {
		if (incomingJobRequests[i]->source == node && incomingJobRequests[i]->id == id) {
			requestFound = true;
			delete incomingJobRequests[i];
			incomingJobRequests.removeAt(i);
			break;
		}
	}
	if (!requestFound) {
		qWarning("onJobData(): Invaild job id.");
		return;
	}
	// Parse packet data
	QString toolchain;
	stream >> toolchain;
	QString language;
	stream >> language;
	QStringList compilerParameters;
	stream >> compilerParameters;
	bool inputCompressed;
	stream >> inputCompressed;
	QList<QByteArray> fileContent;
	stream >> fileContent;
	// Create input files
	QStringList inputFiles;
	QStringList outputFiles;
	for (int i = 0; i < fileContent.size(); i++) {
		InputOutputFilePair filePair(".c", ".o");
		inputFiles.append(filePair.getInputFilename());
		outputFiles.append(filePair.getOutputFilename());
		QFile inputFile(filePair.getInputFilename());
		if (!inputFile.open(QIODevice::WriteOnly)) {
			qFatal("Could not open previously created temporary file.");
		}
		if (inputCompressed) {
			inputFile.write(qUncompress(fileContent[i]));
		} else {
			inputFile.write(fileContent[i]);
		}
		inputFile.close();
	}
	// Get toolchain path
	bool toolchainSupported = false;
	QStringList compatibilityParameters;
	ToolChain toolChainInfo;
	for (int i = 0; i < toolChains.size(); i++) {
		if (ToolChain::isCompatible(toolchain, toolChains[i].getVersion(),
				&compatibilityParameters)) {
			toolChainInfo = toolChains[i];
			toolchainSupported = true;
			break;
		} else {
			qDebug("Incompatible: %s/%s", toolchain.toAscii().data(), toolChains[i].getVersion().toAscii().data());
		}
	}
	compilerParameters.append(compatibilityParameters);
	if (!toolchainSupported) {
		QByteArray packetData;
		QDataStream stream(&packetData, QIODevice::WriteOnly);
		stream << qToBigEndian(id);
		// The job was not executed
		stream << false;
		Packet packet = Packet::fromData(PacketType::JobFinished, packetData);
		network->send(node, packet);
		return;
	} else {
		qDebug("Toolchain chosen: %s", toolChainInfo.getPath().toAscii().data());
	}
	// Create job
	Job *job = new Job(inputFiles, outputFiles, QStringList(), QStringList(),
			compilerParameters, toolChainInfo, QDir::tempPath(), true, false,
			QByteArray(), language);
	IncomingJob *incoming = new IncomingJob(node, job, id);
	job->setIncomingJob(incoming);
	incomingJobs.append(incoming);
	qDebug("Created remote job (id: %d)", incoming->getId());
	emit receivedJob(job);
	// Send packet indicating that the job was received
	qDebug("Sending JobDataReceived...");
	Packet reply(PacketType::JobDataReceived, qToBigEndian(id));
	network->send(node, reply);
}
void CompilerNetwork::onJobDataReceived(NetworkNode *node, const Packet &packet) {
	qDebug("onJobDataReceived");
	unsigned int id = qFromBigEndian(*packet.getPayload<unsigned int>());
	// Restart outgoing job timeout (we can wait longer for actual compilation
	// than for receiving the job data)
	OutgoingJob *outgoing = NULL;
	for (int i = 0; i < delegatedJobs.size(); i++) {
		if (delegatedJobs[i]->getTargetPeer() == node && delegatedJobs[i]->getId() == id) {
			outgoing = delegatedJobs[i];
			break;
		}
	}
	if (outgoing == NULL) {
		qWarning("onJobDataReceived(): Invaild job id.");
		return;
	}
	connect(&outgoing->getTimer(), SIGNAL(timeout()), this, SLOT(onOutgoingJobTimeout()));
	outgoing->getTimer().setSingleShot(true);
	// This time we choose a longer interval as compiling might take some time
	// for large files
	// TODO: There still might be files which take longer than this, at least
	// with C++ and complex templates - do we have to care about these
	// pathologic cases? No real project should hit this
	outgoing->getTimer().start(120000);
}
void CompilerNetwork::onJobFinished(NetworkNode *node, const Packet &packet) {
	qDebug("onJobFinished");
	QByteArray packetData((const char*)packet.getPayloadData(), packet.getPayloadSize());
	QDataStream stream(packetData);
	unsigned int id;
	stream >> id;
	id = qFromBigEndian(id);
	OutgoingJob *outgoing = NULL;
	int outgoingIndex = -1;
	for (int i = 0; i < delegatedJobs.size(); i++) {
		if (delegatedJobs[i]->getTargetPeer() == node && delegatedJobs[i]->getId() == id) {
			outgoing = delegatedJobs[i];
			outgoingIndex = i;
			break;
		}
	}
	if (outgoing == NULL) {
		qWarning("onJobFinished(): Invaild job id.");
		return;
	}
	Job *job = outgoing->getJob();
	// The peer might not have executed the job at all e.g. if the toolchain
	// was removed before the job could be completed
	bool executed;
	stream >> executed;
	if (!executed) {
		emit outgoingJobCancelled(job);
		// Delete the job
		job->setOutgoingJob(NULL);
		delete outgoing;
		delegatedJobs.removeAt(outgoingIndex);
		return;
	}
	// Get output data
	int returnValue;
	stream >> returnValue;
	returnValue = qFromBigEndian(returnValue);
	QByteArray stdout;
	stream >> stdout;
	QByteArray stderr;
	stream >> stderr;
	QList<QByteArray> outputFileContent;
	stream >> outputFileContent;
	// Create output files
	if (outputFileContent.size() > job->getOutputFiles().size()) {
		qWarning("onJobFinished(): Received too many output files.");
		outputFileContent = outputFileContent.mid(0, job->getOutputFiles().size());
	}
	for (int i = 0; i < outputFileContent.size(); i++) {
		QFile file(job->getWorkingDirectory() + "/" + job->getOutputFiles()[i]);
		if (!file.open(QIODevice::WriteOnly)) {
			qWarning("Could not open output file.");
			stderr.append(QString("\nddcn: Could not open output file.").toAscii());
			if (returnValue == 0) {
				returnValue = -1;
			}
			continue;
		}
		file.write(outputFileContent[i]);
	}
	// Finish job
	job->setFinished(returnValue, stdout, stderr);
	// Delete the job
	job->setOutgoingJob(NULL);
	delete outgoing;
	delegatedJobs.removeAt(outgoingIndex);
	qDebug("Job finished (id: %d), %d delegated jobs remaining.", id, delegatedJobs.size());
	delete job;
}
void CompilerNetwork::onAbortJob(NetworkNode *node, const Packet &packet) {
	qDebug("onAbortJob");
	// Get job id
	unsigned int id = 0;
	const unsigned int *idPtr = packet.getPayload<unsigned int>();
	if (idPtr) {
		id = qFromBigEndian(*idPtr);
	} else {
		qWarning("onAbortJob: Invalid packet received.");
		return;
	}
	// Check if the job has potentially already been started
	for (int i = 0; i < incomingJobs.count(); i++) {
		if (incomingJobs[i]->getSourcePeer() == node && incomingJobs[i]->getId() == id) {
			// Kill the job
			emit incomingJobAborted(incomingJobs[i]->getJob());
			// Delete the job
			delete incomingJobs[i]->getJob();
			delete incomingJobs[i];
			incomingJobs.removeAt(i);
			return;
		}
	}
	// Cancel an incoming job request if it has not
	for (int i = incomingJobRequests.size() - 1; i >= 0; i--) {
		if (incomingJobRequests[i]->source == node && incomingJobRequests[i]->id == id) {
			delete incomingJobRequests[i];
			incomingJobRequests.removeAt(i);
			return;
		}
	}
	qWarning("onAbortJob(): Invalid job id.");
}

void CompilerNetwork::addWaitingJob(Job *job) {
	if (job->wasPreprocessed()) {
		waitingPreprocessedJobs.append(job);
	} else if (job->isPreprocessing()) {
		// We do not need to connect to the preprocessingFinished() signal as
		// we are already connected to it, CompilerNetwork started preprocessing
		// anyways
		waitingPreprocessingJobs.append(job);
	} else {
		waitingJobs.append(job);
	}
}
Job *CompilerNetwork::removeWaitingJob() {
	// Prefer to pick a job from one of the lists where little work has already
	// been done
	if (!waitingJobs.empty()) {
		Job *job = waitingJobs.last();
		waitingJobs.removeLast();
		return job;
	} else if (!waitingPreprocessingJobs.empty()) {
		Job *job = waitingPreprocessingJobs.last();
		waitingPreprocessingJobs.removeLast();
		return job;
	} else if (!waitingPreprocessedJobs.empty()) {
		Job *job = waitingPreprocessedJobs.last();
		waitingPreprocessedJobs.removeLast();
		return job;
	} else {
		return NULL;
	}
}
Job *CompilerNetwork::removePreprocessedWaitingJob() {
	if (waitingPreprocessedJobs.empty()) {
		return NULL;
	}
	Job *job = waitingPreprocessedJobs.last();
	waitingPreprocessedJobs.removeLast();
	return job;
}
unsigned int CompilerNetwork::getWaitingJobCount() {
	return waitingJobs.count() + waitingPreprocessedJobs.count()
			+ waitingPreprocessingJobs.count();
}
unsigned int CompilerNetwork::getPreprocessingWaitingJobCount() {
	return waitingPreprocessedJobs.size();
}
unsigned int CompilerNetwork::getPreprocessedWaitingJobCount() {
	return waitingPreprocessedJobs.size();
}

void CompilerNetwork::preprocessWaitingJob() {
	qDebug("preprocessWaitingJob");
	if (waitingJobs.empty()) {
		// This should not happen, should be checked in createJobRequests()
		qCritical("preprocessWaitingJob() called without unpreprocessed waiting jobs!");
		return;
	}
	// We pick the first job because it has been waiting for the longest time
	// to avoid timeouts
	Job *job = waitingJobs.first();
	waitingJobs.removeFirst();
	waitingPreprocessingJobs.append(job);
	// Start preprocessing
	connect(job, SIGNAL(preprocessingFinished(Job*)),
			this, SLOT(onPreprocessingFinished(Job*)));
	job->preProcess();
}

void CompilerNetwork::delegateJob(Job *job, OutgoingJobRequest *request) {
	qDebug("delegateJob");
	// Collect input data
	QStringList inputFiles = job->getPreprocessedFiles();
	QList<QByteArray> fileContent;
	foreach (QString fileName, inputFiles) {
		QFile file(fileName);
		if (!file.open(QIODevice::ReadOnly)) {
			qFatal("Could not open previously created temporary file.");
		}
		if (compressionEnabled) {
			fileContent.append(qCompress(file.readAll()));
		} else {
			fileContent.append(file.readAll());
		}
	}
	// Get parameters
	QStringList compilerParameters = job->getCompilerParameters();
	ToolChain toolchain = job->getToolchain();
	// Create job data packet
	QByteArray packetData;
	QDataStream stream(&packetData, QIODevice::WriteOnly);
	stream << qToBigEndian(request->id);
	stream << toolchain.getVersion();
	stream << job->getLanguage();
	stream << compilerParameters;
	stream << compressionEnabled;
	stream << fileContent;
	Packet packet = Packet::fromData(PacketType::JobData, packetData);
	qDebug("Outgoing job size: %d bytes", packetData.size());
	network->send(request->target, packet);
	// Store outgoing job info
	OutgoingJob *outgoing = new OutgoingJob(request->target, job, request->id);
	connect(&outgoing->getTimer(), SIGNAL(timeout()), this, SLOT(onOutgoingJobTimeout()));
	outgoing->getTimer().setSingleShot(true);
	outgoing->getTimer().start(60000);
	job->setOutgoingJob(outgoing);
	delegatedJobs.append(outgoing);
	qDebug("Delegated job (id: %d)", outgoing->getId());
}
