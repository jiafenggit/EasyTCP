#ifdef __linux__
#ifndef EASYTCPCLIENT_H
#define EASYTCPCLIENT_H

#include "EasyTcpIClient.h"
#include "EasyTcpConnection_linux.h"
#include "EasyTcpEventPoll.h"
#include "EasyTcpContext_linux.h"
#include <atomic>

namespace EasyTcp
{
    class Client : public IClient, public Connection
    {
    public:
        static IClientPtr create();
        static IClientPtr create(WorkerPtr worker);

        ~Client();

        bool connect(const std::string& host, unsigned short port);

    private:
        Client(WorkerPtr worker);
        void close(void* userdata);

    private:
        std::atomic<bool> m_connecting;
        WorkerPtr m_worker;
        EventPoll* m_eventPoll;
        bool m_detained;
    };
}

#endif
#endif
