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

#ifndef SHORTCUTMODEL_H
#define SHORTCUTMODEL_H

#include <QAbstractListModel>
#include "short-cut-manager.h"
#include "edit-shortcut-model.h"
#include "shortcut-filter-model.h"

namespace UkuiShortcut {

class ShortcutItem;

class ShortcutModel : public QAbstractListModel
{
    Q_OBJECT
public:

    enum Role {
        Disabled = 0,
        Name,
        StatusName,
        WidgetId,
        Icon,
        ToolTip,
        Color,
        Value,
        MaxValue,
        Menu,
        Row,
        Column,
        ItemType,
        IsEnable,
        IsVisible,
        AddState,
        PluginIndex,
        PluginId,
        FixState,
        IconClickable
    };
    Q_ENUM(Role)

    explicit ShortcutModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void insertShortcut(Shortcut *shortcut);

    Q_INVOKABLE UkuiShortcut::EditShortcutModel* getModel(PluginStateType::AddState state);

    Q_INVOKABLE UkuiShortcut::ShortcutFilterModel* prosessBarModel();

    Q_INVOKABLE QString editIconName();

    void active(int index, PluginMetaType::Action action);

    /**
     * 给插件设置value，可以反射
     * @brief setValue
     * @param index
     * @param value
     */
    void setValue(int index, int value);

    /**
     * 前端在拖动进度条时，需要屏蔽插件的value更新，用于保证前端进度条拖动的流畅性
     * @param disable
     */
    void disableValueUpdate(bool disable);

    void updateValue(int idx);

Q_SIGNALS:
    void requestExecAction(UkuiShortcut::PluginMetaType::PredefinedAction action);

private Q_SLOTS:
    void updateCurrentMode(const QString &key);

private:
    void initShortcutModel();

    ShortcutManager *m_shortcutManager = nullptr;
    QVector<ShortcutItem *> m_shortcutItems;

    PluginMetaType::SystemMode m_currentMode = PluginMetaType::PC;
    bool m_disableValueUpdate = false;

    EditShortcutModel* m_addModel = nullptr;
    EditShortcutModel* m_notAddModel = nullptr;
    ShortcutFilterModel* m_progressBarModel = nullptr;
};

} // UkuiShortcut
#endif // SHORTCUTMODEL_H
