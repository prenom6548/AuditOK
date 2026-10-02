#ifndef CGROUPCONTROLINTERFACE_H
#define CGROUPCONTROLINTERFACE_H

#include <QDBusInterface>
#include <cstdint>
#include <string>
#include <vector>

class CgroupControlInterface
{
public:
    explicit CgroupControlInterface();

    int limitProcessCPUPercent(int pid, double cpuPercent);

    int limitProcessCPUWeight(int pid, int cpuWeight);

    int limitProcessMemoryMax(int pid, uint64_t memoryMax);

    int limitProcessIOWeight(int pid, int ioWeight);

    int freezeProcess(int pid);

    int limitThreadCPUPercent(int pid, int tid, double cpuPercent);

    int limitThreadCPUWeight(int pid, int tid, int cpuWeight);

    int limitThreadMemoryMax(int pid, int tid, uint64_t memoryMax);

    int limitThreadIOWeight(int pid, int tid, int ioWeight);

    int freezeThread(int pid, int tid);

    bool releaseLimitByIndex(int index);

    bool setCurrentUserProperty(std::string currentUserUID,
                                std::string currentUserSlice, std::string currentUserService, std::string currentUserSession);

    void setGroupPath(std::string currentUserUID, std::string name, std::string path);

    void setGroupProperty(std::string currentUserUID, std::string name, std::string node, std::string value);

    void createGroupPath(std::string currentUserUID, std::string name);

    void removeGroupPath(std::string currentUserUID, std::string name);

    void limitProcessByGroup(std::string name, int pid);

    void setGroupEnable(bool enable);

    void setFreezeEnable(bool enable);

    void thawedProcess(int pid);

private:
    void asyncCallWithArgumentList(const QString &method, const QList<QVariant> &args);
    QVariant blockCallWithArgumentList(const QString &method, const QList<QVariant> &args);

private:
    QDBusInterface dbusInterface;
};

#endif // !CGROUPCONTROLINTERFACE_H
