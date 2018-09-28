#if  defined(WIN32) || defined(WIN64)
#include "EasyTcpServer_win.h"
#include <time.h>
#include <unistd.h>

using namespace EasyTCP;
using namespace std::placeholders;

Server::Server()
    : m_opened(false)
{

}

IServerPtr Server::create()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
    {
        return IServerPtr();
    }

    return IServerPtr(new Server());
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

    do
    {
        m_workers.reset(new IOCPs());
        if(!m_workers->run(nWorkerNum))
            break;

        m_acceptor.reset(new Acceptor());
        m_acceptor->onAccepted = std::bind(&Server::addConnection, this, _1);
        if(!m_acceptor->accept(port, backlog))
            break;

        m_opened = true;
        return true;
    } while(0);

    m_workers.reset();
    m_acceptor.reset();
    return false;
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
    IConnectionPtr con(new Connection(sock, true));
    con->updateEndPoint();
    con->onBufferSent = onBufferSent;
    con->onBufferReceived = onBufferReceived;
    con->onDisconnected = std::bind(&Server::removeConnection, this, _1);

    if(m_workers->getNextWorker()->attach(sock))
    {
        {
            std::lock_guard<std::recursive_mutex> lockGuard(m_lockConnections);
            m_connections.insert(std::make_pair(con.get(), con));
        }

        if (onConnected)
            onConnected(con.get());
    }
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
