/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "AudioPulginUkui2.h"

#include <QDebug>
#include <QLibraryInfo>
#include <QStandardPaths>
#include <QLabel>

#include "../ui_audio.h"
#include "../MainWidget.h"


AudioPulginUkui2::AudioPulginUkui2() : m_firstLoad(true)
{
    /* ukui 2.0的控制面板加载插件的机制只会加载指定的插件列表，
    这里的翻译需要和控制面板的翻译一致，即中文对应为声音 */
    m_pluginName = tr("声音");
    m_pluginType = DEVICES;
}

AudioPulginUkui2::~AudioPulginUkui2()
{
    if (!m_firstLoad) {
        delete m_pUi;
    }
}

QString AudioPulginUkui2::get_plugin_name() {
    return m_pluginName;
}

int AudioPulginUkui2::get_plugin_type() {
    return m_pluginType;
}

QWidget* AudioPulginUkui2::get_plugin_ui() {
    if (m_firstLoad) {
        m_firstLoad = false;
        m_pUi = new Ui::Audio;
        m_pPluginWidget = new MainWidget();
        QLabel* pLable = new QLabel("test", m_pPluginWidget);
        m_pPluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        m_pUi->setupUi(m_pPluginWidget);
    }
    return m_pPluginWidget;
}

void AudioPulginUkui2::plugin_delay_control(){

}

const QString AudioPulginUkui2::name() const {
    return QStringLiteral("audio");
}

