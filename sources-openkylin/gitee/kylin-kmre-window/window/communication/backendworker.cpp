/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *  Kobe Lee    lixiang@kylinos.cn
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

#include "app_control_manager.h"
#include "backendworker.h"
#include "utils.h"
#include "dbusclient.h"

#include <unistd.h>
#include <sys/syslog.h>


BackendWorker::BackendWorker(QObject *parent)
    : QObject(parent)
    , mKmreLibWrapper(new LibWrapper("/usr/lib/libkmre.so"))
{
}

BackendWorker::~BackendWorker()
{
}

void BackendWorker::launchApp(const QString &pkgName, bool fullscreen, int width, int height, int density)
{
    if (pkgName.isEmpty() || (width <= 0) || (height <= 0)) {
        syslog(LOG_ERR, "[%s] Launch app(%s) failed! Invalid parameters!", __func__, pkgName.toStdString().c_str()); 
        return;
    }

    auto [ret, launch_app] = mKmreLibWrapper->getSymbol("launch_app");
    if (ret && launch_app) {
        ret = ((bool(*)(char *, bool, int, int, int))launch_app)(const_cast<char *>(pkgName.toStdString().c_str()), fullscreen, width, height, density);
    }

    //emit this->launchFinished(pkgName, ret);
    syslog(ret ? LOG_DEBUG : LOG_ERR, "[%s] Launch app(%s) %s", __func__, pkgName.toStdString().c_str(), ret ? "succeed." : "failed!");
}

void BackendWorker::closeApp(const QString &appName, const QString &pkgName)
{
    if (appName.isEmpty() && pkgName.isEmpty()) {
        syslog(LOG_ERR, "[%s] Close app failed! Invalid parameters!", __func__); 
        return;
    }

    auto [ret, close_app] = mKmreLibWrapper->getSymbol("close_app");
    if (ret && close_app) {
        ret = ((bool(*)(char*, char*))close_app)(const_cast<char *>(appName.toStdString().c_str()), const_cast<char *>(pkgName.toStdString().c_str()));
    }

    //emit this->closeFinished(pkgName, ret);
    if (ret) {
        //AppControlManager::getInstance()->removeAppFromMapByPkgName(pkgName);
    }
    syslog(ret ? LOG_DEBUG : LOG_ERR, "[%s] Close app %s", __func__, ret ? "succeed." : "failed!");
}

void BackendWorker::focusWindow(int display_id)
{
    auto [ret, focus_win_id] = mKmreLibWrapper->getSymbol("focus_win_id");
    if (ret && focus_win_id) {
        ret = ((bool(*)(int))focus_win_id)(display_id);
    }

    if (!ret) {
        syslog(LOG_ERR, "[%s] Focus display(%d) failed!", __func__, display_id);
    }
}

void BackendWorker::getRunningAppList()
{
    char *appListStr = nullptr;
    auto [ret, get_running_applist] = mKmreLibWrapper->getSymbol("get_running_applist");
    if (ret && get_running_applist) {
        appListStr = ((char *(*)())get_running_applist)();
    }
    
    if (appListStr) {
        //QByteArray byteArray(appListStr, std::string(appListStr).length());
        //emit sendRunningAppList(byteArray);
    }
    else {
        syslog(LOG_ERR, "[%s] Get running app list failed!", __func__);
    }
}

void BackendWorker::setClipboardToAndroid(const QString &content)
{
    if (content.isEmpty()) {
        syslog(LOG_ERR, "[%s] Set clipboard failed! Invalid parameters!", __func__); 
        return;
    }

    auto [ret, send_clipboard] = mKmreLibWrapper->getSymbol("send_clipboard");
    if (ret && send_clipboard) {
        ret = ((bool(*)(char *))send_clipboard)(const_cast<char *>(content.toStdString().c_str()));
    }

    if (!ret) {
        syslog(LOG_ERR, "[%s] Send clipboard failed!", __func__);
    }
}

void BackendWorker::pasueAllApps()
{
    auto [ret, pause_all_apps] = mKmreLibWrapper->getSymbol("pause_all_apps");
    if (ret && pause_all_apps) {
        ret = ((bool(*)())pause_all_apps)();
    }

    if (!ret) {
        syslog(LOG_ERR, "[%s] Pause all apps failed!", __func__);
    }
}

// cmd -> 0：返回；1：音量增加；2：音量减小；3：亮度增大；4：亮度减小；5：切到后台；6：切到前台; 7:关闭应用；8：截图; 9：弹虚拟键盘, 10：开启录屏，11：关闭录屏
void BackendWorker::controlApp(int displayId, const QString &pkgName, int cmd, int value)
{
    auto [ret, control_app] = mKmreLibWrapper->getSymbol("control_app");
    if (ret && control_app) {
        ret = ((bool(*)(int, char *, int, int))control_app)(displayId, const_cast<char *>(pkgName.toStdString().c_str()), cmd, value);
    }

    if (!ret) {
        syslog(LOG_ERR, "[%s] Control app with event type(%d) failed!", __func__, cmd);
    }
}

void BackendWorker::onDragFile(const QString& filePath, const QString& pkgName, int displayId, bool hasDoubleDisplay)
{
    if (filePath.isEmpty() || pkgName.isEmpty() || (displayId < 0)) {
        syslog(LOG_ERR, "[%s] invalid parameter!", __func__);
        return;
    }

    auto [ret, request_drag_file] = mKmreLibWrapper->getSymbol("request_drag_file");
    if (ret && request_drag_file) {
        ret = ((bool(*)(const char*, const char*, int, bool))request_drag_file)(
            filePath.toStdString().c_str(), 
            pkgName.toStdString().c_str(), 
            displayId, 
            hasDoubleDisplay);
    }

    if (!ret) {
        syslog(LOG_ERR, "[%s] request_drag_file failed!", __func__);
    }
}

// rotation:  0表示竖向   1表示横向     2表示方型
void BackendWorker::changeRotation(int displayId, const QString &pkgName, int width, int height, int rotation)
{
    auto [ret, rotation_changed] = mKmreLibWrapper->getSymbol("rotation_changed");
    if (ret && rotation_changed) {
        ret = ((bool(*)(int, char *, int, int, int))rotation_changed)(
            displayId, const_cast<char *>(pkgName.toStdString().c_str()), width, height, rotation);
    }

    if (!ret) {
        syslog(LOG_ERR, "[%s] Change app(%s) display(%d)rotation failed!", __func__, pkgName.toStdString().c_str(), displayId);
    }
}

QByteArray BackendWorker::getInstalledAppListJsonStr()
{
    char* appListStr = nullptr;
    auto [ret, get_installed_applist] = mKmreLibWrapper->getSymbol("get_installed_applist");
    if (ret && get_installed_applist) {
        appListStr = ((char *(*)())get_installed_applist)();
    }

    if (appListStr) {
        QByteArray byteArray(appListStr, std::string(appListStr).length());
        return byteArray;
    }
    else {
        syslog(LOG_ERR, "[%s] Get installed app list failed!", __func__);
        return QString("[]").toUtf8();
    }
}

// type: 0-prop,1-setting
void BackendWorker::setSystemProp(int type, const QString &propName, const QString &propValue)
{
    if (propName.isEmpty() || propValue.isEmpty()) {
        syslog(LOG_ERR, "[%s] Set system prop failed! Invalid parameter.", __func__);
        return;
    }

    auto [ret, set_system_prop] = mKmreLibWrapper->getSymbol("set_system_prop");
    if (ret && set_system_prop) {
        ret = ((bool(*)(int, char *, char *))set_system_prop)(type, const_cast<char *>(propName.toStdString().c_str()), 
                                                            const_cast<char *>(propValue.toStdString().c_str()));
    }

    if (!ret) {
        syslog(LOG_ERR, "[%s] Set system prop(%s) failed!", propName.toStdString().c_str());
    }
}

// type: 0-prop,1-setting
QString BackendWorker::getSystemProp(int type, const QString &propName)
{
    if (propName.isEmpty()) {
        syslog(LOG_ERR, "[%s] Get system prop failed! Invalid parameter.", __func__);
        return "";
    }

    char *value = nullptr;
    auto [ret, get_system_prop] = mKmreLibWrapper->getSymbol("get_system_prop");
    if (ret && get_system_prop) {
        value = ((char* (*)(int, char *))get_system_prop)(type, const_cast<char *>(propName.toStdString().c_str()));
    }

    if (!value) {
        syslog(LOG_ERR, "[%s] Get system prop(%s) failed!", propName.toStdString().c_str());
    }

    return value ? QString(value) : QString("");
}

void BackendWorker::updateAppWindowSize(const QString &pkgName, int displayId, int width, int height)
{
    if (pkgName.isEmpty() || (displayId < 0) || (width <= 0) || (height <= 0)) {
        syslog(LOG_ERR, "[%s] invalid parameter!", __func__);
        return;
    }
    //syslog(LOG_ERR, "[%s] =========== displayId = %d, width = %d, height = %d", __func__, displayId, width, height);
    auto [ret, update_app_window_size] = mKmreLibWrapper->getSymbol("update_app_window_size");
    if (ret && update_app_window_size) {
        ret = ((int (*)(const char*, int, int, int))update_app_window_size)(
            pkgName.toStdString().c_str(), 
            displayId, 
            width, 
            height) == 0;
    }
    
    if (!ret) {
        syslog(LOG_ERR, "[%s] update_app_window_size failed!", __func__);
    }
}

void BackendWorker::updateDisplaySize(int displayId, int width, int height)
{
    if ((width <= 0) || (height <= 0)) {
        syslog(LOG_ERR, "[%s] invalid parameter!", __func__);
        return;
    }

    auto [ret, update_display_size] = mKmreLibWrapper->getSymbol("update_display_size");
    if (ret && update_display_size) {
        //syslog(LOG_DEBUG, "[%s] Update display size to %d x %d", __func__, width, height);
        ret = ((int (*)(int, int, int))update_display_size)(displayId, width, height) == 0;
    }

    if (!ret) {
        syslog(LOG_ERR, "[%s] update_display_size failed!", __func__);
    }
}

void BackendWorker::answerCall(bool answer)
{
    auto [ret, answer_call] = mKmreLibWrapper->getSymbol("answer_call");
    if (ret && answer_call) {
        ret = ((int (*)(bool))answer_call)(answer) == 0;
    }

    if (!ret) {
        syslog(LOG_ERR, "[%s] answer_call failed!", __func__);
    }
}

bool BackendWorker::hasEnvBootcompleted()
{
    QString boot_completed = kmre::DbusClient::getInstance()->GetPropOfContainer(kmre::utils::getUserName(), getuid(), "sys.kmre.boot_completed");
    return (boot_completed == "1");
}
