#ifndef PRIORITYSCHEDULINGMANAGER_H
#define PRIORITYSCHEDULINGMANAGER_H

#include "resourcemanagerinterface.h"
#include <unordered_map>
#include <vector>

class PrioritySchedulingManager
{
public:
    void insertTopTask(int pid, int topType, int topPrio, int topSlice, int topTypeByTgid);
    void eraseTopTask(int pid);
    void enable();
    void disable();
    void clearTopList();

private:
    ResourceManagerInterface m_resourceManager;
};

#endif // PRIORITYSCHEDULINGMANAGER_H