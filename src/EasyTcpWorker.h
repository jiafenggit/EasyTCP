#ifndef EASYTCPWORKER_H
#define EASYTCPWORKER_H

#include <memory>

namespace EasyTcp
{
    class Worker;
    typedef std::shared_ptr<Worker> WorkerPtr;

    class Worker
    {
    public:
        Worker(){}
        virtual ~Worker(){}

        static WorkerPtr create(int arg = 0);
    };
}

#endif // EASYTCPWORKER_H
