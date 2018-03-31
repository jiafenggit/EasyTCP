#ifndef EASYTCPCLIENT_H
#define EASYTCPCLIENT_H

#include "EasyTcpConnection.h"
#include "EasyTcpWorker.h"
#include "EasyTcpContext.h"
#include <atomic>

namespace EasyTcp
{
    class Client;
    typedef std::shared_ptr<Client> SPTRClient;

    class Client : public Connection
    {
    public:
        static SPTRClient create();
        static SPTRClient create(std::shared_ptr<Worker> worker);

        ~Client();

        bool connect(const std::string& host, unsigned short port);

    private:
        Client();

    public:
            std::function<void (Connection*)> onConnected;
            std::function<void (Connection*, int)> onConnectFailed;

    private:
        std::shared_ptr<Worker> m_worker;
        std::atomic<bool> m_connecting;
    };
}

#endif
