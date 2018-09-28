#if  defined(WIN32) || defined(WIN64)
#include "EasyTcpIOCPs.h"
#include "EasyTcpIOCP.h"

using namespace EasyTCP;

IOCPs::IOCPs()
    : m_isRunning(false), m_indexNext(0)
{

}

IOCPs::~IOCPs()
{

}

bool IOCPs::run(unsigned int numWorker)
{
    if (m_isRunning)
        return false;

    for (unsigned int i = 0; i < numWorker; i++)
    {
        std::shared_ptr<IOCP> worker(new IOCP());
        if (!worker->init())
        {
            m_workers.clear();
            return false;
        }

        m_workers.push_back(worker);
    }

    m_isRunning = true;
    return true;
}

IOCP* IOCPs::getNextWorker()
{
    return m_workers.at(++m_indexNext % m_workers.size()).get();
}
#endif
