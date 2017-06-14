#ifndef EASYTCPCLIENT_H
#define EASYTCPCLIENT_H

#include "EasyTcpConnection.h"
#include "EasyTcpWorker.h"
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

        void whenDoConnectSucceed(Context::IAction* pAction);
        void whenDoConnectFailed(Context::IAction* pAction, int err);

    public:
            std::function<void (Connection*)> onConnected;
            std::function<void (Connection*, int)> onConnectFailed;

    private:
        std::shared_ptr<Worker> m_worker;
        long m_isDoingConnect;
        Context::SPTRIAction m_sptrConnectAction;
    };
}

#endif
