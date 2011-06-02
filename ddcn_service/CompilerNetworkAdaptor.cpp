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
	        SIGNAL(encryptionChanged(bool)),
	        this,
	        SLOT(onEncryptionChanged(bool)));
	connect(network,
	        SIGNAL(publicKeyChanged(QString)),
	        this,
	        SLOT(onPublicKeyChanged(QString)));
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
}

void CompilerNetworkAdaptor::setPeerName(QString peerName) {
	network->setPeerName(peerName);
}
QString CompilerNetworkAdaptor::getPeerName() {
	return network->getPeerName();
}

void CompilerNetworkAdaptor::setEncryption(bool encryptionEnabled) {
	network->setEncryption(encryptionEnabled);
}
bool CompilerNetworkAdaptor::getEncryption() {
	return network->getEncryption();
}

void CompilerNetworkAdaptor::setKeys(QString publicKey, QString privateKey) {
	network->setKeys(publicKey, privateKey);
}
void CompilerNetworkAdaptor::generateKeys() {
	network->generateKeys();
}
QString CompilerNetworkAdaptor::getPublicKey() {
	return network->getPublicKey();
}

void CompilerNetworkAdaptor::addTrustedPeer(QString name, QString publicKey) {
	network->addTrustedPeer(name, publicKey);
}
void CompilerNetworkAdaptor::removeTrustedPeer(QString name, QString publicKey) {
	network->removeTrustedPeer(name, publicKey);
}
QList<TrustedPeerInfo> CompilerNetworkAdaptor::getTrustedPeers() {
	return toTrustedPeerInfo(network->getTrustedPeers());
}

void CompilerNetworkAdaptor::addTrustedGroup(QString name, QString publicKey) {
	network->addTrustedGroup(name, publicKey);
}
void CompilerNetworkAdaptor::removeTrustedGroup(QString name, QString publicKey) {
	network->removeTrustedGroup(name, publicKey);
}
QList<TrustedGroupInfo> CompilerNetworkAdaptor::getTrustedGroups() {
	return toTrustedGroupInfo(network->getTrustedGroups());
}

void CompilerNetworkAdaptor::addGroupMembership(QString name, QString publicKey, QString privateKey) {
	network->addGroupMembership(name, publicKey, privateKey);
}
void CompilerNetworkAdaptor::removeGroupMembership(QString name, QString publicKey) {
	network->removeGroupMembership(name, publicKey);
}
QList<GroupMembershipInfo> CompilerNetworkAdaptor::getGroupMemberships() {
	return toGroupMembershipInfo(network->getGroupMemberships());
}

void CompilerNetworkAdaptor::onPeerNameChanged(QString peerName) {
	emit peerNameChanged(peerName);
}
void CompilerNetworkAdaptor::onEncryptionChanged(bool encryptionEnabled) {
	emit encryptionChanged(encryptionEnabled);
}
void CompilerNetworkAdaptor::onPublicKeyChanged(QString publicKey) {
	emit publicKeyChanged(publicKey);
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

TrustedPeerInfo CompilerNetworkAdaptor::toTrustedPeerInfo(TrustedPeer *trustedPeer) {
	TrustedPeerInfo info;
	info.name = trustedPeer->getName();
	info.publicKey = trustedPeer->getPublicKey();
	return info;
}
TrustedGroupInfo CompilerNetworkAdaptor::toTrustedGroupInfo(TrustedGroup *trustedGroup) {
	TrustedGroupInfo info;
	info.name = trustedGroup->getName();
	info.publicKey = trustedGroup->getPublicKey();
	return info;
}
GroupMembershipInfo CompilerNetworkAdaptor::toGroupMembershipInfo(GroupMembership *groupMembership) {
	GroupMembershipInfo info;
	info.name = groupMembership->getName();
	info.publicKey = groupMembership->getPublicKey();
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

