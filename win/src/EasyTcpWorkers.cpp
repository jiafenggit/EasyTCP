#include "EasyTcpWorkers.h"

using namespace EasyTcp;

Workers::Workers()
    : m_isRunning(false), m_indexNext(0)
{

}

Workers::~Workers()
{

}

bool Workers::run(unsigned int numWorker)
{
    if (m_isRunning)
        return false;

    for (unsigned int i = 0; i < numWorker; i++)
    {
        std::shared_ptr<Worker> sptrWorker(new Worker());
        if (!sptrWorker->init())
        {
            m_workers.clear();
            return false;
        }

        m_workers.push_back(sptrWorker);
    }

    m_isRunning = true;
    return true;
}

Worker* Workers::getNextWorker()
{
    m_indexNext++;
    if (m_indexNext >= m_workers.size())
        m_indexNext = 0;

    return m_workers.at(m_indexNext).get();
}
