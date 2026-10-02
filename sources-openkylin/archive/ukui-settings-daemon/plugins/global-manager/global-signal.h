#ifndef GSETTINGSCONFIG_H
#define GSETTINGSCONFIG_H

#include <QObject>
#include <QMetaType>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusContext>
#include <QDBusMetaType>
#include <QDBusInterface>
#include <QDBusPendingReply>

#include "QGSettings/qgsettings.h"
#include "dbus-security.h"
#include "clib-syslog.h"
#include "qdbus-login1-abstract-interface.h"
#include "usd_base_class.h"

struct UKUILite{
    QString function;
    QString animation;
};


Q_DECLARE_METATYPE(UKUILite)

inline QDBusArgument &operator << (QDBusArgument &argument, const UKUILite &lite)
{
    argument.beginStructure();
    argument << lite.function;
    argument << lite.animation;
    argument.endStructure();

    return argument;
}

inline const QDBusArgument &operator >> (const QDBusArgument &argument, UKUILite &lite) {
    argument.beginStructure();
    argument >> lite.function;
    argument >> lite.animation;
    argument.endStructure();
    return argument;
}

class GlobalSignal: public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","org.ukui.SettingsDaemon.GlobalSignal")

public:
    GlobalSignal(QObject *parent = nullptr);
    ~GlobalSignal();
Q_SIGNALS:

   /**
    * @brief userActive
    * @return
    * 用户登录，登出状态。
    */
    void Active(bool userState);

    /**
     * @brief getUKUILiteLevel
     * @param liteMode
     * UKUILite 模式
     */
//    void UKUILiteChanged(UKUILite liteMode);
    void UKUILiteChanged(QMap<QString, QVariant> liteMode);
public Q_SLOTS:

    /**
     * @brief sendUserActiveSignal
     */
    void sendUserActiveSignal(QString interface, QVariantMap qvarMap, QStringList strList);

    /**
     * @brief hadLightSensor
     * @return
     */
    bool isPresenceLightSensor();

    /**
     * @brief 支持gamma
     * @return
     */
    bool isSupportGamma();

    /**
     * @brief getShutdownDisableOptions
     * @return
     * 获取关机页面需要屏蔽的选项
     */
    QStringList getShutdownDisableOptions();

    /**
     * @brief isHidePoweroffUi
     * @return
     */
    bool isHidePoweroffUi();

    /**
     * @brief getUKUILiteLevel
     * @param function 功能等级
     * @param animation 动效等级
     * @return
     */
    QMap<QString, QVariant> getUKUILiteLevel();

    /**
     * @brief getUKUILiteFunction
     * @return
     * 获取ukuilite功能级别
     */
    QString getUKUILiteFunction();

    /**
     * @brief getUKUILiteFunction
     * @return
     * 获取ukuilite动效级别
     */
    QString getUKUILiteAnimation();

private Q_SLOTS:
    /**
     * @brief doGMsettinsChange
     * @param key
     * 同步全局电源选项配置。
     */
    void doGMsettinsChange(QString key);

private:
    /**
     * @brief connectUserActiveSignal
     */
    void connectUserActiveSignal();

    void connectUserActiveSignalWithPath(QString path);

    void connectUserLogin1Signal();
    /**
     * @brief getGlobalConfig
     * @param schema
     * @param key
     * @return
     */
    QVariant getGlobalConfig(QString schema, QString key);
    /**
     * @brief getShutDownDisableOptionInGsettings
     * @return
     */
    QList<QString> getShutDownDisableOptionInGsettings();
    /**
     * @brief getShutDownDisableOptionFromGlobalManager
     * @return
     */
    QStringList getShutDownDisableOptionFromGlobalManager();


    QMap<QString, QVariant> getUKUILite();

    bool getHidePowerUiFromGlobalManager();

private:
    DBusLogin1Interface *m_plogin1 = nullptr;
    QDBusInterface *m_plogin2 = nullptr;
    QDBusInterface *m_ploginWithPath  = nullptr;
    QGSettings *m_pGMsettings = nullptr;
    const QList<QString> m_allOptions = {"switchuser","hibernate","suspend","lockscreen","logout","restart","shutdown"};

};

#endif // GSETTINGSCONFIG_H
