#include "cpriorityschedulingmanager.h"

#include "loghelper.h"
#include "priorityschedulinghelper.h"
#include <cstdint>
#include <dirent.h>
#include <fstream>

void CPrioritySchedulingManager::insertTopTask(pid_t pid, TopState topState)
{
    if (m_topTaskEnable == 1){
        if (m_topTask.find(pid) != m_topTask.end()){
            m_topTask[pid]++;
        } else {
            m_topTask[pid] = 1;
        }
        PrioritySchedulingHelper::insertTopTask(pid, topStateToNumber64(topState));
    }
}

void CPrioritySchedulingManager::eraseTopTask(int pid)
{
    if (m_topTaskEnable == 1){
        if (m_topTask.find(pid) != m_topTask.end()){
            m_topTask[pid]--;
        }
        if (m_topTask[pid] == 0){
            PrioritySchedulingHelper::insertTopTask(pid, 0);
        }
    }
}

void CPrioritySchedulingManager::enable()
{
    m_topTaskEnable = 1;
    PrioritySchedulingHelper::setTopTaskEnable(m_topTaskEnable);
}

void CPrioritySchedulingManager::disable()
{
    m_topTaskEnable = 0;
    PrioritySchedulingHelper::setTopTaskEnable(m_topTaskEnable);
}

void CPrioritySchedulingManager::clearTopList()
{
    PrioritySchedulingHelper::clearTopList();
}

int64_t CPrioritySchedulingManager::topStateToNumber64(TopState topState)
{
    int64_t ret = topState.topType << 24 | topState.topPrio << 16 | topState.topSlice << 8 | topState.topTypeByTgid;
    return ret;
}