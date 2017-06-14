#ifndef EASYTCPEVENTPOLL_H
#define EASYTCPEVENTPOLL_H

#include <thread>
#include <sys/epoll.h>

namespace EasyTcp
{
    class EventPoll
    {
    public:
        EventPoll();
        ~EventPoll();

        bool init(int maxEvents);
        bool add(int fd, struct epoll_event *event);
        bool modify(int fd, struct epoll_event *event);
        bool remove(int fd, struct epoll_event *event);

    private:
        void execute(int maxEvents);

    private:
        int m_handle;
        bool m_terminated;
        std::thread m_thread;
        __pid_t m_pid;
    };

    typedef std::shared_ptr<EventPoll> SPTREventPoll;
}

#endif
