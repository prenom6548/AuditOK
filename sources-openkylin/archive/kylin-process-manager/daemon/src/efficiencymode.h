#ifndef EFFICIENCYMODE_H
#define EFFICIENCYMODE_H

#include "cgroupmanager.h"
#include <QList>
#include <QObject>
#include <QTimer>
#include <QVariant>
#include <atomic>
#include <map>
#include <unordered_map>

class EfficiencyMode : public QObject
{
    Q_OBJECT
public:
    EfficiencyMode(CGroupManager *cgroupManager, QObject *parent = nullptr);
    ~EfficiencyMode();

    void SetProcessMode(int pid, bool enable);
    bool ProcessHasFeature(int pid);

    void SetProcessFreeze(int pid, bool enable);
    bool ProcessHasFreezeFeature(int pid);

    // ret "none" "freeze" "efficiency"
    QString GetProcessMode(int pid);

    QMap<int, QString> ListProcessModes();

    void SetPidFilter(int pid, bool enable);

private:
    bool getProcessMode(int pid);
    QList<int> listProcessModes();
    bool getProcessFreeze(int pid);
    QList<int> listProcessFreezes();
    void startDefaultTimer();

Q_SIGNALS:
    void ProcessModeChanged(int pid, QString mode);

private:
    CGroupManager *m_cgroupManager;
    std::unordered_map<int, std::vector<int>> m_limitIndex;
    QTimer m_defaultTimer;
};
#endif // !EFFICIENCYMODE_H