#ifndef DAEMOND1BUSINTERFACE_H
#define DAEMOND1BUSINTERFACE_H

#include <QDBusInterface>
#include <string>
#include <vector>

class Daemon1DbusInterface
{
public:
    explicit Daemon1DbusInterface();

public:
    void insertTopTask(int pid, int topType, int topPrio, int topSlice, int topTypeByTgid);
    void eraseTopTask(int pid);
    void enable();
    void disable();
    void clearTopList();
private:
    void asyncCallWithArgumentList(const QString &method, const QList<QVariant> &args);

private:
    QDBusInterface dbusInterface;
};

#endif // DAEMOND1BUSINTERFACE_H
