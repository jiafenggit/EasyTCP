#ifndef EASYTCPACCEPTOR_H
#define EASYTCPACCEPTOR_H

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
        std::function<void (int sock)> onAccepted;

    private:
        int m_socket;
        bool m_terminated;
        std::thread m_thread;
    };
}

#endif
