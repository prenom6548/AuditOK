/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
#include "globalsettings.h"
#include <QVariant>
#include <mutex>

#define PANEL_SETTINGS "org.ukui.panel.settings"
#define PANEL_POSITION_KEY "panelposition"

using namespace UkuiWindowSwitch;

static std::once_flag onceFlag;
static GlobalSettings *g_globalSettings = nullptr;

GlobalSettings *GlobalSettings::globalInstance(QObject *parent)
{
    std::call_once(onceFlag, [ & ] {
        g_globalSettings = new GlobalSettings(parent);
    });
    return g_globalSettings;

}

int GlobalSettings::panelPosition()
{
    return m_panelPosition;
}

GlobalSettings::GlobalSettings(QObject *parent)
{
    initPanelGSettings();
}

void GlobalSettings::initPanelGSettings()
{
    const QByteArray id(PANEL_SETTINGS);
    if (!QGSettings::isSchemaInstalled(id)) {
        return;
    }
    if (m_panelGSettings == nullptr) {
        m_panelGSettings = new QGSettings(id, QByteArray(), this);
    }
    if (m_panelGSettings->keys().contains(PANEL_POSITION_KEY)) {
        m_panelPosition = m_panelGSettings->get(PANEL_POSITION_KEY).toInt();
        Q_EMIT panelPositionChanged(m_panelPosition);
    }
    connect(m_panelGSettings, &QGSettings::changed, this, [&] (const QString &key) {
        if (key == PANEL_POSITION_KEY) {
            m_panelPosition = m_panelGSettings->get(PANEL_POSITION_KEY).toInt();
            Q_EMIT panelPositionChanged(m_panelPosition);
        }
    });
}
