#ifndef EASYTCPCONNECTION_H
#define EASYTCPCONNECTION_H

#include <WinSock2.h>
#include <string>
#include <functional>
#include "EasyTcpAutoBuffer.h"
#include "EasyTcpContext.h"
#include <set>
#include <mutex>
#include <atomic>

namespace EasyTcp
{
    class Connection : public std::enable_shared_from_this<Connection>
    {
    public:
        Connection();
        Connection(SOCKET sock, bool connected = true);
        ~Connection();

        std::shared_ptr<Connection> share();

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
        bool post(Context *context, bool isSend);
        bool post(AutoBuffer buffer, bool isSend, std::function<void(Context*, size_t)> doneCallback,
                std::function<void(Context*, int)> errorCallback);

        void whenDone(Context *context, size_t increase,
            std::function<void (Connection*, AutoBuffer data)> callback);
        void whenSendDone(Context *context, size_t increase);
        void whenRecvDone(Context *context, size_t increase);
        void whenError(Context *context, int err);

        int increasePostCount();
        int decreasePostCount();

    protected:
        SOCKET m_handle;

        bool m_connected;
        std::atomic<bool> m_disconnecting;

        std::string m_localIP;
        unsigned short m_localPort;

        std::string m_peerIP;
        unsigned short m_peerPort;

        std::recursive_mutex m_lock;
        void* m_userdata;

        std::atomic<int> m_countPost;
    };
}

#endif
