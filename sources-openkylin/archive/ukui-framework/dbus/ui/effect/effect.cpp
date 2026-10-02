/*
 * UKUI Framework
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "effect.h"


#undef signals
#include <gio/gio.h>
#include "qgsettings.h"


using namespace UKUI;

Effect::Effect(QObject *parent)
    : QObject{parent}
{
    m_pGsettingsManager = new GsettingsManager(parent, "org.ukui.control-center.personalise",
                                                       setChangedHandler());
    WindowOpacity = m_pGsettingsManager->get(C_transparency).toReal() * 100;
    WindowBlur = m_pGsettingsManager->get(C_effect).toBool();
}

Effect::~Effect()
{

}

int Effect::GetWindowOpacity() const
{
    PEEK_SCHEMA(C_transparency, WindowOpacity);
    return WindowOpacity;
}

bool Effect::SetWindowOpacity(int opacity)
{
    PEEK_AND_SET_AND_RET(C_transparency,  opacity/100.0, false);
    return true;
}

bool Effect::GetWindowBlur() const
{
    PEEK_SCHEMA(C_transparency, WindowBlur);
    return WindowBlur;
}

bool Effect::SetWindowBlur(bool blur)
{
    PEEK_AND_SET_AND_RET(C_effect,  blur, false);
    return false;
}

QHash<QString, OnKeyChanged> Effect::setChangedHandler()
{
    m_settingsChangedHandlers[C_effect] = [this]{
        WindowBlur = m_pGsettingsManager->get(C_effect).toBool();
        emit WindowOpacityChanged();
    };

    m_settingsChangedHandlers[C_transparency] = [this]{
        WindowOpacity = m_pGsettingsManager->get(C_transparency).toReal() * 100;
        emit WindowBlurChanged();
    };

    return m_settingsChangedHandlers;
}
