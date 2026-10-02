#ifndef CPRIORITYSCHEDULINGMANAGER_H
#define CPRIORITYSCHEDULINGMANAGER_H

#include <unordered_map>
#include <QString>
#include <vector>
#include <QList>

struct TopState
{
    uint8_t topType;
    uint8_t topPrio;
    uint8_t topSlice;
    uint8_t topTypeByTgid;
};

class CPrioritySchedulingManager
{
public:
    void insertTopTask(pid_t pid, TopState topState);
    void eraseTopTask(int pid);
    void enable();
    void disable();
    void clearTopList();
private:
    int64_t topStateToNumber64(TopState topState);
private:
    int m_topTaskEnable;
    std::unordered_map<pid_t, int> m_topTask;
};

#endif // CPRIORITYSCHEDULINGMANAGER_H