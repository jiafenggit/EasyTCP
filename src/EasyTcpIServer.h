#ifndef EASYTCPISERVER_H
#define EASYTCPISERVER_H

#include <memory>
#include <functional>
#include "EasyTcpIConnection.h"

namespace EasyTCP
{
    class IServer;
    typedef std::shared_ptr<IServer> IServerPtr;

    class IServer
    {
    public:
        IServer(){}
        virtual ~IServer(){}

        virtual bool open(unsigned short port, unsigned int numWorker = 2,
                    unsigned int backlog = 15) = 0;
        virtual void close() = 0;

    public:
        std::function<void (IConnection*)> onConnected;
        std::function<void (IConnection*)> onDisconnected;
        std::function<void (IConnection*, AutoBuffer data)> onBufferSent;
        std::function<void (IConnection*, AutoBuffer data)> onBufferReceived;
    };

}


#endif // EASYTCPISERVER_H
