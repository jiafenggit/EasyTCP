#include "EasyTcpClient.h"
#include <mstcpip.h>
#include <mswsock.h>
#include <WinBase.h>
#include <functional>

using namespace EasyTcp;
using namespace std::placeholders;

Client::Client()
    : m_worker(new Worker()),
      onConnected(nullptr),
      onConnectFailed(nullptr)
{

}

SPTRClient Client::create()
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
    {
        return SPTRClient();
    }

    SPTRClient ret(new Client());
    if (!ret->m_worker->init())
    {
        ret.reset();
        return ret;
    }

    return ret;
}

Client::~Client()
{
    disconnect();
    WSACleanup();
}

bool Client::connect(const std::string& host, unsigned short port)
{
    if (m_connected)
        return false;

    if (InterlockedCompareExchange(&m_isDoingConnect, 1, 0) == 1)
        return false;

    m_handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_handle == INVALID_SOCKET)
    {
        m_isDoingConnect = 0;
        return false;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.S_un.S_addr = INADDR_ANY;
    addr.sin_port = 0;
    if (bind(m_handle, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR
        || !m_worker->attach(this))
    {
        m_isDoingConnect = 0;
        return false;
    }

    m_sptrConnectAction.reset(
        new Context::ConnectAction(m_handle, host, port,
            std::bind(&Client::whenDoConnectSucceed, this, _1),
            std::bind(&Client::whenDoConnectFailed, this, _1, _2)));

    int err;
    if (!m_sptrConnectAction->invoke(err))
    {
        m_isDoingConnect = 0;
        return false;
    }

    return true;
}


void Client::whenDoConnectSucceed(Context::IAction* pAction)
{
    m_connected = true;
    m_isDoingConnect = 0;
    m_didDisconnect = false;

    setsockopt(m_handle, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
    updateEndPoint();

    if (onConnected)
        this->onConnected(this);
}

void Client::whenDoConnectFailed(Context::IAction* pAction, int err)
{
    m_isDoingConnect = 0;

    if (onConnectFailed)
        this->onConnectFailed(this, err);
}

