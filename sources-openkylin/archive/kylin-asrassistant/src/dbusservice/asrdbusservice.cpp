#include "asrdbusservice.h"

#include <QDBusConnection>
#include "mainwindow.h"

AsrDbusService::AsrDbusService(QObject *parent) : QObject(parent)
{
    QDBusConnection::sessionBus().registerService(KYLIN_ASR_SERVICE);
    //注册对象路径，导出所有此对象的插槽 ,registerObject参数：路径，interface,对象，options
    QDBusConnection::sessionBus().registerObject(KYLIN_ASR_INTERFACE_PATH, this,
                                                 QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
}

void AsrDbusService::showWindow()
{
    MainWindow::getInstance()->showView();
}
