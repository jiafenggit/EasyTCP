#if  defined(WIN32) || defined(WIN64)
#ifndef EASYTCPCLIENT_H
#define EASYTCPCLIENT_H

#include "EasyTcpConnection_win.h"
#include "EasyTcpIOCP.h"
#include "EasyTcpContext_win.h"
#include "EasyTcpIClient.h"
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
        bool disconnect();

    private:
        Client(WorkerPtr worker);

    private:
        WorkerPtr m_worker;
        IOCP* m_iocp;
        std::atomic<bool> m_connecting;
        bool m_detained;
    };
}

#endif
#endif

