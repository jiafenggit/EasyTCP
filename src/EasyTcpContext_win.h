#if  defined(WIN32) || defined(WIN64)
#ifndef EASYTCPCONTEXT_H
#define EASYTCPCONTEXT_H

#include <Winsock2.h>
#include <memory>
#include <functional>
#include "EasyTcpAutoBuffer.h"
#include <atomic>

namespace EasyTcp
{
    class Context : public OVERLAPPED
    {
    public:
        Context();
        Context(AutoBuffer buffer);

        void increase();
        void decrease();

        void increaseProgress(size_t increase);
        void error(int err);
        size_t progress();

        AutoBuffer buffer();
        WSABUF* WSABuf();
        bool finished();

    public:
        std::function<void(Context*, size_t increase)> onDone;
        std::function<void(Context*, int err)> onError;

    private:
        ~Context();

    private:
        AutoBuffer m_buffer;
        std::atomic<int> m_ref;
        size_t m_progress;
        WSABUF m_wsaBuffer;
    };
}

#endif
#endif
