#include "EasyTcpAutoBuffer.h"
#include <stdio.h>
#include <string.h>

using namespace EasyTCP;

AutoBuffer::AutoBuffer()
    : m_size(0),
      m_capacity(0)
{

}

AutoBuffer::AutoBuffer(size_t capacity)
    :   AutoBuffer()
{
    reset(capacity);
}

AutoBuffer::AutoBuffer(const char *data, size_t size)
    :   AutoBuffer()
{
    reset(data, size);
}

void AutoBuffer::reset()
{
	m_sptrData.reset();
	m_size = 0;
    m_capacity = 0;
}

void AutoBuffer::reset(size_t capacity)
{
    if (capacity == 0)
    {
        m_sptrData.reset();
        m_capacity = 0;
    }
    else
    {
        m_sptrData.reset(new char[capacity], free);
        m_capacity = capacity;
    }

    m_size = 0;
}

void AutoBuffer::reset(const char *data, size_t size)
{
    if (size == 0)
        return;
        
    m_sptrData.reset(new char[size], free);
	memcpy(m_sptrData.get(), data, size);
    m_capacity = m_size = size;
}

bool AutoBuffer::fill(size_t offset, const char *data, size_t size)
{
    if (offset + size > m_capacity)
        return false;

    memcpy(m_sptrData.get() + offset, data, size);
    return true;
}

void AutoBuffer::resize(size_t size)
{
    if (size <= m_capacity)
        m_size = size;
    else
        reset(size);
}

char* AutoBuffer::data()
{
	return m_sptrData.get();
}

size_t AutoBuffer::size() const
{
    return m_size;
}

size_t AutoBuffer::capacity() const
{
    return m_capacity;
}

void AutoBuffer::free(char *p)
{
	delete[] p;
}

bool AutoBuffer::loadFromFile(const std::string& file)
{
    size_t size;
    FILE *pf;

    reset();

    pf = fopen(file.c_str(), "rb");
    if (!pf)
        return false;

    fseek(pf, 0, SEEK_END);
    size = ftell(pf);
    fseek(pf, 0, SEEK_SET);

    if (size)
    {
        m_sptrData.reset(new char[size], free);
        if (fread(m_sptrData.get(), 1, size, pf) != size)
        {
            m_sptrData.reset();
            fclose(pf);
            return false;
        }
    }

    m_capacity = m_size = size;
    fclose(pf);
    return true;
}

bool AutoBuffer::saveToFile(const std::string& file) const
{
    FILE *pf = fopen(file.c_str(), "wb");
    if (!pf)
        return false;

    if(fwrite(m_sptrData.get(), 1, m_size, pf) != m_size)
    {
        fclose(pf);
        return false;
    }

    fclose(pf);
    return true;
}
