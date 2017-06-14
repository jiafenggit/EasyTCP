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
        std::shared_ptr<EventPoll> sptrEventPoll(new EventPoll());
        if (!sptrEventPoll->init(maxEventsOfSinglePoll))
        {
            m_eventPolls.clear();
            return false;
        }

        m_eventPolls.push_back(sptrEventPoll);
    }

    m_isRunning = true;
    return true;
}

EventPoll* EventPolls::getNextEventPoll()
{
    m_indexNext++;
    if (m_indexNext >= m_eventPolls.size())
        m_indexNext = 0;

    return m_eventPolls.at(m_indexNext).get();
}
