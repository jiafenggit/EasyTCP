#include "EasyTcpEventPoll.h"
#include "EasyTcpContext.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#include <bits/types.h>
#include <pthread.h>

using namespace EasyTcp;

EventPoll::EventPoll()
    : m_handle(-1),
      m_pid(-1),
      m_terminated(false)
{

}

EventPoll::~EventPoll()
{
    m_terminated = true;
    close(m_handle);

    if (m_thread.joinable())
    {
        kill(m_pid, SIGUSR1);
        m_thread.join();
    }
}

bool EventPoll::init(int maxEvents)
{
    if (m_handle != -1)
        return false;

    m_handle = epoll_create(1);
    if (m_handle == -1)
    {
        return false;
    }

    m_thread = std::thread(std::bind(&EventPoll::execute, this, maxEvents));
    while (m_pid == -1);
    if (!m_pid)
        return false;

    return true;
}

bool EventPoll::add(int fd, epoll_event *event)
{
    return !epoll_ctl(m_handle, EPOLL_CTL_ADD, fd, event);
}

bool EventPoll::modify(int fd, epoll_event *event)
{
    return !epoll_ctl(m_handle, EPOLL_CTL_MOD, fd, event);
}

bool EventPoll::remove(int fd, epoll_event *event)
{
    return !epoll_ctl(m_handle, EPOLL_CTL_DEL, fd, event);
}

void EventPoll::execute(int maxEvents)
{
    int i, ret;
    epoll_event* events;
    Context::Context *pContext;

    struct sigaction act;
    act.sa_handler = [](int){};
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if(sigaction( SIGUSR1, &act, NULL))
    {
        m_pid = 0;
        return;
    }

    m_pid = syscall(SYS_gettid);

    if (maxEvents <= 0)
        maxEvents = 1;

    events = new epoll_event[maxEvents];
    while (!m_terminated)
    {
        ret = epoll_wait(m_handle, events, maxEvents, -1);

        for (i = 0; i < ret; i++)
        {
            pContext = (Context::Context *)events[i].data.ptr;
            pContext->update(events[i].events);
        }
    }

    delete[] events;
}
