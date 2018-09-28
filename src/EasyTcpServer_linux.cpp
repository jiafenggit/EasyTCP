#ifdef __linux__
#include "EasyTcpServer_linux.h"
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

using namespace EasyTCP;
using namespace std::placeholders;

Server::Server()
    : m_opened(false)
{

}

IServerPtr Server::create()
{
    return IServerPtr(new Server());
}

Server::~Server()
{
    close();
}

bool Server::open(unsigned short port, unsigned int numPoll,
    unsigned int backlog)
{
    if (m_opened)
        return false;

    do
    {
        m_eventPolls.reset(new EventPolls());
        if(!m_eventPolls->run(numPoll, 1024))
            break;

        m_acceptor.reset(new Acceptor());
        m_acceptor->onAccepted = std::bind(&Server::addConnection, this, _1);
        if(!m_acceptor->accept(port, backlog))
            break;

        m_opened = true;
        return true;
    } while(0);


    m_eventPolls.reset();
    m_acceptor.reset();
    return false;
}

void Server::close()
{
    m_acceptor.reset();

    {
        std::lock_guard<std::recursive_mutex> lockGuard(m_lockConnections);
        std::vector<IConnection*> tmpConnections;
        for (auto it : m_connections)
        {
            tmpConnections.push_back(it.first);
        }

        for (auto it : tmpConnections)
        {
            it->disconnect();
        }
    }

    while (!m_connections.empty())
    {
        usleep(1);
    }

    m_eventPolls.reset();

    if (!m_connections.empty())
    {
        std::lock_guard<std::recursive_mutex> lockGuard(m_lockConnections);
        m_connections.clear();
    }

    m_opened = false;
}

void Server::addConnection(SOCKET sock)
{
    int flag = fcntl(sock, F_GETFL, 0);
    if (-1 == flag )
        return;
    fcntl(sock, F_SETFL, flag | O_NONBLOCK);

    IConnectionPtr con(new Connection(m_eventPolls->getNextEventPoll(), sock, true));
    con->updateEndPoint();
    con->onBufferSent = onBufferSent;
    con->onBufferReceived = onBufferReceived;
    con->onDisconnected = std::bind(&Server::removeConnection, this, _1);


    {
        std::lock_guard<std::recursive_mutex> lockGuard(m_lockConnections);
        m_connections.insert(std::make_pair(con.get(), con));
    }

    if (onConnected)
        onConnected(con.get());

}

void Server::removeConnection(IConnection *con)
{
    if (onDisconnected)
        onDisconnected(con);

    {
        std::lock_guard<std::recursive_mutex> lockGuard(m_lockConnections);
        m_connections.erase(con);
    }
}

#endif
