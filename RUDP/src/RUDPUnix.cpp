#include "../include/RUDP.h"
#include <random>

#ifdef RUDP_DEBUG
	#define PACKET_LOSS_PERC 0
#endif

 static uint32_t timeBase = 0;


int RUDP::Initialize()
{
    return 0;
}

int RUDP::Deinitialize()
{
    return 0;
}

 uint32_t RUDP::GetTime()
 {
    timeval timeVal;

     gettimeofday(&timeVal, NULL);

     return timeVal.tv_sec * 1000 + timeVal.tv_usec / 1000 - timeBase;
 }

 int RUDP::AddressSetHostIp(RUDPAddress *address, const char *ip)
 {
     if (!inet_pton(AF_INET, ip, &address->host))
         return -1;

     return 0;
 }

 int RUDP::SocketCreate()
 {
    return socket(AF_INET, SOCK_DGRAM, 0);
 }

 int RUDP::SetSocketNonBlock(const RUDPSocket& socket)
 {
    int result = -1;

     result = fcntl(socket, F_SETFL, O_NONBLOCK | (fcntl(socket, F_GETFL) & ~O_NONBLOCK));

     return result == -1 ? -1 : 0;
 }

 int RUDP::SocketSend(RUDPSocket socket, RUDPAddress* address, const RUDPBuffer* buffers, size_t bufferCount)
{
    msghdr msgHeader;
	sockaddr_in sin;
    int sentLength = 0;
	int recvAddrSize = sizeof(address);

    memset(&msgHeader, 0, sizeof(msghdr));

    if (address != nullptr)
    {
        memset(&sin, 0, sizeof(sockaddr_in));

        sin.sin_family = AF_INET;
        sin.sin_port = htons(address->port);
        sin.sin_addr.s_addr = address->host;

        char stringBuffer[128];
        std::cout << inet_ntop(AF_INET, &address->host, stringBuffer, 128);

        msgHeader.msg_name = &sin;
        msgHeader.msg_namelen = sizeof(sockaddr_in);
    }

    msgHeader.msg_iov = (iovec *) buffers;
    msgHeader.msg_iovlen = bufferCount;

    // Packet Loss Debug
#ifdef RUDP_DEBUG
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist100(1, 100);
    int randomValue = dist100(rng);

    if (randomValue < PACKET_LOSS_PERC)
    {
        std::cout << "Failed sending packet" << std::endl;
        return 0;
    }

#endif

    sentLength = sendmsg(socket, &msgHeader, MSG_NOSIGNAL);

    if (sentLength == -1)
    {
        if (errno == EWOULDBLOCK)
            return 0;

        std::cout << "Error while sending packet: "  << errno << std::endl;
        return -1;
    }


    return sentLength;
}

int RUDP::SocketRecv(RUDPSocket socket, RUDPBuffer* buffers, size_t bufferCount, RUDPAddress* outAddress)
{
	int addressLength = sizeof(outAddress);

    msghdr msgHeader;
    sockaddr_in sin;
	int recvLength;

    memset(&msgHeader, 0, sizeof(msgHeader));

    if (outAddress != nullptr)
    {
        msgHeader.msg_name = &sin;
        msgHeader.msg_namelen = sizeof(sockaddr_in);
    }

    msgHeader.msg_iov = (iovec *) buffers;
    msgHeader.msg_iovlen = bufferCount;

    recvLength = recvmsg(socket, &msgHeader, MSG_NOSIGNAL);

    if (recvLength == -1)
    {
        if (errno == EWOULDBLOCK)
            return 0;

        return -1;
    }

    if (outAddress != nullptr)
    {
        outAddress->host = (uint32_t) sin.sin_addr.s_addr;
        outAddress->port = ntohs(sin.sin_port);
    }

	return recvLength;
}