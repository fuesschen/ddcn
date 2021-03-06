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

#include "CompilerNetworkAdaptor.h"

CompilerNetworkAdaptor::CompilerNetworkAdaptor(CompilerNetwork *network)
		: QDBusAbstractAdaptor(network), network(network) {
	// We have to relay the signals of the CompilerNetwork instance to our
	// signals, we do this via intermediate on* slots
	connect(network,
	        SIGNAL(peerNameChanged(QString)),
	        this,
	        SLOT(onPeerNameChanged(QString)));
	connect(network,
	        SIGNAL(localKeyChanged(PrivateKey)),
	        this,
	        SLOT(onLocalKeyChanged(PrivateKey)));
	connect(network,
	        SIGNAL(bootstrapHintsChanged(QString)),
	        this,
	        SLOT(onBootstrapHintsChanged(QString)));
	connect(network,
	        SIGNAL(endpointsChanged(QString)),
	        this,
	        SLOT(onEndpointsChanged(QString)));
	connect(network,
	        SIGNAL(trustedPeersChanged(QList<TrustedPeer*>)),
	        this,
	        SLOT(onTrustedPeersChanged(QList<TrustedPeer*>)));
	connect(network,
	        SIGNAL(trustedGroupsChanged(QList<TrustedGroup*>)),
	        this,
	        SLOT(onTrustedGroupsChanged(QList<TrustedGroup*>)));
	connect(network,
	        SIGNAL(groupMembershipsChanged(QList<GroupMembership*>)),
	        this,
	        SLOT(onGroupMembershipsChanged(QList<GroupMembership*>)));
	connect(network,
	        SIGNAL(nodeStatusChanged(QString, QString, QString, NodeStatus, QStringList, QStringList)),
	        this,
	        SLOT(onNodeStatusChanged(QString, QString, QString, NodeStatus, QStringList, QStringList)));
}

void CompilerNetworkAdaptor::setPeerName(QString peerName) {
	network->setPeerName(peerName);
}
QString CompilerNetworkAdaptor::getPeerName() {
	return network->getPeerName();
}

void CompilerNetworkAdaptor::setCompression(bool compressionEnabled) {
	network->setCompression(compressionEnabled);
}
bool CompilerNetworkAdaptor::getCompression() {
	return network->getCompression();
}

void CompilerNetworkAdaptor::setLocalKey(QString privateKey) {
	PrivateKey key = PrivateKey::fromPEM(privateKey);
	if (!key.isValid()) {
		return;
	}
	network->setLocalKey(key);
}
void CompilerNetworkAdaptor::generateLocalKey(int keyLength) {
	network->generateLocalKey(keyLength);
}
QString CompilerNetworkAdaptor::getLocalKey() {
	// We only want to pass back the public key
	PublicKey publicKey = network->getLocalKey();
	return publicKey.toPEM();
}

void CompilerNetworkAdaptor::setBootstrapHints(const QString &bootstrapHints) {
	network->setBootstrapHints(bootstrapHints);
}
QString CompilerNetworkAdaptor::getBootstrapHints() {
	return network->getBootstrapHints();
}

void CompilerNetworkAdaptor::setEndpoints(const QString &endpoints) {
	network->setEndpoints(endpoints);
}
QString CompilerNetworkAdaptor::getEndpoints() {
	return network->getEndpoints();
}

void CompilerNetworkAdaptor::addTrustedPeer(QString name, QString publicKey) {
	PublicKey key = PublicKey::fromPEM(publicKey);
	if (!key.isValid()) {
		return;
	}
	network->addTrustedPeer(name, key);
}
void CompilerNetworkAdaptor::removeTrustedPeer(QString name, QString publicKey) {
	PublicKey key = PublicKey::fromPEM(publicKey);
	if (!key.isValid()) {
		return;
	}
	network->removeTrustedPeer(name, key);
}
QList<TrustedPeerInfo> CompilerNetworkAdaptor::getTrustedPeers() {
	return toTrustedPeerInfo(network->getTrustedPeers());
}

void CompilerNetworkAdaptor::addTrustedGroup(QString name, QString publicKey) {
	PublicKey key = PublicKey::fromPEM(publicKey);
	if (!key.isValid()) {
		return;
	}
	network->addTrustedGroup(name, key);
}
void CompilerNetworkAdaptor::removeTrustedGroup(QString name, QString publicKey) {
	PublicKey key = PublicKey::fromPEM(publicKey);
	if (!key.isValid()) {
		return;
	}
	network->removeTrustedGroup(name, key);
}
QList<TrustedGroupInfo> CompilerNetworkAdaptor::getTrustedGroups() {
	return toTrustedGroupInfo(network->getTrustedGroups());
}

void CompilerNetworkAdaptor::addGroupMembership(QString name, QString privateKey) {
	PrivateKey key = PrivateKey::fromPEM(privateKey);
	if (!key.isValid()) {
		return;
	}
	network->addGroupMembership(name, key);
}
void CompilerNetworkAdaptor::removeGroupMembership(QString name, QString publicKey) {
	PublicKey key = PublicKey::fromPEM(publicKey);
	if (!key.isValid()) {
		return;
	}
	network->removeGroupMembership(name, key);
}
QList<GroupMembershipInfo> CompilerNetworkAdaptor::getGroupMemberships() {
	return toGroupMembershipInfo(network->getGroupMemberships());
}

void CompilerNetworkAdaptor::queryNetworkStatus() {
	network->queryNetworkStatus();
}

void CompilerNetworkAdaptor::onPeerNameChanged(QString peerName) {
	emit peerNameChanged(peerName);
}
void CompilerNetworkAdaptor::onCompressionChanged(bool compressionEnabled) {
	emit compressionChanged(compressionEnabled);
}
void CompilerNetworkAdaptor::onLocalKeyChanged(const PrivateKey &privateKey) {
	emit publicKeyChanged(PublicKey(privateKey).toPEM());
}

void CompilerNetworkAdaptor::onBootstrapHintsChanged(const QString &bootstrapHints) {
	emit bootstrapHintsChanged(bootstrapHints);
}
void CompilerNetworkAdaptor::onEndpointsChanged(const QString &endpoints) {
	emit endpointsChanged(endpoints);
}

void CompilerNetworkAdaptor::onTrustedPeersChanged(const QList<TrustedPeer*> &trustedPeers) {
	emit trustedPeersChanged(toTrustedPeerInfo(trustedPeers));
}
void CompilerNetworkAdaptor::onTrustedGroupsChanged(const QList<TrustedGroup*> &trustedGroups) {
	emit trustedGroupsChanged(toTrustedGroupInfo(trustedGroups));
}
void CompilerNetworkAdaptor::onGroupMembershipsChanged(const QList<GroupMembership*> &groupMemberships) {
	emit groupMembershipsChanged(toGroupMembershipInfo(groupMemberships));
}
void CompilerNetworkAdaptor::onNodeStatusChanged(QString name, QString publicKey, QString fingerPrint,
			NodeStatus nodeStatus, QStringList groupNames, QStringList groupKeys) {
	qDebug("Node status changed!");
	emit nodeStatusChanged(name, publicKey, fingerPrint, nodeStatus, groupNames, groupKeys);
}

TrustedPeerInfo CompilerNetworkAdaptor::toTrustedPeerInfo(TrustedPeer *trustedPeer) {
	TrustedPeerInfo info;
	info.name = trustedPeer->getName();
	info.publicKey = trustedPeer->getPublicKey().toPEM();
	return info;
}
TrustedGroupInfo CompilerNetworkAdaptor::toTrustedGroupInfo(TrustedGroup *trustedGroup) {
	TrustedGroupInfo info;
	info.name = trustedGroup->getName();
	info.publicKey = trustedGroup->getPublicKey().toPEM();
	return info;
}
GroupMembershipInfo CompilerNetworkAdaptor::toGroupMembershipInfo(GroupMembership *groupMembership) {
	GroupMembershipInfo info;
	info.name = groupMembership->getName();
	info.privateKey = groupMembership->getPrivateKey().toPEM();
	return info;
}
QList<TrustedPeerInfo> CompilerNetworkAdaptor::toTrustedPeerInfo(const QList<TrustedPeer*> &trustedPeers) {
	QList<TrustedPeerInfo> dbusData;
	TrustedPeer *peer;
	foreach (peer, trustedPeers) {
		dbusData.append(toTrustedPeerInfo(peer));
	}
	return dbusData;
}
QList<TrustedGroupInfo> CompilerNetworkAdaptor::toTrustedGroupInfo(const QList<TrustedGroup*> &trustedGroups) {
	QList<TrustedGroupInfo> dbusData;
	TrustedGroup *group;
	foreach (group, trustedGroups) {
		dbusData.append(toTrustedGroupInfo(group));
	}
	return dbusData;
}
QList<GroupMembershipInfo> CompilerNetworkAdaptor::toGroupMembershipInfo(const QList<GroupMembership*> &groupMemberships) {
	QList<GroupMembershipInfo> dbusData;
	GroupMembership *membership;
	foreach (membership, groupMemberships) {
		dbusData.append(toGroupMembershipInfo(membership));
	}
	return dbusData;
}

