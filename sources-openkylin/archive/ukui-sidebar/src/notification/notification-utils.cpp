/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include "notification-utils.h"

#include <utility>

using namespace UkuiNotification;

NotificationAction::NotificationAction(bool e, bool d, int index, Mode mode, QString icon, QString name, QString state)
: m_enable(e), m_default(d), m_index(index), m_mode(mode), m_icon(std::move(icon)), m_name(std::move(name)), m_state(std::move(state))
{

}

bool NotificationAction::isEnable() const
{
    return m_enable;
}

bool NotificationAction::isDefault() const
{
    return m_default;
}

void NotificationAction::setEnable(bool e)
{
    m_enable = e;
}

void NotificationAction::setDefault(bool d)
{
    m_default = d;
}

int NotificationAction::index() const
{
    return m_index;
}

NotificationAction::Mode NotificationAction::mode() const
{
    return m_mode;
}

QString NotificationAction::icon() const
{
    return m_icon;
}

QString NotificationAction::name() const
{
    return m_name;
}

QString NotificationAction::state() const
{
    return m_state;
}

void NotificationAction::setIndex(int index)
{
    m_index = index;
}

void NotificationAction::setMode(NotificationAction::Mode mode)
{
    m_mode = mode;
}

void NotificationAction::setIcon(const QString &icon)
{
    m_icon = icon;
}

void NotificationAction::setName(const QString &name)
{
    m_name = name;
}

void NotificationAction::setState(const QString &state)
{
    m_state = state;
}

QString NotificationAction::action() const
{
    return m_action;
}

void NotificationAction::setAction(const QString &action)
{
    m_action = action;
}
