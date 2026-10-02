#ifndef GLOBALMANAGER_H
#define GLOBALMANAGER_H

#include <QDBusConnection>
#include <QDBusError>


#include "brightness.h"
#include "global-signal.h"

class GlobalManager
{
public:
    GlobalManager();
    GlobalManager(GlobalManager&)=delete;
    GlobalManager&operator=(const GlobalManager&)=delete;

public:
    ~GlobalManager();
    static GlobalManager *GlobalManagerNew();
    void start();
    void stop();
private:
    static GlobalManager *m_globalManager;
    Brightness *m_pBright = nullptr;
    GlobalSignal *m_pSignal = nullptr;
};

#endif // GLOBALMANAGER_H
