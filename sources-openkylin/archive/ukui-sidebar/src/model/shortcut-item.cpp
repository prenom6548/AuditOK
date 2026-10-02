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
 * Authors: youdiansaodongxi <guojiaqi@kylinos.cn>
 *
 */

#include "shortcut-item.h"
#include "shortcut-model.h"
#include "shortcut-config.h"

using namespace UkuiShortcut;

class UkuiShortcut::ShortcutItemPrivate
{
public:
    Shortcut* m_shortcut = {nullptr};
    StatusInfo m_statusInfos;
    PluginStateType::AddState m_addState;
    int m_pluginIndex;
    PluginStateType::FixState m_fixState;
};

UkuiShortcut::ShortcutItem::ShortcutItem(Shortcut *shortcut, QObject *parent) : QObject{parent}, d(new ShortcutItemPrivate)
{
    d->m_shortcut = shortcut;

    if (d->m_shortcut->pluginMetaData().value(PluginMetaType::PC).pluginType() == PluginMetaType::ProgressBar) {
        d->m_addState = PluginStateType::NotAdd;
        d->m_pluginIndex = d->m_shortcut->pluginMetaData().value(PluginMetaType::PC).index();
        d->m_shortcut->setAddedState(true);
        d->m_fixState = PluginStateType::Fix;
    } else {
        QString id = d->m_shortcut->pluginId();
        ShortcutConfig::instance().recordShortcut(id);
        d->m_addState = ShortcutConfig::instance().getShortcutState(id);
        d->m_pluginIndex = ShortcutConfig::instance().getShortcutIndex(id, d->m_addState);
        d->m_shortcut->setAddedState(d->m_addState == PluginStateType::Add);
        d->m_fixState = PluginStateType::NotFix;

        connect(&ShortcutConfig::instance(), &ShortcutConfig::configChanged, this, [=] {
            QString id = d->m_shortcut->pluginId();
            auto state = ShortcutConfig::instance().getShortcutState(id);
            int index = ShortcutConfig::instance().getShortcutIndex(id, state);
            QVector<int> roles(0);
            if (d->m_addState != state) {
                d->m_addState = state;
                d->m_shortcut->setAddedState(d->m_addState == PluginStateType::Add);
                roles.append(ShortcutModel::AddState);
            }
            if (d->m_pluginIndex != index) {
                d->m_pluginIndex = index;
                roles.append(ShortcutModel::PluginIndex);
            }
            if (roles.isEmpty()) return;
            Q_EMIT itemDataChanged(roles);
        });
    }

    d->m_statusInfos = d->m_shortcut->currentStatus();

    connect(d->m_shortcut, &Shortcut::enableStatusChanged, this, [=] {
        QVector<int> roles(0);
        roles.append(ShortcutModel::IsEnable);
        Q_EMIT itemDataChanged(roles);
    });

    connect(d->m_shortcut, &Shortcut::statusChanged, this, [=] (const StatusInfo &info) {
        QVector<int> roles(0);
        compareDataChanges(roles, info, d->m_statusInfos);
        d->m_statusInfos = info;
        if (roles.isEmpty()) return;
        Q_EMIT itemDataChanged(roles);
    });
}

ShortcutItem::~ShortcutItem()
{
    if(d) {
        delete d;
        d = nullptr;
    }
}

QString ShortcutItem::pluginId() const
{
    return d->m_shortcut->pluginId();
}

bool ShortcutItem::isEnable() const
{
    return d->m_shortcut->isEnable();
}

bool ShortcutItem::isVisible(PluginMetaType::SystemMode systemMode) const
{
    return d->m_shortcut->pluginMetaData().value(systemMode).isVisible();
}

bool ShortcutItem::disabled() const
{
    return d->m_statusInfos.isDisabled();
}

bool ShortcutItem::iconClickable() const
{
    return d->m_statusInfos.isIconClickable();
}

QString ShortcutItem::name() const
{
    return d->m_statusInfos.getName();
}

QString ShortcutItem::statusName() const
{
    return d->m_statusInfos.getStatusName();
}

QString ShortcutItem::widgetId() const
{
    return d->m_statusInfos.getWidgetId();
}

QString ShortcutItem::icon() const
{
    return d->m_statusInfos.getIcon();
}

QString ShortcutItem::tooltip() const
{
    return d->m_statusInfos.getToolTip();
}

Color::ColorRole ShortcutItem::color() const
{
    return d->m_statusInfos.getColor();
}

int ShortcutItem::value() const
{
    return d->m_statusInfos.getValue();
}

int ShortcutItem::maxValue() const
{
    return d->m_statusInfos.getMaxValue();
}

QVariant ShortcutItem::menu() const
{
    return QVariant::fromValue<QList<UkuiShortcut::StatusInfo::MenuItem> >(d->m_statusInfos.getMenu());
}

PluginMetaType::PluginType ShortcutItem::itemType(PluginMetaType::SystemMode systemMode) const
{
    return d->m_shortcut->pluginMetaData().value(systemMode).pluginType();
}

int ShortcutItem::row() const
{
    return 1;
}

int ShortcutItem::column(PluginMetaType::SystemMode systemMode) const
{
    return d->m_shortcut->pluginMetaData().value(systemMode).pluginType()
            == PluginMetaType::MenuButton ? 2 : 1;
}

PluginStateType::AddState ShortcutItem::addState() const
{
    return d->m_addState;
}

int ShortcutItem::pluginIndex() const
{
    return d->m_pluginIndex;
}

PluginStateType::FixState ShortcutItem::fixState() const
{
    return d->m_fixState;
}

void ShortcutItem::active(PluginMetaType::Action action, PluginMetaType::SystemMode systemMode)
{
    PluginMetaType::PredefinedAction preAction = d->m_shortcut->pluginMetaData().value(systemMode).preAction();
    if (preAction != PluginMetaType::NoAction) {
        Q_EMIT requestExecAction(preAction);
    }
    d->m_shortcut->active(action);
}

void ShortcutItem::setValue(int value)
{
    d->m_shortcut->setValue(value);
}

void ShortcutItem::compareDataChanges(QVector<int> &roles, const StatusInfo &infoA, const StatusInfo &infoB) const
{
    if (infoA.getName() != infoB.getName()) {
        roles.append(ShortcutModel::Name);
    }

    if (infoA.getStatusName() != infoB.getStatusName()) {
        roles.append(ShortcutModel::StatusName);
    }

    if (infoA.getWidgetId() != infoB.getWidgetId()) {
        roles.append(ShortcutModel::WidgetId);
    }

    if (infoA.getColor() != infoB.getColor()) {
        roles.append(ShortcutModel::Color);
    }

    if (infoA.getToolTip() != infoB.getToolTip()) {
        roles.append(ShortcutModel::ToolTip);
    }

    if (infoA.getIcon() != infoB.getIcon()) {
        roles.append(ShortcutModel::Icon);
    }

    if (infoA.isDisabled() != infoB.isDisabled()) {
        roles.append(ShortcutModel::Disabled);
    }

    if (infoA.isIconClickable() != infoB.isIconClickable()) {
        roles.append(ShortcutModel::IconClickable);
    }

    if (infoA.getValue() != infoB.getValue()) {
        roles.append(ShortcutModel::Value);
    }

    if (infoA.getMaxValue() != infoB.getMaxValue()) {
        roles.append(ShortcutModel::MaxValue);
    }
}
