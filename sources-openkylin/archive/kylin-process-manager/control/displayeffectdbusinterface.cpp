#include "displayeffectdbusinterface.h"
#include "loghelper.h"
#include <QApplication>
#include <QDBusMetaType>
#include <QDebug>
#include <QSettings>
#include <vector>

namespace {
const char *wlcom_dbus_service = "com.kylin.Wlcom";
const char *effect_debus_path = "/com/kylin/Wlcom/Effect";
const char *effect_dbus_interface = "com.kylin.Wlcom.Effect";

const char *maintain_mode_dbus_service = "com.kylin.MaintainMode";
const char *maintain_mode_dbus_path = "/com/kylin/MaintainMode/Object";
const char *maintain_mode_interface = "com.kylin.MaintainMode.interface";
}


// 重载操作符以便D-Bus可以序列化/反序列化这个结构体
QDBusArgument &operator<<(QDBusArgument &argument, const EffectOption &effect) {
    argument.beginStructure();
    argument << effect.name << effect.value << effect.enabled;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, EffectOption &effect) {
    argument.beginStructure();
    argument >> effect.name >> effect.value >> effect.enabled;
    argument.endStructure();
    return argument;
}

EffectDbusInterface::EffectDbusInterface()
    : m_dbusInterface(wlcom_dbus_service, effect_debus_path, effect_dbus_interface, QDBusConnection::sessionBus())
    , m_maintainModeDbusInterface(maintain_mode_dbus_service, maintain_mode_dbus_path, maintain_mode_interface, QDBusConnection::systemBus())
    , m_effectBootSettings("kylin-process-manager", "effect_manage")
    {
        PRINT_DBG("EffectDbusInterface::EffectDbusInterface()\n");
        registerMetaType();
        restoreEffectsStatus();
        //initActiveEffects();
}

void EffectDbusInterface::initActiveEffects() 
{
     // 调用ListAllEffects方法（无参数）
     PRINT_DBG("EffectDbusInterface::initActiveEffects()\n");
     QDBusReply<QList<EffectOption>> reply = m_dbusInterface.call("ListAllEffects");

     // 检查调用是否成功
     if (!reply.isValid()) {
         //qDebug() << "D-Bus调用失败:" << reply.error().message();
         PRINT_NOTICE("EffectDbusInterface::initActiveEffects(): call ListAllEffects FAILED %s\n", reply.error().message().toStdString().c_str());
         return;
     }

    // 获取返回的效果列表
    m_enabledEffects = reply.value();
    m_enabledEffects.erase(std::remove_if(m_enabledEffects.begin(), m_enabledEffects.end(), 
    [](EffectOption effect) { return effect.enabled == false; }), 
    m_enabledEffects.end());

    // 解析并显示每个效果的信息
    for (int i = 0; i < m_enabledEffects.size(); ++i) {
        const EffectOption &effect = m_enabledEffects.at(i);
        PRINT_DBG("EffectDbusInterface::initActiveEffects(): name: %s, value: %d\n", effect.name.toStdString().c_str(), effect.value);
    }
    
}

void EffectDbusInterface::suspendEffectEffects(std::vector<std::string> &filterList)
{
    initActiveEffects();
    for (const auto &effect : m_enabledEffects) {
        auto it = std::find(filterList.begin(), filterList.end(), effect.name.toStdString());
        if(it != filterList.end()) continue;
        enableEffect(effect.name, false);
        m_effectBootSettings.setValue(effect.name, false);
    }
    m_effectBootSettings.sync();
}

void EffectDbusInterface::resumeEffectEffects()
{
    restoreEffectsStatus();
}

void EffectDbusInterface::registerMetaType()
{
    PRINT_DBG("EffectDbusInterface::registerMetaType()\n");
    qRegisterMetaType<EffectOption>("EffectOption");
    qRegisterMetaType<EffectOptionList>("EffectOptionList");
    qDBusRegisterMetaType<EffectOption>();
    qDBusRegisterMetaType<EffectOptionList>();
}

void EffectDbusInterface::printEffectStatus()
{
    PRINT_DBG("EffectDbusInterface::printEffectStatus()\n");
    for (const auto &effect : m_enabledEffects) {
        PRINT_DBG("EffectDbusInterface::printEffectStatus(): name: %s, value: %d\n", effect.name.toStdString().c_str(), effect.value);
    }
}

QString EffectDbusInterface::getCurrentMaintainMode()
{
    PRINT_DBG("EffectDbusInterface::getCurrentMaintainMode()\n");
    QDBusReply<QString> reply = m_maintainModeDbusInterface.call("GetBootMode");
    if (!reply.isValid()) {
        PRINT_NOTICE("getCurrentMaintainMode() failed, %s\n", reply.error().message().toStdString().c_str());
        return QString();
    }
    PRINT_DBG("current maintain mode is %s\n", reply.value().toStdString().c_str());
    return reply.value();
}

bool EffectDbusInterface::enableEffect(QString effect, bool status)
{
    PRINT_DBG("EffectDbusInterface::enableEffect effect: %s, status: %s", effect.toStdString().c_str(), status ? "TRUE" : "FALSE");
    QDBusReply<void> reply = m_dbusInterface.call("EnableEffect", effect, status);
    if (!reply.isValid()) {
        PRINT_NOTICE("enableEffect failed: %s\n", reply.error().message().toStdString().c_str());
        return false;
    }
    return true;
}

void EffectDbusInterface::restoreEffectsStatus()
{
    const QStringList keys = m_effectBootSettings.allKeys();
    for(const QString &k : keys) {
        if(!m_effectBootSettings.value(k).toBool()) {
            enableEffect(k, true);
        }
        m_effectBootSettings.remove(k);
    }
    m_effectBootSettings.sync();
}
