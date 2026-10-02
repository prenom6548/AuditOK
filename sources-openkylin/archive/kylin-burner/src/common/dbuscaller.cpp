#include "dbuscaller.h"
#include "src/frame/logger.h"

#include <QDBusPendingReply>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QEventLoop>
#include <QDebug>


DBusCaller::DBusCaller(QObject *parent) : QObject(parent)
{

}

int DBusCaller::callSensitiveInfoCheck(const QString &orderNo, const QStringList &filePathGroup, int timeoutMs)
{
    const QString service = "com.kylin.dlp.TransControl";
    const QString path = "/com/kylin/dlp/TransControl";
    const QString interface = "com.kylin.dlp.TransControlInterface";
    const QString method = "SensitiveInformationCheck";

    // 1. 创建接口并检查有效性
    QDBusInterface dbusInterface(service, path, interface, QDBusConnection::systemBus());
    if (!dbusInterface.isValid()) {
        LOGGER->error(QString("D-Bus Interface : [%1] is Non Valid").arg(interface));
        return DBusError;
    }

    // 2. 检查方法是否存在
    QByteArray methodSignature = "SensitiveInformationCheck(QString,QStringList)";
    if (dbusInterface.metaObject()->indexOfMethod(methodSignature.constData()) == -1) {
        LOGGER->error(QString("[%1] does not exist in D-Bus Interface [%2] ").arg(method).arg(interface));
        return MethodNotExist;
    }

    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    int result = DBusError;

    // 3. 设置超时
    QObject::connect(&timeoutTimer, &QTimer::timeout, [&loop, &result]() {
        qWarning() << "调用超时";
        result = Timeout;
        loop.quit();
    });
    timeoutTimer.start(timeoutMs);

    // 4. 异步调用
    QDBusPendingCall call = dbusInterface.asyncCall(method, orderNo, filePathGroup);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);

    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this,
                     [&](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<int> reply = *watcher;
        watcher->deleteLater();
        if (reply.isError()) {
            qWarning() << "DBus call error:" << reply.error().message();
            result = DBusError;
        } else {
            result = reply.value(); // 获取实际返回值(1或0)
        }
        loop.quit();
    });

    // 7. 阻塞等待
    loop.exec();

    // 8. 清理
    if (timeoutTimer.isActive()) {
        timeoutTimer.stop();
    }

    return result;
}

bool DBusCaller::handleSensitiveInfoCheckResult(int result)
{
    switch (result) {
    case DBusCaller::Success:
        LOGGER->info("DBusCaller::callSensitiveInfoCheck DBusCaller::Success");
        return true;
    case DBusCaller::Failure:
        LOGGER->info("DBusCaller::callSensitiveInfoCheck DBusCaller::Failure");
        return false;
    case DBusCaller::Timeout:
        LOGGER->info("DBusCaller::callSensitiveInfoCheck DBusCaller::Timeout");
        return false;
    case DBusCaller::MethodNotExist:
        LOGGER->info("DBusCaller::callSensitiveInfoCheck DBusCaller::MethodNotExist");
        return true;
    case DBusCaller::DBusError:
        LOGGER->info("DBusCaller::callSensitiveInfoCheck DBusCaller::DBusError");
        return true;
    default:
        LOGGER->info("DBusCaller::callSensitiveInfoCheck None");
    }

    return false;
}

