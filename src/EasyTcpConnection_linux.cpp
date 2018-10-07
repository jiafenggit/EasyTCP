#ifdef __linux__
#include "EasyTcpConnection_linux.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

using namespace EasyTCP;
using namespace std::placeholders;

Connection::Connection(EventPoll *eventPoll)
    : Connection(eventPoll, INVALID_SOCKET, false)
{

}

Connection::Connection(EventPoll *eventPoll, int sock, bool connected)
    : m_eventPoll(eventPoll),
      m_handle(sock),
      m_context(std::bind(&Connection::handleEvents, this, _1)),
      m_connected(connected),
      m_localPort(0),
      m_peerPort(0),
      m_userdata(NULL),
      m_disconnecting(false)
{
    m_context.events = EPOLLIN;
    m_eventPoll->add(m_handle, &m_context);
}

Connection::~Connection()
{
    disconnect();
    while (m_connected || m_disconnecting)
        usleep(1);
}

IConnectionPtr Connection::share()
{
    return std::dynamic_pointer_cast<IConnection>(this->shared_from_this());
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
    if (!m_connected || m_disconnecting)
        return false;

    std::lock_guard<std::recursive_mutex> lockGuard(m_lock);
    if (!m_connected || m_disconnecting)
        return false;

    m_disconnecting = true;
    m_eventPoll->asynchronous(std::bind(&Connection::_close, this, _1), NULL);

    return true;
}

bool Connection::send(AutoBuffer buffer, bool completely)
{
    if (!buffer.capacity() || buffer.capacity() == buffer.size())
        return false;

    if (!m_connected)
        return false;

    {
        std::lock_guard<std::recursive_mutex> lockGuard(m_lock);

        if (!m_connected)
            return false;

        bool isEmpty = m_tasksSend.empty();
        m_tasksSend.push_back(SPTRTask(new Task(buffer, completely)));
        if (isEmpty)
        {
            m_context.events = EPOLLIN | EPOLLOUT;
            m_eventPoll->modify(m_handle, &m_context);
        }
    }

    return true;
}

bool Connection::recv(AutoBuffer buffer, bool completely)
{
    if (!buffer.capacity() || buffer.capacity() == buffer.size())
        return false;

    if (!m_connected || m_taskReceive.get())
        return false;

    {
        std::lock_guard<std::recursive_mutex> lockGuard(m_lock);

        if (!m_connected || m_taskReceive.get())
            return false;

        m_taskReceive.reset(new Task(buffer, completely));
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

bool Connection::setLinger(unsigned short onoff, unsigned short linger)
{
    struct linger opt = {onoff, linger};
    return !setsockopt(m_handle, SOL_SOCKET, SO_LINGER, (char*)&opt, sizeof(opt));
}

const std::string& Connection::localIP() const
{
    return m_localIP;
}

unsigned short Connection::localPort() const
{
    return m_localPort;
}

const std::string& Connection::peerIP() const
{
    return m_peerIP;
}

unsigned short Connection::peerPort() const
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

void *Connection::userdata() const
{
    return m_userdata;
}

void Connection::handleEvents(uint32_t events)
{
    int ret;
    size_t offset, size;
    SPTRTask taskSend, taskReceive;
    bool needDoDisconnect = false,
        needCallbackBufferSent = false,
        needCallbackBufferReceived = false;

    if (!m_connected)
        return;

    do
    {
        std::lock_guard<std::recursive_mutex> lockGuard(m_lock);
        if (!m_connected)
            return;

        if (events & EPOLLERR)
        {
            needDoDisconnect = true;
            break;
        }

        if (events & EPOLLIN)
        {
            if (m_taskReceive.get())
            {
                offset = m_taskReceive->progress();
                size = m_taskReceive->data().capacity() - offset;
                ret = ::recv(m_handle,
                    m_taskReceive->data().data() + offset, size, 0);
                if (ret <= 0)
                {
                    needDoDisconnect = true;
                    break;
                }
                m_taskReceive->increase(ret);

                if (!m_taskReceive->completely() || m_taskReceive->finished())
                {
                    taskReceive = m_taskReceive;
                    m_taskReceive.reset();
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
                size = taskSend->data().capacity() - offset;
                ret = ::send(m_handle,
                    taskSend->data().data() + offset, size, 0);
                if (ret <= 0)
                {
                    needDoDisconnect = true;
                    break;
                }
                taskSend->increase(ret);

                if (!taskSend->completely() || taskSend->finished())
                {
                    m_tasksSend.pop_front();
                    needCallbackBufferSent = true;
                }
            }

            if (m_tasksSend.empty())
            {
                m_context.events = EPOLLIN;
                m_eventPoll->modify(m_handle, &m_context);
            }
        }

    } while(false);

    if (needCallbackBufferSent && onBufferSent)
        onBufferSent(this, taskSend->data());

    if (needCallbackBufferReceived && onBufferReceived)
        onBufferReceived(this, taskReceive->data());

    if (needDoDisconnect)
        disconnect();
}

void Connection::close(void *userdata)
{
    std::lock_guard<std::recursive_mutex> lockGuard(m_lock);

    m_eventPoll->remove(m_handle, &m_context);
    closesocket(m_handle);
    m_handle = INVALID_SOCKET;
    m_connected = false;
    m_disconnecting = false;
    m_tasksSend.clear();
    m_taskReceive.reset();

    if (onDisconnected)
    {
        this->onDisconnected(this);
    }
}

void Connection::_close(void *userdata)
{
    close(userdata);
}

Connection::Task::Task(AutoBuffer data, bool completely)
    : m_data(data),
      m_progress(data.size()),
      m_completely(completely)
{

}

Connection::Task::~Task()
{

}

AutoBuffer Connection::Task::data()
{
    return m_data;
}

void Connection::Task::increase(size_t progress)
{
    assert(progress);
    m_progress += progress;

    assert(m_progress <= m_data.capacity());
    m_data.resize(m_progress);
}

size_t Connection::Task::progress()
{
    return m_progress;
}

bool Connection::Task::finished()
{
    return m_progress == m_data.capacity();
}

bool Connection::Task::completely()
{
    return m_completely;
}

#endif
