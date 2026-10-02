/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#ifndef UKUISHOUTCUT_H
#define UKUISHOUTCUT_H

#include <memory>
#include <QList>
#include "ukui-shortcut_global.h"
#include "ukui-shortcut-plugin.h"
namespace UkuiShortcut {

class ShortcutManagerPrivate;
class UKUISHORTCUT_EXPORT ShortcutManager
{
public:
    static ShortcutManager* getInstance();
    virtual ~ShortcutManager();
    QList<Shortcut*> getShortcuts();

    QPair<bool, QString> getWidgetInfo(const QString &widgetId);

private:
    ShortcutManager();

private:
    const std::unique_ptr<ShortcutManagerPrivate> d;
};
}
#endif // UKUISHOUTCUT_H
