#ifdef __linux__
#include "EasyTcpContext_linux.h"

using namespace EasyTCP::Context;

Context::Context(std::function<void (uint32_t)> callback)
    : m_callback(callback)
{
    this->events = EPOLLIN;
    this->data.ptr = this;
}

Context::~Context()
{

}

void Context::setCallback(std::function<void (uint32_t)> callback)
{
    m_callback = callback;
}

void Context::update(uint32_t events)
{
    if (m_callback)
        m_callback(events);
}

#endif
