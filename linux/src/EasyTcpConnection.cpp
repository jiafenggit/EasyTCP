#include "EasyTcpConnection.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

using namespace EasyTcp;
using namespace std::placeholders;

Connection::Connection(EventPoll *eventPoll)
    : Connection(eventPoll, -1, false)
{

}

Connection::Connection(EventPoll *eventPoll, int sock, bool connected)
    : m_eventPoll(eventPoll),
      m_handle(sock),
      m_context(std::bind(&Connection::handleEvents, this, _1)),
      m_connected(connected),
      m_registered(false),
      m_localPort(0),
      m_peerPort(0),
      m_userdata(NULL),
      onDisconnected(nullptr),
      onBufferSent(nullptr),
      onBufferReceived(nullptr),
      onError(nullptr)
{

}

Connection::~Connection()
{

}

int Connection::handle()
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

    {
        std::lock_guard<std::recursive_mutex> lockGuard(m_lock);

        if (!m_connected)
            return false;

        removeHandle();
        close(m_handle);
        m_connected = false;
        m_tasksSend.clear();
        m_tasksReceive.clear();

    }

    if (onDisconnected)
    {
        this->onDisconnected(this);
    }

    return true;
}

bool Connection::send(AutoBuffer buffer)
{
    if (!m_connected)
        return false;

    {
        std::lock_guard<std::recursive_mutex> lockGuard(m_lock);

        if (!m_connected)
            return false;

        m_tasksSend.push_back(SPTRTask(new Task(buffer)));
        registerHandle();
    }

    return true;
}

bool Connection::recv(AutoBuffer buffer)
{
    if (!m_connected)
        return false;

    {
        std::lock_guard<std::recursive_mutex> lockGuard(m_lock);

        if (!m_connected)
            return false;

        m_tasksReceive.push_back(SPTRTask(new Task(buffer)));
        registerHandle();
    }

    return true;
}

bool Connection::enableKeepalive(unsigned long nInterval, unsigned long nTime)
{
    int onoff = 1;
    int keepaliveinterval = nInterval;
    int keepalivetime = nTime;

    return !setsockopt(m_handle, SOL_SOCKET, SO_KEEPALIVE, (void*)&onoff, sizeof(onoff))
        && !setsockopt(m_handle, SOL_TCP, TCP_KEEPIDLE, (void*)&keepalivetime, sizeof(keepalivetime))
        && !setsockopt(m_handle, SOL_TCP, TCP_KEEPINTVL, (void*)&keepalivetime, sizeof(keepalivetime));
}

bool Connection::disableKeepalive()
{
    int onoff = 0;

    return !setsockopt(m_handle, SOL_SOCKET, SO_KEEPALIVE, (void*)&onoff, sizeof(onoff));
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
    socklen_t len = sizeof(sockaddr_in);
    sockaddr_in addrLocal, addrPeer;

    if(getpeername(m_handle, (sockaddr*)&addrPeer, &len) == -1)
        return false;

    m_peerIP = inet_ntoa(addrPeer.sin_addr);
    m_peerPort = ntohs(addrPeer.sin_port);

    if(getsockname(m_handle, (sockaddr*)&addrLocal, &len) == -1)
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

void Connection::handleEvents(uint32_t events)
{
    int ret;
    size_t offset, size;
    SPTRTask taskSend, taskRecv;
    std::string errstr;
    bool needDoDisconnect = false, needCallbackBufferSent = false,
        needCallbackBufferReceived = false, needCallbackError = false;

    if (!m_connected)
        return;

    do
    {
        std::lock_guard<std::recursive_mutex> lockGuard(m_lock);
        if (!m_connected)
            return;

        if (events & EPOLLIN)
        {
            if (!m_tasksReceive.empty())
            {
                taskRecv = m_tasksReceive.front();

                offset = taskRecv->progress();
                size = taskRecv->data().size() - offset;
                ret = ::recv(m_handle,
                    taskRecv->data().data() + offset, size, MSG_DONTWAIT);
                if (ret <= 0)
                {
                    needDoDisconnect = true;
                    errstr = strerror(errno);
                    needCallbackError = true;
                    break;
                }

                if (!taskRecv->increase(ret) && onError)
                {
                    errstr = "increase a recv task failed";
                    needCallbackError = true;
                    break;
                }

                if (taskRecv->finished())
                {
                    m_tasksReceive.pop_front();
                    needCallbackBufferReceived = true;
                }
            }
        }

        if (events & EPOLLOUT)
        {
            if (!m_tasksSend.empty())
            {
                taskSend = m_tasksSend.front();
                offset = taskSend->progress();
                size = taskSend->data().size() - offset;
                ret = ::send(m_handle,
                    taskSend->data().data() + offset, size, MSG_DONTWAIT);
                if (ret <= 0)
                {
                    needDoDisconnect = true;
                    errstr = strerror(errno);
                    needCallbackError = true;
                    break;
                }

                if (!taskSend->increase(ret) && onError)
                {
                    errstr = "increase a send task failed";
                    needCallbackError = true;
                    break;
                }

                if (taskSend->finished())
                {
                    m_tasksSend.pop_front();
                    needCallbackBufferSent = true;
                }
            }
        }

    } while(false);

    if (needCallbackBufferSent && onBufferSent)
        onBufferSent(this, taskSend->data());

    if (needCallbackBufferReceived && onBufferReceived)
        onBufferReceived(this, taskRecv->data());

    if (needCallbackError && onError)
        onError(this, errstr);

    if (needDoDisconnect)
    {
        disconnect();
        return;
    }
}

void Connection::registerHandle()
{
    if (!m_registered && m_eventPoll->add(m_handle, &m_context))
        m_registered = true;
}

void Connection::removeHandle()
{
    if(m_eventPoll->remove(m_handle, &m_context))
        m_registered = false;
}
