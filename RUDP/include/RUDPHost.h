#pragma once

#include <iostream>
#include <vector>
#include "RUDP.h"
#include "RUDPProtocol.h"
#include "RUDPPeer.h"
#include "RUDPPacket.h"

class RUDPHost
{
public:
	RUDPHost(bool isServer, u_short port);

	RUDPPeer* Connect(const char* addressIP, u_short port);
	int SendPacket(RUDPPacket* packet);
	int HostService(size_t timeOut);
	const RUDPSocket& GetSocket() { return _socket; }
	const RUDPPeer* GetPeer() { return _peer; }
	const u_short GetPort() { return _address.port; }

private:
	int SendOutgoingCommands();
	int ReceiveIncomingCommands();
	int HandleIncomingCommands();
	int SendAcknowledgements();
	int CheckTimeouts();

	int HandleConnect(RUDPCommand* command);
	int HandleVerifyConnect(RUDPCommand* command);
	int HandleDisconnect(RUDPCommand* command);
	int HandleAcknowledgement(RUDPCommand* command);
	int HandleSendReliable(RUDPCommand* command);

	RUDPOutgoingCommand* QueueOutgoingCommand(RUDPCommand* command, RUDPPacket* packet, int packetLength);
	RUDPAcknowledgement* QueueAcknowledgment(RUDPCommand* command, u_short sentTime);

private:
	RUDPSocket _socket;
	RUDPAddress _address;
	RUDPAddress _receivedAddress;
	RUDPPeer* _peer;

	size_t _serviceTime;

	// Buffers we use to send commands
	RUDPBuffer _buffers[64];
	u_int _bufferCount;

	// Buffer we use to store incoming packets
	u_char _packetData[2][2048];

	u_char* _receivedData;
	size_t _receivedDataLength;

	size_t _totalReceivedData;
	size_t _totalReceivedPackets;
};

