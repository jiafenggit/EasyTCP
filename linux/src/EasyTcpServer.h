#ifndef EASYTCPSERVER_H
#define EASYTCPSERVER_H

#include "EasyTcpConnection.h"
#include "EasyTcpAcceptor.h"
#include "EasyTcpEventPolls.h"
#include <map>
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

        bool open(unsigned short port, unsigned int numPoll = 2,
            unsigned int backlog = 15, int maxEventsOfSinglePoll = 10);
        void close(long timeout = -1);

    private:
        Server();

        void addConnection(int sock);
        void removeConnection(Connection* con);

    public:
        std::function<void (Connection*)> onConnected;
        std::function<void (Connection*)> onDisconnected;
        std::function<void (Connection*, AutoBuffer data)> onBufferSent;
        std::function<void (Connection*, AutoBuffer data)> onBufferReceived;
        std::function<void (Connection*, const std::string& errstr)> onError;

    private:
        std::shared_ptr<Acceptor> m_acceptor;
        std::shared_ptr<EventPolls> m_eventPolls;
        std::map<Connection*, std::shared_ptr<Connection>> m_connections;
        bool m_opened;

        std::recursive_mutex m_lockConnections;
    };
}

#endif
