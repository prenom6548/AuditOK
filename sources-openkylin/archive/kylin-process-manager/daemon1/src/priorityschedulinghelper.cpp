#include "priorityschedulinghelper.h"

#include "loghelper.h"
#include <fstream>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

bool PrioritySchedulingHelper::setTopTask(std::unordered_map<pid_t, int64_t> topTask)
{
    if (topTask.empty())    return false;
    std::ofstream file("/proc/kylin_sched/top_task");
    if (!file.is_open()) return false;
    std::unordered_map<pid_t, int64_t>::iterator topTaskIter = topTask.begin();
    std::string topTaskStr = std::to_string(topTaskIter->first) + ":" + std::to_string(topTaskIter->second);
    topTaskIter++;
    for (; topTaskIter != topTask.end(); ++topTaskIter)
    {
        topTaskStr = topTaskStr + "," + std::to_string(topTaskIter->first) + ":" + std::to_string(topTaskIter->second);
    }
    PRINT_NOTICE("kylin-process-resource-manager-daemon : PrioritySchedulingHelper:58 topTaskStr : %s\n", topTaskStr.c_str());
    file << topTaskStr; 
    file.close();
    return true;
}

bool PrioritySchedulingHelper::insertTopTask(pid_t pid, int64_t value)
{
    std::ofstream file("/proc/kylin_sched/top_task");
    if (!file.is_open()) return false;
    std::string topTaskStr = std::to_string(pid) + ":" + std::to_string(value);
    file << topTaskStr; 
    file.close();
    return true;
}

bool PrioritySchedulingHelper::setTopTaskEnable(int topTaskEnable)
{
    std::ofstream file("/proc/kylin_sched/enable");
    if (!file.is_open()) return false;
    file << topTaskEnable; 
    file.close();
    return true;
}

bool PrioritySchedulingHelper::clearTopList()
{
    std::ofstream file("/proc/kylin_sched/top_list");
    if (!file.is_open()) return false;
    file << "0";
    file.close();
    return true;
}