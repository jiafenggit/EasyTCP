#include "EasyTcpServer.h"
#include <time.h>
#include <unistd.h>

using namespace EasyTcp;
using namespace std::placeholders;

Server::Server()
    : m_opened(false)
{

}

SPTRServer Server::create()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
    {
        return SPTRServer();
    }

    return SPTRServer(new Server());
}

Server::~Server()
{
    close();
    WSACleanup();
}

bool Server::open(unsigned short port, unsigned int nWorkerNum, unsigned int backlog)
{
    if (m_opened)
        return false;

    m_workers.reset(new Workers());
    if(!m_workers->run(nWorkerNum))
    {
        m_workers.reset();
        return false;
    }

    m_acceptor.reset(new Acceptor());
    m_acceptor->onAccepted = std::bind(&Server::addConnection, this, _1);
    if(!m_acceptor->accept(port, backlog))
    {
        m_workers.reset();
        m_acceptor.reset();
        return false;
    }

    m_opened = true;
    return true;
}

void Server::close()
{
    m_acceptor.reset();

    {
        std::lock_guard<std::recursive_mutex> lockGuard(m_lockConnections);
        auto connections = m_connections;
        for (auto it : connections)
            it.second->disconnect();
    }

    while (!m_connections.empty())
        usleep(1);

    m_workers.reset();


    m_opened = false;
}

void Server::addConnection(SOCKET sock)
{
    std::shared_ptr<Connection> sptrConnection(new Connection(sock, true));
    sptrConnection->updateEndPoint();
    sptrConnection->onBufferSent = onBufferSent;
    sptrConnection->onBufferReceived = onBufferReceived;
    sptrConnection->onDisconnected = std::bind(&Server::removeConnection, this, _1);

    if(m_workers->getNextWorker()->attach(sptrConnection.get()))
    {
        {
            std::lock_guard<std::recursive_mutex> lockGuard(m_lockConnections);
            m_connections.insert(std::make_pair(sptrConnection.get(), sptrConnection));
        }

        if (onConnected)
            onConnected(sptrConnection.get());
    }
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
