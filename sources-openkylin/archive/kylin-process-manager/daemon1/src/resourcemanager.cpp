#include "resourcemanager.h"

ResourceManager::ResourceManager(QObject *parent)
    : QObject{parent}
{
    
}

void ResourceManager::InsertTopTask(int pid, int topType, int topPrio, int topSlice, int topTypeByTgid)
{
    m_cprioritySchedulingManager.insertTopTask(static_cast<pid_t>(pid), {static_cast<uint8_t>(topType), static_cast<uint8_t>(topPrio), static_cast<uint8_t>(topSlice), static_cast<uint8_t>(topTypeByTgid)});
}

void ResourceManager::EraseTopTask(int pid)
{
    m_cprioritySchedulingManager.eraseTopTask(pid);
}

void ResourceManager::Enable()
{
    m_cprioritySchedulingManager.enable();
}

void ResourceManager::Disable()
{
    m_cprioritySchedulingManager.disable();
}

void ResourceManager::ClearTopList()
{
    m_cprioritySchedulingManager.clearTopList();
}