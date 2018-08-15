#if  defined(WIN32) || defined(WIN64)
#include "EasyTcpContext_win.h"
#include <ws2tcpip.h>
#include <mswsock.h>
#include "assert.h"

using namespace EasyTcp;

Context::Context()
    :   m_ref(0),
        m_progress(0)
{
    this->hEvent = 0;
    this->Internal = 0;
    this->InternalHigh = 0;
    this->Offset = 0;
    this->OffsetHigh = 0;
    this->Pointer = 0;

    m_wsaBuffer.buf = NULL;
    m_wsaBuffer.len = 0;
    increase();
}

Context::Context(EasyTcp::AutoBuffer buffer)
    :   Context()
{
    m_buffer = buffer;
    m_wsaBuffer.buf = m_buffer.data();
    m_wsaBuffer.len = m_buffer.size();
}

void Context::increase()
{
    ++m_ref;
}

void Context::decrease()
{
    if(!--m_ref)
        delete this;
}

void Context::increaseProgress(size_t increase)
{
    m_progress += increase;
    m_wsaBuffer.buf = m_buffer.data() + m_progress;
    m_wsaBuffer.len = m_buffer.size() - m_progress;

    assert(m_progress <= m_buffer.size());
    if (onDone)
        onDone(this, increase);
}

void Context::error(int err)
{
    if (onError)
        onError(this, err);
}

size_t Context::progress()
{
    return m_progress;
}

EasyTcp::AutoBuffer Context::buffer()
{
    return m_buffer;
}

WSABUF *Context::WSABuf()
{
    return &m_wsaBuffer;
}

bool Context::finished()
{
    return m_progress == m_buffer.size();
}

Context::~Context()
{

}

#endif
