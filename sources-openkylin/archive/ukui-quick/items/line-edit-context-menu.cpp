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

#include "line-edit-context-menu.h"
#include <QWindow>
#include <QQuickWindow>
#include <QGuiApplication>

namespace UkuiQuick {
LineEditContextMenu::LineEditContextMenu(QObject *prent) : QObject(prent)
{
}

LineEditContextMenu::~LineEditContextMenu()
{
    if(m_menu != nullptr) {
        delete m_menu;
        m_menu = nullptr;
    }
}

QQmlListProperty<QAction> LineEditContextMenu::qmlActions()
{
    return QQmlListProperty<QAction>(this, &m_actions);
}

void LineEditContextMenu::show(int x, int y, QQuickItem* item)
{
    if(m_menu == nullptr) {
        m_menu = new QMenu();
    }
    //调用winId()强制创建原生窗口资源，否则QWidget控件的windowHandle为空
    if(item) {
        m_menu->winId();
        if(m_menu->windowHandle()) {
            m_menu->windowHandle()->setTransientParent(item->window());
        }
    }
    m_menu->clear();
    for (QAction *action : m_actions) {
        m_menu->addAction(action);
    }
    if (x < 0 || y < 0) {
        m_menu->popup(QCursor::pos());
    } else {
        m_menu->popup(QPoint(x, y));
    }
}

void LineEditContextMenu::hide()
{
    if (m_menu) {
        m_menu->hide();
    }
}

QList<QAction *> LineEditContextMenu::actions() const
{
    return m_actions;
}

QString LineEditContextMenu::getShortCutString(QKeySequence::StandardKey shortcut)
{
    if(QCoreApplication::testAttribute(Qt::AA_DontShowShortcutsInContextMenus)) {
        return QString();
    }
    QList<QKeySequence> list = QKeySequence::keyBindings(shortcut);
    if(list.isEmpty()) {
        return QString();
    }
    return QLatin1Char('\t') + list.first().toString(QKeySequence::NativeText);
}

}
