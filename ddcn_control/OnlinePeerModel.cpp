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

#include "OnlinePeerModel.h"

void OnlinePeerModel::clear() {
	onlinePeers.clear();
	reset();
}

void OnlinePeerModel::updateNode(QString name, QString key, bool trusted,
		float load, bool inTrustedGroup) {
	bool found = false;
	for (int i = 0; i < onlinePeers.size(); i++) {
		if (onlinePeers[i].key == key) {
			found = true;
			onlinePeers[i].name = name;
			onlinePeers[i].trusted = trusted;
			onlinePeers[i].load = load;
			onlinePeers[i].inTrustedGroup = inTrustedGroup;
		}
	}
	if (!found) {
		OnlinePeerInfo newPeer;
		newPeer.name = name;
		newPeer.key = key;
		newPeer.trusted = trusted;
		newPeer.load = load;
		newPeer.inTrustedGroup = inTrustedGroup;
		onlinePeers.append(newPeer);
	}
	// TODO: Emit proper change signals, this is way too slow
	reset();
}

int OnlinePeerModel::columnCount(const QModelIndex &parent) const {
	// Columns: Trusted status, name, key fingerprint, peer load, member of trusted group
	return 5;
}
QVariant OnlinePeerModel::data(const QModelIndex &index, int role) const {
	if (index.row() < 0 || index.row() >= onlinePeers.size()) {
		return QVariant();
	}
	OnlinePeerInfo peer = onlinePeers[index.row()];
	switch (index.column()) {
		case 0:
			return peer.trusted;
		case 1:
			return peer.name;
		case 2:
			return peer.key;
		case 3:
			return peer.load;
		case 4:
			return peer.inTrustedGroup;
		default:
			return QVariant();
	}
}
QVariant OnlinePeerModel::headerData(int section, Qt::Orientation orientation, int role) const {
	switch (section) {
		case 0:
			return QVariant();
		case 1:
			return QString("Name");
		case 2:
			return QString("Key fingerprint");
		case 3:
			return QString("Load");
		case 4:
			return QString("Member of trusted group");
		default:
			return QVariant();
	}
}
QModelIndex OnlinePeerModel::index(int row, int column, const QModelIndex &parent) const {
	return createIndex(row, column);
}
int OnlinePeerModel::rowCount(const QModelIndex &parent) const {
	if (!parent.isValid()) {
		return 0;
	}
	return onlinePeers.count();
}
