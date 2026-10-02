#ifndef EVENTBUS_H
#define EVENTBUS_H

#include "registrinfo.h"

#include <QObject>
#include <QMetaType>

class EventBus : public QObject {
    Q_OBJECT
public:
    EventBus()
    {
        qRegisterMetaType<RegistrWinInfo>("RegistrWinInfo");
        qRegisterMetaType<RegistrProcessInfo>("RegistrProcessInfo");
        qRegisterMetaType<RegistrAppInfo>("RegistrAppInfo");
        qRegisterMetaType<std::string>("std::string");
        qRegisterMetaType<std::vector<int>>("std::vector<int>");
    }

    static EventBus *instance() {
        static EventBus bus;
        return &bus;
    }

public:
    void notifyStartWatcher() {
        Q_EMIT startWatcher();
    }

    void notifyStopWatcher() {
        Q_EMIT stopWatcher();
    }

    void notifyStartFreezerWatcher() {
        Q_EMIT startFreezerWatcher();
    }

    void notifyStopFreezerWatcher() {
        Q_EMIT stopFreezerWatcher();
    }

    void notifyStartWindowWatcher() {
        Q_EMIT startWindowWatcher();
    }

    void notifyStopWindowWatcher() {
        Q_EMIT stopWindowWatcher();
    }

    void notifyStartScreenWatcher() {
        Q_EMIT startScreenWatcher();
    }

    void notifyStopScreenWatcher() {
        Q_EMIT stopScreenWatcher();
    }

    void notifyStartBatteryEstimate() {
        Q_EMIT startBatteryEstimate();
    }
    
    void notifyStopBatteryEstimate() {
        Q_EMIT stopBatteryEstimate();
    }

    void notifyRegisterAppWinChangedSpellWatcher(const std::string &uuid, const RegistrAppInfo &process, int spell) {
        Q_EMIT registerAppWinChangedSpellWatcher(uuid, process, spell);
    }

    void notifyUnregisterAppWinChangedSpellWatcher(const std::string &uuid) {
        Q_EMIT unregisterAppWinChangedSpellWatcher(uuid);
    }

    void notifyRegisterAppFocusSpellWatcher(const std::string &uuid, const RegistrProcessInfo &process, int spell) {
        Q_EMIT registerAppFocusSpellWatcher(uuid, process, spell);
    }

    void notifyUnregisterAppFocusSpellWatcher(const std::string &uuid) {
        Q_EMIT unregisterAppFocusSpellWatcher(uuid);
    }

    void notifyAddProcessGroup(const std::string &id, const std::vector<int> &pids) {
        Q_EMIT addProcessGroup(id, pids);
    }
    
Q_SIGNALS:
    void startWatcher();
    void stopWatcher();
    
    void startFreezerWatcher();
    void stopFreezerWatcher();

    void startWindowWatcher();
    void stopWindowWatcher();
    
    void startScreenWatcher();
    void stopScreenWatcher();

    void startBatteryEstimate();
    void stopBatteryEstimate();

    void registerAppWinChangedSpellWatcher(const std::string &uuid, const RegistrAppInfo &process, int spell);
    void unregisterAppWinChangedSpellWatcher(const std::string &uuid);

    void registerAppFocusSpellWatcher(const std::string &uuid, const RegistrProcessInfo &process, int spell);
    void unregisterAppFocusSpellWatcher(const std::string &uuid);

    void addProcessGroup(const std::string &id, const std::vector<int> &pids);
};


#endif // !EVENTBUS_H