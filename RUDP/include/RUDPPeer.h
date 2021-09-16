#pragma once

#include <iostream>
#include <queue>
#include "RUDP.h"
#include "RUDPHost.h"

class RUDPHost;

class RUDPPeer
{
public:
	RUDPPeer(RUDPHost* host, RUDPAddress address)
	{
		_host = host;
		_address = address;

		_outgoingCommands = new std::queue<RUDPOutgoingCommand*>();
        _sentReliableCommands = new std::vector<RUDPOutgoingCommand *>();
		_acknowledgements = new std::vector<RUDPAcknowledgement*>();

		_packetsLost = 0;
		_roundTripTime = 1;

		_reliableSequenceNumber = 0;
		_roundTripTime = RUDP_PEER_DEFAULT_ROUND_TRIP_TIME;
	}

    RUDPAddress& GetAddress() { return _address; }

	const u_short GetSequenceNumber() { return _reliableSequenceNumber; }
	void SetSequenceNumber(u_short newSequenceNumber) { _reliableSequenceNumber = newSequenceNumber; }

	const int GetAckNumber() { return _ackNumber; }
	void SetAckNumber(int newAckNumber) { _ackNumber = newAckNumber; }

	const RUDPPeerState GetState() { return _state; }
	void SetState(RUDPPeerState newState) { _state = newState; }

	std::queue<RUDPOutgoingCommand*>* GetOutgoingCommands() { return _outgoingCommands; }
	std::vector<RUDPOutgoingCommand*>* GetSentReliableCommands() { return _sentReliableCommands; }
	std::vector<RUDPAcknowledgement*>* GetAcknowledgements() { return _acknowledgements; }

	const size_t GetRTT() { return _roundTripTime; }
	void SetRTT(size_t newRTT) { _roundTripTime = newRTT; }

	size_t GetPacketsLost() const { return _packetsLost; }
	void SetPacketsLost(size_t newPacketsLost) { _packetsLost = newPacketsLost; }

	const size_t GetLastReceiveTime() { return _lastReceiveTime; }
	void SetLastReceiveTime(size_t newLastReceiveTime) { _lastReceiveTime = newLastReceiveTime; }

private:
	RUDPPeerState _state;
	RUDPHost* _host;
	RUDPAddress _address;

	std::queue<RUDPOutgoingCommand*>* _outgoingCommands;
	// Commands that are sent and waiting for acks
	std::vector<RUDPOutgoingCommand*>* _sentReliableCommands;
	std::vector<RUDPAcknowledgement*>* _acknowledgements;

	size_t _roundTripTime;
	size_t _lastReceiveTime;

	size_t _packetsLost;

	u_short _reliableSequenceNumber;
	u_short _ackNumber;
};
