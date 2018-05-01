#include "EasyTcpAcceptor.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace EasyTcp;

Acceptor::Acceptor()
    : m_socket(-1),
      m_thread(std::thread()),
      m_terminated(false),
      onAccepted(nullptr)
{

}

Acceptor::~Acceptor()
{
    m_terminated = true;
    shutdown(m_socket, SHUT_RDWR);
    close(m_socket);

    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

bool Acceptor::accept(unsigned short port, int backlog)
{
    if (m_socket != -1)
            return false;

    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == -1)
        return false;

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    int reuse = 1;
    setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    if(bind(m_socket, (sockaddr*)&addr, sizeof(addr)) == -1)
    {
        goto ACCEPT_FAILED;
    }

    if(listen(m_socket, backlog) == -1)
    {
        goto ACCEPT_FAILED;
    }

    m_thread = std::thread(std::bind(&Acceptor::execute, this));

    return true;

ACCEPT_FAILED:
    close(m_socket);
    m_socket = -1;
    return false;
}

void Acceptor::execute()
{
    socklen_t lenAddr = sizeof(sockaddr_in);
    sockaddr_in addr;
    int sock;

    while (!m_terminated)
    {
        sock = ::accept(m_socket, (sockaddr*)&addr, &lenAddr));
        if (sock > 0 && onAccepted)
            onAccepted(sock);
    }
}
