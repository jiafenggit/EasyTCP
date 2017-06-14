#include "EasyTcpServer.h"
#include <time.h>
#include <unistd.h>

using namespace EasyTcp;
using namespace std::placeholders;

Server::Server()
    : m_opened(false),
      onConnected(nullptr),
      onDisconnected(nullptr),
      onBufferSent(nullptr),
      onBufferReceived(nullptr),
      onError(nullptr)
{

}

SPTRServer Server::create()
{
    return SPTRServer(new Server());
}

Server::~Server()
{
    close(-1);
}

bool Server::open(unsigned short port, unsigned int numPoll,
    unsigned int backlog, int maxEventsOfSinglePoll)
{
    if (m_opened)
        return false;

    m_eventPolls.reset(new EventPolls());
    if(!m_eventPolls->run(numPoll, maxEventsOfSinglePoll))
    {
        m_eventPolls.reset();
        return false;
    }

    m_acceptor.reset(new Acceptor());
    m_acceptor->onAccepted = std::bind(&Server::addConnection, this, _1);
    if(!m_acceptor->accept(port, backlog))
    {
        m_eventPolls.reset();
        m_acceptor.reset();
        return false;
    }

    m_opened = true;
    return true;
}

void Server::close(long timeout)
{
    clock_t t0 = clock();

    m_acceptor.reset();

    {
        std::lock_guard<std::recursive_mutex> lockGuard(m_lockConnections);
        for (auto &v : m_connections)
        {
            v.second->disconnect();
        }
    }

    while (timeout == -1 || clock() - t0 < timeout)
    {
        if (m_connections.empty())
        {
            break;
        }
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

void Server::addConnection(int sock)
{
    std::shared_ptr<Connection> sptrConnection(new Connection(m_eventPolls->getNextEventPoll(), sock, true));
    sptrConnection->updateEndPoint();
    sptrConnection->onBufferSent = onBufferSent;
    sptrConnection->onBufferReceived = onBufferReceived;
    sptrConnection->onError = onError;
    sptrConnection->onDisconnected = std::bind(&Server::removeConnection, this, _1);

    {
        std::lock_guard<std::recursive_mutex> lockGuard(m_lockConnections);
        m_connections.insert(std::make_pair(sptrConnection.get(), sptrConnection));
    }

    if (onConnected)
        onConnected(sptrConnection.get());

}

void Server::removeConnection(Connection *con)
{
    if (onDisconnected)
        onDisconnected(con);

    {
        std::lock_guard<std::recursive_mutex> lockGuard(m_lockConnections);
        m_connections.erase(con);
    }
}
