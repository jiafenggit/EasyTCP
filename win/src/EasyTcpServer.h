#ifndef EASYTCPSERVER_H
#define EASYTCPSERVER_H

#include "EasyTcpConnection.h"
#include "EasyTcpAcceptor.h"
#include "EasyTcpWorkers.h"
#include <unordered_map>
#include <mutex>

namespace EasyTcp
{
    class Server;
    typedef std::shared_ptr<Server> SPTRServer;

    class Server
    {
    public:
        static SPTRServer create();
        ~Server();

        bool open(unsigned short port, unsigned int numWorker = 2,
            unsigned int backlog = 15);
        void close();

    private:
        Server();

        void addConnection(SOCKET sock);
        void removeConnection(Connection* con);

    public:
        std::function<void (Connection*)> onConnected;
        std::function<void (Connection*)> onDisconnected;
        std::function<void (Connection*, AutoBuffer data)> onBufferSent;
        std::function<void (Connection*, AutoBuffer data)> onBufferReceived;

    private:
        std::shared_ptr<Acceptor> m_acceptor;
        std::shared_ptr<Workers> m_workers;
        std::unordered_map<Connection*, std::shared_ptr<Connection>> m_connections;
        bool m_opened;

        std::recursive_mutex m_lockConnections;
    };
}

#endif
