#ifdef __linux__
#ifndef EASYTCPSERVER_H
#define EASYTCPSERVER_H

#include "EasyTcpConnection_linux.h"
#include "EasyTcpAcceptor.h"
#include "EasyTcpEventPolls.h"
#include "EasyTcpIServer.h"
#include <unordered_map>
#include <mutex>

namespace EasyTCP
{
    class Server : public IServer
    {
    public:
        static IServerPtr create();
        ~Server();

        bool open(unsigned short port, unsigned int numPoll = 2,
            unsigned int backlog = 15);
        void close();

    private:
        Server();

        void addConnection(SOCKET sock);
        void removeConnection(IConnection* con);

    private:
        std::shared_ptr<Acceptor> m_acceptor;
        std::shared_ptr<EventPolls> m_eventPolls;
        std::unordered_map<IConnection*, IConnectionPtr> m_connections;
        bool m_opened;

        std::recursive_mutex m_lockConnections;
    };
}

#endif
#endif
