#include "HorizontalOrVerticalMode.h"
#include "data_warehouse.h"

static const QString KYLIN_ROTATION_PATH = "/";

static const QString KYLIN_ROTATION_SERVICE = "com.kylin.statusmanager.interface";

static const QString KYLIN_ROTATION_INTERFACE = "com.kylin.statusmanager.interface";

HorizontalOrVerticalMode::HorizontalOrVerticalMode()
{
    QDBusConnection::sessionBus().connect(KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH, KYLIN_ROTATION_INTERFACE,
                                          QString("rotations_change_signal"), this, SLOT(rotationChanged(QString)));
    QDBusConnection::sessionBus().connect(KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH, KYLIN_ROTATION_INTERFACE,
                                          QString("mode_change_signal"), this, SLOT(modeChanged(bool)));
}
deviceMode HorizontalOrVerticalMode::defaultModeCapture()
{ // method
    QDBusMessage message_pcORpad = QDBusMessage::createMethodCall(KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH,
                                                          KYLIN_ROTATION_INTERFACE, QString("get_current_tabletmode"));
    QDBusPendingReply<bool> reply_pcORpad = QDBusConnection::sessionBus().call(message_pcORpad);
    if (!reply_pcORpad.isValid()) {
        DataWarehouse::getInstance()->intelMode = false;
        return PCMode; //pc模式
    }
    if (reply_pcORpad.value()) {//平板模式
        DataWarehouse::getInstance()->intelMode = true;
        QDBusMessage message = QDBusMessage::createMethodCall(KYLIN_ROTATION_SERVICE, KYLIN_ROTATION_PATH,
                                                              KYLIN_ROTATION_INTERFACE, QString("get_current_rotation"));
        QDBusPendingReply<QString> reply = QDBusConnection::sessionBus().call(message);
        if (!reply.isValid()) {
            return PADHorizontalMode; //横屏
        }
        if (reply.value() == "normal" || reply.value() == "upside-down") {
            return PADHorizontalMode; //横屏
        } else {
            return PADVerticalMode; //竖屏
        }

    } else {
        DataWarehouse::getInstance()->intelMode = false;
        return PCMode; //pc模式
    }
}

void HorizontalOrVerticalMode::rotationChanged(QString res)
{ // signal
    emit RotationSig(PADVerticalMode);
}
void HorizontalOrVerticalMode::modeChanged(bool res){
    emit RotationSig(PADVerticalMode);
}
