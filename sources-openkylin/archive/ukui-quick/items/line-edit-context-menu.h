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
 * Authors: amingamingaming <wangyiming01@kylinos.cn>
 *
 */
#ifndef UKUI_QUICK_ITEMS_LINE_EDIT_CONTENT_MENU_H
#define UKUI_QUICK_ITEMS_LINE_EDIT_CONTENT_MENU_H

#include <QMenu>
#include <QObject>
#include <QQmlListProperty>
#include <QQuickItem>

namespace UkuiQuick {
class LineEditContextMenu : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QAction> actions READ qmlActions)
public:
    explicit LineEditContextMenu(QObject *prent = nullptr);
    ~LineEditContextMenu();
    QQmlListProperty<QAction> qmlActions();
    Q_INVOKABLE void show(int x = -1, int y = -1, QQuickItem* item = nullptr);
    Q_INVOKABLE void hide();
    Q_INVOKABLE QString getShortCutString(QKeySequence::StandardKey shortcut);
    QList<QAction *> actions() const;

private:
    QList<QAction *> m_actions;
    QMenu *m_menu = nullptr;
};

}

#endif //UKUI_QUICK_ITEMS_LINE_EDIT_CONTENT_MENU_H
