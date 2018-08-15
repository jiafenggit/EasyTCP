#ifndef EASYTCPACCEPTOR_H
#define EASYTCPACCEPTOR_H

#include "EasyTcpDef.h"
#include <functional>
#include <thread>

namespace EasyTcp
{
    class Acceptor
    {
    public:
        Acceptor();
        ~Acceptor();

        bool accept(unsigned short port, int backlog = 15);

    private:
        void execute();

    public:
        std::function<void (SOCKET sock)> onAccepted;

    private:
        SOCKET m_socket;
        bool m_terminated;
        std::thread m_thread;
    };
}

#endif
