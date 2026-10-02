/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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

#include "screen_gamma.h"


ScreenGamma::ScreenGamma(QObject *parent)
{
    qDebug()<<__FUNCTION__ << "init screen gamma";
}

ScreenGamma::~ScreenGamma()
{

}

AbstractResult ScreenGamma::setScreenBrightness(QString name, uint screenBrightness)
{
    AbstractResult ret;
    QDBusReply<int> reply = callGamma("setScreenBrightness", MY_NAME, name, screenBrightness);
    if (false == reply.isValid()) {
        ret.errorMsg = reply.error().message();
        return ret;
    }

    return ret;
}

AbstractResult ScreenGamma::getScreenBrightness(QString name)
{
    AbstractResult ret;
    QDBusReply<OutputGammaInfoList> reply = callGamma("getScreensGammaList", MY_NAME);

    if (false == reply.isValid()) {
        ret.errorMsg = "call get screensGammaList error";
        return ret;
    }

    Q_FOREACH(const OutputGammaInfo info, reply.value()) {
        if (info.outputName == name) {
            ret.value = info.brignthess;
            return ret;
        }
    }
    ret.errorMsg = QString("can't find :%1").arg(name);
    return ret;
}

AbstractResult ScreenGamma::setScreensBrightness(OutputGammaInfoList outputsInfo)
{
    AbstractResult ret;
    bool hadSet = false;
    Q_FOREACH(const OutputGammaInfo &info, outputsInfo) {
        hadSet = true;
        ret.value = true;
        callGamma("setScreenBrightness", MY_NAME, info.outputName, info.brignthess);
    }

    if (false == hadSet) {
        ret.errorMsg = "set gammam error!";
    }

    return ret;
}

AbstractResult ScreenGamma::setAllScreenSameBrightness(uint brighenss)
{
    AbstractResult ret;
    QDBusReply<int> reply = callGamma("setAllScreenBrightness", MY_NAME, (int)brighenss);
    if (false == reply.isValid()) {
        ret.errorMsg = reply.error().message();
        return ret;
    }
    ret.value = reply.value();
    return ret;
}

OutputGammaInfoList ScreenGamma::getAllScreenBrightness()
{
    OutputGammaInfoList ret;

    QDBusReply<OutputGammaInfoList>  reply = callGamma("getScreensGammaList", MY_NAME);

    if (false == reply.isValid()) {
        return ret;
    }
    return reply.value();
}

uint ScreenGamma::getPrimaryScreenBrightness()
{

    return 0;
}

template <typename...Args>
QDBusMessage ScreenGamma::callGamma(const QString &method, Args &&...args)
{
    QDBusInterface globalManagerInterface("org.ukui.SettingsDaemon",
                                  "/org/ukui/SettingsDaemon/GammaManager",
                                  "org.ukui.SettingsDaemon.GammaManager");

    return globalManagerInterface.call(method, std::forward<Args>(args)...);
}
