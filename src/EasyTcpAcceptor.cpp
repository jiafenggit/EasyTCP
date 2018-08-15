#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif
#include <unistd.h>
#include "EasyTcpAcceptor.h"

using namespace EasyTcp;

Acceptor::Acceptor()
    : m_socket(INVALID_SOCKET),
      m_terminated(false),
      onAccepted(nullptr)
{

}

Acceptor::~Acceptor()
{
    m_terminated = true;
#ifdef __linux__
    shutdown(m_socket, SHUT_RDWR);
#endif
    closesocket(m_socket);

    if (m_thread.joinable())
        m_thread.join();
}

bool Acceptor::accept(unsigned short port, int backlog)
{
    if (m_socket != INVALID_SOCKET)
            return false;

    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET)
        return false;

    do
    {
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if(bind(m_socket, (sockaddr*)&addr, sizeof(addr)) == -1)
            break;

        if(listen(m_socket, backlog) == -1)
            break;

        m_thread = std::thread(std::bind(&Acceptor::execute, this));

        return true;
    }
    while(0);

    close(m_socket);
    m_socket = INVALID_SOCKET;
    return false;
}

void Acceptor::execute()
{
    socklen_t lenAddr = sizeof(sockaddr_in);
    sockaddr_in addr;
    SOCKET sock;

    while (!m_terminated)
    {
        sock = ::accept(m_socket, (sockaddr*)&addr, &lenAddr);
#ifdef __linux__
        if (sock > 0)
#else
        if (sock != INVALID_SOCKET)
#endif
            if (onAccepted)
                onAccepted(sock);
            else
                closesocket(sock);
    }
}
