#ifdef __linux__
#ifndef EASYTCPEVENTPOLLS_H
#define EASYTCPEVENTPOLLS_H

#include "EasyTcpEventPoll.h"
#include <vector>
#include <set>

namespace EasyTcp
{
    class EventPolls
    {
    public:
        EventPolls();
        ~EventPolls();

        bool run(unsigned int numPoll, int maxEventsOfSinglePoll);
        EventPoll* getNextEventPoll();

    private:
        std::vector<std::shared_ptr<EventPoll>> m_workers;
        bool m_isRunning;
        unsigned int m_indexNext;
    };
}

#endif
#endif
