/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "windowmanager.h"
#include "app_control_manager.h"
#include "eventmanager.h"
#include "clipboard.h"
#include "appsettings.h"
#include "kmreenv.h"

#include <QProcess>
#include <QThread>
#include <QDesktopServices>
#include <QSessionManager>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QDebug>
#include <sys/syslog.h>

#include "cmd_signal_manager.h"
#include "backendworker.h"
#include "connection/kmre_service_loop.h"
#include "connection/kmre_socket_connector.h"
#include "connection/kmre_connection_worker.h"
#include "cmdprocessor/kmre_message_parser.h"
#include "cmdprocessor/kmre_message_receiver.h"

#include "utils.h"
#include "dbusclient.h"
#include "metatypes.h"


AppControlManager::AppControlManager()
{
    mCmdSignalManager = CmdSignalManager::getInstance();
    mWindowManager = KmreWindowManager::getInstance();
    m_loop = kmre::KmreLoop::initLoop();
    initRuntimeSocket();
    initBackendThread();
    initConnections();
    initNetworkSettings();
}

AppControlManager::~AppControlManager()
{
    m_backendThread->quit();
    m_backendThread->wait();
}

// 创建socket server: kmre_desktop，android端将通过该socket server给Linux发送数据
void AppControlManager::initRuntimeSocket()
{
    //创建unix domain socket server： kmre_desktop
    std::string android_socket_path;
    android_socket_path = KmreEnv::GetContainerSocketPath().toStdString() + "/kmre_desktop";
    unlink(android_socket_path.c_str());

    m_socketConnector = std::make_shared<kmre::KmreSocketConnector>(android_socket_path, m_loop,
        std::make_shared<kmre::KmreConnectionWorker>(m_loop, [&](const std::shared_ptr<kmre::KmreMessageReceiver> &)
        {
            //message function
            return std::make_shared<kmre::KmreMessageParser>();
        }
    ));
    m_loop->startLoop();
}

void AppControlManager::initBackendThread()
{
    m_backendWorker = new BackendWorker();
    m_backendThread = new QThread();
    m_backendWorker->moveToThread(m_backendThread);
    connect(m_backendThread, &QThread::finished, m_backendWorker, &BackendWorker::deleteLater);
    connect(m_backendThread, &QThread::finished, m_backendThread, &QThread::deleteLater);
    
    m_backendThread->start();
}

void AppControlManager::initNetworkSettings()
{
    m_networkSettings.updateNetworkSettings();
}

void AppControlManager::initConnections()
{
    // 容器环境已启动
    connect(mCmdSignalManager, &CmdSignalManager::containerEnvBooted, this, &AppControlManager::onContainerEnvBooted);
    //
    connect(mCmdSignalManager, &CmdSignalManager::requestSendClipboardToAndroid, m_backendWorker, &BackendWorker::setClipboardToAndroid);
    //
    connect(mCmdSignalManager, &CmdSignalManager::requestRunningAppList, m_backendWorker, &BackendWorker::getRunningAppList);
    //绑定通知消息
    connect(mCmdSignalManager, &CmdSignalManager::sendNotification, this, &AppControlManager::onHandleNotification, Qt::QueuedConnection);
    //
    connect(mCmdSignalManager, &CmdSignalManager::sendEventInfo, this, &AppControlManager::onProcessEventInfo, Qt::QueuedConnection);
    //绑定Android已经关闭了app虚拟屏的消息
    connect(mCmdSignalManager, &CmdSignalManager::sendCloseInfo, mWindowManager, &KmreWindowManager::onAppCloseSucceed, Qt::QueuedConnection);
    //绑定app启动成功的消息，接收到android发送的app已经启动或切换成功的消息
    connect(mCmdSignalManager, &CmdSignalManager::sendLaunchInfo, mWindowManager, &KmreWindowManager::onAppLaunchSucceed, Qt::QueuedConnection);
    //绑定Android切换平行视界的消息
    connect(mCmdSignalManager, &CmdSignalManager::sendMultiplierSwitchInfo, mWindowManager, &KmreWindowManager::onSwitchMultipler, Qt::QueuedConnection);
    //点击任务栏app的图标后，切换app   &   绑定重启app端请求
    connect(mCmdSignalManager, &CmdSignalManager::requestLaunchApp, m_backendWorker, &BackendWorker::launchApp, Qt::QueuedConnection);
    //绑定关闭应用的请求
    connect(mCmdSignalManager, &CmdSignalManager::requestCloseApp, m_backendWorker, &BackendWorker::closeApp, Qt::QueuedConnection);
    //绑定安卓返回按钮点击的请求
    connect(mCmdSignalManager, &CmdSignalManager::requestControlApp, m_backendWorker, &BackendWorker::controlApp, Qt::QueuedConnection);
    //绑定聚焦某个虚拟窗口的请求
    connect(mCmdSignalManager, &CmdSignalManager::requestFocusWindow, m_backendWorker, &BackendWorker::focusWindow, Qt::QueuedConnection);
    //告诉安卓去暂停所有安卓应用
    connect(mCmdSignalManager, &CmdSignalManager::requestPauseAllAndroidApps, m_backendWorker, &BackendWorker::pasueAllApps, Qt::QueuedConnection);
    //绑定设置Android属性的请求
    connect(mCmdSignalManager, &CmdSignalManager::requestSetSystemProp, m_backendWorker, &BackendWorker::setSystemProp, Qt::QueuedConnection);
    //绑定设置Android属性的请求
    connect(mCmdSignalManager, &CmdSignalManager::requestGetSystemProp, m_backendWorker, &BackendWorker::getSystemProp, Qt::BlockingQueuedConnection);
    //绑定更新App安装/卸载状态的请求
    connect(mCmdSignalManager, &CmdSignalManager::requestUpdatePackageStatus, this, &AppControlManager::onUpdateAppDesktopAndIcon, Qt::QueuedConnection);
    //绑定android剪切板数据信号
    connect(mCmdSignalManager, &CmdSignalManager::sendClipboardInfo, &Clipboard::getInstance(), &Clipboard::onSyncClipboardFromAndroid, Qt::QueuedConnection);
    //
    connect(mCmdSignalManager, &CmdSignalManager::sendInputMethodRequest, mWindowManager, &KmreWindowManager::onEnableInputMethod, Qt::QueuedConnection);
    //绑定android焦点窗口id，确认是哪个窗口获取到了焦点
    connect(mCmdSignalManager, &CmdSignalManager::sendFocusWinId, this, &AppControlManager::onFocusDisplayById, Qt::QueuedConnection);
    // 当type = 0时，表示接收的时所有安卓文件数目，android会进行多次发送，每次发送N条，如5条，直到所有数据发送完毕。  totalNum: 需要接收的数据总条数    metas.length: 本次接收的数据总条数
    connect(mCmdSignalManager, &CmdSignalManager::sendFilesList, this, &AppControlManager::onSyncFiles, Qt::QueuedConnection);
    //
    connect(mCmdSignalManager, &CmdSignalManager::requestOpenUrl, this, &AppControlManager::onOpenUrl, Qt::QueuedConnection);
    //绑定拖拽文件到android内
    connect(mCmdSignalManager, &CmdSignalManager::requestDragFile, m_backendWorker, &BackendWorker::onDragFile);
    //绑定安卓返回按钮点击的请求
    connect(mCmdSignalManager, &CmdSignalManager::requestChangeRotation, m_backendWorker, &BackendWorker::changeRotation);
    //绑定安卓多媒体播放状态变化，进行Linux锁屏与否处理
    connect(mCmdSignalManager, &CmdSignalManager::sendMediaPlayStatus, this, &AppControlManager::onSyncMediaPlayStatus, Qt::QueuedConnection);
    //初始化平行视界支持的应用列表及其状态
    connect(mCmdSignalManager, &CmdSignalManager::initAppMultipliersList, this , &AppControlManager::onInitAppMultipliersList, Qt::QueuedConnection);
    //
    connect(mCmdSignalManager, &CmdSignalManager::requestResponseInfo, this, &AppControlManager::onResponseInfo, Qt::QueuedConnection);
    //
    connect(mCmdSignalManager, &CmdSignalManager::requestUpdateAppWindowSize, m_backendWorker, &BackendWorker::updateAppWindowSize);
    //
    connect(mCmdSignalManager, &CmdSignalManager::requestUpdateDisplaySize, m_backendWorker, &BackendWorker::updateDisplaySize);
    //
    connect(mCmdSignalManager, &CmdSignalManager::requestAnswerCall, m_backendWorker, &BackendWorker::answerCall);
    //
}

void AppControlManager::onProcessEventInfo(const int &eventId, const QString &pkgName)
{
    if (!pkgName.isNull() && !pkgName.isEmpty()) {
        if (eventId == 0) {//Android向Linux请求获取系统当前语言和主题
            int language = mWindowManager->getCurrentSystemLanguage();
            int theme = mWindowManager->getCurrentSystemTheme();
            
            controlApp(0, "android", 18, language);
            controlApp(0, "android", 19, theme);

            syslog(LOG_DEBUG, "[%s] Set system language to %s", __func__, language ? "English" : "中文");
            syslog(LOG_DEBUG, "[%s] Set system theme to %s", __func__, theme ? "Dark" : "Light");
        }
    }
}

void AppControlManager::getRunningAppList()
{
    emit mCmdSignalManager->requestRunningAppList();
}

void AppControlManager::onHandleNotification(const QString &appName, const QString &pkgName, const QString &text, 
                                            int winId, bool hide, bool call, const QString &title)
{
    if (appName == "linux" && pkgName == "local") {//使用Linux端应用打开Android应用内的文件
        QString filePath = text;
        filePath.replace("/data/media/0", KmreEnv::GetAndroidDataPath());
        QFile file(filePath);
        if (file.exists()) {
            QProcess::execute("xdg-open", QStringList() << filePath);
        }
        else {
            syslog(LOG_ERR, "[%s] Open file(%s) failed!", __func__, filePath.toStdString().c_str());
        }
    }
    else {
        mWindowManager->handleNotification(appName, pkgName, text, winId, hide, call, title);
    }
}

void AppControlManager::onOpenUrl(const QString link)
{
    if (!QDesktopServices::openUrl(QUrl(link))) {//xdg-open
        syslog(LOG_ERR, "[%s] Open link(%s) failed!", __func__, link.toStdString().c_str());
    }
}

void AppControlManager::onFocusDisplayById(int displayId)
{
    EventManager::getInstance()->setCurrentFocusedDisplay(displayId);
}

void AppControlManager::onUpdateAppDesktopAndIcon(const QString &pkgName, int status, int type)
{
    // send dbus signal to manager
    emit mWindowManager->updateAppStatus(pkgName, status, type);
    if (status == 1) {// app uninstalled
        mWindowManager->appUninstalled(pkgName);
    }
}

void AppControlManager::onSyncFiles(int type, AndroidMetaList metas, int totalNum)
{
    // send dbus signal to manager
    emit mWindowManager->syncFiles(type, metas, totalNum);
}

void AppControlManager::onSyncMediaPlayStatus(bool playing)
{
    mWindowManager->inhibitSystemLockScreen(playing);//阻止/放开锁屏
}

void AppControlManager::onInitAppMultipliersList(const QString &jsonStr)
{
    // send to settings-ui
    emit mWindowManager->postAppMultipliers(jsonStr);
    AppSettings::getInstance().initAppMultipliersList(jsonStr);
}

void AppControlManager::onResponseInfo(int id, const QString &pkgName, const QString &category, int ret, const QString &info)
{
    // send signal to ui-settings
    emit mWindowManager->postResponseInfo(id, pkgName, category, ret, info);
    //如果是平行视界应用的开关设置，则更新本地AppMultiplier.json文件中的信息
    if (category == "app_multiplier") {
        AppSettings::getInstance().updateAppMultipliersList(pkgName, ret);        
    }
}

// 向android发送激活某个app的请求
void AppControlManager::setFocusWindow(int displayId)
{
    // send this id to android to switch to correct display
    emit mCmdSignalManager->requestFocusWindow(displayId);
}

//向android发送关闭app的请求
void AppControlManager::closeApp(const QString &appName, const QString& pkgName, bool forceKill)
{
    Q_UNUSED(forceKill);
    // send appName to android to close and kill process.
    emit mCmdSignalManager->requestCloseApp(appName, pkgName);
}

// cmd -> 0：返回；1：音量增加；2：音量减小；3：亮度增大；4：亮度减小；5：切到后台；6：切到前台; 7:关闭应用；8：截图; 
// 9：弹虚拟键盘, 10：开启录屏，11：关闭录屏，12：横屏，13：竖屏，14：锁屏，15：开启平行界面，16：关闭平行界面
void AppControlManager::controlApp(int displayId, const QString &pkgName, int cmd, int value)
{
    emit mCmdSignalManager->requestControlApp(displayId, pkgName, cmd, value);
}

void AppControlManager::sendDragFileInfo(const QString& filePath, const QString& pkgName, int displayId, bool hasDoubleDisplay)
{
    emit mCmdSignalManager->requestDragFile(filePath, pkgName, displayId, hasDoubleDisplay);
}

//wayland上需要图形程序调用该函数给Android发送剪切板数据
void AppControlManager::sendClipboardData(const QString &content)
{
    if (!content.isEmpty()) {
        emit mCmdSignalManager->requestSendClipboardToAndroid(content);
    }
}

void AppControlManager::changeRotation(int id, const QString &pkgName, int width, int height, int rotation)
{
    emit mCmdSignalManager->requestChangeRotation(id, pkgName, width, height, rotation);
}

void AppControlManager::setSystemProp(int type, const QString &propName, const QString &propValue)
{
    emit mCmdSignalManager->requestSetSystemProp(type, propName, propValue);
}

QString AppControlManager::getSystemProp(int type, const QString &propName)
{
    return mCmdSignalManager->requestGetSystemProp(type, propName);
}

void AppControlManager::updateAppWindowSize(const QString &pkgName, int displayId, int width, int height)
{
    emit mCmdSignalManager->requestUpdateAppWindowSize(pkgName, displayId, width, height);
}

void AppControlManager::updateDisplaySize(int displayId, int width, int height)
{
    emit mCmdSignalManager->requestUpdateDisplaySize(displayId, width, height);
}

void AppControlManager::onContainerEnvBooted(bool status, QString errInfo)
{
    if (status) {
        syslog(LOG_INFO, "[%s] Container Env boot successfully", __func__);
        mWindowManager->setSettingsForAndroid();
        emit mWindowManager->containerEnvBooted(status, errInfo);
    }
    else {
        syslog(LOG_INFO, "[%s] Container Env boot failed! (error msg: '%s')", __func__, errInfo.toStdString().c_str());
    }
}

void AppControlManager::answerCall(bool answer)
{
    emit mCmdSignalManager->requestAnswerCall(answer);
}
