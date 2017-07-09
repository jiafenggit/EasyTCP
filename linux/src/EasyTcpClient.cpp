#include "EasyTcpClient.h"
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace EasyTcp;
using namespace std::placeholders;

Client::Client()
    : Connection(new EventPoll()),
      onConnected(nullptr),
      onConnectFailed(nullptr)
{

}

SPTRClient Client::create()
{
    SPTRClient ret(new Client());
    if (!ret->m_eventPoll->init(10))
    {
        ret.reset();
        return ret;
    }

    return ret;
}

Client::~Client()
{
    disconnect();
    delete m_eventPoll;
}

bool Client::connect(const std::string& host, unsigned short port)
{
    if (m_connected)
        return false;

    m_handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_handle == -1)
    {
        goto CONNECT_FAILED;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host.c_str());
    addr.sin_port = htons(port);

    if(::connect(m_handle, (sockaddr*)&addr, sizeof(addr)))
    {
        close(m_handle);
        m_handle = -1;
        goto CONNECT_FAILED;
    }

    m_connected = true;
    updateEndPoint();
    if (onConnected)
        onConnected(this);
    return true;

CONNECT_FAILED:
    if (onConnectFailed)
        onConnectFailed(this, errno);
    return false;
}


