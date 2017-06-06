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
      m_didDisconnect(false),
      m_localPort(0),
      m_peerPort(0),
      m_userdata(NULL),
      onDisconnected(nullptr),
      onBufferSent(nullptr),
      onBufferReceived(nullptr)
{

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
    bool needCallback = false;

    if (!m_connected || m_didDisconnect)
        return false;

    {
        std::lock_guard<std::recursive_mutex> lockGuard(m_lock);

        if (!m_connected || m_didDisconnect)
            return false;

        shutdown(m_handle, SD_BOTH);
        m_didDisconnect = true;

        if (m_posts.empty())
        {
            closesocket(m_handle);
            m_connected = false;    
            needCallback = true;
        }
    }

    if (needCallback && onDisconnected)
    {
        this->onDisconnected(this);
    }

    return true;
}

bool Connection::send(AutoBuffer buffer)
{
    if (!m_connected)
        return false;

    Context::SPTRIAction sptrAction(new Context::SendAction(m_handle,
        buffer, std::bind(&whenSent, this, _1),
        std::bind(&whenSendOrReceiveFailed, this, _1, _2)));

    if(!recordToPosts(sptrAction))
    {
        return false;
    }

    int err;
    if (!sptrAction->invoke(err))
    {
        removeFromPosts(sptrAction.get());
        return false;
    }

    return true;
}

bool Connection::recv(AutoBuffer buffer)
{
    if (!m_connected)
        return false;

   Context::SPTRIAction sptrAction(new Context::ReceiveAction(m_handle,
        buffer, std::bind(&whenReceived, this, _1),
        std::bind(&whenSendOrReceiveFailed, this, _1, _2)));

   if(!recordToPosts(sptrAction))
   {
       return false;
   }

   int err;
   if (!sptrAction->invoke(err))
   {
       removeFromPosts(sptrAction.get());
       return false;
   }

    return true;
}

bool Connection::enableKeepalive(unsigned long nInterval, unsigned long nTime)
{
    tcp_keepalive inKeepAlive = {0};
    tcp_keepalive outKeepAlive = {0};
    unsigned long numBytes = 0;

    inKeepAlive.onoff = 1;
    inKeepAlive.keepaliveinterval = nInterval;
    inKeepAlive.keepalivetime = nTime;

    return !WSAIoctl(m_handle, SIO_KEEPALIVE_VALS, (LPVOID)&inKeepAlive, sizeof(tcp_keepalive),
            (LPVOID)&outKeepAlive, sizeof(tcp_keepalive), &numBytes, NULL, NULL);
}

bool Connection::disableKeepalive()
{
    tcp_keepalive inKeepAlive = {0};
    tcp_keepalive outKeepAlive = {0};
    unsigned long numBytes = 0;

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

void Connection::whenSent(Context::IAction* pAction)
{
    Context::TransmitAction *pTransmitAction = static_cast<Context::TransmitAction *>(pAction);
    EasyTcp::AutoBuffer data = pTransmitAction->data();

    bool ret = removeFromPostsAndSafelyRelease(pAction);

    if (onBufferSent)
        this->onBufferSent(this, data);

    if (ret && onDisconnected)
        onDisconnected(this);

}

void Connection::whenReceived(Context::IAction* pAction)
{
    Context::TransmitAction *pTransmitAction = static_cast<Context::TransmitAction *>(pAction);
    EasyTcp::AutoBuffer data = pTransmitAction->data();

    bool ret = removeFromPostsAndSafelyRelease(pAction);

    if (onBufferReceived)
        this->onBufferReceived(this, data);

    if (ret && onDisconnected)
        onDisconnected(this);
}

void Connection::whenSendOrReceiveFailed(Context::IAction* pAction, int err)
{
    disconnect();

    if(removeFromPostsAndSafelyRelease(pAction) && onDisconnected)
        onDisconnected(this);
}

static int dbgCount = 0;

bool Connection::recordToPosts(Context::SPTRIAction sptrAction)
{
    if (m_didDisconnect)
        return false;

    std::lock_guard<std::recursive_mutex> lockGuard(m_lock);
    if (m_didDisconnect)
        return false;

    m_posts.insert(sptrAction);
    return true;
}

bool Connection::removeFromPostsAndSafelyRelease(Context::IAction *pAction)
{
    std::lock_guard<std::recursive_mutex> lockGuard(m_lock);

    size_t t = removeFromPosts(pAction);
    if (m_connected && m_didDisconnect && !t)
    {
        closesocket(m_handle);
        m_connected = false;
        return true;
    }
    return false;
}

size_t Connection::removeFromPosts(Context::IAction* pAction)
{
    std::lock_guard<std::recursive_mutex> lockGuard(m_lock);
    m_posts.erase(pAction->shared_from_this());

    return m_posts.size();
}
