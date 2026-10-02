/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "xrandr-dbus.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QProcess>
#include <QDebug>
#include <QAction>
#include <KF5/KGlobalAccel/KGlobalAccel>

#include <glib.h>
#include <gio/gio.h>

#include "clib-syslog.h"
#include "usd_base_class.h"

#include "xrandr-manager.h"

#define UKUI_DAEMON_NAME    "ukui-settings-daemon"
XrandrManager *xrandrManager = nullptr;
xrandrDbus::xrandrDbus(QObject* parent) : QObject(parent)
{
    mXsettings =new QGSettings("org.ukui.SettingsDaemon.plugins.xsettings");
    mScale = mXsettings->get("scaling-factor").toDouble();
    xrandrManager = static_cast<XrandrManager*>(parent);
}

xrandrDbus::~xrandrDbus()
{
    if (mXsettings) {
        delete mXsettings;
        mXsettings = nullptr;
    }
}

int xrandrDbus::setScreenMode(QString modeName,QString appName){
    USD_LOG(LOG_DEBUG,"change screen :%s, appName:%s",modeName.toLatin1().data(), appName.toLatin1().data());
    if (xrandrManager->getInitStatus()) {
        Q_EMIT setScreenModeSignal(modeName);
        return true;
    } else {
        return false;
    }
}

int xrandrDbus::getScreenMode(QString appName){
    USD_LOG(LOG_DEBUG,"get screen mode appName:%s", appName.toLatin1().data());
    if (xrandrManager->getInitStatus()) {
        return xrandrManager->discernScreenMode();
    } else {
        return 0;
    }
}

int xrandrDbus::setScreensParam(QString screensParam, QString appName)
{
    USD_LOG(LOG_DEBUG,"appName:%s",screensParam.toLatin1().data(),appName);
    if (xrandrManager->getInitStatus()) {
        Q_EMIT setScreensParamSignal(screensParam);
    }
    return 1;
}

QString xrandrDbus::getScreensParam(QString appName)
{
    USD_LOG(LOG_DEBUG,"dbus from %s",appName.toLatin1().data());
    if (xrandrManager->getInitStatus()) {
        return xrandrManager->getOutputsInfo();
    } else {
        return "";
    }
}

void xrandrDbus::sendModeChangeSignal(int screensMode)
{
    static int lastScreenMode = 0xff;
    if (lastScreenMode == screensMode) {
        return;
    }

    lastScreenMode = screensMode;
    USD_LOG(LOG_DEBUG,"send mode:%d",screensMode);

    Q_EMIT screenModeChanged(screensMode);
}

void xrandrDbus::sendScreensParamChangeSignal(QString screensParam)
{
    USD_LOG(LOG_DEBUG,"send param");
    Q_EMIT screensParamChanged(screensParam);
}

void xrandrDbus::sendScreenAddedSignal(QString screenName)
{
    USD_LOG(LOG_DEBUG,".");
    Q_EMIT screenAdded(screenName);
}

void xrandrDbus::sendScreenRemovedSignal(QString screenName)
{
    USD_LOG(LOG_DEBUG,".");
    Q_EMIT screenRemoved(screenName);
}

void xrandrDbus::sendScreenStateChangedSignal(QString screenName, bool state)
{
    USD_LOG(LOG_DEBUG,".");
    Q_EMIT screenStateChanged(screenName, state);
}

void xrandrDbus::setScreenMap()
{
    if (xrandrManager->getInitStatus()) {
        xrandrManager->onlyCalibrate();
    }
}

QString xrandrDbus::controlScreenSlot(const QString &conRotation)
{
    USD_LOG(LOG_DEBUG,"control call this slot");
    if (xrandrManager->getInitStatus()) {
        Q_EMIT controlScreen(conRotation);
    }

    return QString("Get messageMethod reply: %1").arg(conRotation);
}

bool xrandrDbus::SetScreenScale(double scale)
{
    if (!c_scaleList.contains(scale)) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidArgs, tr("args error:1%").arg(scale));
        }
        return false;
    }

    QString errorMsg;
    if (!xrandrManager->setSingleScreenScale(scale, errorMsg)) {
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::InvalidArgs, errorMsg);
        }
        return false;
    }
    return true;
}

double xrandrDbus::GetScreenScale()
{
     QString errorMsg = "";

     double scale = xrandrManager->getSingleScreenScale(errorMsg);
     if (!errorMsg.isEmpty()) {
          sendErrorReply(QDBusError::Failed, errorMsg);
     }
     return scale;
}

double xrandrDbus::GetPreferredScale(int outputWidth, int outputHeight, int physicalWidthMM, int physicalHeightMM)
{
    // 验证物理尺寸的合理性
    if (physicalWidthMM < 0.0 || physicalHeightMM < 0.0) {
        // 记录警告日志或返回默认缩放比例
        return 1.0; // 默认不缩放
    }

    // 验证分辨率参数的有效性
    if (outputWidth <= 0 || outputHeight <= 0) {
        return 1.0;
    }
    //对外提供的接口必须使用100%作为默认参数，不能使用缺省的系统参数，会受系统影响导致
    return UsdBaseClass::getPreferredScale(physicalWidthMM, physicalHeightMM,
                                            outputWidth, outputHeight, 1);
}


