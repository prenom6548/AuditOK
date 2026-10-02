/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: anyongtai <anyongtai@kylinos.cn>
 *
 */
#include "full-screen-state-helper.h"
#include "settings.h"

namespace UkuiMenu {
FullScreenStateHelper *FullScreenStateHelper::instance()
{
    static FullScreenStateHelper instance(nullptr);
    return &instance;
}

bool FullScreenStateHelper::getFullScreenState()
{
    return m_windowMaximizeControl ? SecurityFunctionControl::instance()->windowMaximize() : m_gsettingsFullScreen;
}

void FullScreenStateHelper::setFullScreenState(bool isFullScreen)
{
    if (!m_windowMaximizeControl) {
        if (m_gsettingsFullScreen != isFullScreen) {
            m_gsettingsFullScreen = isFullScreen;
            MenuSetting::instance()->set(FULL_SCREEN, isFullScreen);
        }
    }
}

FullScreenStateHelper::FullScreenStateHelper(QObject *parent) : QObject(parent)
{
    auto sfc = SecurityFunctionControl::instance();
    m_windowMaximizeControl = sfc->windowMaximizeControl();
    m_gsettingsFullScreen = MenuSetting::instance()->get(FULL_SCREEN).toBool();

    connect(sfc, &SecurityFunctionControl::windowMaximizeControlChanged, this, [this, sfc] {
        if (m_windowMaximizeControl == sfc->windowMaximizeControl()) {
            return;
        }
        m_windowMaximizeControl = sfc->windowMaximizeControl();
        bool isFullScreen = m_windowMaximizeControl ? sfc->windowMaximize() : m_gsettingsFullScreen;
        Q_EMIT fullScreenStateChanged(isFullScreen);
    });

    connect(sfc, &SecurityFunctionControl::windowMaximizeChanged, this, [this, sfc] {
        if (!sfc->windowMaximizeControl()) {
            return;
        }
        Q_EMIT fullScreenStateChanged(sfc->windowMaximize());
    });

    connect(MenuSetting::instance(), &MenuSetting::changed, this, [this] (const QString& key) {
        if (key == FULL_SCREEN) {
            bool gsettingsFullScreen = MenuSetting::instance()->get(FULL_SCREEN).toBool();
            if (m_gsettingsFullScreen != gsettingsFullScreen) {
                m_gsettingsFullScreen = gsettingsFullScreen;
                if (!m_windowMaximizeControl) {
                    Q_EMIT fullScreenStateChanged(m_gsettingsFullScreen);
                }
            }
        }
    });
}
} // UkuiMenu
