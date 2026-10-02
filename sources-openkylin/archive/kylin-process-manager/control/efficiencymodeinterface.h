#ifndef EFFICIENCYMODEINTERFACE_H
#define EFFICIENCYMODEINTERFACE_H

#include <QDBusInterface>
#include <cstdint>
#include <string>
#include <vector>

class EfficiencyModeInterface
{
public:
    explicit EfficiencyModeInterface();

    void setPidFilter(int pid, bool enable);

private:
    void asyncCallWithArgumentList(const QString &method, const QList<QVariant> &args);

private:
    QDBusInterface dbusInterface;
};

#endif // !EFFICIENCYMODEINTERFACE_H
