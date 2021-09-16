#include "../include/RUDPHost.h"
#include "../include/RUDPUnix.h"
#include <stdlib.h>
#include <time.h>
#include "../include/RUDPTime.h"

RUDPHost::RUDPHost(bool isServer, u_short port)
{
	_socket = RUDP::SocketCreate();

	if (_socket == -1)
	{
		std::cout << "Socket creation failed with error: " << errno << std::endl;
		return;
	}

    sockaddr_in sin;

    memset(&sin, 0, sizeof(sockaddr_in));

	if (isServer)
	{
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY;
		sin.sin_port = htons(port);

        _address.port  = port;
        _address.host = sin.sin_addr.s_addr;

		if (bind(_socket, (sockaddr*)&sin, sizeof(sockaddr_in)) == -1)
		{
			std::cout << "Error on port binding! " << std::endl;
			return;
		}
		std::cout << "Binding to the port " << GetPort() << " is successful" << std::endl;
	}
	// This is for testing packet loss
	// Implict socket binding for client
#ifdef RUDP_DEBUG
	else
	{
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY;
		sin.sin_port = htons(7776);

		if (bind(_socket, (sockaddr*)&_address, sizeof(sockaddr_in)) == -1)
		{
			std::cout << "Error on client port binding" << std::endl;
			return;
		}

		std::cout << "Binding client to the port: " << GetPort() << " is successful" << std::endl;
	}
#endif

	u_long mode = 1;
	RUDP::SetSocketNonBlock(_socket);

	_bufferCount = 0;
	_receivedDataLength = 0;
	_totalReceivedData = 0;
	_totalReceivedPackets = 0;
	_peer = nullptr;
}

RUDPPeer* RUDPHost::Connect(const char* addressIP, u_short port)
{
	sockaddr_in address;
    RUDPAddress rudpAddress;

    memset(&address, 0, sizeof(sockaddr_in));

	address.sin_family = AF_INET;	
	inet_pton(AF_INET, addressIP, &(address.sin_addr));
	address.sin_port = htons(7777);

    rudpAddress.host = address.sin_addr.s_addr;
    rudpAddress.port = port;

	char host[256];
	char servInfo[256];
	char stringBuffer[128];

	RUDPPeer* peer = new RUDPPeer(this, rudpAddress);
	_peer = peer;

	srand(time(NULL));

	_peer->SetSequenceNumber(0);
	_peer->SetState(RUDP_PEER_STATE_CONNECTING);

	RUDPCommand command;
	command.header.command = RUDP_PROTOCOL_COMMAND_CONNECT | RUDP_PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
	command.header.reliableSequenceNumber = htonl(_peer->GetSequenceNumber());

	QueueOutgoingCommand(&command, NULL, 0);

	return peer;
}

// This is the function application layer will use to send packet to the peer
int RUDPHost::SendPacket(RUDPPacket* packet)
{	
	if (_peer == nullptr)
		return -1;

	if (_peer->GetState() != RUDP_PEER_STATE_CONNECTED)
		return -1;

	RUDPCommand command;

	if (packet->Flags & RUDP_PACKET_FLAG_RELIABLE)
	{
		command.header.command = RUDP_PROTOCOL_COMMAND_SEND_RELIABLE | RUDP_PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
		command.sendReliable.dataLength = htons(packet->GetLength());
	}

	if (QueueOutgoingCommand(&command, packet, packet->GetLength()))
		return -1;

	return 0;
}

int RUDPHost::SendAcknowledgements()
{
	auto acks = _peer->GetAcknowledgements();

	while (!acks->empty())
	{
		RUDPAcknowledgement* ack = acks->front();
		RUDPCommand command = ack->command;

		command.header.command = RUDP_PROTOCOL_COMMAND_ACKNOWLEDGE;
		command.acknowledge.receivedReliableSequenceNumber = ack->command.header.reliableSequenceNumber;
		command.acknowledge.receivedSentTime = htons(ack->sentTime);

		QueueOutgoingCommand(&command, nullptr, sizeof(RUDPCommandAcknowledge));

#ifdef RUDP_DEBUG
		std::cout << "Sending ack for sequence number: " << htons(command.acknowledge.receivedReliableSequenceNumber) << std::endl;
#endif

		acks->erase(acks->begin());
	}

	return 0;
}

RUDPOutgoingCommand* RUDPHost::QueueOutgoingCommand(RUDPCommand* command, RUDPPacket* packet, int packetLength)
{
	RUDPOutgoingCommand* outgoingCommand = new RUDPOutgoingCommand();

	outgoingCommand->command = *command;
	outgoingCommand->sendAttemps = 1;
	outgoingCommand->roundTripTime = _peer->GetRTT();
	outgoingCommand->packet = packet;

	if (command->header.command & RUDP_PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE)
	{
		_peer->SetSequenceNumber(_peer->GetSequenceNumber() + 1);

		outgoingCommand->reliableSequenceNumber = _peer->GetSequenceNumber();
	}

	outgoingCommand->command.header.reliableSequenceNumber = htons(outgoingCommand->reliableSequenceNumber);

	_peer->GetOutgoingCommands()->push(outgoingCommand);

	return outgoingCommand;
}

int RUDPHost::CheckTimeouts()
{
	auto outgoingCommands = _peer->GetOutgoingCommands();
	auto sentReliableCommands = _peer->GetSentReliableCommands();

	for (auto it = sentReliableCommands->begin(); it != sentReliableCommands->end();)
	{
		RUDPOutgoingCommand* outgoingCommand = *it;
		
		if (RUDP_TIME_DIFFERENCE(_serviceTime, outgoingCommand->sentTime) < outgoingCommand->roundTripTime)
		{
			it++;
			continue;
		}

		_peer->SetPacketsLost(_peer->GetPacketsLost() + 1);

		outgoingCommand->roundTripTime *= 2;
		outgoingCommand->sendAttemps += 1;

#ifdef RUDP_DEBUG
		std::cout << "Sending command with header: " << (int) outgoingCommand->command.header.command << " sequence number: " << outgoingCommand->reliableSequenceNumber << " again. Total send Attemps: " << outgoingCommand->sendAttemps  << std::endl;
#endif

		it = sentReliableCommands->erase(it);
		outgoingCommands->push(outgoingCommand);
	}

	return 0;
}

int RUDPHost::SendOutgoingCommands()
{
	if (_peer == nullptr)
		return -1;

	auto outgoingCommands = _peer->GetOutgoingCommands();

	SendAcknowledgements();
	CheckTimeouts();

	while (!outgoingCommands->empty())
	{
		RUDPOutgoingCommand* outgoingCommand = outgoingCommands->front();

		outgoingCommand->sentTime = _serviceTime;
		outgoingCommand->command.header.sentTime = htons(_serviceTime);

		_buffers[_bufferCount].length = sizeof(RUDPCommand);
		_buffers[_bufferCount].data = new unsigned char[_buffers[_bufferCount].length];
		memcpy(_buffers[_bufferCount].data, &(outgoingCommand->command), _buffers[_bufferCount].length);

		_bufferCount++;

		if (outgoingCommand->packet != nullptr)
		{
			size_t packetLength = outgoingCommand->packet->GetLength();

			_buffers[_bufferCount].data = new unsigned char[packetLength];
			_buffers[_bufferCount].length = packetLength;

			memcpy(_buffers[_bufferCount].data, outgoingCommand->packet->GetData(), packetLength);

			_bufferCount++;
		}

		outgoingCommands->pop();

		if (outgoingCommand->command.header.command & RUDP_PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE)
			_peer->GetSentReliableCommands()->push_back(outgoingCommand);
	}

	int sentLength = 0;

	if (_bufferCount > 0)
		sentLength = RUDP::SocketSend(_socket, &_peer->GetAddress(), _buffers, _bufferCount);

	for (int i = 0; i < _bufferCount; i++)
	{
		delete _buffers[i].data;
		_buffers[i].data = nullptr;
	}

	_bufferCount = 0;

	return 0;
}

int RUDPHost::ReceiveIncomingCommands()
{
	RUDPBuffer buffer;
	buffer.data = _packetData[0];
	buffer.length = sizeof(_packetData[0]);

	int receivedLength = 0;

	receivedLength = RUDP::SocketRecv(_socket, &buffer, 1, &_receivedAddress);

	if (receivedLength < 0)
		return -1;

	if (receivedLength == 0)
		return 0;

#ifdef RUDP_DEBUG
	char stringBuffer[128];
	// inet_ntop(AF_INET, &_receivedAddress.host, stringBuffer, 128);
	// std::cout << "We received a packet from: " << stringBuffer << ":" << ntohs(_receivedAddress.sin_port) << " length is: " << receivedLength << std::endl;
#endif

	_receivedData = _packetData[0];
	_receivedDataLength = receivedLength;

	HandleIncomingCommands();

	return 0;
}

int RUDPHost::HandleIncomingCommands()
{
	if (_receivedData == nullptr)
		return -1;

	RUDPCommand* command = (RUDPCommand*)_receivedData;

	unsigned char commandNumber = command->header.command & RUDP_PROTOCOL_COMMAND_MASK;

	switch (commandNumber)
	{
	case RUDP_PROTOCOL_COMMAND_CONNECT:
#ifdef RUDP_DEBUG
		std::cout << "Recevied a connect command" << std::endl;
#endif
		HandleConnect(command);
		break;
	case RUDP_PROTOCOL_COMMAND_VERIFY_CONNECT:
#ifdef RUDP_DEBUG
		std::cout << "Recevied a verify connect command" << std::endl;
#endif
		HandleVerifyConnect(command);
		break;
	case RUDP_PROTOCOL_COMMAND_DISCONNECT:
#ifdef RUDP_DEBUG
		std::cout << "Recevied a disconnect command" << std::endl;
#endif
		HandleDisconnect(command);
		break;
	case RUDP_PROTOCOL_COMMAND_ACKNOWLEDGE:
#ifdef RUDP_DEBUG
		std::cout << "Recevied an ack command" << std::endl;
#endif
		HandleAcknowledgement(command);
		break;
	case RUDP_PROTOCOL_COMMAND_SEND_RELIABLE:
#ifdef RUDP_DEBUG
		std::cout << "Received a send reliable command" << std::endl;
#endif
		HandleSendReliable(command);
		break;
	default:
		std::cout << "Unknown command type " << std::endl;
		break;
	}

	u_short sentTime = ntohs(command->header.sentTime);

	if (_peer != nullptr && command->header.command & RUDP_PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE)
	{
#ifdef RUDP_DEBUG
		std::cout << "Last received command has ack flag " << std::endl;
#endif

		switch (_peer->GetState())
		{
		case RUDP_PEER_STATE_ACKNOWLEDGING_CONNECT:
			break;
		default:
			QueueAcknowledgment(command, sentTime);
			break;
		}
	}

	return 0;
}

int RUDPHost::HandleConnect(RUDPCommand* command)
{
	if (_peer != nullptr)
	{
		std::cout << "Already connected to a peer. Send an information back to the peer " << std::endl;
		return -1;
	}

	RUDPCommand verifyConnect;

	verifyConnect.header.command = RUDP_PROTOCOL_COMMAND_VERIFY_CONNECT | RUDP_PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
	_peer = new RUDPPeer(this, _receivedAddress);
	_peer->SetState(RUDP_PEER_STATE_ACKNOWLEDGING_CONNECT);
	_peer->SetSequenceNumber(0);

	QueueOutgoingCommand(&verifyConnect, nullptr, 0);
}

int RUDPHost::HandleVerifyConnect(RUDPCommand* command)
{
	if (_peer->GetState() != RUDP_PEER_STATE_CONNECTING)
		return 1;

	auto reliableCommands = _peer->GetSentReliableCommands();
	// Remove the connect command from sendReliableCommand list
	auto connectCommand = reliableCommands->front();

	unsigned char commandNumber = connectCommand->command.header.command & RUDP_PROTOCOL_COMMAND_MASK;

	if (commandNumber != RUDP_PROTOCOL_COMMAND_CONNECT)
	{
		std::cout << "Couldn't verify connection command, aborting!" << std::endl;
		return 1;
	}

	// The first in the sentReliableCommands list is the connection command
	// Now that we receive the verify connection command from the peer that means our connection command reached to it
	// So remove it from the sentReliableCommands list
	reliableCommands->erase(reliableCommands->begin());

	_peer->SetState(RUDP_PEER_STATE_CONNECTED);

	char stringBuffer[128];
	inet_ntop(AF_INET, &_peer->GetAddress().host, stringBuffer, 128);
	std::cout << "A connection established with peer: " << stringBuffer << ":" << ntohs(_receivedAddress.port) << std::endl;

	return 0;
}

int RUDPHost::HandleDisconnect(RUDPCommand* command)
{
	return 0;
}

int RUDPHost::HandleSendReliable(RUDPCommand* command)
{
	return 0;
}

RUDPAcknowledgement* RUDPHost::QueueAcknowledgment(RUDPCommand* command, u_short sentTime)
{
	RUDPAcknowledgement* ack = new RUDPAcknowledgement();

	ack->command = *command;
	ack->sentTime = sentTime;

	_peer->GetAcknowledgements()->push_back(ack);

#ifdef RUDP_DEBUG
	std::cout << "Queued ack packet for command header: " << (int)ack->command.header.command << " sequence number: " << ntohs(ack->command.header.reliableSequenceNumber) << std::endl;
#endif

	return ack;
}

int RUDPHost::HandleAcknowledgement(RUDPCommand* command)
{
	u_short roundTripTime;
	size_t receivedSentTime;
	u_short receivedReliableSequenceNumber;

	receivedSentTime = ntohs(command->acknowledge.receivedSentTime);
	receivedSentTime |= _serviceTime & 0xFFFF0000;
	if ((receivedSentTime & 0x8000) > (_serviceTime & 0x8000))
		receivedSentTime -= 0x10000;

	if (RUDP_TIME_LESS(_serviceTime, receivedSentTime))
		return 0;

	roundTripTime = RUDP_TIME_DIFFERENCE(_serviceTime, receivedSentTime);

#ifdef RUDP_DEBUG
	std::cout << "New rtt is: " << roundTripTime << std::endl;
#endif

	_peer->SetRTT(roundTripTime);
	_peer->SetLastReceiveTime(_serviceTime);

	receivedReliableSequenceNumber = ntohs(command->acknowledge.receivedReliableSequenceNumber);

#ifdef RUDP_DEBUG
	std::cout << "Received a ack command with sequence number: " << receivedReliableSequenceNumber << std::endl;
#endif

	auto reliableCommands = _peer->GetSentReliableCommands();
	RUDPOutgoingCommand* reliableOutgoingCommand = nullptr;

	for (auto it = reliableCommands->begin(); it != reliableCommands->end(); it++)
	{
		reliableOutgoingCommand = *it;
	
		if (reliableOutgoingCommand->reliableSequenceNumber == receivedReliableSequenceNumber)
		{
			reliableCommands->erase(it);
			break;
		}
	}

	if (reliableOutgoingCommand == nullptr)
		return 0;

	unsigned char commandNumber = reliableOutgoingCommand->command.header.command & RUDP_PROTOCOL_COMMAND_MASK;

	switch (_peer->GetState())
	{
	case RUDP_PEER_STATE_ACKNOWLEDGING_CONNECT:
		if (commandNumber != RUDP_PROTOCOL_COMMAND_VERIFY_CONNECT)
			return -1;

		_peer->SetState(RUDP_PEER_STATE_CONNECTED);
		break;
	}

	return 0;
}

int RUDPHost::HostService(size_t timeOut)
{
	uint32_t startingTime = RUDP::GetTime();

	do
	{
		_serviceTime = RUDP::GetTime();

		switch (SendOutgoingCommands())
		{
		case 1:
			return 1;
		default:
			break;
		}

		_serviceTime = RUDP::GetTime();

		switch (ReceiveIncomingCommands())
		{
		case 1:
			return 1;
		default:
			break;
		}

	} while (timeOut >= (_serviceTime - startingTime));

	return 0;
}