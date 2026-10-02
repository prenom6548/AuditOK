#include "checkpower.h"

CheckPower::CheckPower()
{
    QDBusInterface upgrade_interface( "com.kylin.systemupgrade",
                               "/com/kylin/systemupgrade",
                               "com.kylin.systemupgrade.interface",
                               QDBusConnection::systemBus() );
    QDBusMessage rebootresult = upgrade_interface.call("DeployLatestUpdate","reboot");
    int  reboot =rebootresult.arguments().value(0).toInt();
    QString reason = rebootresult.arguments().value(1).toString();
    if(!reboot)
    {
        qInfo()<<"reboot success!";
    }
}
