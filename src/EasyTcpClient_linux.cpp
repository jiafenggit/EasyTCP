#ifdef __linux__
#include "EasyTcpClient_linux.h"
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

using namespace EasyTcp;
using namespace std::placeholders;

Client::Client(WorkerPtr worker)
    : Connection(static_cast<EventPoll*>(worker.get())),
      m_worker(worker),
      m_connecting(false),
      m_eventPoll(static_cast<EventPoll*>(worker.get()))
{

}

IClientPtr Client::create()
{
    EventPoll* ep;
    WorkerPtr worker(ep = new EventPoll());

    if (!ep->init(1024))
        return IClientPtr();

    return create(worker);
}

IClientPtr Client::create(WorkerPtr worker)
{
    IClientPtr ret;
    if (!dynamic_cast<EventPoll*>(worker.get()))
        return ret;

    ret.reset(new Client(worker));
    return ret;
}

Client::~Client()
{
    disconnect();
    while (m_connected || m_connecting)
        usleep(1);
}

bool Client::connect(const std::string& host, unsigned short port)
{
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

        int flag = fcntl(m_handle, F_GETFL, 0);
        if (-1 == flag )
            break;
        fcntl(m_handle, F_SETFL, flag | O_NONBLOCK);

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(host.c_str());
        addr.sin_port = htons(port);

        if(::connect(m_handle, (sockaddr*)&addr, sizeof(addr)) == 0)
        {
            m_context.events = EPOLLIN;
            m_context.setCallback(std::bind(&Client::handleEvents, this, _1));
            m_eventPoll->add(m_handle, &m_context);
            m_connected = true;
            m_connecting = false;
            updateEndPoint();
            if (onConnected)
                onConnected(this);
            return true;
        }
        else if (errno == EINPROGRESS)
        {
            m_context.events = EPOLLOUT | EPOLLET;
            m_context.setCallback([this](uint32_t events)
            {
                if ((events & EPOLLOUT) && !(events & EPOLLERR))
                {
                    m_context.events = EPOLLIN;
                    m_eventPoll->modify(m_handle, &m_context);
                    m_connected = true;
                    m_connecting = false;
                    updateEndPoint();
                    if (onConnected)
                        onConnected(this);

                    m_context.setCallback(std::bind(&Client::handleEvents, this, _1));
                }
                else
                {
                    m_connecting = false;
                    int err;
                    socklen_t len = sizeof(err);
                    getsockopt(m_handle, SOL_SOCKET, SO_ERROR, &err, &len);
                    closesocket(m_handle);
                    m_handle = INVALID_SOCKET;
                    if (onConnectFailed)
                        onConnectFailed(this, err);
                }
            });

            if (!m_eventPoll->add(m_handle, &m_context))
                break;
            return true;
        }
        else
            break;
    }
    while(0);

    closesocket(m_handle);
    m_handle = INVALID_SOCKET;
    m_connecting = false;
    if (onConnectFailed)
        onConnectFailed(this, errno);
    return false;
}

#endif



