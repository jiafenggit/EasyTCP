#include "EasyTcpWorker.h"

#ifdef __linux__
#include "EasyTcpEventPoll.h"
#else
#include "EasyTcpIOCP.h"
#endif

EasyTcp::WorkerPtr EasyTcp::Worker::create(int arg)
{
    WorkerPtr ret;

#ifdef __linux__
    EventPoll *pl;
    ret.reset(pl = new EventPoll());
    if (!pl->init(arg))
        ret.reset();
#else
    IOCP *cp;
    ret.reset(cp = new IOCP());
    if (!cp->init())
        ret.reset();
#endif

    return ret;
}
