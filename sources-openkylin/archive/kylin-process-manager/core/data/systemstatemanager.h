#ifndef SYSTEMSTATEMANAGER_H
#define SYSTEMSTATEMANAGER_H

#include <string>
#include <memory>

#include <QHash>
#include <QObject>
#include <QScreen>
#include <QTimer>

#include "cgroupcontrolinterface.h"
#include "login1dbusinterface.h"
#include "schedpolicy.h"
#include "statemanager.h"

class SystemStateManager : public QObject
{
    Q_OBJECT
public:
    SystemStateManager(const std::shared_ptr<StateManager> &stateManager);
    ~SystemStateManager();

    // 屏幕信息
    sched_policy::ScreenMode currScreenMode();
    sched_policy::ScreenMode lastScreenMode();
    
    // 系统版本
    std::string systemVersion();
    
    // 是否wayland
    bool isWaylandPlatform();

    // StatusMode
    sched_policy::StatusMode currStatusMode();
    sched_policy::StatusMode lastStatusMode();

    // InputMode
    sched_policy::InputMode currInputMode();
    sched_policy::InputMode lastInputMode();

    // 资源阈值
    std::unordered_map<std::string, int> &resourceThreshold();

    void startScreenWatcher();
    void stopScreenWatcher();

    void startFreezeWatcher();
    void stopFreezeWatcher();

public Q_SLOTS:
    void resourceWarning(const std::string &resource, int level);

private Q_SLOTS:
    void onScreenModeChanged();
    void onScreenModeChanged(int screenMode);
    void onStatusModeChangedAcpuired(uint status);
    void onInputIdleChangeAcpuired(bool state);
    void onPrepareForSleepAcpuired(bool state);
    void onPrepareForShutdownAcpuired(bool state);


private:
    void initSystemVersion();
    void initIsWaylandPlatform();
    void initScreenMode();
    void initStatusMode();

private:
    std::shared_ptr<StateManager> m_stateManager;
    Login1DbusInterface m_login1DbusInterface;
    CgroupControlInterface m_cgroupControlInterface;
    sched_policy::ScreenMode m_currScreenMode;
    sched_policy::ScreenMode m_lastScreenMode;
    
    std::string m_systemVersion;
    
    bool m_isWaylandPlatform;

    sched_policy::StatusMode m_currStatusMode;
    sched_policy::StatusMode m_lastStatusMode;

    sched_policy::InputMode m_currInputMode;
    sched_policy::InputMode m_lastInputMode;

    std::unordered_map<std::string, int> m_resourceThreshold;
    QTimer m_resourceTimer;

    QMetaObject::Connection screen_add_conn;
    QMetaObject::Connection screen_remove_conn;
    QMetaObject::Connection screen_mode_conn;
    QHash<QScreen*, QMetaObject::Connection> m_screens;
};

#endif // !SYSTEMSTATEMANAGER_H