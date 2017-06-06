#include "EasyTcpWorker.h"
#include "EasyTcpContext.h"

using namespace EasyTcp;

Worker::Worker()
    : m_ioCompletionPort(NULL), m_terminated(false)
{

}

Worker::~Worker()
{
    m_terminated = true;
    CloseHandle(m_ioCompletionPort);
    if (m_thread.joinable())
        m_thread.join();
}

bool Worker::init()
{
    m_ioCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (!m_ioCompletionPort)
    {
        return false;
    }

    m_thread = std::thread(std::bind(&Worker::execute, this));

    return true;
}

bool Worker::attach(Connection *con)
{
    if(!CreateIoCompletionPort((HANDLE)con->handle(),
        m_ioCompletionPort, 0, 0))
    {
        return false;
    }

    return true;
}

void Worker::execute()
{
    int errcode;
    int ret;
    DWORD numBytes;
    ULONG_PTR key;
    Context::Context *pContext;

    while (!m_terminated)
    {
        ret = GetQueuedCompletionStatus(m_ioCompletionPort, &numBytes,
                        &key, (LPOVERLAPPED*)&pContext, INFINITE);
        if (ret)
        {
            if (!pContext)
                break;

            pContext->action()->update(numBytes);
        }
        else
        {
            errcode = WSAGetLastError();

            if (pContext)
            {
                pContext->action()->error(errcode);
            }
            else
                break;
        }
    }
}
