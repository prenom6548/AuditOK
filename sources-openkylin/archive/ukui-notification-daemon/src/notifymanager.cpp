/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */

#include "notifymanager.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include <dconf/dconf.h>
#ifdef __cplusplus
}
#endif

NotifyManager::NotifyManager(QObject *parent) : QObject(parent)
{
    m_pTranslator = new QTranslator;
    if (m_pTranslator->load(QLocale(), QLatin1String("ukui-notification-daemon"), QLatin1String("_"), QLatin1String("/usr/share/ukui-notification-daemon"))) {
        QApplication::installTranslator(m_pTranslator);
    } else {
        qDebug() << "cannot load translator " << QLocale::system().name() << ".qm!";
    }

    m_pTopWidget = new topTransparentWidget(this);
    m_psqlInfoData = new sqlInfoData(this);

    connect(m_pTopWidget, &topTransparentWidget::notificationClosed, this, &NotifyManager::popupItemWidgetDismissed);
    connect(m_pTopWidget, &topTransparentWidget::actionInvoked, this, &NotifyManager::popupItemWidgetActionInvoked);

    getAppinfo();
    registerAsService();
    initGsettingValue();
}

NotifyManager::~NotifyManager()
{
    if (m_pNodisturbModeGsetting) {
        delete m_pNodisturbModeGsetting;
        m_pNodisturbModeGsetting = NULL;
    }
    if (m_pPopupWidgetModeGsetting) {
        delete m_pPopupWidgetModeGsetting;
        m_pPopupWidgetModeGsetting = NULL;
    }
    if (m_pControlNotifyMainGsetting) {
        delete m_pControlNotifyMainGsetting;
        m_pControlNotifyMainGsetting = NULL;
    }
}

void NotifyManager::CloseNotification(uint id)
{
    m_pTopWidget->closePopupWidgetAccordId(id);
    emit NotificationClosed(id, NotifyManager::CLOSEDBYAPP);
    nextShowAction();
    return;
}

QStringList NotifyManager::GetCapabilities()
{
    QStringList result;
    result << "action-icons"
           << "actions"
           << "body"
           << "body-hyperlinks"
           << "body-markup";
    return result;
}

void NotifyManager::GetServerInformation(QString &name, QString &vendor, QString &version, QString &specVersion)
{
    QString serverVersion = "UKUI " + QString::number(SERVERVERSION, 10, 1);
    QString protocolVersion = "Desktop Notifications Specification " + QString::number(PROTOCOLVERSION, 10, 1);
    name = QString("ukui-notification-daemon");
    vendor = QString("Kylin OS");
    version = serverVersion;
    specVersion = protocolVersion;
    return;
}

uint NotifyManager::Notify(
    const QString &appName,
    uint replacesId,
    const QString &appIcon,
    const QString &summary,
    const QString &body,
    const QStringList &actions,
    const QVariantMap hints,
    int expireTimeout)
{
#ifdef QT_DEBUG
    qInfo() << "a new Notify1:"
            << "appName2:" + appName << "replaceID3:" + QString::number(replacesId) << "appIcon:" + appIcon
            << "summary:" + summary << "body:" + body << "actions:" << actions << "hints:" << hints
            << "expireTimeout:" << expireTimeout;
#endif

    m_pTopWidget->initScreenNotifications();

    //分配/获取 popupWidget ID，实例化 NotifyReceiveInfo 信息类
    uint popupWidgetID = 0;
    uint id = m_pTopWidget->isPopupExist(appName);
    if(id == 0) {
        popupWidgetID = ++m_counter;
    } else {
        popupWidgetID = id;
    }

#if 0
    // -1 默认时间 0 消息常驻 其他： 显示相应时长
    // -_-
    // 判断是否为控制面板内的应用，若为应用内的应用看设置项
    if (getNotifyShowStyle(appName) ==  NOTIFYCATION_SHOW_STYLE_MUTATIVE){
        // 显示在屏幕右上角，会自动消失，消失时间由应用自己去确定
        if (expireTimeout == 0) {
            expireTimeout = -1;
            // 此判断为了兼容，某些应用在参数中设置了常驻信息，但控制面板设置了自动消失选项，优先级为控制面板
        }

    } else if (getNotifyShowStyle(appName) ==  NOTIFYCATION_SHOW_STYLE_ALWAYS) {
        // 会一直显示在右上角，直到被手动关闭
        expireTimeout = 0;
    }
#endif

    NotifyReceiveInfo *notifyInfo = new NotifyReceiveInfo(
        appName,
        QString::number(popupWidgetID),
        appIcon,
        summary,
        body,
        actions,
        hints,
        QString::number(QDateTime::currentMSecsSinceEpoch()),
        QString::number(replacesId),
        QString::number(expireTimeout),
        this);

    if (PLATFORM::g_platformType == PLATFORM::V101) {
        // 控制面板通知总开关
        if (!m_bNotifyMainSwitch) {
            qInfo() << "Mask all messages,No pop-up window";
            quitHandle();
            return 0;
        }
        // 应用通知开关
        bool enable = getControlCentorAppNotify(appInfoMap[appName]);
        if (!enable) {
            qInfo() << "Mask " << appName << " messages,No pop-up window";
            quitHandle();
            return 0;
        }
    }

    //检测通知展示开关是开启
    if(!notifySwitch(appName)) {
        showToSidebar(notifyInfo);
        quitHandle();
        return 0;
    }

    // 在不显示消息并且不在锁屏状态时，走这个逻辑
    if ((getNotifyShowStyle(appInfoMap[appName]) ==  NOTIFYCATION_SHOW_STYLE_NONE)) {
        // 不显示，直接收纳到侧边栏
        showToSidebar(notifyInfo);
        quitHandle();
        return 0;

    }

    // 检测是否为锁屏
    if (getScreenLockStates()) {
        // 检测当前应用是否允许锁屏展示
        if (!getScreenAppNotify(appInfoMap[appName])) {
            // 不允许，直接给侧边栏发送，不展示
            showToSidebar(notifyInfo);
            quitHandle();
            return 0;
        }
    }

    // 添加到数据库
    m_psqlInfoData->addOne(notifyInfo);

    // 判断此条应用是否可播放声音
    if (getVoiceAppNotify(appInfoMap[appName])) {
        // 播放弹窗声音
        appNotifySound(hints);
    }

    if (m_pTopWidget->isPopupExist(replacesId)) { //替换通知弹窗内容
        m_pTopWidget->replacePopupItemWidget(replacesId, notifyInfo);
    } else { //新建通知弹窗
        if(hints.find(DISPLAY_LOCATION) != hints.end() && hints.value(DISPLAY_LOCATION).toInt() == 1) {
            //使用霸屏弹窗显示该窗口（弹窗显示在主屏中心）
            m_pTopWidget->AddCenterPopupItemWidget(notifyInfo);
        } else {
            // 单弹窗模式 多弹窗模式
            if (m_bPopupWidgetModeStatus) { //单弹窗模式
                m_pTopWidget->addEntryInfo(notifyInfo);
                if (!m_pTopWidget->isVisible())
                    m_pTopWidget->consumeEntities();
            } else { //多弹窗模式
                //判断m_pTopWidget中是否存在该应用的通知，如果不存在，添加一条新弹窗，如果存在则加入弹窗的队列中，
                if(m_pTopWidget->isPopupExist(appName) == 0) {
                    m_pTopWidget->AddPopupItemWidget(notifyInfo);
                    if (!m_pTopWidget->isVisible())
                        m_pTopWidget->show();
                        m_pTopWidget->setNotifyPopWidgetSite();
                } else {
                    m_pTopWidget->popupItemAddInfo(notifyInfo);
                }
            }
        }
    }
    return popupWidgetID;
}
/* 针对Python的通知接口 */
uint NotifyManager::NotifyPy(
    const QString &appName,
    int replacesId,
    const QString &appIcon,
    const QString &summary,
    const QString &body,
    const QVariantList &actions,
    const QVariantMap hints,
    int expireTimeout)
{
    // Python的列表[]对应C++中的QVariantList类型，需要将QVariantList转换成QStringList类型后Notify()才可使用
    QStringList actionsList;
    for (int i = 0; i < actions.size(); i++) {
        actionsList.append(actions.at(i).toString());
    }
    return Notify(appName, replacesId, appIcon, summary, body, actionsList, hints, expireTimeout);
}
/* 简单通知接口：无action和hints 字段 */
uint NotifyManager::NotifySimple(
    const QString &appName,
    uint replacesId,
    const QString &appIcon,
    const QString &summary,
    const QString &body,
    int expireTimeout)
{
    const QStringList actions;
    const QVariantMap hints;
    return (Notify(appName, replacesId, appIcon, summary, body, actions, hints, expireTimeout));
}

/* 注册 freedesktop Notifications 服务*/
void NotifyManager::registerAsService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.interface()->registerService(
        NotificationsDBusService,
        QDBusConnectionInterface::ReplaceExistingService,
        QDBusConnectionInterface::AllowReplacement);
    connection.registerObject(NotificationsDBusPath, this);
}

/* 显示下一条通知：单弹窗模式下会调用，当上一条通知超时退出后，显示队列中的下一条 */
void NotifyManager::nextShowAction()
{
    if (m_bPopupWidgetModeStatus && !m_pTopWidget->isVisible()) {
        m_pTopWidget->consumeEntities();
    } else if (0 == m_pTopWidget->m_popWidgetList.count()) {
        quitHandle();
    }
    return;
}

/* 初始化 弹窗模式、弹窗禁用 Gsetting */
void NotifyManager::initGsettingValue()
{
    // 初始化免打扰
    const QByteArray id(UKUI_VOLUME_BRIGHTNESS_GSETTING_ID);
    if (QGSettings::isSchemaInstalled(id)) {
        m_pNodisturbModeGsetting = new QGSettings(id);
        connect(m_pNodisturbModeGsetting, &QGSettings::changed, this, [=](QString key) {
            if (key == KYLIN_DISTURB_GSETTING_VALUE_NOTIFYCLOSE) {
                m_bNoDisturbMode = m_pNodisturbModeGsetting->get(KYLIN_DISTURB_GSETTING_VALUE_NOTIFYCLOSE).toBool();
            }
        });
        if (m_pNodisturbModeGsetting != nullptr
            && m_pNodisturbModeGsetting->keys().contains(KYLIN_DISTURB_GSETTING_VALUE_NOTIFYCLOSE))
            m_bNoDisturbMode = m_pNodisturbModeGsetting->get(KYLIN_DISTURB_GSETTING_VALUE_NOTIFYCLOSE).toBool();
    }
    // 初始化弹窗模式
    const QByteArray id_2(UKUI_NOTIFICATION_DEMO_GSETTING_ID);
    if (QGSettings::isSchemaInstalled(id_2)) {
        m_pPopupWidgetModeGsetting = new QGSettings(id_2);
        connect(m_pPopupWidgetModeGsetting, &QGSettings::changed, this, [=](QString key) {
            if (key == KYLIN_NOTIFICATION_DEMO_CLOSE_MODE_KEY) {
                m_bPopupWidgetModeStatus
                    = m_pPopupWidgetModeGsetting->get(KYLIN_NOTIFICATION_DEMO_CLOSE_MODE_KEY).toBool();
            } else if(key == AUTO_START_DISTURB || key == TIME_START || key == TIME_END) {
                m_autoStartDisturb = m_pPopupWidgetModeGsetting->get(AUTO_START_DISTURB).toBool();
                m_timeStart = m_pPopupWidgetModeGsetting->get(TIME_START).toString();
                m_timeEnd = m_pPopupWidgetModeGsetting->get(TIME_END).toString();
            } else if(key == FULL_SCREEN_DISTURB) {
                m_fullScreenDisturb = m_pPopupWidgetModeGsetting->get(FULL_SCREEN_DISTURB).toBool();
            } else if(key == ALARM_CLOCK_DISSTURB) {
                m_alarmClockDisturb = m_pPopupWidgetModeGsetting->get(ALARM_CLOCK_DISSTURB).toBool();
            } else if(key == PROJECTIN_SCREEN_DISTURB) {
                m_projectionScreenDisturb = m_pPopupWidgetModeGsetting->get(PROJECTIN_SCREEN_DISTURB).toBool();
            }
        });
        if (m_pPopupWidgetModeGsetting != nullptr
            && m_pPopupWidgetModeGsetting->keys().contains(KYLIN_NOTIFICATION_DEMO_CLOSE_MODE_KEY)) {
            m_bPopupWidgetModeStatus = m_pPopupWidgetModeGsetting->get(KYLIN_NOTIFICATION_DEMO_CLOSE_MODE_KEY).toBool();
            m_autoStartDisturb = m_pPopupWidgetModeGsetting->get(AUTO_START_DISTURB).toBool();
            m_timeStart = m_pPopupWidgetModeGsetting->get(TIME_START).toString();
            m_timeEnd = m_pPopupWidgetModeGsetting->get(TIME_END).toString();
            m_fullScreenDisturb = m_pPopupWidgetModeGsetting->get(FULL_SCREEN_DISTURB).toBool();
            m_alarmClockDisturb = m_pPopupWidgetModeGsetting->get(ALARM_CLOCK_DISSTURB).toBool();
            m_projectionScreenDisturb = m_pPopupWidgetModeGsetting->get(PROJECTIN_SCREEN_DISTURB).toBool();
        }
    }

    if (PLATFORM::g_platformType == PLATFORM::V101) {
        // 初始化通知弹窗总开关
        const QByteArray id_3(CONTROL_CENTER_GSETTING_NOTIFY_MAIN_SWITCH);
        if (QGSettings::isSchemaInstalled(id_3)) {
            m_pControlNotifyMainGsetting = new QGSettings(id_3);
            if (m_pControlNotifyMainGsetting != nullptr
                && m_pControlNotifyMainGsetting->keys().contains(CONTROL_GSETTING_NOTIFY_MAIN_KEY)) {
                m_bNotifyMainSwitch = m_pControlNotifyMainGsetting->get(CONTROL_GSETTING_NOTIFY_MAIN_KEY).toBool();
                connect(m_pControlNotifyMainGsetting, &QGSettings::changed, this, [=](QString key) {
                    if (key == CONTROL_GSETTING_NOTIFY_MAIN_KEY) {
                        m_bNotifyMainSwitch
                            = m_pControlNotifyMainGsetting->get(CONTROL_GSETTING_NOTIFY_MAIN_KEY).toBool();
                    }
                });
            }
        }
    }
    return;
}

/* 获取系统当前声效列表：四类声效 system-setting、 window-close、alert-sound、volume-changed*/
QList<char *> NotifyManager::listExistsPath()
{
    char **childs;
    int len;
    DConfClient *client = dconf_client_new();
    childs = dconf_client_list(client, KEYBINDINGS_CUSTOM_DIR, &len);
    g_object_unref(client);

    QList<char *> vals;
    for (int i = 0; childs[i] != NULL; i++) {
        if (dconf_is_rel_dir(childs[i], NULL)) {
            char *val = g_strdup(childs[i]);
            vals.append(val);
        }
    }
    g_strfreev(childs);
    return vals;
}
/* 播放通知提示音 */
void NotifyManager::appNotifySound(const QVariantMap hints)
{
    //获取通知声效开关状态
    bool b_soundSwitchStatus = true;
    const QByteArray soundSwitch(CONTROL_PATH_SOUNDS_SCHEMA);
    if (QGSettings::isSchemaInstalled(soundSwitch)) {
        QGSettings *soundSwitchSetting = new QGSettings(soundSwitch);
        b_soundSwitchStatus = soundSwitchSetting->get(CONTROL_EVENT_SOUNDS_KEY).toBool();
        delete soundSwitchSetting;
        soundSwitchSetting = NULL;
    }

    //未打开消息音效,直接返回
    if (!b_soundSwitchStatus) {
        return;
    }
    ca_context_create(&m_pCaContext);

    gint retval;
    const gchar *desc = "alert sound";
    QString filenameStr = nullptr;

    filenameStr = hints.value("sound-name").toString();


    if (filenameStr == nullptr) {
        // 若未获取到声音名称(为空)，就从系统获取默认音效
        QList<char *> existsPath = this->listExistsPath();
        for (char *path : existsPath) {
            char *prepath = QString(KEYBINDINGS_CUSTOM_DIR).toLatin1().data();
            char *allpath = strcat(prepath, path);
            const QByteArray ba(KEYBINDINGS_CUSTOM_SCHEMA);
            const QByteArray bba(allpath);
            if (QGSettings::isSchemaInstalled(ba)) {
                QGSettings *settings = new QGSettings(ba, bba);
                filenameStr = settings->get(FILENAME_KEY).toString();
                QString nameStr = settings->get(NAME_KEY).toString();
                qInfo() << filenameStr << nameStr;
                if (nameStr == "generic-notification") {
                    break;
                }
                delete settings;
                settings = nullptr;
            }
        }
    } else if (filenameStr == "mute") {
        // 若传入字符串为 mute，表示不需要提示音，直接返回
        return ;
    }

    //播放通知声音
    const QByteArray text = filenameStr.toLocal8Bit();
    const gchar *id = text.data();
    const gchar *eventId = id;
    retval = ca_context_play(m_pCaContext, 0, CA_PROP_EVENT_ID, eventId, CA_PROP_EVENT_DESCRIPTION, desc, NULL);
    if (retval != 0) {
        qWarning()<<"声音播放错误";
    }
    qInfo() << desc << id << retval;
}

/* 获取应用通知开关状态 */
bool NotifyManager::getControlCentorAppNotify(QString appName)
{
    // 若空，为控制面板列表中没有的应用，默认策略显示此通知
    if (appName.isEmpty()) {
        return true;
    }
    // 初始化控制面板对于通知开关读取
    const QByteArray id_3(CONTROL_CENTER_GSETTING_PATH);
    if (QGSettings::isSchemaInstalled(id_3)) {
        QString dynamicPath = QString("%1%2/").arg(CONTROL_CERTER_DYNAMIC_GSETTING_PATH).arg(QString(appName));
        const QByteArray id_4(dynamicPath.toUtf8().data());
        m_pAppNotifyGseting = new QGSettings(id_3, id_4, this);
        bool status = m_pAppNotifyGseting->get(CONTROL_CENTER_GSETTING_NOTIFYCATION).toBool();
        delete m_pAppNotifyGseting;
        m_pAppNotifyGseting = NULL;
        return status;
    } else {
        return false;
    }
}

bool NotifyManager::getScreenAppNotify(QString appName)
{
    if (appName.isEmpty()) {
        return false;
    }
    // 初始化控制面板对于通知开关读取
    const QByteArray id_3(CONTROL_CENTER_GSETTING_PATH);
    if (QGSettings::isSchemaInstalled(id_3)) {
        QString dynamicPath = QString("%1%2/").arg(CONTROL_CERTER_DYNAMIC_GSETTING_PATH).arg(QString(appName));
        const QByteArray id_4(dynamicPath.toUtf8().data());
        m_pAppNotifyGseting = new QGSettings(id_3, id_4, this);
        bool status = m_pAppNotifyGseting->get(CONTROL_CENTER_GSETTING_SCREEN_NOTIFYCATION).toBool();
        delete m_pAppNotifyGseting;
        m_pAppNotifyGseting = NULL;
        return status;
    } else {
        return false;
    }
}

bool NotifyManager::getVoiceAppNotify(QString appName)
{
    if (appName.isEmpty()) {
        return false;
    }
    // 初始化控制面板对于通知开关读取
    const QByteArray id_3(CONTROL_CENTER_GSETTING_PATH);
    if (QGSettings::isSchemaInstalled(id_3)) {
        QString dynamicPath = QString("%1%2/").arg(CONTROL_CERTER_DYNAMIC_GSETTING_PATH).arg(QString(appName));
        const QByteArray id_4(dynamicPath.toUtf8().data());
        m_pAppNotifyGseting = new QGSettings(id_3, id_4, this);
        bool status = m_pAppNotifyGseting->get(CONTROL_CENTER_GSETTING_VOICE_NOTIFYCATION).toBool();
        delete m_pAppNotifyGseting;
        m_pAppNotifyGseting = NULL;
        return status;
    } else {
        return false;
    }
}

/* 检测通知显示开关是否开启：免打扰模式、通知总开关、应用通知开关 */
bool NotifyManager::notifySwitch(QString appName)
{
    // 免打扰模式判断
    if (m_bNoDisturbMode) {
        qInfo() << "Undisturbed mode，return";
        return false;
    }

    //勿扰模式
    bool isNoDisturb = false;
    //1、自动开启：设置某个时间段开启勿扰模式；
    if(m_autoStartDisturb) {
        QDateTime currentTime =QDateTime::currentDateTime();
        uint curHour = currentTime.toString("hh").toUInt();
        uint curMin = currentTime.toString("mm").toUInt();
        QStringList startTimeList = m_timeStart.split(":");
        uint startHour = QString(startTimeList.at(0)).toUInt();
        uint startMin  = QString(startTimeList.at(1)).toUInt();
        QStringList endTimeList = m_timeEnd.split(":");
        uint endHour = QString(endTimeList.at(0)).toUInt();
        uint endMin  = QString(endTimeList.at(1)).toUInt();
        if((curHour>startHour || (curHour==startHour && curMin>=startMin)) &&
           (curHour<endHour   || (curHour==endHour && curMin<=endMin))) {
            isNoDisturb = true;
        }
    }
    //2、是否开启：镜像屏幕或者投影时
    if(m_projectionScreenDisturb) {
        QDBusMessage msg = QDBusMessage::createMethodCall(KYLIN_SETTINGDAEMON_NAME,
                                                          KYLIN_SETTINGDAEMON_PATH,
                                                          KYLIN_SETTINGDAEMON_INTERFACE,
                                                          KYLIN_SETTINGDAEMON_METHOD);
        msg << "ukui-notifications-daemon";
        QDBusMessage response = QDBusConnection::sessionBus().call(msg);
        if (response.type() == QDBusMessage::ReplyMessage) {
            int32_t ret = response.arguments().takeFirst().toInt();
            if(ret == ScreenMode::CloneScreenMode) {
                isNoDisturb = true;
            }
        } else {
            qInfo()<<"get getScreenMode failed";
        }
    }
    //3、是否开启：全屏模式下
    if(m_fullScreenDisturb) {
        QDBusMessage msg = QDBusMessage::createMethodCall(KYLIN_KWIN_NAME,
                                                          KYLIN_KWIN_PATH,
                                                          KYLIN_KWIN_INTERFACE,
                                                          KYLIN_KWIN_METHOD);
        QDBusMessage response = QDBusConnection::sessionBus().call(msg);
        if (response.type() == QDBusMessage::ReplyMessage) {
            bool ret = response.arguments().takeFirst().toBool();
            if(ret) {
                isNoDisturb = true;
            }
        } else {
            qInfo()<<"get haveFullScreenWindow failed";
        }
    }
    //4、是否开启：勿扰模式下是否允许闹钟提示

    if(isNoDisturb) {
        if(appName=="Alarm" && m_alarmClockDisturb) {
            return true;
        }
        return false;
    } else {
        return true;
    }
}

QString NotifyManager::getNotifyShowStyle(QString appName)
{
    if (appName.isEmpty()) {
        return "";
    }

    // 初始化控制面板对于通知开关读取
    const QByteArray id_3(CONTROL_CENTER_GSETTING_PATH);
    if (QGSettings::isSchemaInstalled(id_3)) {
        QString dynamicPath = QString("%1%2/").arg(CONTROL_CERTER_DYNAMIC_GSETTING_PATH).arg(QString(appName));
        const QByteArray id_4(dynamicPath.toUtf8().data());
        m_pAppNotifyGseting = new QGSettings(id_3, id_4, this);
        QString status = m_pAppNotifyGseting->get(CONTROL_CENTER_GSETTING_STYLE_NOTIFYCATION).toString();
        delete m_pAppNotifyGseting;
        m_pAppNotifyGseting = NULL;
        return status;
    }
}

bool NotifyManager::getScreenLockStates()
{
    QDBusInterface m_quickOperationInterface("org.ukui.ScreenSaver",
                                             "/",
                                             "org.ukui.ScreenSaver",
                                             QDBusConnection::sessionBus());

    QDBusReply<bool> m_bool = m_quickOperationInterface.call("GetLockState");

    return m_bool;
}

void NotifyManager::showToSidebar(NotifyReceiveInfo *notifyInfo)
{
    QDBusMessage msg = QDBusMessage::createMethodCall("org.ukui.Sidebar",
                                                      "/org/ukui/Sidebar/notification",
                                                      "org.ukui.Sidebar.notification",
                                                      "sidebarNotification");
    //发送的数据：appName, appIcon, summary, body, urlStr,  actions
    msg << notifyInfo->appName() << notifyInfo->appIcon() << notifyInfo->summary() << notifyInfo->bodyText()
        << notifyInfo->bodyUrl() << notifyInfo->defaultActions(); //填充要发送的数据
    qInfo() << "Stow to sidebar:" << msg;
    QDBusPendingCall async = QDBusConnection::sessionBus().asyncCall(msg);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    delete watcher;
    watcher = NULL;
}

// 预加载应用名称
void NotifyManager::getAppinfo()
{
    QStringList whitelist;
    whitelist.append("kylin-screenshot.desktop");
    whitelist.append("peony.desktop");
    whitelist.append("kylin-device-daemon.desktop");
    whitelist.append("ukui-powermanagement-daemon.desktop");
    whitelist.append("kylin-system-update.desktop");
    whitelist.append("ukui-bluetooth.desktop");

    for (QString desktopName : whitelist) {
        QFileInfo file(SYSTEM_DESKTOP_PATH + desktopName);
        if (file.exists()) {
            QString mainSection = QStringLiteral("Desktop Entry");
            KSharedConfig::Ptr sharedConfig = KSharedConfig::openConfig(SYSTEM_DESKTOP_PATH + desktopName, KConfig::NoGlobals);
            KConfigGroup cfg(sharedConfig, mainSection);
            QString name = cfg.readEntry("Name");
            QString fileName = desktopName.left(desktopName.indexOf(QLatin1Char('.')));
            if (existList.contains(name))
                continue;
            existList.append(name);
            appInfoMap[name] = fileName;
        }
    }

    KApplicationTrader::query([=](const KService::Ptr &service) {
        if (service->exec().isEmpty() || service->noDisplay())
            return false;

        QString appname, desktopEntryName;
        appname = service->name();
        desktopEntryName = service->desktopEntryName();

        if (existList.contains(appname))
            return false;
        existList.append(appname);
        appInfoMap[appname] = desktopEntryName;
        return true;
    });
    existList.clear();
}

/* 点击关闭按钮、消息体、超时等 弹窗退出 */
void NotifyManager::popupItemWidgetDismissed(uint id, uint closeReson)
{
    emit NotificationClosed(id, closeReson);
    nextShowAction();
    return;
}

/* 点击弹窗的动作按钮退出 */
void NotifyManager::popupItemWidgetActionInvoked(uint id, QString actionKey)
{
    emit ActionInvoked(id, actionKey);
    nextShowAction();
    return;
}

void NotifyManager::quitHandle()
{
    if (0 == m_pTopWidget->m_popWidgetList.count()) {
        qApp->quit();
    }
}
