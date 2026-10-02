#ifndef DISPLAYEFFECTDBUSINTERFACE_H
#define DISPLAYEFFECTDBUSINTERFACE_H

#include <QDBusInterface>
#include <QDBusReply>
#include <QVariant>
#include <QString>
#include <QSettings>
#include <QMap>

// 定义效果选项结构体，对应返回的 (String, UInt32, Boolean)
struct EffectOption {
    QString name;        // 效果名称 (String)
    quint32 value;       // 效果值 (UInt32)
    bool enabled;        // 是否启用 (Boolean)
    
    // 需要为Qt的元对象系统提供支持
    EffectOption() : name(""), value(0), enabled(false) {}
    EffectOption(const QString &n, quint32 v, bool e) : name(n), value(v), enabled(e) {}
};

// 注册结构体到Qt的元对象系统
Q_DECLARE_METATYPE(EffectOption)
typedef QList<EffectOption> EffectOptionList;
Q_DECLARE_METATYPE(EffectOptionList)


class EffectDbusInterface {
public:
    explicit EffectDbusInterface();
    void initActiveEffects();
    void suspendEffectEffects(std::vector<std::string> &filterList);
    void resumeEffectEffects();

    void registerMetaType();
    void printEffectStatus();
    QString getCurrentMaintainMode();

private:
    QDBusInterface m_dbusInterface;
    QDBusInterface m_maintainModeDbusInterface;
    bool enableEffect(QString effect, bool status);
    QList<EffectOption> m_enabledEffects;
    void restoreEffectsStatus();
    QSettings m_effectBootSettings;

};

#endif