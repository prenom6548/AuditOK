//
// Created by sqp on 2022/11/8.
//

#include <QDBusPendingReply>
#include "tablemodelistener.h"

#define KYLIN_STATUS_MANAGER_PATH "/"
#define KYLIN_STATUS_MANAGER_NAME "com.kylin.statusmanager.interface"
#define KYLIN_STATUS_MANAGER_INTERFACE "com.kylin.statusmanager.interface"

TableModeListener::TableModeListener(QObject *parent) : QObject(parent)
{
    // 平板模式切换
    m_modeInterface = new QDBusInterface(KYLIN_STATUS_MANAGER_NAME, KYLIN_STATUS_MANAGER_PATH,
                                         KYLIN_STATUS_MANAGER_INTERFACE, QDBusConnection::sessionBus());
    QObject::connect(m_modeInterface, SIGNAL(mode_change_signal(bool)), this, SIGNAL(sigModeChange(bool)));
    QObject::connect(m_modeInterface, SIGNAL(rotations_change_signal(QString)), this,
                     SIGNAL(sigRotationsChange(QString)));
}

TableModeListener::~TableModeListener()
{
    if (m_modeInterface) {
        delete m_modeInterface;
        m_modeInterface = nullptr;
    }
}

bool TableModeListener::getMode()
{
    if (m_modeInterface && m_modeInterface->isValid()) {
        QDBusPendingReply<bool> resultMode = m_modeInterface->call("get_current_tabletmode");
        if (resultMode.isValid()) {
            return resultMode.value();
        }
    }
    return false;
}

QString TableModeListener::getRotations()
{
    if (m_modeInterface && m_modeInterface->isValid()) {
        QDBusPendingReply<QString> resultMode = m_modeInterface->call("get_current_rotation");
        if (resultMode.isValid()) {
            qInfo() << "Table value " << resultMode.value();
            return resultMode.value();
        }
    }
    return NORMAL;
}