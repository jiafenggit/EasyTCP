#ifndef EASYTCPWORKERS_H
#define EASYTCPWORKERS_H

#include "EasyTcpWorker.h"
#include <vector>
#include <set>

namespace EasyTcp
{
    class Workers
    {
    public:
        Workers();
        ~Workers();

        bool run(unsigned int nWorkerNum);
        Worker* getNextWorker();

    private:
        std::vector<std::shared_ptr<Worker>> m_workers;
        bool m_isRunning;
        unsigned int m_indexNext;
    };
}

#endif
