#include "EasyTcpConnection.h"
#include <mstcpip.h>
#include <mswsock.h>
#include <WinBase.h>

using namespace EasyTcp;
using namespace std::placeholders;

Connection::Connection()
    : Connection(INVALID_SOCKET, false)
{

}

Connection::Connection(SOCKET sock, bool connected)
    : m_handle(sock),
      m_connected(connected),
      m_localPort(0),
      m_peerPort(0),
      m_userdata(NULL),
      m_disconnecting(false),
      m_countPost(0)
{

}

std::shared_ptr<Connection> Connection::share()
{
    return this->shared_from_this();
}

SOCKET Connection::handle()
{
    return m_handle;
}

bool Connection::connected()
{
    return m_connected;
}

bool Connection::disconnect()
{
    if (!m_connected)
        return false;

    do
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        if (m_disconnecting || !m_connected)
            break;

        m_disconnecting = true;
        closesocket(m_handle);
        m_handle = INVALID_SOCKET;
    }
    while(0);

    if (!m_countPost)
    {
        {
            std::lock_guard<std::recursive_mutex> lockGuard(m_lock);
            if (!m_connected)
                return false;

            m_connected = false;
            m_disconnecting = false;
        }

        if (onDisconnected)
            onDisconnected(this);
    }
    return true;
}

bool Connection::send(AutoBuffer buffer)
{
    if (!buffer.size())
        return false;

    return post(buffer, true,
        std::bind(&Connection::whenSendDone, this, _1, _2),
        std::bind(&Connection::whenError, this, _1, _2));
}

bool Connection::recv(AutoBuffer buffer)
{
    if (!buffer.size())
        return false;

    return post(buffer, false,
        std::bind(&Connection::whenRecvDone, this, _1, _2),
        std::bind(&Connection::whenError, this, _1, _2));
}

bool Connection::enableKeepalive(unsigned long nInterval, unsigned long nTime)
{
    tcp_keepalive inKeepAlive;
    tcp_keepalive outKeepAlive;
    unsigned long numBytes = 0;

    memset(&inKeepAlive, 0, sizeof(inKeepAlive));
    memset(&outKeepAlive, 0, sizeof(outKeepAlive));

    inKeepAlive.onoff = 1;
    inKeepAlive.keepaliveinterval = nInterval;
    inKeepAlive.keepalivetime = nTime;

    return !WSAIoctl(m_handle, SIO_KEEPALIVE_VALS, (LPVOID)&inKeepAlive, sizeof(tcp_keepalive),
            (LPVOID)&outKeepAlive, sizeof(tcp_keepalive), &numBytes, NULL, NULL);
}

bool Connection::disableKeepalive()
{
    tcp_keepalive inKeepAlive;
    tcp_keepalive outKeepAlive;
    unsigned long numBytes = 0;

    memset(&inKeepAlive, 0, sizeof(inKeepAlive));
    memset(&outKeepAlive, 0, sizeof(outKeepAlive));

    inKeepAlive.onoff = 0;

    return !WSAIoctl(m_handle, SIO_KEEPALIVE_VALS, (LPVOID)&inKeepAlive, sizeof(tcp_keepalive),
            (LPVOID)&outKeepAlive, sizeof(tcp_keepalive), &numBytes, NULL, NULL);
}

bool Connection::setSendBufferSize(unsigned long nSize)
{
    return !setsockopt(m_handle, SOL_SOCKET, SO_SNDBUF, (char*)&nSize, sizeof(unsigned long));
}

bool Connection::setReceiveBufferSize(unsigned long nSize)
{
    return !setsockopt(m_handle, SOL_SOCKET, SO_RCVBUF, (char*)&nSize, sizeof(unsigned long));
}

bool Connection::setLinger(u_short onoff, u_short linger)
{
    struct linger opt = {onoff, linger};
    return !setsockopt(m_handle, SOL_SOCKET, SO_LINGER, (char*)&opt, sizeof(opt));
}

const std::string& Connection::localIP()
{
    return m_localIP;
}

unsigned short Connection::localPort()
{
    return m_localPort;
}

const std::string& Connection::peerIP()
{
    return m_peerIP;
}

unsigned short Connection::peerPort()
{
    return m_peerPort;
}

bool Connection::updateEndPoint()
{
    int len = sizeof(sockaddr_in);
    sockaddr_in addrLocal, addrPeer;

    if(getpeername(m_handle, (sockaddr*)&addrPeer, &len) == SOCKET_ERROR)
        return false;

    m_peerIP = inet_ntoa(addrPeer.sin_addr);
    m_peerPort = ntohs(addrPeer.sin_port);

    if(getsockname(m_handle, (sockaddr*)&addrLocal, &len) == SOCKET_ERROR)
        return false;

    m_localIP = inet_ntoa(addrLocal.sin_addr);
    m_localPort = ntohs(addrLocal.sin_port);

    return true;
}

void Connection::bindUserdata(void *userdata)
{
    m_userdata = userdata;
}

void *Connection::userdata()
{
    return m_userdata;
}

bool Connection::post(Context *context, bool isSend)
{
    if (!m_connected || m_disconnecting)
        return false;

    context->increase();
    do
    {
        m_lock.lock();
        if (!m_connected || m_disconnecting)
        {
            m_lock.unlock();
            break;
        }

        increasePostCount();

        int ret;
        DWORD flags = 0;
        if (isSend)
            ret = WSASend(m_handle, context->WSABuf(), 1,  NULL, flags, context, NULL);
        else
            ret = WSARecv(m_handle, context->WSABuf(), 1,  NULL, &flags, context, NULL);
        m_lock.unlock();

        if (ret == SOCKET_ERROR)
        {
            int err = WSAGetLastError();
            if(err != WSA_IO_PENDING)
            {
                std::shared_ptr<Connection> self = share();     //确保this指针可用
                decreasePostCount();
                disconnect();
                break;
            }
        }

        return true;
    }while(0);

    context->decrease();
    return false;
}

bool Connection::post(AutoBuffer buffer, bool isSend,
    std::function<void(Context*, size_t)> doneCallback, std::function<void(Context*, int)> errorCallback)
{
    Context *context = new Context(buffer);
    context->onDone = doneCallback;
    context->onError = errorCallback;

    bool ret = post(context, isSend);
    context->decrease();
    return ret;
}

void Connection::whenDone(Context *context, size_t increase,
    std::function<void (Connection *, AutoBuffer)> callback)
{
    if (context->finished())
    {
        if (callback)
            callback(this, context->buffer());
    }

    do
    {
        if (!m_disconnecting)
        {
            if (increase)
            {
                if (!context->finished())
                {
                    if(!post(context, true))
                        break;
                }
            }
            else
                break;
        }
        else
        {
            if (m_countPost - 1 == 0)
                break;
        }

        if (m_countPost - 1 == 0)
        {
            std::shared_ptr<Connection> self = share();     //确保this指针可用
            decreasePostCount();
            if (m_disconnecting)
                disconnect();
            return;
        }

        decreasePostCount();
        return;
    }while (0);


    std::shared_ptr<Connection> self = share();     //确保this指针可用
    decreasePostCount();
    disconnect();
    return;
}

void Connection::whenSendDone(Context *context, size_t increase)
{
    whenDone(context, increase, onBufferSent);
}
void Connection::whenRecvDone(Context *context, size_t increase)
{
    whenDone(context, increase, onBufferReceived);
}

void Connection::whenError(Context *context, int err)
{
    std::shared_ptr<Connection> self = share();     //确保this指针可用
    decreasePostCount();
    disconnect();
}

int Connection::increasePostCount()
{
    return ++m_countPost;
}

int Connection::decreasePostCount()
{
    return --m_countPost;
}

