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

#ifndef DBUSSTRUCTS_H_INCLUDED
#define DBUSSTRUCTS_H_INCLUDED

#include "ToolChain.h"
#include "NodeStatus.h"
#include <QString>
#include <QDBusMetaType>
#include "Job.h"

/*
 * This class contains several functions/operators to be able to transmit
 * custom structs via D-Bus. Do not forget to register them in main() when you
 * add them here.
 */

/**
 * Encapsulates the information about a TrustedPeer for D-Bus function calls.
 */
struct TrustedPeerInfo {
	QString name;
	QString publicKey;
};

Q_DECLARE_METATYPE(TrustedPeerInfo)
Q_DECLARE_METATYPE(QList<TrustedPeerInfo>)

QDBusArgument &operator<<(QDBusArgument &argument, const TrustedPeerInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &argument, TrustedPeerInfo &info);

/**
 * Encapsulates the information about a TrustedGroup for D-Bus function calls.
 */
struct TrustedGroupInfo {
	QString name;
	QString publicKey;
};

Q_DECLARE_METATYPE(TrustedGroupInfo)
Q_DECLARE_METATYPE(QList<TrustedGroupInfo>)

QDBusArgument &operator<<(QDBusArgument &argument, const TrustedGroupInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &argument, TrustedGroupInfo &info);

/**
 * Encapsulates the information about a GroupMembership for D-Bus function calls.
 */
struct GroupMembershipInfo {
	QString name;
	QString privateKey;
};

Q_DECLARE_METATYPE(GroupMembershipInfo)
Q_DECLARE_METATYPE(QList<GroupMembershipInfo>)

QDBusArgument &operator<<(QDBusArgument &argument, const GroupMembershipInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &argument, GroupMembershipInfo &info);


/**
 * Encapsulates the information about a ToolChain for D-Bus function calls.
 */
struct ToolChainInfo {
	QString path;
	QString version;
};

Q_DECLARE_METATYPE(ToolChainInfo)
Q_DECLARE_METATYPE(QList<ToolChainInfo>)

QDBusArgument &operator<<(QDBusArgument &argument, const ToolChainInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &argument, ToolChainInfo &info);

Q_DECLARE_METATYPE(JobResult)

QDBusArgument &operator<<(QDBusArgument &argument, const JobResult &jobResult);
const QDBusArgument &operator>>(const QDBusArgument &argument, JobResult &jobResult);

Q_DECLARE_METATYPE(NodeStatus)

QDBusArgument &operator<<(QDBusArgument &argument, const NodeStatus &nodeStatusInfo);
const QDBusArgument &operator>>(const QDBusArgument &argument, NodeStatus &nodeStatusInfo);

void registerCustomDBusTypes();

#endif
