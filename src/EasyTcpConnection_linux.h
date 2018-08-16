#ifdef __linux__
#ifndef EASYTCPCONNECTION_H
#define EASYTCPCONNECTION_H

#include <string>
#include <functional>
#include "EasyTcpDef.h"
#include "EasyTcpAutoBuffer.h"
#include "EasyTcpContext_linux.h"
#include "EasyTcpEventPoll.h"
#include "EasyTcpIConnection.h"
#include <list>
#include <mutex>

namespace EasyTcp
{
    class Connection : virtual public IConnection
    {
        class Task
        {
        public:
            Task(AutoBuffer data);
            ~Task();

            AutoBuffer data();
            bool increase(size_t progress);
            size_t progress();
            bool finished();

        private:
            AutoBuffer m_data;
            size_t m_progress;
        };

        typedef std::shared_ptr<Task> SPTRTask;

    public:
        Connection(EventPoll *eventPoll);
        Connection(EventPoll *eventPoll, int sock, bool connected = true);
        ~Connection();

        IConnectionPtr share();
        SOCKET handle();

        bool connected();

        bool disconnect();
        bool send(AutoBuffer buffer);
        bool recv(AutoBuffer buffer);

        bool enableKeepalive(unsigned long interval = 1000, unsigned long time = 2000);
        bool disableKeepalive();

        bool setSendBufferSize(unsigned long size);
        bool setReceiveBufferSize(unsigned long size);

        bool setLinger(unsigned short onoff, unsigned short linger);

        const std::string& localIP();
        unsigned short localPort();

        const std::string& peerIP();
        unsigned short peerPort();

        bool updateEndPoint();

        void bindUserdata(void* userdata);
        void* userdata();

    protected:
        void handleEvents(uint32_t events);
        virtual void close(void* userdata);
        void _close(void* userdata);

    protected:
        SOCKET m_handle;
        EventPoll* m_eventPoll;
        Context::Context m_context;
        bool m_connected;

        std::string m_localIP;
        unsigned short m_localPort;

        std::string m_peerIP;
        unsigned short m_peerPort;

        std::recursive_mutex m_lock;

        std::list<SPTRTask> m_tasksSend;
        SPTRTask m_taskReceive;

        bool m_disconnecting;

        void* m_userdata;
    };
}

#endif
#endif
