#ifndef EASYTCPWORKER_H
#define EASYTCPWORKER_H

#include <thread>
#include "EasyTcpConnection.h"
#include <WinBase.h>

namespace EasyTcp
{
    class Worker
    {
    public:
        Worker();
        ~Worker();

        bool init();
        bool attach(Connection *con);

    private:
        void execute();

    private:
        HANDLE m_ioCompletionPort;
        bool m_terminated;
        std::thread m_thread;
    };
}

#endif
