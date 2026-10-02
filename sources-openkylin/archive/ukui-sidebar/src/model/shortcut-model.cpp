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

#include "shortcut-model.h"
#include "shortcut-config.h"
#include "shortcut-item.h"
#include "global-settings.h"

#include <QDebug>
#include <QIcon>

using namespace UkuiShortcut;
ShortcutModel::ShortcutModel(QObject *parent) : QAbstractListModel(parent)
{
    initShortcutModel();
    m_addModel = new EditShortcutModel(this, PluginStateType::Add, this);
    m_notAddModel = new EditShortcutModel(this, PluginStateType::NotAdd, this);
    m_progressBarModel = new ShortcutFilterModel(this, PluginStateType::Fix, this);
    qRegisterMetaType<EditShortcutModel*>("EditShortcutModel*");
    qRegisterMetaType<ShortcutFilterModel*>("ShortcutFilterModel*");
    updateCurrentMode(TABLET_MODE);
}

int ShortcutModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_shortcutItems.length();
}

QVariant ShortcutModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= m_shortcutItems.length()) {
        return {};
    }

    ShortcutItem* shortcut = m_shortcutItems.value(row);
    switch (role) {
        case Disabled: {
            return shortcut->disabled();
        }
        case Name: {
            return shortcut->name();
        }
        case StatusName: {
            return shortcut->statusName();
        }
        case WidgetId: {
            return shortcut->widgetId();
        }
        case Icon: {
            return shortcut->icon();
        }
        case ToolTip: {
            return shortcut->tooltip();
        }
        case Color: {
            return shortcut->color();
        }
        case Value: {
            return shortcut->value();
        }
        case MaxValue: {
          return shortcut->maxValue();
        }
        case Menu: {
            return shortcut->menu();
        }
        case Row: {
            return shortcut->row();
        }
        case Column: {
            return shortcut->column(m_currentMode);
        }
        case ItemType: {
            return shortcut->itemType(m_currentMode);
        }
        case IsEnable: {
            return shortcut->isEnable();
        }
        case IsVisible: {
            return shortcut->isVisible(m_currentMode);
        }
        case AddState: {
            return shortcut->addState();
        }
        case PluginIndex: {
            return shortcut->pluginIndex();
        }
        case PluginId: {
            return shortcut->pluginId();
        }
        case FixState: {
            return shortcut->fixState();
        }
        case IconClickable: {
            return shortcut->iconClickable();
        }
        default:
            break;
    }

    return {};
}

QHash<int, QByteArray> ShortcutModel::roleNames() const
{
    QHash<int, QByteArray> names;
    names.insert(Disabled, "disabled");
    names.insert(Name, "name");
    names.insert(StatusName, "statusName");
    names.insert(WidgetId, "widgetId");
    names.insert(Icon, "icon");
    names.insert(ToolTip, "tooltip");
    names.insert(Color, "color");
    names.insert(Value, "value");
    names.insert(MaxValue, "maxValue");
    names.insert(Menu, "menu");
    names.insert(Row, "row");
    names.insert(Column, "column");
    names.insert(ItemType, "itemType");
    names.insert(IsEnable, "isEnable");
    names.insert(IsVisible, "isVisible");
    names.insert(AddState, "addState");
    names.insert(PluginIndex, "pluginIndex");
    names.insert(PluginId, "pluginId");
    names.insert(IconClickable, "iconClickable");
//    names.insert(FixState, "fixState");
    return names;
}

void ShortcutModel::insertShortcut(Shortcut *shortcut)
{
    if (!shortcut) return;

    ShortcutItem* shortcutItem = new ShortcutItem(shortcut, this);

    beginInsertRows(QModelIndex(), m_shortcutItems.length(), m_shortcutItems.length());
    m_shortcutItems.append(shortcutItem);
    endInsertRows();

    connect(shortcutItem, &ShortcutItem::requestExecAction, this, &ShortcutModel::requestExecAction);
    connect(shortcutItem, &ShortcutItem::itemDataChanged, this, [=] (QVector<int> &roles) {
        QModelIndex current = index(m_shortcutItems.indexOf(shortcutItem), 0);
        if (m_disableValueUpdate && roles.contains(Value)) {
            roles.remove(roles.indexOf(Value));
            if (roles.empty()) {
                return;
            }
        }
        Q_EMIT dataChanged(current, current, roles);
    });
}

EditShortcutModel *ShortcutModel::getModel(PluginStateType::AddState state)
{
    return state == PluginStateType::Add ? m_addModel : m_notAddModel;
}

ShortcutFilterModel *ShortcutModel::prosessBarModel()
{
    return m_progressBarModel;
}

QString ShortcutModel::editIconName()
{
    return QIcon::fromTheme("edit-symbolic").isNull() ?
                "://icon/edit.svg" : "edit-symbolic";
}

void ShortcutModel::active(int index, PluginMetaType::Action action)
{
    if (index < 0 || index >= m_shortcutItems.size()) return;

    ShortcutItem *shortcutItem = m_shortcutItems.at(index);
    shortcutItem->active(action, m_currentMode);
}

void ShortcutModel::setValue(int index, int value)
{
    if (index < 0 || index >= m_shortcutItems.size()) return;

    ShortcutItem *shortcutItem = m_shortcutItems.at(index);
    shortcutItem->setValue(value);
}

void ShortcutModel::disableValueUpdate(bool disable)
{
    m_disableValueUpdate = disable;
}

void ShortcutModel::updateValue(int idx)
{
    if (idx < 0 || idx >= m_shortcutItems.size()) return;
    QModelIndex current = index(idx, 0);
    Q_EMIT dataChanged(current, current, {ShortcutModel::Value});
}

void ShortcutModel::updateCurrentMode(const QString &key)
{
    if (key == TABLET_MODE) {
        bool isTabletMode = Sidebar::GlobalSettings::globalInstance()->getValue(TABLET_MODE).toBool();
        auto mode = isTabletMode ? PluginMetaType::Tablet : PluginMetaType::PC;
        if (mode != m_currentMode) {
            m_currentMode = mode;
            if (m_shortcutItems.length() == 0) return;
            QModelIndex from = index(0, 0);
            QModelIndex to = index(m_shortcutItems.length() - 1, 0);
            dataChanged(from, to, {Column, ItemType, IsVisible, IconClickable});
        }
    }
}

void ShortcutModel::initShortcutModel()
{
    m_shortcutManager = ShortcutManager::getInstance();

    for (const auto &item : m_shortcutManager->getShortcuts()) {
        insertShortcut(item);
    }

    connect(Sidebar::GlobalSettings::globalInstance(), &Sidebar::GlobalSettings::valueChanged,
            this, &ShortcutModel::updateCurrentMode);
}
