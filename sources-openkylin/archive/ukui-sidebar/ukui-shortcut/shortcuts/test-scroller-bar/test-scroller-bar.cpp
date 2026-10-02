/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 *
 */

//
// Created by hxf on 22-7-21.
//

#include "test-scroller-bar.h"

#include <QDebug>

namespace UkuiShortcut {

TestScrollerBar::TestScrollerBar(QObject *parent) : UkuiShortcutPlugin(parent)
{
    m_currentStatus.setName("Scroller Bar");
    m_currentStatus.setIcon(":/image/wifi_no.svg");
    m_currentStatus.setToolTip("test");
    m_currentStatus.setColor("white");
}

TestScrollerBar::~TestScrollerBar()
{

}

UkuiShortcutPlugin::PluginType TestScrollerBar::pluginType()
{
    return PluginType::ProgressBar;
}

void TestScrollerBar::active(UkuiShortcutPlugin::Action action)
{
    switch (action) {
        case Action::Click:
            break;
        case Action::LongClick:

            break;
        default:
            break;
    }
}

StatusInfo TestScrollerBar::currentStatus()
{
    return m_currentStatus;
}

bool TestScrollerBar::isEnable()
{
    return true;
}

void TestScrollerBar::updateSize(double size)
{
    m_size = size;

    if (m_size == 0) {
        m_currentStatus.setIcon(":/image/wifi_no.svg");

    } else if (m_size < 10) {
        m_currentStatus.setIcon(":/image/wifi_10.svg");

    } else if (m_size < 30) {
        m_currentStatus.setIcon(":/image/wifi_30.svg");

    } else {
        m_currentStatus.setIcon(":/image/wifi_80.svg");
    }

    m_currentStatus.setValue(m_size);
    StatusInfo statusInfo;
    statusInfo.setName("newName");
    statusInfo.setValue(80);
    statusInfo.setIcon(":/image/wifi_30.svg");
    qDebug() << "==TestScrollerBar== status info ready!" << &m_currentStatus;
//    Q_EMIT statusChanged(statusInfo);
    Q_EMIT statusChanged(m_currentStatus);
}

void TestScrollerBar::setValue(int value)
{
    updateSize(value);
}

} // UkuiShortCut
