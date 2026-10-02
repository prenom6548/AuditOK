/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "settings.h"
#include "glinfo-query.h"
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <mutex>
#include <QGuiApplication>
#include <QJsonObject>
#include <QJsonParseError>
#include <QFile>

static UkuiQuick::Settings *g_settings = nullptr;
static std::once_flag onceFlag;
static std::once_flag rendererFlag;

const QString USD_SERVICE = QStringLiteral("org.ukui.SettingsDaemon");
const QString USD_GLOBAL_SIGNAL_PATH = QStringLiteral("/GlobalSignal");
const QString USD_GLOBAL_SIGNAL_INTERFACE = QStringLiteral("org.ukui.SettingsDaemon.GlobalSignal");
const QString UKUI_LITE_CHANGED = QStringLiteral("UKUILiteChanged");
static const QString STATUS_MANAGER_SERVICE = QStringLiteral("com.kylin.statusmanager.interface");
static const QString STATUS_MANAGER_PATH = QStringLiteral("/");
static const QString STATUS_MANAGER_INTERFACE = QStringLiteral("com.kylin.statusmanager.interface");
static const QString MODE_CHANGE_SIGNAL = QStringLiteral("mode_change_signal");
static const QStringList SPECIAL_RENDERER_DEVICES {"ZX C-1190", "Phytium FTG340"};

namespace UkuiQuick {
class SettingsPrivate : public QObject
{
    Q_OBJECT
public:
    explicit SettingsPrivate(QObject *parent = nullptr);
    void updateLiteLevel(const QVariantMap &map);
    void updateTabletMode(bool tabletMode);
    void refresh();

    QString m_liteAnimation = QStringLiteral("normal");
    QString m_liteFunction = QStringLiteral("normal");
    bool m_isTabletMode = false;
    static bool s_verticalSync;
    static bool s_isHardWareRendering;
    QDBusInterface *m_usdGlobalSignalIface = nullptr;
    QDBusInterface *m_statusManagerIface = nullptr;
    Settings *q = nullptr;
};

bool SettingsPrivate::s_isHardWareRendering = true;
bool SettingsPrivate::s_verticalSync = true;

SettingsPrivate::SettingsPrivate(QObject *parent) : QObject(parent)
{
    q = qobject_cast<Settings *>(parent);
    refresh();
}

void SettingsPrivate::updateLiteLevel(const QVariantMap &map)
{
    if(map.find(QStringLiteral("ukui-lite-animation")) != map.constEnd()) {
        m_liteAnimation = map.value(QStringLiteral("ukui-lite-animation")).toString();
        Q_EMIT q->liteAnimationChanged();
    }
    if(map.find(QStringLiteral("ukui-lite-function")) != map.constEnd()) {
        m_liteFunction = map.value(QStringLiteral("ukui-lite-function")).toString();
        Q_EMIT q->liteFunctionChanged();
    }
}

void SettingsPrivate::updateTabletMode(bool tabletMode)
{
    if(m_isTabletMode != tabletMode) {
        m_isTabletMode = tabletMode;
        Q_EMIT q->tabletModeChanged();
    }
}

void SettingsPrivate::refresh()
{
    //ukui lite
    if(!m_usdGlobalSignalIface) {
        m_usdGlobalSignalIface = new QDBusInterface(USD_SERVICE, USD_GLOBAL_SIGNAL_PATH, USD_GLOBAL_SIGNAL_INTERFACE,
                                                    QDBusConnection::sessionBus(), this);
    }
    if(!m_usdGlobalSignalIface->isValid()) {
        qWarning() << "Init USD interface error:" << m_usdGlobalSignalIface->lastError();
    } else {
        QDBusPendingCall animationCall = m_usdGlobalSignalIface->asyncCall(QStringLiteral("getUKUILiteLevel"));
        connect(new QDBusPendingCallWatcher(animationCall, this), &QDBusPendingCallWatcher::finished, [&](QDBusPendingCallWatcher *call = nullptr){
            QDBusPendingReply<QVariantMap> reply = *call;
            if (reply.isError()) {
                qWarning() << "USD interface error" << reply.error();
            } else {
                updateLiteLevel(reply.value());
            }
            call->deleteLater();
        });
        if(!QDBusConnection::sessionBus().connect(USD_SERVICE, USD_GLOBAL_SIGNAL_PATH, USD_GLOBAL_SIGNAL_INTERFACE,
                                                  UKUI_LITE_CHANGED,
                                                  this, SLOT(updateLiteLevel(const QVariantMap &map)))) {
            qWarning() << "USD interface error, connect " << UKUI_LITE_CHANGED << "failed!";
        }
    }
    //tablet mode
    if(!m_statusManagerIface) {
        m_statusManagerIface = new QDBusInterface(STATUS_MANAGER_SERVICE, STATUS_MANAGER_PATH, STATUS_MANAGER_INTERFACE,
                                                  QDBusConnection::sessionBus(), this);
    }
    if(!m_statusManagerIface->isValid()) {
        qWarning() << "Init kylin status manager interface error:" << m_statusManagerIface->lastError();
    } else {
        QDBusPendingCall tabletCall = m_statusManagerIface->asyncCall(QStringLiteral("get_current_tabletmode"));
        connect(new QDBusPendingCallWatcher(tabletCall, this), &QDBusPendingCallWatcher::finished, [&](QDBusPendingCallWatcher *call = nullptr){
            QDBusPendingReply<bool> reply = *call;
            if (reply.isError()) {
                qWarning() << "Kylin status manager interface error" << reply.error();
            } else {
                updateTabletMode(reply.value());
            }
            call->deleteLater();
        });
        if(!QDBusConnection::sessionBus().connect(STATUS_MANAGER_SERVICE, STATUS_MANAGER_PATH, STATUS_MANAGER_INTERFACE,
                                                  MODE_CHANGE_SIGNAL,
                                                  this, SLOT(updateTabletMode(bool)))) {
            qWarning() << "Kylin status manager interface error, connect " << MODE_CHANGE_SIGNAL << "failed!";
        }
    }
}

Settings *Settings::instance()
{
    std::call_once(onceFlag, [ & ] {
        g_settings = new Settings();
    });
    return g_settings;
}

UkuiQuick::Settings::Settings(QObject *parent) : QObject(parent), d(new SettingsPrivate(this))
{
    refresh();
}

QString UkuiQuick::Settings::liteAnimation()
{
    return d->m_liteAnimation;
}

QString UkuiQuick::Settings::liteFunction()
{
    return d->m_liteFunction;
}

bool Settings::tabletMode()
{
    return d->m_isTabletMode;
}

void Settings::refresh()
{
    d->refresh();
}

QString Settings::platformName()
{
    return QGuiApplication::platformName();
}

/*!
    获取渲染设备信息，是否支持垂直同步和硬件加速。
    程序启动之前会初始化完成对应信息，需要在 QGuiAppliction 实例化之后的操作不要放在当前函
    首选配置文件获取禁用垂直同步的渲染设备列表，没有配置文件则使用默认列表 SPECIAL_RENDERER_DEVICES
    配置文件：/etc/ukui/ukui-quick/disable-vertical-sync.json
    格式：
    \code
    {
        "RendererDevice":["111111111", "llvmpipe (LLVM 17.0.6, 256 bits)"]
    }
    \endcode
*/

bool Settings::getRendererInfo()
{
    auto getConfig = [](const QString &filepath) -> QJsonObject {
        QFile file(filepath);
        if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
            return QJsonObject();
        }

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
        file.close();
        if (error.error != QJsonParseError::NoError) {
            return QJsonObject();
        }

        if (!doc.isObject()) {
            return QJsonObject();
        }
        return doc.object();
    };

    GLInfoQuery glInfoQuery;
    const QString renderDevice = glInfoQuery.glRenderDevice();
    auto config = getConfig(QStringLiteral("/etc/ukui/ukui-quick/disable-vertical-sync.json"));
    if (!config.isEmpty() && config.contains(QStringLiteral("RendererDevice"))) {
        const QStringList& specialRendererDevice = config.value(QStringLiteral("RendererDevice")).toVariant().toStringList();
        SettingsPrivate::s_verticalSync = !specialRendererDevice.contains(renderDevice);
        qWarning() << "The rendering device comes from the configuration file. RendererDevice : " << specialRendererDevice;
    } else {
        SettingsPrivate::s_verticalSync = !SPECIAL_RENDERER_DEVICES.contains(renderDevice);
    }

    SettingsPrivate::s_isHardWareRendering = glInfoQuery.isHardwareRendering();
    return true;
};

bool Settings::verticalSync()
{
    std::call_once(rendererFlag, [ & ] {
        Settings::getRendererInfo();
    });
    return SettingsPrivate::s_verticalSync;
}

bool Settings::isHardWareRendering()
{
    std::call_once(rendererFlag, [ & ] {
        Settings::getRendererInfo();
    });
    return SettingsPrivate::s_isHardWareRendering;
}
}
#include "settings.moc"