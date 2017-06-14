#ifndef EASYTCPCLIENT_H
#define EASYTCPCLIENT_H

#include "EasyTcpConnection.h"
#include "EasyTcpEventPoll.h"
#include "EasyTcpContext.h"

namespace EasyTcp
{
    class Client;
    typedef std::shared_ptr<Client> SPTRClient;

    class Client : public Connection
    {
    public:
        static SPTRClient create();
        ~Client();

        bool connect(const std::string& host, unsigned short port);

    private:
        Client();

    public:
            std::function<void (Connection*)> onConnected;
            std::function<void (Connection*, int)> onConnectFailed;
    };
}

#endif
