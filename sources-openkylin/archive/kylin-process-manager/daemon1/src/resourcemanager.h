#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QObject>
#include "cpriorityschedulingmanager.h"

class ResourceManager : public QObject
{
    Q_OBJECT
public:
    explicit ResourceManager(QObject *parent = nullptr);

    void InsertTopTask(int pid, int topType, int topPrio, int topSlice, int topTypeByTgid);
    void EraseTopTask(int pid);
    void Enable();
    void Disable();
    void ClearTopList();
private:
    CPrioritySchedulingManager m_cprioritySchedulingManager;
};

#endif // RESOURCEMANAGER_H