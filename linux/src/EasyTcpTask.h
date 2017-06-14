#ifndef EASYTCPTASK_H
#define EASYTCPTASK_H

#include "EasyTcpAutoBuffer.h"

namespace EasyTcp
{
    class Task
    {
    public:
        Task(AutoBuffer data);
        ~Task();

        AutoBuffer data();
        bool increase(size_t progress);
        size_t progress();
        bool finished();

    private:
        AutoBuffer m_data;
        size_t m_progress;
    };

    typedef std::shared_ptr<Task> SPTRTask;
}

#endif
