#ifndef SYSTEMUPGRADECHECK_H
#define SYSTEMUPGRADECHECK_H
#include <QDBusInterface>

class systemUpgradeCheck
{
public:
    systemUpgradeCheck();

    QDBusInterface *interface;
    bool isDbusActive = false;
    bool isSystemUpgrade = false;

    bool checkSystemUpgrade();
    bool doUpgradeThenRboot();
    bool doUpgradeThenShutdown();
};

#endif // SYSTEMUPGRADECHECK_H
