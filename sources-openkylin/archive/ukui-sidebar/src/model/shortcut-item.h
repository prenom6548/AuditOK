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

#ifndef SHORTCUTITEM_H
#define SHORTCUTITEM_H

#include <QObject>
#include <QColor>
#include <QVariant>
#include "ukui-shortcut.h"
#include "plugin-state.h"

namespace UkuiShortcut {

class ShortcutItemPrivate;
class ShortcutItem : public QObject
{
    Q_OBJECT
public:
    explicit ShortcutItem(Shortcut* shortcut, QObject *parent = nullptr);
    ~ShortcutItem();

    QString pluginId() const;
    bool isEnable() const;
    bool isVisible(PluginMetaType::SystemMode systemMode) const;

    bool disabled() const;
    bool iconClickable() const;
    QString name() const;
    QString statusName() const;
    QString widgetId() const;
    QString icon() const;
    QString tooltip() const;
    Color::ColorRole color() const;
    int value() const;
    int maxValue() const;
    QVariant menu() const;
    PluginMetaType::PluginType itemType(PluginMetaType::SystemMode systemMode) const;
    int row() const;
    int column(PluginMetaType::SystemMode systemMode) const;

    PluginStateType::AddState addState() const;
    int pluginIndex() const;
    PluginStateType::FixState fixState() const;

    void active(PluginMetaType::Action action, PluginMetaType::SystemMode systemMode);
    void setValue(int value);

Q_SIGNALS:
    void itemDataChanged(QVector<int> &roles);
    void requestExecAction(UkuiShortcut::PluginMetaType::PredefinedAction action);

private:
    void compareDataChanges(QVector<int> &roles, const StatusInfo &infoA, const StatusInfo &infoB) const;

    ShortcutItemPrivate *d = nullptr;
};
}
#endif // SHORTCUTITEM_H
