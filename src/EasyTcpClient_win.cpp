#if  defined(WIN32) || defined(WIN64)
#include "EasyTcpClient_win.h"
#include <mstcpip.h>
#include <mswsock.h>
#include <WinBase.h>
#include <functional>
#include <unistd.h>

using namespace EasyTCP;
using namespace std::placeholders;

Client::Client(WorkerPtr worker)
    :   m_worker(worker),
        m_connecting(false),
        m_iocp(static_cast<IOCP*>(worker.get())),
        m_detained(false)
{

}

IClientPtr Client::create()
{
    IClientPtr ret;
    IOCP* iocp;
    WorkerPtr worker(iocp = new IOCP());

    if (!iocp->init())
        return ret;

    return create(worker);
}

IClientPtr Client::create(WorkerPtr worker)
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
    {
        return IClientPtr();
    }

    IClientPtr ret;
    if (!dynamic_cast<IOCP*>(worker.get()))
        return ret;

    ret.reset(new Client(worker));
    return ret;
}

Client::~Client()
{
    disconnect();
    while(m_connected || m_countPost || m_detained)
        usleep(1);
    WSACleanup();
}

bool Client::connect(const std::string& host, unsigned short port)
{
    static LPFN_CONNECTEX ConnectEx = NULL;

    if (m_connected)
        return false;

    bool e = false;
    if (!m_connecting.compare_exchange_strong(e, true))
        return false;

    do
    {
        m_handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_handle == INVALID_SOCKET)
            break;

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.S_un.S_addr = INADDR_ANY;
        addr.sin_port = 0;
        if (bind(m_handle, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR
            || !m_iocp->attach(m_handle))
            break;

        if (!ConnectEx)
        {
            GUID guid = WSAID_CONNECTEX;
            DWORD numBytes;

            if(WSAIoctl(m_handle, SIO_GET_EXTENSION_FUNCTION_POINTER,
                &guid, sizeof(guid), &ConnectEx,
                sizeof(ConnectEx), &numBytes, NULL, NULL) == SOCKET_ERROR)
                    break;
        }

        Context *context = new Context(true);
        context->onDone = [this](Context*, size_t)
        {
            decreasePostCount();
            m_connected = true;
            m_connecting = false;

            setsockopt(m_handle, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
            updateEndPoint();

            if (onConnected)
                this->onConnected(this);
        };

        context->onError = [this](Context* , int err)
        {
            decreasePostCount();
            closesocket(m_handle);
            m_handle = INVALID_SOCKET;
            m_detained = true;
            m_connecting = 0;

            if (onConnectFailed)
                this->onConnectFailed(this, err);
            m_detained = false;
        };

        {
            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_addr.S_un.S_addr = inet_addr(host.c_str());
            addr.sin_port = htons(port);

            increasePostCount();
            if(!ConnectEx(m_handle, (sockaddr*)&addr, sizeof(addr), NULL, 0, NULL, context))
            {
                int err = WSAGetLastError();
                if (err != ERROR_IO_PENDING)
                {
                    decreasePostCount();
                    break;
                }
            }
        }

        return true;
    }
    while (0);

    closesocket(m_handle);
    m_handle = INVALID_SOCKET;
    m_connecting = false;
    return false;
}

bool Client::disconnect()
{
    m_detained = true;
    bool ret = Connection::disconnect();
    m_detained = false;

    return ret;
}
#endif



