#ifndef PRIORITYSCHEDULINGHELPER_H
#define PRIORITYSCHEDULINGHELPER_H

#include <cstdint>
#include <sys/types.h>
#include <unordered_map>

class PrioritySchedulingHelper
{
public:
    static bool setTopTask(std::unordered_map<pid_t, int64_t> topTask);
    static bool insertTopTask(pid_t pid, int64_t value);
    static bool setTopTaskEnable(int topTaskEnable);
    static bool clearTopList();
}; 

#endif // PRIORITYSCHEDULINGHELPER_H
