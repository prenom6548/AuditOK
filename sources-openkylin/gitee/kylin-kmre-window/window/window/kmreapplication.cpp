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

#include "kmreapplication.h"
#include "windowmanager.h"
#include "preferences.h"
#include "kmreenv.h"
#include "signalevent.h"
#include "sessionsettings.h"

#include <signal.h>
#include <syslog.h>
#include <QTextCodec>
#include <QLocale>
#include <QDir>
#include <QLibraryInfo>

#include "wayland/ukui/plasma-shell-manager.h"

#define TRANSLATION_PATH "/usr/share/kylin-kmre-window/translations"

typedef struct {
    unsigned long winId;
    int32_t type;// 0 -> leave event; 1 -> enter event
    int32_t posX;
    int32_t posY;
}SignalMsg;

static void sig_handle(int signo, siginfo_t *info, void *ctx)
{
    /*
     * 在兼容模式下，如'emugl'使用的是'egl2egl'模式，底层Xlib子窗口会将‘leave’事件通过信号‘SIGUSR1’发送出来，
     * 上层QT窗口在此处接收并处理，此方案是为了解决在该模式下因收不到‘leave’事件而无法限制鼠标在窗口内的问题。
     */
    if (signo == SIGUSR1) {
        SignalMsg *signalMsg = (SignalMsg *)info->si_value.sival_ptr;
        //syslog(LOG_DEBUG, "[%s] Received signal 'SIGUSR1', info: ", __func__);
        //syslog(LOG_DEBUG, "[%s] windId = %ld, type = %d, posX = %d, posY = %d", 
        //    __func__, signalMsg->winId, signalMsg->type, signalMsg->posX, signalMsg->posY);
        
        if (signalMsg->type == 0) {// leave event
            SignalEvent *signalEvent = new SignalEvent(signalMsg->winId, SignalEvent::eLeaveEvent, signalMsg->posX, signalMsg->posY);
            QCoreApplication::postEvent(KmreWindowManager::getInstance(), signalEvent);
        }
    }
    else if (signo == SIGPIPE) {
        //do nothing (ignore)
    }
}

KmreApplication::KmreApplication(int &argc, char **argv, int flag)
    : QApplication(argc, argv, flag)
{
    QGuiApplication::setOrganizationName("TianJin Kylin");
    QGuiApplication::setApplicationName("kylin-kmre-window");
    QGuiApplication::setApplicationVersion("3.0");
    //最后一个可视窗口关闭时，不退出程序
    QGuiApplication::setQuitOnLastWindowClosed(false);

    struct sigaction act;
    act.sa_sigaction = sig_handle;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;

    if (sigaction(SIGUSR1, &act, NULL) == -1) {
        syslog(LOG_ERR, "[%s] Register signal 'SIGUSR1' failed!", __func__);
    }
    if (sigaction(SIGPIPE, &act, NULL) == -1) {
        syslog(LOG_CRIT, "[%s] Register signal 'SIGPIPE' failed!", __func__);
        exit(-1);
    }
}

KmreApplication::~KmreApplication()
{
    if (mWindowManager) {
        mWindowManager->destroy();
    }
}

bool KmreApplication::init()
{
    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        PlasmaShellManager::getInstance();//避免在wayland下窗口第一次设置置顶无效
    }
    initTranslator();
    KmreEnv::envCheck();
    initGlobalSettings();

    mWindowManager = KmreWindowManager::getInstance();
    mWindowManager->initialize();

    syslog(LOG_INFO, "[%s] kylin-kmre-window started.", __func__);
    return true;
}

void KmreApplication::initTranslator()
{
#if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
    QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

    QString locale = QLocale::system().name();
    QString translatorFile = "kylin-kmre-window_" + locale + ".qm";
    if (!mTranslator.load(translatorFile, getTranslationPath())) {
        syslog(LOG_ERR, "[%s] Load translation file: '%s' failed!", __func__, translatorFile.toStdString().c_str());
    }
    else {
        this->installTranslator(&mTranslator);
    }

    //加载Qt对话框默认的国际化
    mDefTranslator.load("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));// /usr/share/qt5/translations
    this->installTranslator(&mDefTranslator);
}

void KmreApplication::initGlobalSettings()
{
    // init kmre config
    KmreConfig::Preferences *preferences = KmreConfig::Preferences::getInstance();
    KmreConfig::Feature::getInstance();

    kmre::DbusClient *dbusClient = kmre::DbusClient::getInstance();

    // init hardware config
    // init camera device in other thread to avoid blocking main thread
    std::thread([=] {
        QString cameraDeviceName = dbusClient->getCameraDevice();
        preferences->m_cameraDeviceName = kmre::utils::getFixedDefaultCameraDevice(cameraDeviceName);
        dbusClient->setCameraDevice(preferences->m_cameraDeviceName);
        preferences->updateCameraConfig();
    }).detach();

    syslog(LOG_INFO, "[%s] Init global settings finished.", __func__);
}

QString KmreApplication::getTranslationPath()
{
    if (QDir(TRANSLATION_PATH).exists()) {
        return TRANSLATION_PATH;
    }
    else {
        return qApp->applicationDirPath() + "/translations";
    }
}
