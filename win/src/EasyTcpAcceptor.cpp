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

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.S_un.S_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if(bind(m_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        goto ACCEPT_FAILED;
    }

    if(listen(m_socket, backlog) == SOCKET_ERROR)
    {
        goto ACCEPT_FAILED;
    }

    m_thread = std::thread(std::bind(&Acceptor::execute, this));

    return true;

ACCEPT_FAILED:
    closesocket(m_socket);
    m_socket = INVALID_SOCKET;
    return false;
}

void Acceptor::execute()
{
    int lenAddr = sizeof(sockaddr_in);
    sockaddr_in addr;
    SOCKET sock;

    while (!m_terminated)
    {
        sock = ::accept(m_socket, (sockaddr*)&addr, &lenAddr));
        if (sock != INVALID_SOCKET)
            if (onAccepted)
                onAccepted(sock);
            else
                closesocket(sock);
    }
}
