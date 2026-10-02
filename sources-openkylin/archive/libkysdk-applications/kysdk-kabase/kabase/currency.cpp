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

#include "currency.hpp"

namespace kdk
{
namespace kabase
{

Currency::Currency() = default;

Currency::~Currency() = default;

QString Currency::getAppName(AppName appName)
{
    switch (appName) {
    case AppName::KylinCalculator:
        return QString("kylin-calaulator");
    case AppName::KylinCalendar:
        return QString("kylin-calendar");
    case AppName::KylinCamera:
        return QString("kylin-camera");
    case AppName::KylinFontViewer:
        return QString("kylin-font-viewer");
    case AppName::KylinGpuController:
        return QString("kylin-gpu-controller");
    case AppName::KylinIpmsg:
        return QString("kylin-ipmsg");
    case AppName::KylinMusic:
        return QString("kylin-music");
    case AppName::KylinPhotoViewer:
        return QString("kylin-photo-viewer");
    case AppName::KylinPrinter:
        return QString("kylin-printer");
    case AppName::KylinRecorder:
        return QString("kylin-recorder");
    case AppName::KylinServiceSupport:
        return QString("kylin-service-support");
    case AppName::KylinWeather:
        return QString("kylin-weather");
    case AppName::KylinNotebook:
        return QString("kylin-notebook");
    case AppName::KylinOsManager:
        return QString("kylin-os-manager");
    case AppName::KylinNetworkCheck:
        return QString("kylin-network-check-tools");
    case AppName::KylinGallery:
        return QString("kylin-gallery");
    case AppName::KylinScanner:
        return QString("kylin-scanner");
    case AppName::KylinMobileAssistant:
        return QString("kylin-mobile-assistant");
    default:
        return QString("");
    }

    /* 不应该被执行 */
    return "";
}

} // namespace kabase
} /* namespace kdk */
