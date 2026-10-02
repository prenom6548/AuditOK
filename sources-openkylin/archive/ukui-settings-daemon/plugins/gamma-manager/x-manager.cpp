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

#include "x-manager.h"

xManager *xManager::m_gammaManager = nullptr;
/*
 *X11环境色温对USD的需求：
 * 0.响应相关gsettings的设置，如色温配置调整，强调色。
 * 1.计算经纬度。
 * 2.获取当前分钟色温值。
 * 3.如果当前与预期存在差异则需要渐变。
 * 4.通过色温计算为RGB调整gamma。
 * 5.监听屏幕增加，减少以及主屏切换的信号，并进行记录。
 * 6.支持以gamma方式调整亮度，并发送主屏改变信号，屏幕改变信号。
*/
xManager::xManager()
{
    init(UsdBaseClass::isWayland());

    m_pGmDbus        = new GmDbus(this);
    m_pGmAdaptor     = new GmAdaptor(m_pGmDbus);
    m_pGmThread      = new GmWorkThread(this);

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService(DBUS_GM_NAME)) {
        sessionBus.registerObject(DBUS_GM_PATH,
                                  m_pGmDbus,
                                  QDBusConnection::ExportAllContents);
        USD_LOG(LOG_DEBUG, "register gamma manager dbus success");
    } else {
        USD_LOG(LOG_ERR, "register dbus error");
    }
}

xManager::~xManager()
{
}

xManager *xManager::GammaManagerNew()
{
    if (m_gammaManager == nullptr) {
        m_gammaManager = new xManager();
    }

    return m_gammaManager;
}

bool xManager::Start()
{
    USD_LOG(LOG_DEBUG,"--Color start--");

    m_pGmLocation->start();
    doColorSettingsChanged("");
    return true;
}

void xManager::Stop()
{

}

int xManager::setPrimaryBrightness(int brightness)
{
     return m_pGmThread->setPrimaryBrightness(brightness);
}

int xManager::getPrimaryBrightness()
{
     return m_pGmThread->getPrimaryBrightness();
}

int xManager::setTemperature(int temp)
{
    USD_LOG(LOG_DEBUG,"set temp to %d",temp);
    if (m_pGmThread->getTemperature() == temp) {
        USD_LOG(LOG_DEBUG,"same value!!!");
        return 0;
    }

    m_pGmThread->setTemperature(temp);
    if (!m_pGmThread->isRunning()) {
        m_pGmThread->start();
    }

    return 0;
}

int xManager::enablePrimarySignal(int state)
{
    m_pGmThread->enablePrimarySignal(state);
    return state;
}

OutputGammaInfo xManager::getScreensInfo()
{
    OutputGammaInfo hdmi;
    OutputGammaInfo vga;
    hdmi.OutputName = "hdmi";
    hdmi.Gamma = 6500;
    hdmi.Brignthess = 100;

    vga.OutputName = "vga";
    vga.Gamma = 6000;
    vga.Brignthess = 80;

    return hdmi;
}

OutputGammaInfoList xManager::getScreensInfoList()
{
    return m_pGmThread->getAllOutputGammaInfo();
}

void xManager::doScreenBrightnessChanged(QString outputName, int outputBrightness)
{
    m_pGmThread->setBrightness(outputName,outputBrightness);
    if (!m_pGmThread->isRunning()) {
        m_pGmThread->start();
    }
    USD_LOG(LOG_DEBUG,"set %s to %d", outputName.toLatin1().data(), outputBrightness);
}

void xManager::setBrightness(const uint value)
{
    USD_LOG(LOG_DEBUG, "set brightness:%d", value);
    m_pGmThread->setAllOutputsBrightness(value);
    if (!m_pGmThread->isRunning()) {
        m_pGmThread->start();
    }
}
