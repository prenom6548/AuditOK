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

#ifndef SHORTCUTCONFIG_H
#define SHORTCUTCONFIG_H

#include <QObject>
#include <QPair>
#include <config.h>
#include "plugin-state.h"

namespace UkuiShortcut {

class ShortcutConfig : public QObject
{
    Q_OBJECT
public:
    static ShortcutConfig & instance();

    void setShortcutOrder(const int &indexFrom, const int &indexTo, PluginStateType::AddState state);
    void setShortcutOrder(const QString &idFrom, const int &indexTo, PluginStateType::AddState stateTo);

    void changeShortcutState(const QString &idFrom, PluginStateType::AddState stateFrom, bool atEnd);

    void recordShortcut(const QString &key);

    PluginStateType::AddState getShortcutState(const QString &key);

    int getShortcutIndex(const QString &key, PluginStateType::AddState state);

Q_SIGNALS:
    void configChanged(const QString &key);

private:
    explicit ShortcutConfig(QObject *parent = nullptr);

    void initConfig();
    void verify();

    UkuiQuick::Config *m_config {nullptr};
    int m_separateIndex = 5;
    QStringList m_orderList;
    QStringList m_addedList;
    QStringList m_notAddedList;

    QMap<QString, QString> m_idMap;
};
}
#endif // SHORTCUTCONFIG_H
