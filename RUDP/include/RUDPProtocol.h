#pragma once
#include "RUDP.h"
#include "RUDPPacket.h"

#define RUDP_DEBUG

enum
{
	RUDP_PEER_DEFAULT_ROUND_TRIP_TIME = 500
};

enum RUDPProtocolCommand
{
	RUDP_PROTOCOL_COMMAND_NONE = 0,
	RUDP_PROTOCOL_COMMAND_CONNECT,
	RUDP_PROTOCOL_COMMAND_VERIFY_CONNECT,
	RUDP_PROTOCOL_COMMAND_DISCONNECT,
	RUDP_PROTOCOL_COMMAND_ACKNOWLEDGE,
	RUDP_PROTOCOL_COMMAND_PING,
	RUDP_PROTOCOL_COMMAND_SEND_RELIABLE,

	RUDP_PROTOCOL_COMMAND_MASK = 0x0F
};

enum RUDPProtocolFlag
{
	RUDP_PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE = (1 << 7)
};

enum RUDPPacketFlag
{
	RUDP_PACKET_FLAG_RELIABLE = (1 << 0)
};

enum RUDPPeerState
{
	RUDP_PEER_STATE_CONNECTING = 0,
	RUDP_PEER_STATE_ACKNOWLEDGING_CONNECT, 
	RUDP_PEER_STATE_CONNECTION_SUCCEEDED,
	RUDP_PEER_STATE_CONNECTED,
	RUDP_PEER_STATE_DISCONNECTED
};

struct RUDPCommandHeader
{
	unsigned char command;
	u_short reliableSequenceNumber;
	u_short sentTime;
};

struct RUDPCommandConnect
{
	RUDPCommandHeader header;
	size_t mtu;
};

struct RUDPCommandDisconnect
{
	RUDPCommandHeader header;
};

struct RUDPCommandVerifyConnect
{
	RUDPCommandHeader header;
};

struct RUDPCommandPing
{
	RUDPCommandHeader header;
};

struct RUDPCommandAcknowledge
{
	RUDPCommandHeader header;
	u_short receivedReliableSequenceNumber;
	u_short receivedSentTime;
};

struct RUDPCommandSendReliable
{
	RUDPCommandHeader header;
	u_short dataLength;
};

union RUDPCommand
{
	RUDPCommandHeader header;
	RUDPCommandConnect connect;
	RUDPCommandVerifyConnect verifyConnect;
	RUDPCommandDisconnect disconnect;
	RUDPCommandPing ping;
	RUDPCommandAcknowledge acknowledge;
	RUDPCommandSendReliable sendReliable;
};

struct RUDPAcknowledgement
{
	RUDPCommand command;
	size_t sentTime;
};

struct RUDPOutgoingCommand
{
	RUDPCommand command;
	size_t sentTime;
	u_short sendAttemps;
	u_short reliableSequenceNumber;
	u_short roundTripTime;
	RUDPPacket* packet;
};

#define MAX_MTU 1500
