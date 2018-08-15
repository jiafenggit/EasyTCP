#if  defined(WIN32) || defined(WIN64)
#ifndef EASYTCPWORKERS_H
#define EASYTCPWORKERS_H

#include "EasyTcpIOCP.h"
#include <vector>
#include <set>

namespace EasyTcp
{
    class IOCPs
    {
    public:
        IOCPs();
        ~IOCPs();

        bool run(unsigned int nWorkerNum);
        IOCP* getNextWorker();

    private:
        std::vector<std::shared_ptr<IOCP>> m_workers;
        bool m_isRunning;
        unsigned int m_indexNext;
    };
}

#endif
#endif
