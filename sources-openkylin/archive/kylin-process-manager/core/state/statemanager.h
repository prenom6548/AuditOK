#ifndef STATEMANAGER_H
#define STATEMANAGER_H

#include <QTimer>
#include <QObject>

#include <unordered_map>

#include "statetarget.h"
#include "multilockmutex.h"

class StateManager : public QObject 
{
    Q_OBJECT
public:
    StateManager();
    ~StateManager();

    void enableTarget(const StateTarget &target, bool isEmit = true);
    void disableTarget(const StateTarget &target);
    bool isHaveStateTarget(const StateTarget &target);
    const StateTarget &stateTarget();

    FairMutex &fairMutex(const StateTarget &target);

    void multiLockMutexUnlock();

Q_SIGNALS:
    void stateTargetChanged();
    void notifyTimerStart();

private Q_SLOTS:
    void handleTimerTimeOut();

private:
    void initTimer();
    void initMutex();
    
private:
    StateTarget m_stateTarget;
    
    QTimer m_timer;

    std::map<StateTarget, size_t> m_mtxMap;
    std::vector<QMetaObject::Connection> m_conns;
};

#endif // !STATEMANAGER_H