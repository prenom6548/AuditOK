/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "sessionhelper.h"
#include <sys/file.h>
#include <pwd.h>
#include <QString>
#include <QDebug>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QDBusInterface>
#include <QProcess>
#include "xdgdesktopfile.h"
#include "login1helper.h"
#include "../lock-backend/gsettingshelper.h"
#include "freedesktophelper.h"

const static QString sessionService = QStringLiteral("org.gnome.SessionManager");
const static QString sessionPath = QStringLiteral("/org/gnome/SessionManager");
const static QString sessionInterface = QStringLiteral("org.gnome.SessionManager");

const static QString wlcomService = QStringLiteral("com.kylin.Wlcom");
const static QString wlcomPath = QStringLiteral("/com/kylin/Wlcom/View");
const static QString wlcomInterface = QStringLiteral("com.kylin.Wlcom.View");

QDBusArgument &InhibitInfo::operator<<(QDBusArgument &argument, const InhibitInfo::InhibitorInfo &mystruct)
{
    argument.beginStructure();
    argument << mystruct.name << mystruct.icon << mystruct.uid;
    argument.endStructure();
    return argument;
}

const QDBusArgument &InhibitInfo::operator>>(const QDBusArgument &argument, InhibitInfo::InhibitorInfo &mystruct)
{
    argument.beginStructure();
    argument >> mystruct.name >> mystruct.icon;
    if (!argument.atEnd()) {
        argument >> mystruct.uid;  // 新版本有这个字段
    } else {
        mystruct.uid = -1;  // 旧版本没有，设置默认值
    }
    qDebug() << "argument" << mystruct.name << mystruct.icon;
    argument.endStructure();
    return argument;
}

QDebug InhibitInfo::operator<<(QDebug stream, const InhibitInfo::InhibitorInfo &inhibitor)
{
    stream << "InhibitorInfo("
           << "name: " << inhibitor.name << ", "
           << "uid: " << inhibitor.uid << ", "
           << ")";
    return stream;
}

const QDBusArgument &SystemdInhibitor::operator>>(const QDBusArgument &argument, SystemdInhibitor::Inhibitor &mystruct)
{
    argument.beginStructure();
    argument >> mystruct.action >> mystruct.name >> mystruct.reason >> mystruct.mode >> mystruct.uid >> mystruct.pid;
    argument.endStructure();
    return argument;
}

QDBusArgument &SystemdInhibitor::operator<<(QDBusArgument &argument, const SystemdInhibitor::Inhibitor &mystruct)
{
    argument.beginStructure();
    argument << mystruct.action << mystruct.name << mystruct.reason << mystruct.mode << mystruct.uid << mystruct.pid;
    argument.endStructure();
    return argument;
}

SessionHelper::SessionHelper(QSharedPointer<Login1Helper> login1Helper, bool isLockStartupMode, QObject *parent)
    : QObject(parent), m_login1Helper(login1Helper)
{
    m_isLockStartupMode = isLockStartupMode;
    init();
}

void SessionHelper::init()
{
    if (!m_sessionInterface && FreedesktopHelper::getInstance()->NameHasOwner(sessionService)) {
        qDebug() << "Init session interface!";
        m_sessionInterface
            = new QDBusInterface(sessionService, sessionPath, sessionInterface, QDBusConnection::sessionBus(), this);
        m_wlcomInterface
            = new QDBusInterface(wlcomService, wlcomPath, wlcomInterface, QDBusConnection::sessionBus(), this);
        if (m_sessionInterface && m_sessionInterface->isValid()) {
            bool isLegacy = false;
            if (m_wlcomInterface && m_wlcomInterface->isValid()) {
                if (m_sessionInterface->metaObject()->indexOfMethod("GetAppWindows()") < 0 || m_wlcomInterface->metaObject()->indexOfMethod("CloseWindows()") < 0) {
                    qWarning() << "GetAppWindows method or CloseWindows method does not exist";
			        isLegacy = true;
                } else {
                    connect(m_sessionInterface, SIGNAL(windowAdd(QString, QString)), this, SIGNAL(windowAdd(QString, QString)));
                    connect(m_sessionInterface, SIGNAL(windowRemove(QString)), this, SIGNAL(windowRemove(QString)));
                }
            } else {
		        qDebug() << "wlcom interface is invalid!!!";
                isLegacy = true;
            }
            if (isLegacy != m_isLegacy) {
                m_isLegacy = isLegacy;
                Q_EMIT sessionLegacyChanged();
            }
            if (m_isLockStartupMode) {
                setAuthenticateCompeteState(SaverStatesForAbnormal::SaverISSTARTUP);
            } else {
                setAuthenticateCompeteState(SaverStatesForAbnormal::SaverUNDEFINE);
            }

            // 通知 gnome-session-manager 启动完成
            m_sessionInterface->call("startupfinished", "ukui-screensaver-backend", "startupfinished");
        }
    } else {
        connect(
            FreedesktopHelper::getInstance(),
            &FreedesktopHelper::serviceStatusChanged,
            this,
            &SessionHelper::onSessionDbusActiveChanged);
    }

    if (!QDBusConnection::sessionBus().connect(
            sessionService,
            sessionPath,
            sessionInterface,
            "logoutInhibitAdd",
            this,
            SLOT(onLogoutHibitedAddWatcherMessage()))) {
                qDebug() << "logoutInhibitAdd signal error:" << QDBusConnection::sessionBus().lastError();
            }

    if (!QDBusConnection::sessionBus().connect(
            sessionService,
            sessionPath,
            sessionInterface,
            "logoutInhibitRemove",
            this,
            SLOT(onLogoutHibitedRemoveWatcherMessage()))) {
                qDebug() << "logoutInhibitRemove signal error:" << QDBusConnection::sessionBus().lastError();
            }
}

void SessionHelper::onSessionDbusActiveChanged(const QString &strServerName, bool isActive)
{
    if (strServerName == sessionService && isActive) {
        qDebug() << "Init session interface!";
        if (!m_sessionInterface) {
            m_sessionInterface = new QDBusInterface(
                sessionService, sessionPath, sessionInterface, QDBusConnection::sessionBus(), this);
        }
        if (!m_wlcomInterface) {
            m_wlcomInterface = new QDBusInterface(
                wlcomService, wlcomPath, wlcomInterface, QDBusConnection::sessionBus(), this);
        }
        // 检查接口是否有效
        if (m_sessionInterface && m_sessionInterface->isValid()) {
            qWarning() << "session interface is invalid";
            // 使用introspection检查方法是否存在
            bool isLegacy = false;
            if (m_wlcomInterface && m_wlcomInterface->isValid()) {
                if (m_sessionInterface->metaObject()->indexOfMethod("GetAppWindows()") < 0 || m_wlcomInterface->metaObject()->indexOfMethod("CloseWindows()") < 0) {
                    qWarning() << "GetAppWindows method or CloseWindows method does not exist";
                    isLegacy = true;
                } else {
                    connect(m_sessionInterface, SIGNAL(windowAdd(QString, QString)), this, SIGNAL(windowAdd(QString, QString)));
                    connect(m_sessionInterface, SIGNAL(windowRemove(QString)), this, SIGNAL(windowRemove(QString)));
                }
            } else {
                qDebug() << "wlcom interface is invalid!!!";
                isLegacy = true;
            }
            if (isLegacy != m_isLegacy) {
                m_isLegacy = isLegacy;
                Q_EMIT sessionLegacyChanged();
            }
            if (m_isLockStartupMode) {
                setAuthenticateCompeteState(SaverStatesForAbnormal::SaverISSTARTUP);
            } else {
                setAuthenticateCompeteState(SaverStatesForAbnormal::SaverUNDEFINE);
            }
        }
        disconnect(
            FreedesktopHelper::getInstance(),
            &FreedesktopHelper::serviceStatusChanged,
            this,
            &SessionHelper::onSessionDbusActiveChanged);
    }
}

SessionHelper::~SessionHelper() {}

bool SessionHelper::canAction(Action action)
{
    // 以下为代码结构调整
    if (m_sessionInterface) {
        QString command;
        switch (action) {
            case PowerSwitchUser:
                command = QLatin1String("canSwitch");
                break;
            case PowerHibernate:
                command = QLatin1String("canHibernate");
                break;
            case PowerSuspend:
                command = QLatin1String("canSuspend");
                break;
            case PowerMonitorOff:
                command = QLatin1String("canLockscreen");
                break;
            case PowerLogout:
                command = QLatin1String("canLogout");
                break;
            case PowerReboot:
                command = QLatin1String("canReboot");
                break;
            case PowerShutdown:
                command = QLatin1String("canPowerOff");
                break;
            default:
                break;
        }

        if (!m_sessionInterface->isValid()) {
            qWarning() << "dbusCall: Session QDBusInterface is invalid";
            return false;
        }

        QDBusReply<bool> testReply = m_sessionInterface->call(QLatin1String("canLockscreen"));
        if (!testReply.isValid()) {
            // 解决老版本升级到新版本接口不兼容的问题，在session接口不存在的情况下，调用systemd的接口
            QDBusError error = testReply.error();
            Login1Helper *m_login1Helper = new Login1Helper(this);
            if (error.type() == QDBusError::UnknownMethod) {
                qInfo() << "updating ! old ukui-session dose not have canAction method";
                if (action == PowerLogout || action == PowerMonitorOff) {
                    return true;
                }
                if (m_login1Helper) {
                    return m_login1Helper->canAction(action);
                }
            }
            qInfo() << "dbus error";
            return false;
        }
        QDBusReply<bool> reply = m_sessionInterface->call(command);
        qInfo() << "command = " << command << "reply = " << m_sessionInterface->call(command);
        return reply.value();
    } else if (m_login1Helper) {
        if (action == PowerLogout || action == PowerMonitorOff) {
            return true;
        }
        return m_login1Helper->canAction(action);
    } else {
        return false;
    }
}

void SessionHelper::onLogoutHibitedAddWatcherMessage()
{
    qDebug() << "recv logoutinhibitAdd";
    Q_EMIT blockLogoutInhibitedChanged();
}

void SessionHelper::onLogoutHibitedRemoveWatcherMessage()
{
    qDebug() << "recv logoutinhibitRemove";
    Q_EMIT blockLogoutInhibitedChanged();
}

bool SessionHelper::doAction(const QString &powerManagerfunc)
{
    qDebug() << "doAction = " << powerManagerfunc << m_inClosePhase;
    if (m_sessionInterface) {
        if (!m_inClosePhase && (powerManagerfunc == "Reboot" || powerManagerfunc == "PowerOff" || powerManagerfunc == "Logout") && !m_isLegacy)
        {
            //第一次调用reboot、poweroff、logout先调用关闭窗口接口并展示未关闭窗口列表，不执行电源操作
            m_inClosePhase = true;
            QString command;
            if (powerManagerfunc == "Reboot") {
                command = QLatin1String("reboot");
            } else if (powerManagerfunc == "PowerOff") {
                command = QLatin1String("powerOff");
            } else if (powerManagerfunc == "Logout") {
                command = QLatin1String("logout");
            } else {
                return false;
            }
            QDBusMessage mes;
            mes = m_sessionInterface->call("closeWindows", command);
        } else {
            //1、不需要展示未关闭窗口列表的电源操作  2、当前未关闭的窗口已经被关闭，不存在未关闭的窗口，直接执行电源操作  3、session未升级情况
            if (!playShutdownMusic(powerManagerfunc)) {
                doPowerManager(powerManagerfunc);
            }
        }
    } else if (m_login1Helper) {
        m_login1Helper->setPowerManager(powerManagerfunc);
    }

    return true;
}

void SessionHelper::setClosePhase(bool inClosePhase)
{
    m_inClosePhase = inClosePhase;
    if (!m_inClosePhase) {
        m_sessionInterface->call("execComplete", "cancel");
    }
}

bool SessionHelper::getIsLegacy()
{
    return m_isLegacy;
}

void SessionHelper::setAuthenticateCompeteState(int state)
{
    if (m_sessionInterface) {
        m_sessionInterface->call("setAuthenticateCompeteState", state);
    } else {
        qInfo() << " session Interface not init !!! " ;
    }
}

int SessionHelper::getAuthenticateCompeteState()
{
    int state = SaverStatesForAbnormal::SaverAUTHCOMPLETE;
    if (m_sessionInterface) {
        QDBusReply<int> status = m_sessionInterface->call("getAuthenticateCompeteState");
        state = status;
    } else {
        qInfo() << " session Interface not init !!! " ;
    }
    return state;
}

void SessionHelper::doPowerManager(const QString &powerManagerfunc)
{
    if (m_sessionInterface) {
        QString command;
        if (powerManagerfunc == "Hibernate") {
            command = QLatin1String("hibernate");
        } else if (powerManagerfunc == "Suspend") {
            command = QLatin1String("suspend");
        } else if (powerManagerfunc == "Reboot") {
            command = QLatin1String("reboot");
        } else if (powerManagerfunc == "PowerOff") {
            command = QLatin1String("powerOff");
        } else if (powerManagerfunc == "Logout") {
            command = QLatin1String("logout");
        } else if (powerManagerfunc == "SwitchUser") {
            command = QLatin1String("switchUser");
        } else {
            return;
        }
        QDBusMessage mes;
        if (!m_isLegacy) {
            if ((powerManagerfunc == "Reboot" || powerManagerfunc == "PowerOff" || powerManagerfunc == "Logout")) {
                QDBusMessage mes;
                qDebug() << "call session execComplete!";
                mes = m_sessionInterface->call("execComplete", command);
                if (!(mes.errorName().isEmpty()) && m_login1Helper) {
                    // 本来应该判断错误类别，考虑到运行效率，不做该判断
                    m_login1Helper->setPowerManager(powerManagerfunc);
                }
            } else {
                QDBusMessage mes;
                mes = m_sessionInterface->call(command);
                if (!(mes.errorName().isEmpty())) {
                    if (!(mes.errorName().isEmpty()) && m_login1Helper) {
                        // 本来应该判断错误类别，考虑到运行效率，不做该判断
                        m_login1Helper->setPowerManager(powerManagerfunc);
                    }
                }
            }
        } else {
            mes = m_sessionInterface->call(command);
            if (!(mes.errorName().isEmpty()) && m_login1Helper) {
                // 本来应该判断错误类别，考虑到运行效率，不做该判断
                m_login1Helper->setPowerManager(powerManagerfunc);
            }
        }
    } else if (m_login1Helper) {
        m_login1Helper->setPowerManager(powerManagerfunc);
    }
}

void SessionHelper::getSystemdInhibitor(QString type, QVector<InhibitInfo::InhibitorInfo> &inhibitorVec)
{
    qDBusRegisterMetaType<SystemdInhibitor::Inhibitor>();

    QDBusInterface loginInterface("org.freedesktop.login1", "/org/freedesktop/login1",
                                  "org.freedesktop.login1.Manager", QDBusConnection::systemBus());

    if (loginInterface.isValid()) {
        qDebug() << "create interface success";
    }

    QDBusMessage result = loginInterface.call("ListInhibitors");
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    const QDBusArgument &dbusArgs = first.value<QDBusArgument>();

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        SystemdInhibitor::Inhibitor inhibitor;
        dbusArgs >> inhibitor;
        if (inhibitor.mode == QString("block") && inhibitor.action.contains(type)) {
                InhibitInfo::InhibitorInfo inhibitInfo;
                findNameAndIcon(inhibitor.name, inhibitInfo);

                if (inhibitInfo.name.isEmpty()) {
                    inhibitInfo.name = inhibitor.name;
                }

                inhibitorVec.push_back(inhibitInfo);
        }
    }

    dbusArgs.endArray();
}         

void SessionHelper::findNameAndIcon(QString &inhibitorName, InhibitInfo::InhibitorInfo &inhibitor)
{
    QString icon;
    QString name;
    QStringList desktop_paths = { "/usr/share/applications", "/etc/xdg/autostart" };

    //逻辑问题，规避文件管理器这种多个desktop中exec字段均为peony的情况
    if ("peony" == inhibitorName) {
        XdgDesktopFile desktopFile;
        desktopFile.load("/usr/share/applications/peony.desktop");
        icon = desktopFile.value("Icon", QVariant("application-x-desktop")).toString();
//                name = getAppLocalName(fi.absoluteFilePath());
        //根据系统的本地语言设置获取对应的名称,改为直接使用xdg的接口
        name = desktopFile.localizedValue("Name").toString();
        inhibitor.name = name;
        inhibitor.icon = icon;
        return;
    }

    for (const QString &dirName : const_cast<const QStringList&>(desktop_paths)) {
        QDir dir(dirName);
        if (!dir.exists()) {
            continue;
        }

        const QFileInfoList files = dir.entryInfoList(QStringList(QLatin1String("*.desktop")), QDir::Files | QDir::Readable);
        for (const QFileInfo &fi : files) {
            XdgDesktopFile desktopFile;
            desktopFile.load(fi.absoluteFilePath());
            //不再使用basename判断，使用命令判断
            //QString base = fi.baseName();
            if (desktopFile.expandExecString().isEmpty()) continue;
            QString command = desktopFile.expandExecString().takeFirst();
            QStringList list = command.split("/");
            QString exec = list.last();
            if (exec == inhibitorName) {
                icon = desktopFile.value("Icon", QVariant("application-x-desktop")).toString();
//                name = getAppLocalName(fi.absoluteFilePath());
                //根据系统的本地语言设置获取对应的名称,改为直接使用xdg的接口
                name = desktopFile.localizedValue("Name").toString();

                inhibitor.name = name;
                inhibitor.icon = icon;

                break;
            }
        }
    }
}

QVector<InhibitInfo::InhibitorInfo> SessionHelper::getLockCheckStatus(QString type)
{
    if (m_sessionInterface) {
        qDBusRegisterMetaType<InhibitInfo::InhibitorInfo>();

        QVector<InhibitInfo::InhibitorInfo> resVec;

        if (!m_sessionInterface->isValid()) {
            qDebug() << "session interface invalid!";
        }

        QDBusMessage result;
        if (!m_isLegacy) {
            result = m_sessionInterface->call("ListInhibitorAndUid", QVariant(type));
        } else {
            result = m_sessionInterface->call("ListInhibitor", QVariant(type));
        }
        QList<QVariant> outArgs = result.arguments();
        QVariant first = outArgs.at(0);
        const QDBusArgument &dbusArgs = first.value<QDBusArgument>();

        dbusArgs.beginArray();
        while (!dbusArgs.atEnd()) {
            InhibitInfo::InhibitorInfo inhibtor;
            dbusArgs >> inhibtor;
            resVec.push_back(inhibtor);
        }
        dbusArgs.endArray();

        return resVec;
    } else {
        QVector<InhibitInfo::InhibitorInfo> resVec;
        getSystemdInhibitor(type, resVec);

        return resVec;
    }
}

QVector<WindowInfo::WindowInfo> SessionHelper::getAppWindows()
{
    if (!m_sessionInterface) {
        qWarning() << "session interface is null";
        return QVector<WindowInfo::WindowInfo>();
    }

    if (!m_sessionInterface->isValid()) {
        qWarning() << "session interface invalid!";
        return QVector<WindowInfo::WindowInfo>();
    }

    QDBusMessage result = m_sessionInterface->call("GetAppWindows");

    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "DBus call failed:" << result.errorMessage();
        return QVector<WindowInfo::WindowInfo>();
    }

    QList<QVariant> outArgs = result.arguments();
    if (outArgs.isEmpty()) {
        qWarning() << "No arguments returned";
        return QVector<WindowInfo::WindowInfo>();
    }

    // 直接转换为QStringList
    QVector<WindowInfo::WindowInfo> windows;
    QStringList windowList = outArgs.first().value<QStringList>();
    for (auto window : windowList) {
        int splitPos = window.indexOf('_');
        QString wid, desktopOrAppId, icon, name;
        if (splitPos > 0) {
            wid = window.left(splitPos);
            desktopOrAppId = window.mid(splitPos + 1);
            if (QFile::exists(desktopOrAppId) && desktopOrAppId.endsWith(".desktop")) {
                XdgDesktopFile desktopFile;
                desktopFile.load(desktopOrAppId);
                icon = desktopFile.value("Icon", QVariant("application-x-desktop")).toString();
                name = desktopFile.localizedValue("Name").toString();
            } else {
                icon = "application-x-desktop";
                name = desktopOrAppId;
            }
        } else {
            continue;
        }

        WindowInfo::WindowInfo info;
        info.wid = wid;
        info.name = name;
        info.icon = icon;
        windows.append(info);
    }
    qDebug() << "Got" << windowList.size() << "windows";

    return windows;
}

bool SessionHelper::playShutdownMusic(const QString &powerManagerfunc)
{
    // up_to_time and soundplayer can not be define out of this if().
    // otherwise run ukui-session-tools --suspend with segmente error.
    // because they will be delate at the end of the playShutdownMusic().
    bool play_music = false;
    QDBusMessage msg;
    GSettingsHelper *gsettingsHelper = new GSettingsHelper(this);
    if (powerManagerfunc == "Reboot" || powerManagerfunc == "PowerOff") {
        play_music = gsettingsHelper->GetSessionConf(KEY_SESSION_POWEROFF_MUSIC).toBool();
    } else if (powerManagerfunc == "Logout") {
        play_music = gsettingsHelper->GetSessionConf(KEY_SESSION_LOGOUT_MUSIC).toBool();
        if (m_sessionInterface) {
            msg = m_sessionInterface->call("emitStartLogout");
            if (!msg.errorName().isEmpty()) {
                qWarning() << "Dbus error: " << msg;
            }
        }
    } else {
        if (powerManagerfunc == "SwitchUser" && m_sessionInterface) {
            msg = m_sessionInterface->call("emitPrepareForSwitchuser");
            if (!msg.errorName().isEmpty()) {
                qWarning() << "Dbus error: " << msg;
            }
        }
        return false;
    }

    if (play_music) {
        if (!m_timerPlayMusic) {
            m_timerPlayMusic = new QTimer();
            m_timerPlayMusic->setSingleShot(true);
        }
        disconnect(m_timerPlayMusic, nullptr, this, nullptr);
        connect(m_timerPlayMusic, &QTimer::timeout, this, [&, this, powerManagerfunc]() {
            this->doPowerManager(powerManagerfunc);
            return true;
        });
        QString xdg_session_type = qgetenv("XDG_SESSION_TYPE");
        if (powerManagerfunc == "Reboot" || powerManagerfunc == "PowerOff") {
            if (xdg_session_type == "wayland") {
                QProcess::startDetached("paplay /usr/share/ukui/ukui-session-manager/shutdown.wav");
            } else {
                QProcess::startDetached("aplay /usr/share/ukui/ukui-session-manager/shutdown.wav");
            }
            m_timerPlayMusic->start(5000);
        } else if (powerManagerfunc == "Logout") {
            if (xdg_session_type == "wayland") {
                QProcess::startDetached("paplay /usr/share/ukui/ukui-session-manager/logout.wav");
            } else {
                QProcess::startDetached("aplay /usr/share/ukui/ukui-session-manager/logout.wav");
            }
            m_timerPlayMusic->start(2000);
        } else {
            qDebug() << "error num";
            return false;
        }
    }
    return play_music;
}
