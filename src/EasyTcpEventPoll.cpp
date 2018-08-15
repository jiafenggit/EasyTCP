#ifdef __linux__
#include "EasyTcpEventPoll.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#include <bits/types.h>
#include <pthread.h>
#include <string.h>

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

bool EventPoll::add(int fd, Context::Context *context)
{
    return !epoll_ctl(m_handle, EPOLL_CTL_ADD, fd, context);
}

bool EventPoll::modify(int fd, Context::Context *context)
{
    return !epoll_ctl(m_handle, EPOLL_CTL_MOD, fd, context);
}

bool EventPoll::remove(int fd, Context::Context *context)
{
    return !epoll_ctl(m_handle, EPOLL_CTL_DEL, fd, context);
}

void EventPoll::asynchronous(std::function<void (void *)> callback, void *userdata)
{
    if (!callback)
        return;

    m_mutex.lock();
    if (!m_tasks.get())
    {
        m_tasks.reset(new std::vector<Task>());
        m_tasks->reserve(1024);
    }
    m_tasks->push_back({callback, userdata});
    m_mutex.unlock();

    if (m_thread.joinable())
        kill(m_pid, SIGUSR1);
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
        std::shared_ptr<std::vector<Task>> tasks;

        m_mutex.lock();
        if (m_tasks.get())
        {
            tasks = m_tasks;
            m_tasks.reset();
        }
        m_mutex.unlock();

        if (tasks.get())
        {
            for (Task& it : *tasks)
                it.callback(it.userdata);
        }

        ret = epoll_wait(m_handle, events, maxEvents, -1);
        if (ret == -1)
        {
            switch(errno)
            {
            case EINTR:
            case EAGAIN:
                continue;
            default:
                break;
            }
        }
        for (i = 0; i < ret; i++)
        {
            pContext = (Context::Context *)events[i].data.ptr;
            pContext->update(events[i].events);
        }
    }

    delete[] events;
}
#endif
