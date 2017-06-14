#include "EasyTcpTask.h"
using namespace EasyTcp;

Task::Task(AutoBuffer data)
    : m_data(data),
      m_progress(0)
{

}

Task::~Task()
{

}

AutoBuffer Task::data()
{
    return m_data;
}

bool Task::increase(size_t progress)
{
    if (!progress)
        return true;

    size_t tmpProgress = m_progress + progress;

    if (tmpProgress > m_data.size()
        || (m_progress && (tmpProgress <= m_progress
            || tmpProgress <= progress)))
        return false;


    m_progress = tmpProgress;
    return true;
}

size_t Task::progress()
{
    return m_progress;
}

bool Task::finished()
{
    return m_progress == m_data.size();
}
