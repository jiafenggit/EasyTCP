#include "EasyTcpContext.h"

using namespace EasyTcp::Context;

Context::Context(std::function<void (uint32_t)> callback)
    : m_callback(callback)
{
    this->events = EPOLLIN | EPOLLOUT;
    this->data.ptr = this;
}

Context::~Context()
{

}

void Context::update(uint32_t events)
{
    if (m_callback)
        m_callback(events);
}
