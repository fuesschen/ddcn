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

#ifndef COMPILERNETWORK_H_INCLUDED
#define COMPILERNETWORK_H_INCLUDED

#include "TrustedPeer.h"
#include "TrustedGroup.h"
#include "GroupMembership.h"
#include "Job.h"
#include "NetworkInterface.h"
#include "OutgoingJob.h"
#include "NodeStatus.h"
#include "IncomingJob.h"
#include "JobRequest.h"
#include "ToolChain.h"

#include <QObject>

/**
 * Contains the number of free remote slots after the network node has adverised
 * that it has less local jobs than it could execute.
 */
struct FreeCompilerSlots {
	NetworkNode *node;
	unsigned int slotCount;
	QStringList toolChainVersions;
};

/**
 * Manages a list containing the peers which have advertised free compiler slots
 * on their machine.
 */
class FreeCompilerSlotList {
public:
	/**
	 * Constructor. Creates an empty list.
	 */
	FreeCompilerSlotList() : freeSlotCount(0), maxFreeSlotCount(0) {
	}

	/**
	 * Appends a number of free remote slots to the list.
	 */
	void append(const FreeCompilerSlots &freeSlots);
	/**
	 * Pops a random free remote slot from the list whose toolchain is
	 * compatible to this toolchain version.
	 *
	 * This removes free slots with an incompatible toolchain from the list if
	 * it encounters them.
	 *
	 * @param toolChain Tool chain version which has to be supported.
	 * @return NetworkNode which has had a free remote slot available. Might be
	 * NULL if no slot was found.
	 */
	NetworkNode *removeFirst(QString toolChain);

	/**
	 * Removes all slots advertised by a certain node, e.g. when the node
	 * disconnects.
	 */
	void removeAll(NetworkNode *node);

	/**
	 * Returns the overall number of free remove slots in the list.
	 */
	unsigned int getFreeSlotCount() {
		return freeSlotCount;
	}
	/**
	 * Returns the number of slots after the last call to append(). This is used
	 * to compute the amount of slots used after this peer has received the last
	 * offer as a heuristic as to when this peer has to ask for remote slots
	 * again.
	 */
	unsigned int getMaxFreeSlotCount() {
		return maxFreeSlotCount;
	}
private:
	static bool isCompatible(QString toolChain, FreeCompilerSlots &freeSlots);

	QList<FreeCompilerSlots> slotList;
	unsigned int freeSlotCount;
	unsigned int maxFreeSlotCount;
};

/**
 * Class which communicates with peers in the compiler network and receives
 * and sends compiler jobs.
 */
class CompilerNetwork : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString peerName
	           READ getPeerName
	           WRITE setPeerName
	           NOTIFY peerNameChanged)
public:
	/**
	 * Creates a CompilerNetwork object.
	 *
	 * This does not make much sense on its own, you at least want to connect
	 * the signals to a CompilerService object.
	 */
	CompilerNetwork();
	/**
	 * Destructor.
	 */
	~CompilerNetwork();

	/**
	 * Sets the name of this peer.
	 * The name is loaded from the settings file on startup, the default value
	 * is "ddcn_node". This name is sent every time another peer asks for the
	 * status of this peer.
	 * @param peerName New name of the peer.
	 */
	void setPeerName(QString peerName);
	/**
	 * Returns the current name of this peer.
	 * @return Name of this peer.
	 */
	QString getPeerName();

	/**
	 * Enables compression of outgoing source files.
	 * @param compressionEnabled True if outgoing text files shall be
	 * compressed.
	 */
	void setCompression(bool compressionEnabled);
	/**
	 * Returns whether outgoing shource files are sent compressed.
	 * @return True if outgoing text fules are sent compressed.
	 */
	bool getCompression();

	/**
	 * Sets the private key of the local node.
	 * This key is used to authenticate this peer at other peers.
	 *
	 * @note This causes all connections to drop as their key are invalidated.
	 * @param privateKey New private key for this peer.
	 */
	void setLocalKey(const PrivateKey &privateKey);
	/**
	 * Automatically generates a new RSA private key for this network node.
	 * @param keyLength The key length in bits of the new key.
	 */
	void generateLocalKey(int keyLength);
	/**
	 * Returns the private key of this node.
	 * @return Local private key.
	 */
	PrivateKey getLocalKey();

	/**
	 * Sets the bootstrap hints which are passed to ariba.
	 *
	 * The format is the one documented at
	 * http://www.ariba-underlay.org/wiki/Documentation/Configuration
	 *
	 * @note This causes all connections to drop as ariba is reinitialized.
	 * @param bootstrapHints New bootstrap hints.
	 */
	void setBootstrapHints(const QString &bootstrapHints);
	/**
	 * Returns the bootstrap hints used for ariba.
	 *
	 * @return Bootstrap hints.
	 */
	QString getBootstrapHints();

	/**
	 * Sets the local endpoint settings which are passed to ariba.
	 *
	 * The format is the one documented at
	 * http://www.ariba-underlay.org/wiki/Documentation/Configuration
	 *
	 * @note This causes all connections to drop as ariba is reinitialized.
	 * @param bootstrapHints New endpoints.
	 */
	void setEndpoints(const QString &endpoints);
	/**
	 * Returns the endpoints used by ariba.
	 *
	 * @return Endpoints.
	 */
	QString getEndpoints();

	void addTrustedPeer(QString name, const PublicKey &publicKey);
	void removeTrustedPeer(QString name, const PublicKey &publicKey);
	QList<TrustedPeer*> getTrustedPeers() {
		return trustedPeers;
	}

	void addTrustedGroup(QString name, const PublicKey &publicKey);
	void removeTrustedGroup(QString name, const PublicKey &publicKey);
	QList<TrustedGroup*> getTrustedGroups() {
		return trustedGroups;
	}

	void addGroupMembership(QString name, const PrivateKey &privateKey);
	void removeGroupMembership(QString name, const PublicKey &publicKey);
	QList<GroupMembership*> getGroupMemberships() {
		return groupMemberships;
	}

	/**
	 * Adds a job to the list for the jobs waiting to be delegated to other
	 * peers. Note that it is still possible to cancel the job as long as it
	 * has not actually be sent to another peer.
	 *
	 * @param job Job to be delegated.
	 */
	void delegateOutgoingJob(Job *job);
	/**
	 * Removes one job from the list of the jobs waiting to be delegated. This
	 * does not return any job which has really been sent to another peer, this
	 * is done to ensure that no work is done twice.
	 */
	Job *cancelOutgoingJob();
	/**
	 * Rejects an job which has been received from another peer. This causes
	 * outgoingJobCancelled() to be emitted on the other end.
	 *
	 * @param job Job to be rejected.
	 */
	void rejectIncomingJob(Job *job);

	void setFreeLocalSlots(unsigned int localSlots);
	unsigned int getFreeLocalSlots();

	/**
	 * Asks all connected peers in the network for their identity, load and
	 * group membership list.
	 * This information is returned via the signal nodeStatusChanged when a
	 * peer responds.
	 */
	void queryNetworkStatus();

	/**
	 * Shall be called by CompilerNetwork whenever an incoming job is finished.
	 * @param job Job which has been executed.
	 */
	void onDelegatedJobFinished(Job *job);

	/**
	 * Updates the list of the toolchains this node supports.
	 */
	void setToolChains(QList<ToolChain> toolChains) {
		this->toolChains = toolChains;
	}

	/**
	 * Updates the statistics which are sent out when another peer queries the
	 * node status of this peer.
	 */
	void updateStatistics(unsigned int maxThreads, unsigned int currentThreads) {
		this->maxThreads = maxThreads;
		this->currentThreads = currentThreads;
	}
private slots:
	void onPeerConnected(NetworkNode *node);
	void onPeerDisconnected(NetworkNode *node);
	void onMessageReceived(NetworkNode *node, const Packet &packet);
	void onGroupMessageReceived(McpoGroup *group, NetworkNode *node,
		const Packet &packet);

	void onPreprocessingFinished(Job *job);
	void onOutgoingJobRequestTimeout();
	void onOutgoingJobTimeout();
	void onIncomingJobRequestTimeout();
signals:
	void peerNameChanged(QString peerName);
	void compressionChanged(bool compressionEnabled);
	void localKeyChanged(const PrivateKey &privateKey);
	void bootstrapHintsChanged(const QString &bootstrapHints);
	void endpointsChanged(const QString &endpoints);
	void trustedPeersChanged(QList<TrustedPeer*> trustedPeers);
	void trustedGroupsChanged(QList<TrustedGroup*> trustedGroups);
	void groupMembershipsChanged(QList<GroupMembership*> groupMemberships);
	void receivedJob(Job *job);
	void incomingJobAborted(Job *job);
	void nodeStatusChanged(QString name, QString publicKey, QString fingerPrint,
			NodeStatus nodeStatus, QStringList groupNames, QStringList groupKeys);
	void outgoingJobCancelled(Job *job);
private:
	TrustedPeer *getTrustedPeer(const PublicKey &publicKey);
	TrustedGroup *getTrustedGroup(const PublicKey &publicKey);
	GroupMembership *getGroupMembership(const PublicKey &publicKey);

	void loadSettings();
	void saveSettings();

	void askForFreeSlots();

	void reportNodeStatus(NetworkNode *node);
	void reportNetworkResources(NetworkNode *node);
	void onQueryGroupNetworkResources(NetworkNode *node, const Packet &packet);
	void reportGroupNetworkResources(NetworkNode *node, GroupMembership *group);

	void onNodeStatusChanged(NetworkNode *node, const Packet &packet);
	void onNetworkResourcesAvailable(NetworkNode *node, const Packet &packet);
	void onGroupNetworkResourcesAvailable(NetworkNode *node, const Packet &packet);
	void onGeneralNetworkResourcesAvailable(NetworkNode *node, QDataStream &stream);

	void createJobRequests();
	void onIncomingJobRequest(NetworkNode *node, const Packet &packet);

	void onJobRequestAccepted(NetworkNode *node, const Packet &packet);
	void onJobRequestRejected(NetworkNode *node, const Packet &packet);

	void onJobData(NetworkNode *node, const Packet &packet);
	void onJobDataReceived(NetworkNode *node, const Packet &packet);
	void onJobFinished(NetworkNode *node, const Packet &packet);
	void onAbortJob(NetworkNode *node, const Packet &packet);

	void addWaitingJob(Job *job);
	Job *removeWaitingJob();
	Job *removePreprocessedWaitingJob();
	unsigned int getWaitingJobCount();
	unsigned int getPreprocessingWaitingJobCount();
	unsigned int getPreprocessedWaitingJobCount();
	void preprocessWaitingJob();

	void delegateJob(Job *job, OutgoingJobRequest *request);

	unsigned int generateJobId() {
		return ++lastJobId;
	}

	QString peerName;
	bool encryptionEnabled;
	bool compressionEnabled;
	PrivateKey localKey;

	// TODO: Do we need much lookups here? A hash map then would be faster.
	// We could need public-key based lookups a lot.
	QList<TrustedPeer*> trustedPeers;
	QList<TrustedGroup*> trustedGroups;
	QList<GroupMembership*> groupMemberships;

	NetworkInterface *network;

	QList<Job*> waitingJobs;
	QList<Job*> waitingPreprocessingJobs;
	QList<Job*> waitingPreprocessedJobs;

	FreeCompilerSlotList freeRemoteSlots;
	unsigned int freeLocalSlots;

	QList<OutgoingJob*> delegatedJobs;

	QList<OutgoingJobRequest*> outgoingJobRequests;
	QList<IncomingJobRequest*> incomingJobRequests;

	QList<OutgoingJobRequest*> acceptedJobRequests;

	QList<IncomingJob*> incomingJobs;

	unsigned int lastJobId;

	QList<ToolChain> toolChains;

	QSettings settings;

	BootstrapConfig bootstrapConfig;

	// Statistics which are sent with NodeStatus packets
	unsigned int maxThreads;
	unsigned int currentThreads;
};

#endif
