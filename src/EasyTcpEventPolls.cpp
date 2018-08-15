#ifdef __linux__
#include "EasyTcpEventPolls.h"

using namespace EasyTcp;

EventPolls::EventPolls()
    : m_isRunning(false), m_indexNext(0)
{

}

EventPolls::~EventPolls()
{

}

bool EventPolls::run(unsigned int numPoll, int maxEventsOfSinglePoll)
{
    if (m_isRunning)
        return false;

    for (unsigned int i = 0; i < numPoll; i++)
    {
        std::shared_ptr<EventPoll> worker(new EventPoll());
        if (!worker->init(maxEventsOfSinglePoll))
        {
            m_workers.clear();
            return false;
        }

        m_workers.push_back(worker);
    }

    m_isRunning = true;
    return true;
}

EventPoll* EventPolls::getNextEventPoll()
{
    return m_workers.at(++m_indexNext % m_workers.size()).get();
}

#endif
