#ifndef EASYTCPCONNECTION_H
#define EASYTCPCONNECTION_H

#include <string>
#include <functional>
#include "EasyTcpAutoBuffer.h"
#include "EasyTcpContext.h"
#include "EasyTcpEventPoll.h"
#include "EasyTcpTask.h"
#include <list>
#include <mutex>

namespace EasyTcp
{
    class Connection
    {
    public:
        Connection(EventPoll *eventPoll);
        Connection(EventPoll *eventPoll, int sock, bool connected = true);
        ~Connection();

        int handle();

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
        std::function<void (Connection*, const std::string& errstr)> onError;

    protected:
        void handleEvents(uint32_t events);
        void registerHandle();
        void removeHandle();

    protected:
        int m_handle;
        EventPoll* m_eventPoll;
        Context::Context m_context;
        bool m_registered;

        bool m_connected;

        std::string m_localIP;
        unsigned short m_localPort;

        std::string m_peerIP;
        unsigned short m_peerPort;

        std::recursive_mutex m_lock;

        std::list<SPTRTask> m_tasksSend;
        std::list<SPTRTask> m_tasksReceive;

        void* m_userdata;
    };
}

#endif
