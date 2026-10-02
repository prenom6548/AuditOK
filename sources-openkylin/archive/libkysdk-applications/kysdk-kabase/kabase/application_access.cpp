/*
 * libkysdk-base's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#include "application_access.hpp"
#include "dbus.hpp"
#include "log.hpp"

namespace kdk
{
namespace kabase
{

ApplicationAccess::ApplicationAccess() = default;

ApplicationAccess::~ApplicationAccess() = default;

bool ApplicationAccess::exportFunc(AppName appName, QObject *object)
{
    if (!DBus::registerService(getServiceName(appName))) {
        error << "kabase : register service fail!";
        return false;
    }

    if (!DBus::registerObject(getObjectPath(appName), object)) {
        error << "kabase : register object fail!";
        return false;
    }

    return true;
}

QList<QVariant> ApplicationAccess::callMethod(AppName appName, QString methodName, QList<QVariant> args)
{
    return DBus::callMethod(getServiceName(appName), getObjectPath(appName), getInterfaceName(appName), methodName,
                            args);
}

QString ApplicationAccess::getServiceName(AppName appName)
{
    switch (appName) {
    case AppName::KylinCalculator:
        return QString("org.kylin.calculator");
    case AppName::KylinCalendar:
        return QString("org.kylin.calendar");
    case AppName::KylinCamera:
        return QString("org.kylin.camera");
    case AppName::KylinFontViewer:
        return QString("org.kylin.font.viewer");
    case AppName::KylinGpuController:
        return QString("org.kylin.gpu.controller");
    case AppName::KylinIpmsg:
        return QString("org.kylin.ipmsg");
    case AppName::KylinMusic:
        return QString("org.kylin.music");
    case AppName::KylinPhotoViewer:
        return QString("org.kylin.photo.viewer");
    case AppName::KylinPrinter:
        return QString("org.kylin.printer");
    case AppName::KylinRecorder:
        return QString("org.kylin.recorder");
    case AppName::KylinServiceSupport:
        return QString("org.kylin.service.support");
    case AppName::KylinWeather:
        return QString("org.kylin.weather");
    case AppName::KylinNotebook:
        return QString("org.kylin.notebook");
    default:
        return QString("");
    }

    /* 不应该被执行 */
    return "";
}

QString ApplicationAccess::getObjectPath(AppName appName)
{
    switch (appName) {
    case AppName::KylinCalculator:
        return QString("/org/kylin/calculator");
    case AppName::KylinCalendar:
        return QString("/org/kylin/calendar");
    case AppName::KylinCamera:
        return QString("/org/kylin/camera");
    case AppName::KylinFontViewer:
        return QString("/org/kylin/font/viewer");
    case AppName::KylinGpuController:
        return QString("/org/kylin/gpu/controller");
    case AppName::KylinIpmsg:
        return QString("/org/kylin/ipmsg");
    case AppName::KylinMusic:
        return QString("/org/kylin/music");
    case AppName::KylinPhotoViewer:
        return QString("/org/kylin/photo/viewer");
    case AppName::KylinPrinter:
        return QString("/org/kylin/printer");
    case AppName::KylinRecorder:
        return QString("/org/kylin/recorder");
    case AppName::KylinServiceSupport:
        return QString("/org/kylin/service/support");
    case AppName::KylinWeather:
        return QString("/org/kylin/weather");
    case AppName::KylinNotebook:
        return QString("/org/kylin/notebook");
    default:
        return QString("");
    }

    /* 不应该被执行 */
    return "";
}

QString ApplicationAccess::getInterfaceName(AppName appName)
{
    switch (appName) {
    case AppName::KylinCalculator:
        return QString("org.kylin.calculator.interface");
    case AppName::KylinCalendar:
        return QString("org.kylin.calendar.interface");
    case AppName::KylinCamera:
        return QString("org.kylin.camera.interface");
    case AppName::KylinFontViewer:
        return QString("org.kylin.font.viewer.interface");
    case AppName::KylinGpuController:
        return QString("org.kylin.gpu.controller.interface");
    case AppName::KylinIpmsg:
        return QString("org.kylin.ipmsg.interface");
    case AppName::KylinMusic:
        return QString("org.kylin.music.interface");
    case AppName::KylinPhotoViewer:
        return QString("org.kylin.photo.viewer.interface");
    case AppName::KylinPrinter:
        return QString("org.kylin.printer.interface");
    case AppName::KylinRecorder:
        return QString("org.kylin.recorder.interface");
    case AppName::KylinServiceSupport:
        return QString("org.kylin.service.support.interface");
    case AppName::KylinWeather:
        return QString("org.kylin.weather.interface");
    case AppName::KylinNotebook:
        return QString("org.kylin.notebook.interface");
    default:
        return QString("");
    }

    /* 不应该被执行 */
    return "";
}

} /* namespace kabase */
} /* namespace kdk */
