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

#include <QQuickWindow>
#include <QWindow>
#include <QCursor>
#include "menu.h"

namespace UkuiQuick {

/*!
    \qmltype Menu
    \inqmlmodule org.ukui.quick.items
    \ingroup ukui-quick-items
    \brief 提供 QML 中使用的菜单组件。

    Menu 组件封装了 QMenu，提供了在 QML 中创建和显示上下文菜单的功能。

    \section1 主要特性
    \list
        \li 支持声明式添加菜单项
        \li 可在指定位置显示菜单
        \li 支持设置临时父窗口
        \li 提供菜单可见性状态
    \endlist

    \section1 使用示例

    基本用法：
    \qml
    Menu {
        id: contextMenu
        MenuItem {
            text: "复制"
            onTriggered: console.log("复制")
        }
        MenuItem {
            text: "粘贴"
            onTriggered: console.log("粘贴")
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onClicked: contextMenu.open()
    }
    \endqml

    \sa MenuItem
*/

/*!
    \qmlproperty list<MenuItem> Menu::content
    \default

    菜单项列表属性。

    该属性用于在 QML 中声明式地添加菜单项。这是默认属性，
    可以直接在 Menu 内部声明 MenuItem。
*/

/*!
    \qmlproperty bool Menu::visible
    \readonly

    菜单的可见性状态（只读）。

    当菜单显示时为 true，隐藏时为 false。
*/

/*!
    \qmlproperty Item Menu::transientParent

    菜单的临时父项。

    设置此属性可以确保菜单窗口与指定的 QML Item 所在窗口建立正确的层级关系。
*/

/*!
    \qmlproperty bool Menu::anchoredPositioningEnabled

    是否启用基于 \l transientParent 的默认锚点定位。

    默认值为 \c false，以兼容历史行为：
    \list
        \li 当该属性为 \c false 时，\l transientParent 仅作为菜单窗口的 transient parent；若 \l open() 未显式传入坐标，则菜单回退到鼠标当前位置显示
        \li 当该属性为 \c true 时，若 \l open() 未显式传入坐标且 \l transientParent 可用，则使用 \l transientParent 作为默认定位参考项
    \endlist
*/

/*!
    \qmlproperty enumeration Menu::verticalAnchor

    菜单相对 \l transientParent 的垂直锚点。

    该属性仅在 \l anchoredPositioningEnabled 为 \c true 时生效。

    支持以下取值：
    \list
        \li \c Menu.Top: 菜单显示在 \l transientParent 上方
        \li \c Menu.Bottom: 菜单显示在 \l transientParent 下方
    \endlist

    默认值为 \c Menu.Bottom。
*/

/*!
    \qmlproperty enumeration Menu::horizontalAlignment

    菜单相对 \l transientParent 的水平对齐方式。

    该属性仅在 \l anchoredPositioningEnabled 为 \c true 时生效。

    支持以下取值：
    \list
        \li \c Menu.AlignLeft: 左对齐
        \li \c Menu.AlignCenter: 水平居中对齐
        \li \c Menu.AlignRight: 右对齐
    \endlist

    默认值为 \c Menu.AlignLeft。
*/

/*!
    \qmlproperty int Menu::margin

    菜单与 \l transientParent 之间的主方向间距。

    该属性仅在 \l anchoredPositioningEnabled 为 \c true 时生效。

    该属性仅作用于垂直方向的弹出间距：
    \list
        \li 当 \l verticalAnchor 为 \c Menu.Bottom 时，菜单会在下方额外留出该间距
        \li 当 \l verticalAnchor 为 \c Menu.Top 时，菜单会在上方额外留出该间距
    \endlist

    该属性不会改变 \l horizontalAlignment 的左右对齐结果。默认值为 \c 0。
*/
Menu::Menu(QObject *parent) : QObject(parent)
{
    m_menu = new QMenu(nullptr);
    connect(m_menu, &QMenu::aboutToHide, this, [this]() {
        if (!m_visible) {
            return;
        }
        m_visible = false;
        Q_EMIT visibleChanged();
    });
    connect(m_menu, &QMenu::aboutToShow, this, [this]() {
        if (m_visible) {
            return;
        }
        m_visible = true;
        Q_EMIT visibleChanged();
    });
}

Menu::~Menu()
{
    delete m_menu;
}

QQmlListProperty<MenuItem> Menu::content()
{
    return QQmlListProperty<MenuItem>(this, &m_items);
}

Menu::VerticalAnchor Menu::verticalAnchor() const
{
    return m_verticalAnchor;
}

void Menu::setVerticalAnchor(Menu::VerticalAnchor verticalAnchor)
{
    if (m_verticalAnchor == verticalAnchor) {
        return;
    }

    m_verticalAnchor = verticalAnchor;
    Q_EMIT verticalAnchorChanged();
}

Menu::HorizontalAlignment Menu::horizontalAlignment() const
{
    return m_horizontalAlignment;
}

void Menu::setHorizontalAlignment(Menu::HorizontalAlignment horizontalAlignment)
{
    if (m_horizontalAlignment == horizontalAlignment) {
        return;
    }

    m_horizontalAlignment = horizontalAlignment;
    Q_EMIT horizontalAlignmentChanged();
}

int Menu::margin() const
{
    return m_margin;
}

bool Menu::anchoredPositioningEnabled() const
{
    return m_anchoredPositioningEnabled;
}

void Menu::setAnchoredPositioningEnabled(bool enabled)
{
    if (m_anchoredPositioningEnabled == enabled) {
        return;
    }

    m_anchoredPositioningEnabled = enabled;
    Q_EMIT anchoredPositioningEnabledChanged();
}

void Menu::setMargin(int margin)
{
    if (m_margin == margin) {
        return;
    }

    m_margin = margin;
    Q_EMIT marginChanged();
}

/*!
    \qmlmethod void Menu::open(int x = -1, int y = -1)

    在指定位置打开菜单。

    \list
        \li \a x - 菜单显示的 X 坐标，默认为 -1 表示使用鼠标当前位置
        \li \a y - 菜单显示的 Y 坐标，默认为 -1 表示使用鼠标当前位置
    \endlist

    定位优先级如下：
    \list
        \li 如果显式传入了有效的 \a x 和 \a y，则优先使用该坐标显示菜单
        \li 如果未显式传入坐标，且 \l anchoredPositioningEnabled 为 \c true 并且设置了可用的 \l transientParent ，则根据 \l transientParent 、\l verticalAnchor 、\l horizontalAlignment 和 \l margin 计算菜单位置
        \li 其他情况均回退到鼠标当前位置显示
    \endlist
*/
void Menu::open(int x, int y)
{
    m_menu->clear();
    for (MenuItem *item : m_items) {
        m_menu->addAction(item->action());
    }

    if(m_transientParent) {
        m_menu->winId();
        if(m_menu->windowHandle()) {
            m_menu->windowHandle()->setTransientParent(m_transientParent->window());
        }
    }

    /**
     *  定位优先级：
     * 1. 显式传入的 open(x, y)
     * 2. anchoredPositioningEnabled为true，并有可用的transientParent，根据verticalAnchor、horizontalAlignment、margin计算菜单位置
     * 3. anchoredPositioningEnabled为false，鼠标当前位置
     *
     * [注] QMenu为Popup层级窗口，在显示时，窗管会自动调整窗口位置（避免显示在屏幕外部和任务栏下方），所以在这里调用popup()方法时，不需要获取可用屏幕区域、任务栏位置等信息
     * 后续其他层级窗口在显示时，需要注意此问题。
     */

    if (x >= 0 && y >= 0) {
        m_menu->popup(QPoint(x, y));
        return;
    }

    if (m_anchoredPositioningEnabled) {
        const QRect parentRect = transientParentGlobalRect();
        if (parentRect.isValid() ) {
            m_menu->popup(anchoredPopupPosition(parentRect, m_menu->sizeHint()));
            return;
        }
    }

    m_menu->popup(QCursor::pos());
}

/*!
    \qmlmethod void Menu::close()

    关闭菜单。

    隐藏当前显示的菜单。
*/
void Menu::close()
{
    m_menu->hide();
}

bool Menu::visible() const
{
    return m_visible;
}

void Menu::setTransientParent(QQuickItem *item)
{
    if(m_transientParent == item) {
        return;
    }
    m_transientParent = item;
    Q_EMIT transientParentChanged();
}

QPoint Menu::anchoredPopupPosition(const QRect &parentRect, const QSize &menuSize) const
{
    if (!parentRect.isValid()) {
        return {};
    }

    int x = parentRect.left();
    switch (m_horizontalAlignment) {
        case AlignCenter:
            x = parentRect.left() + (parentRect.width() - menuSize.width()) / 2;
            break;
        case AlignRight:
            x = parentRect.left() + parentRect.width() - menuSize.width();
            break;
        case AlignLeft:
        default:
            break;
    }

    int y = parentRect.top() + parentRect.height() + m_margin;
    if (m_verticalAnchor == Top) {
        y = parentRect.top() - menuSize.height() - m_margin;
    }

    return {x, y};
}

QRect Menu::transientParentGlobalRect() const
{
    if (!m_transientParent || !m_transientParent->window()) {
        return {};
    }

    QRect parentRect = m_transientParent->mapRectToScene(m_transientParent->boundingRect()).toAlignedRect();
    parentRect.moveTopLeft(m_transientParent->window()->mapToGlobal(parentRect.topLeft()));
    return parentRect;
}
}
