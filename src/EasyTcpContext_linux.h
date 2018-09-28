#ifdef __linux__
#ifndef EASYTCPCONTEXT_H
#define EASYTCPCONTEXT_H

#include <memory>
#include <functional>
#include <sys/epoll.h>

namespace EasyTCP
{
    namespace Context
    {

        class Context : public epoll_event
        {
        public:
            Context(std::function<void(uint32_t)> callback);
            ~Context();

            void setCallback(std::function<void(uint32_t)> callback);
            void update(uint32_t events);

        private:
            std::function<void(uint32_t)> m_callback;
        };
        typedef std::shared_ptr<Context> SPTRContext;
    }
}

#endif

#endif
