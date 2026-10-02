#ifndef KGLOBALACCELDBUSPROXY_H
#define KGLOBALACCELDBUSPROXY_H

#include <memory>
#include <QDBusAbstractInterface>
#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QList>
#include <QMap>
#include <QObject>
#include "kglobaldbustypes.h"

class KGlobalAccelDbusProxy : public QObject {
    Q_OBJECT
public:
    explicit KGlobalAccelDbusProxy(QObject *parent = nullptr);
    ~KGlobalAccelDbusProxy() override = default;

Q_SIGNALS:
    void kbdStatusChanged();

public Q_SLOTS:
    void blockGlobalShortcuts();
    void unblockGlobalShortcuts();
    QDBusPendingReply<KGlobalAccelShortcutInfoList> getSystemShortcuts() const;
    QDBusPendingReply<KGlobalAccelShortcutInfoList> getWindowShortcuts() const;

private:
    // kglobalaccel DBus服务名称
    const QString m_serviceName = "org.kde.kglobalaccel";

    // 系统快捷键配置
    const QString m_systemServicePath = "/component/ukui_settings_daemon";
    const QString m_systemServiceInterface = "org.kde.kglobalaccel.Component";
    const QString m_systemComponentName = "default";

    // 窗口快捷键配置
    const QString m_windowServicePath = "/component/kylin_wlcom";
    const QString m_windowServiceInterface = "org.kde.kglobalaccel.Component";
    const QString m_windowComponentName = "default";

    // 屏蔽全局快捷键接口
    const QString m_kglobalAccelServicePath = "/kglobalaccel";
    const QString m_kglobalAccelServiceInterface = "org.kde.KGlobalAccel";

    // DBus方法名称
    const QString m_allShortcutInfosMethod = "allShortcutInfos";
    const QString m_blockGlobalShortcutsMethod = "blockGlobalShortcuts";

    // DBus接口对象
    std::unique_ptr<QDBusInterface> m_kglobalAccelInterface = nullptr;
    std::unique_ptr<QDBusInterface> m_systemInterface = nullptr;
    std::unique_ptr<QDBusInterface> m_windowInterface = nullptr;
};

#endif // KGLOBALACCELDBUSPROXY_H
