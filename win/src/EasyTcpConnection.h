#ifndef EASYTCPCONNECTION_H
#define EASYTCPCONNECTION_H

#include <WinSock2.h>
#include <string>
#include <functional>
#include "EasyTcpAutoBuffer.h"
#include "EasyTcpContext.h"
#include <set>
#include <mutex>

namespace EasyTcp
{
    class Connection
    {
    public:
        Connection();
        Connection(SOCKET sock, bool connected = true);

        SOCKET handle();

        bool connected();

        bool disconnect();
        bool send(AutoBuffer buffer);
        bool recv(AutoBuffer buffer);

        bool enableKeepalive(unsigned long interval = 1000, unsigned long time = 2000);
        bool disableKeepalive();

        bool setSendBufferSize(unsigned long size);
        bool setReceiveBufferSize(unsigned long size);

        bool setLinger(u_short onoff, u_short linger);

        const std::string& localIP();
        unsigned short localPort();

        const std::string& peerIP();
        unsigned short peerPort();

        bool updateEndPoint();

        void bindUserdata(void* userdata);
        void* userdata();

    public:
        std::function<void (Connection*)> onDisconnected;
        std::function<void (Connection*, AutoBuffer data)> onBufferSent;
        std::function<void (Connection*, AutoBuffer data)> onBufferReceived;

    protected:
        bool recordToPosts(Context::SPTRIAction sptrAction);
        size_t removeFromPosts(Context::IAction* pAction);
        bool removeFromPostsAndSafelyRelease(Context::IAction* pAction);

        void whenSent(Context::IAction* pAction);
        void whenReceived(Context::IAction* pAction);
        void whenSendOrReceiveFailed(Context::IAction* pAction, int err);

    protected:
        SOCKET m_handle;

        bool m_connected;
        bool m_didDisconnect;
        std::string m_localIP;
        unsigned short m_localPort;

        std::string m_peerIP;
        unsigned short m_peerPort;

        std::recursive_mutex m_lock;
        std::set<Context::SPTRIAction> m_posts;

        void* m_userdata;
    };
}

#endif
