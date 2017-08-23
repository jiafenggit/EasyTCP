#include "EasyTcpContext.h"
#include <ws2tcpip.h>
#include <mswsock.h>

using namespace EasyTcp::Context;

Context::Context(IAction* action)
    : m_action(action)
{
    this->hEvent = 0;
    this->Internal = 0;
    this->InternalHigh = 0;
    this->Offset = 0;
    this->OffsetHigh = 0;
    this->Pointer = 0;
}

Context::~Context()
{

}

IAction* Context::action()
{
    return m_action;
}

static long sId = 0;

IAction::IAction(const SOCKET sock)
    : m_context(new Context(this)), m_socket(sock)
{

}

IAction::~IAction()
{

}

ConnectAction::ConnectAction(const SOCKET sock, const std::string &host,
    unsigned short port, CALLBACK1 connectedCallback,
     CALLBACK2 connectFailedCallback)
    : IAction(sock), m_host(host), m_port(port),
    m_connectedCallback(connectedCallback),
    m_connectFailedCallback(connectFailedCallback)
{

}

bool ConnectAction::invoke(int& err)
{
    sockaddr_in addr;
    static LPFN_CONNECTEX ConnectEx = NULL;
    GUID guid = WSAID_CONNECTEX;
    DWORD numBytes;

    if (!ConnectEx)
    {
        //获取扩展函数指针
        if(WSAIoctl(m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
            &guid, sizeof(guid), &ConnectEx,
            sizeof(ConnectEx), &numBytes, NULL, NULL) == SOCKET_ERROR)
        {
            err = WSAGetLastError();
            return false;
        }
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.S_un.S_addr = inet_addr(m_host.c_str());
    addr.sin_port = htons(m_port);
    if(!ConnectEx(m_socket, (sockaddr*)&addr, sizeof(addr), NULL, 0, NULL, m_context.get()))
    {
        err = WSAGetLastError();
        if (err != ERROR_IO_PENDING)
            return false;
    }

    return true;
}

void ConnectAction::update(unsigned int increase)
{
    if (m_connectedCallback)
        this->m_connectedCallback(this);
}

void ConnectAction::error(int err)
{
    if (m_connectFailedCallback)
        this->m_connectFailedCallback(this, err);
}

DisconnectAction::DisconnectAction(const SOCKET sock,
    CALLBACK1 disconnectedCallback)
    : IAction(sock), m_disconnectedCallback(disconnectedCallback)
{

}

bool DisconnectAction::invoke(int& err)
{
    static LPFN_DISCONNECTEX DisconnectEx = NULL;
    DWORD numBytes;
    GUID guid = WSAID_DISCONNECTEX;

    if (!DisconnectEx)
    {
        //获取扩展函数指针
        if(WSAIoctl(m_socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
            &guid, sizeof(guid), &DisconnectEx,
            sizeof(DisconnectEx), &numBytes, NULL, NULL) == SOCKET_ERROR)
        {
            err = WSAGetLastError();
            return false;
        }
    }

    if(!DisconnectEx(m_socket, m_context.get(), 0, 0))
    {
        err = WSAGetLastError();
        if (err != ERROR_IO_PENDING)
            return false;
    }

    return true;
}

void DisconnectAction::update(unsigned int increase)
{
    if (m_disconnectedCallback)
        this->m_disconnectedCallback(this);
}

void DisconnectAction::error(int err)
{

}

TransmitAction::TransmitAction(const SOCKET sock, EasyTcp::AutoBuffer data,
        CALLBACK1 finishedCallback, CALLBACK2 badCallback)
    : IAction(sock), m_data(data), m_finishedCallback(finishedCallback),
    m_badCallback(badCallback), m_progress(0)
{
    m_wsaBuffer.buf = m_data.data();
    m_wsaBuffer.len = m_data.size();
}

bool TransmitAction::invoke(int& err)
{
    return transmit(err);
}

void TransmitAction::update(unsigned int increase)
{
    if (!increase || m_progress + increase > m_data.size())
    {

        if (m_badCallback)
            this->m_badCallback(this, 0);
        return ;
    }

    m_progress += increase;

    m_wsaBuffer.buf = m_data.data() + m_progress;
    m_wsaBuffer.len = m_data.size() - m_progress;

    if (m_progress == m_data.size())
    {
        if (m_finishedCallback)
        {
            try
            {
                this->m_finishedCallback(this);
            }
            catch(...){}
        }

        return ;
    }

    int err;
    if(!transmit(err) && m_badCallback)
    {
        this->m_badCallback(this, err);
    }
}

void TransmitAction::error(int err)
{
    if(m_badCallback)
    {
        this->m_badCallback(this, err);
    }
}

EasyTcp::AutoBuffer TransmitAction::data()
{
    return m_data;
}

SendAction::SendAction(const SOCKET sock, EasyTcp::AutoBuffer data,
    CALLBACK1 finishedCallback, CALLBACK2 badCallback)
    : TransmitAction(sock, data, finishedCallback, badCallback)
{

}

bool SendAction::transmit(int& err)
{
    int ret = WSASend(m_socket, &m_wsaBuffer, 1,  NULL, 0, m_context.get(), NULL);

    if (ret == SOCKET_ERROR)
    {
        err = WSAGetLastError();
        if(err != WSA_IO_PENDING)
        {
            return false;
        }
    }

    return true;
}

ReceiveAction::ReceiveAction(const SOCKET sock, EasyTcp::AutoBuffer data,
    CALLBACK1 finishedCallback, CALLBACK2 badCallback)
    : TransmitAction(sock, data, finishedCallback, badCallback), m_flags(0)
{

}

bool ReceiveAction::transmit(int& err)
{
    int ret = WSARecv(m_socket, &m_wsaBuffer, 1,  NULL, &m_flags, m_context.get(), NULL);

    if (ret == SOCKET_ERROR)
    {
        err = WSAGetLastError();
        if(err != WSA_IO_PENDING)
        {
            return false;
        }
    }

    return true;
}
