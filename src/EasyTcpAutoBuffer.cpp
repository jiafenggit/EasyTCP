#include "EasyTcpAutoBuffer.h"
#include <stdio.h>
#include <string.h>

using namespace EasyTCP;

AutoBuffer::AutoBuffer()
	: m_size(0)
{

}

AutoBuffer::AutoBuffer(size_t size)
    :   AutoBuffer()
{
    reset(size);
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
}

bool AutoBuffer::reset(size_t size)
{
    if (size == 0)
        return false;
        
    m_sptrData.reset(new char[size], free);
    m_size = size;
    return true;
}

bool AutoBuffer::reset(const char *data, size_t size)
{
    if (size == 0)
        return false;
        
    m_sptrData.reset(new char[size], free);
	memcpy(m_sptrData.get(), data, size);
	m_size = size;
    return true;
}

char* AutoBuffer::data()
{
	return m_sptrData.get();
}

size_t AutoBuffer::size()
{
	return m_size;
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
        if (!m_sptrData.get()
            || fread(m_sptrData.get(), 1, size, pf) != size)
        {
            m_sptrData.reset();
            fclose(pf);
            return false;
        }
    }

    m_size = size;
    fclose(pf);
    return true;
}

bool AutoBuffer::saveToFile(const std::string& file)
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
