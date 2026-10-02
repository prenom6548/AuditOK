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
 * Authors: qiqi49 <qiqi@kylinos.cn>
 *
 */

#include "menu-item.h"
#include "icon-helper.h"

namespace UkuiQuick {

/*!
    \qmltype MenuItem
    \inqmlmodule org.ukui.quick.items
    \ingroup ukui-quick-items
    \brief 表示菜单中的一个菜单项。

    MenuItem 组件封装了 QAction，提供了在 QML 中创建菜单项的功能。
    支持设置文本、图标、可选状态等属性，并可以响应点击事件。

    \section1 主要特性
    \list
        \li 支持文本和图标显示
        \li 可设置启用/禁用状态
        \li 支持可选中状态
        \li 提供 triggered 信号响应点击
    \endlist

    \section1 使用示例

    基本用法：
    \qml
    Menu {
        MenuItem {
            text: "打开"
            icon: "document-open"
            onTriggered: console.log("打开文件")
        }
        MenuItem {
            text: "保存"
            icon: "document-save"
            enabled: false
        }
        MenuItem {
            text: "自动保存"
            checkable: true
            checked: true
            onTriggered: console.log("自动保存:", checked)
        }
    }
    \endqml

    \sa Menu
*/

/*!
    \qmlproperty Action MenuItem::action

    底层的 QAction 对象。

    可以通过此属性访问或设置底层的 QAction 对象。
*/

/*!
    \qmlproperty string MenuItem::text

    菜单项显示的文本。
*/

/*!
    \qmlproperty string MenuItem::icon

    菜单项显示的图标名称。

    可以是主题图标名称或图标文件路径。
*/

/*!
    \qmlproperty bool MenuItem::enabled

    菜单项是否可用。

    当设置为 false 时，菜单项将显示为灰色且不可点击。
    默认值为 true。
*/

/*!
    \qmlproperty bool MenuItem::checkable

    菜单项是否可选中。

    当设置为 true 时，菜单项可以被选中或取消选中。
    默认值为 false。
*/

/*!
    \qmlproperty bool MenuItem::checked

    菜单项的选中状态。

    仅当 checkable 为 true 时有效。
    默认值为 false。
*/
MenuItem::MenuItem(QObject *parent)
{
    setAction(new QAction(this));
}

QAction *MenuItem::action() const
{
    return m_action;
}

void MenuItem::setAction(QAction *action)
{
    if (m_action != action) {
        if (m_action) {
            disconnect(m_action, nullptr, this, nullptr);
            if (m_action->parent() == this) {
                delete m_action;
                m_action = nullptr;
            }
        }

        if (action) {
            m_action = action;
        } else {
            m_action = new QAction(this);
            m_action->setVisible(false);
        }

        setEnabled(m_action->isEnabled());

        connect(m_action, &QAction::changed, this, &MenuItem::textChanged);
        connect(m_action, &QAction::changed, this, &MenuItem::checkableChanged);
        connect(m_action, &QAction::changed, this, &MenuItem::enabledChanged);
        connect(m_action, &QAction::toggled, this, &MenuItem::toggled);
        connect(m_action, &QAction::triggered, this, &MenuItem::clicked);
        connect(m_action, &QObject::destroyed, this, [this] {
            if (m_action->parent() != this) {
                m_action = new QAction(this);
                m_action->setVisible(false);
                Q_EMIT actionChanged();
            }
        });
        connect(this, &QObject::destroyed, this, &MenuItem::deleteLater);
        Q_EMIT actionChanged();
    }
}

QString MenuItem::icon() const
{
    return m_action->icon().name();
}

void MenuItem::setIcon(const QString &icon)
{
    m_action->setIcon(IconHelper::loadIcon(icon));
}

QString MenuItem::text() const
{
    return m_action->text();
}

void MenuItem::setText(const QString &text)
{
    if (m_action->text() != text) {
        m_action->setText(text);
    }
}

bool MenuItem::checkable() const
{
    return m_action->isCheckable();
}

void MenuItem::setCheckable(bool checkable)
{
    m_action->setCheckable(checkable);
}

bool MenuItem::checked() const
{
    return m_action->isChecked();
}

void MenuItem::setChecked(bool checked)
{
    m_action->setChecked(checked);
}

bool MenuItem::isEnabled() const
{
    return m_action->isEnabled();
}

void MenuItem::setEnabled(bool enabled)
{
    m_action->setEnabled(enabled);
}

}
