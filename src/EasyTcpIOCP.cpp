#if  defined(WIN32) || defined(WIN64)
#include "EasyTcpContext_win.h"
#include "EasyTcpIOCP.h"

using namespace EasyTCP;

IOCP::IOCP()
    : m_ioCompletionPort(NULL),
      m_terminated(false)
{

}

IOCP::~IOCP()
{
    m_terminated = true;
    CloseHandle(m_ioCompletionPort);
    if (m_thread.joinable())
        m_thread.join();
}

bool IOCP::init()
{
    if (m_ioCompletionPort)
        return false;

    m_ioCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (!m_ioCompletionPort)
    {
        return false;
    }

    m_thread = std::thread(std::bind(&IOCP::execute, this));

    return true;
}

bool IOCP::attach(SOCKET sock)
{
    if(!CreateIoCompletionPort((HANDLE)sock,
        m_ioCompletionPort, 0, 0))
    {
        return false;
    }

    return true;
}

void IOCP::execute()
{
    int err;
    int ret;
    DWORD numBytes;
    ULONG_PTR key;
    Context *pContext;

    while (!m_terminated)
    {
        ret = GetQueuedCompletionStatus(m_ioCompletionPort, &numBytes,
                        &key, (LPOVERLAPPED*)&pContext, INFINITE);
        if (ret)
        {
            if (!pContext)
                break;

            pContext->increaseProgress(numBytes);
            pContext->decrease();
        }
        else
        {
            err = WSAGetLastError();

            if (pContext)
            {
                pContext->error(err);
                pContext->decrease();
            }
            else
                break;
        }
    }
}
#endif
