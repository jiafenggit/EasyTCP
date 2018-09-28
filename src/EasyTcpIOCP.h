#if  defined(WIN32) || defined(WIN64)
#ifndef EASYTCPIOCP_H
#define EASYTCPIOCP_H

#include <thread>
#include "EasyTcpDef.h"
#include "EasyTcpWorker.h"

namespace EasyTCP
{
    class IOCP : public Worker
    {
    public:
        IOCP();
        ~IOCP();

        bool init();
        bool attach(SOCKET sock);

    private:
        void execute();

    private:
        void* m_ioCompletionPort;
        bool m_terminated;
        std::thread m_thread;
    };
}

#endif
#endif
