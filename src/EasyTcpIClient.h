#ifndef EASYTCPICLIENT_H
#define EASYTCPICLIENT_H

#include <memory>
#include <functional>
#include "EasyTcpWorker.h"
#include "EasyTcpIConnection.h"

namespace EasyTCP
{
    class IClient;
    typedef std::shared_ptr<IClient> IClientPtr;

    class IClient : virtual public IConnection
    {
    public:
        IClient(){}
        virtual ~IClient(){}

        virtual bool connect(const std::string& host, unsigned short port) = 0;

    public:
        std::function<void (IConnection*)> onConnected;
        std::function<void (IConnection*, int)> onConnectFailed;
    };

}

#endif // EASYTCPICLIENT_H
