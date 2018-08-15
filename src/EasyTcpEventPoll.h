#ifdef __linux__
#ifndef EASYTCPEVENTPOLL_H
#define EASYTCPEVENTPOLL_H

#include <thread>
#include <sys/epoll.h>
#include <functional>
#include <mutex>
#include <vector>
#include "EasyTcpDef.h"
#include "EasyTcpWorker.h"
#include "EasyTcpContext_linux.h"

namespace EasyTcp
{
    class EventPoll : public Worker
    {
        struct Task
        {
            std::function<void(void*)> callback;
            void* userdata;
        };
    public:
        EventPoll();
        ~EventPoll();

        bool init(int maxEvents);
        bool add(int fd, Context::Context *context);
        bool modify(int fd, Context::Context *context);
        bool remove(int fd, Context::Context *context);

        void asynchronous(std::function<void(void*)> callback, void* userdata);
    private:
        void execute(int maxEvents);

    private:
        int m_handle;
        bool m_terminated;
        std::thread m_thread;
        __pid_t m_pid;

        std::mutex m_mutex;
        std::shared_ptr<std::vector<Task>> m_tasks;
    };
}
#endif
#endif
