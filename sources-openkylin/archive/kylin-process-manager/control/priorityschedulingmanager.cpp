#include "priorityschedulingmanager.h"


void PrioritySchedulingManager::insertTopTask(int pid, int topType, int topPrio, int topSlice, int topTypeByTgid)
{
    m_resourceManager.insertTopTask(pid, topType, topPrio, topSlice, topTypeByTgid);
}

void PrioritySchedulingManager::eraseTopTask(int pid)
{
    m_resourceManager.eraseTopTask(pid);
}

void PrioritySchedulingManager::enable()
{
    m_resourceManager.enable();
}

void PrioritySchedulingManager::disable()
{
    m_resourceManager.disable();
}

void PrioritySchedulingManager::clearTopList()
{
    m_resourceManager.clearTopList();
}