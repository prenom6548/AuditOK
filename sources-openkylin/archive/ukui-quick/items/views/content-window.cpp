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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

/*!
    \qmltype ContentWindow
    \inqmlmodule org.ukui.quick.items
    \ingroup ukui-quick-items
    \brief 提供基于屏幕可用区域定位的内容窗口组件。

    ContentWindow 是一个特殊的窗口组件，它基于屏幕可用区域进行定位。
    窗口的尺寸由显示的内容决定，位置通过 position 属性配合 margin 一起使用。

    \section1 主要特性
    \list
        \li 基于屏幕可用区域自动定位
        \li 支持多种位置选项（TopLeft、TopRight、BottomLeft、BottomRight 等）
        \li 窗口尺寸自动适应内容
        \li 支持设置到屏幕边缘的边距
        \li 支持多屏幕环境
    \endlist

    \section1 使用示例

    基本用法：
    \qml
    ContentWindow {
        id: dialog
        // 设置显示在哪个屏幕上
        screen: Qt.application.screens[0]
        // 在屏幕右下角显示
        position: Types.BottomRight
        // 到屏幕边缘的边距
        margin {
            left: 24
            top: 24
            right: 24
            bottom: 24
        }

        // 需要显示的内容，必须定义宽高
        Rectangle {
            width: 500
            height: 500
            color: "white"
            Text {
                anchors.centerIn: parent
                text: "内容窗口"
            }
        }
    }
    \endqml

    \sa Types, Margin
*/

/*!
    \qmlproperty Margin ContentWindow::margin
    \readonly

    到屏幕边缘的边距对象（只读）。

    可以通过此对象设置窗口到屏幕各边缘的距离：
    \qml
    ContentWindow {
        margin.left: 10
        margin.top: 10
        margin.right: 10
        margin.bottom: 10
    }
    \endqml
*/

/*!
    \qmlproperty Item ContentWindow::content
    \default

    窗口显示的内容项（默认属性）。

    内容项必须定义宽度和高度，窗口尺寸将与内容尺寸一致。
*/

/*!
    \qmlproperty var ContentWindow::screen

    窗口显示的屏幕对象。

    设置窗口应该显示在哪个屏幕上，可以使用 Qt.application.screens 获取可用屏幕列表。
*/

/*!
    \qmlproperty enumeration ContentWindow::position

    窗口在屏幕上的位置。

    可选值：
    \list
        \li Types.NoPosition - 无位置（不自动定位）
        \li Types.TopLeft - 左上角
        \li Types.Top - 顶部居中
        \li Types.TopRight - 右上角
        \li Types.Left - 左侧居中
        \li Types.Center - 屏幕中心
        \li Types.Right - 右侧居中
        \li Types.BottomLeft - 左下角
        \li Types.Bottom - 底部居中
        \li Types.BottomRight - 右下角
        \li Types.LeftCenter - 左侧居中（同 Left）
        \li Types.TopCenter - 顶部居中（同 Top）
        \li Types.RightCenter - 右侧居中（同 Right）
        \li Types.BottomCenter - 底部居中（同 Bottom）
    \endlist
*/

/*!
    \qmlproperty bool ContentWindow::useAvailableGeometry

    是否使用屏幕可用区域进行定位。

    \list
        \li true - 使用屏幕可用区域（排除任务栏等）
        \li false - 使用整个屏幕区域
    \endlist

    默认值为 true。
*/

/*!
    \qmlmethod void ContentWindow::reLocated()

    重新计算并更新窗口位置。

    当需要手动触发窗口重新定位时调用此方法。
*/

#include "content-window.h"
#include "ukui/screen-area-utils.h"

#include <QQuickItem>
#include <QScreen>
#include <QRect>

namespace UkuiQuick {

// ====== ContentWindow ======
ContentWindow::ContentWindow(QWindow *parent) : UKUIWindow(parent), m_margin(new Margin(this))
{
    setColor(QColor(Qt::transparent));
    setSkipTaskBar(true);
    setSkipSwitcher(true);

    connect(m_margin, &Margin::leftChanged, this, &ContentWindow::updateLocation);
    connect(m_margin, &Margin::topChanged, this, &ContentWindow::updateLocation);
    connect(m_margin, &Margin::rightChanged, this, &ContentWindow::updateLocation);
    connect(m_margin, &Margin::bottomChanged, this, &ContentWindow::updateLocation);
}

QScreen *ContentWindow::getScreen() const
{
    return screen();
}

void ContentWindow::setWindowScreen(QScreen *screen)
{
    if (screen == UKUIWindow::screen()) {
        return;
    }

    setScreen(screen);
    updateLocation();
    Q_EMIT screenChanged();
}

QQuickItem *ContentWindow::content() const
{
    return m_content;
}

void ContentWindow::setContent(QQuickItem *content)
{
    if (m_content == content) {
        return;
    }

    if (m_content) {
        m_content->setParentItem(nullptr);
        m_content->disconnect(this, nullptr);
    }

    m_content = content;
    if (m_content) {
        m_content->setParentItem(contentItem());

        onContentWidthChanged();
        onContentHeightChanged();
        updateLocation();

        connect(m_content, &QQuickItem::widthChanged, this, &ContentWindow::onContentWidthChanged);
        connect(m_content, &QQuickItem::heightChanged, this, &ContentWindow::onContentHeightChanged);
    }

    emit contentChanged();
}

Types::Pos ContentWindow::getPos() const
{
    return m_pos;
}

void ContentWindow::setPos(Types::Pos pos)
{
    if (m_pos == pos) {
        return;
    }

    m_pos = pos;
    updateLocation();
    emit positionChanged();
}

Margin *ContentWindow::margin() const
{
    return m_margin;
}

bool ContentWindow::useAvailableGeometry() const
{
    return m_useAvailableGeometry;
}

void ContentWindow::setUseAvailableGeometry(bool use)
{
    if (m_useAvailableGeometry == use) {
        return;
    }

    m_useAvailableGeometry = use;
    Q_EMIT useAvailableGeometryChanged();
    updateLocation();
    Q_EMIT positionChanged();
}

void ContentWindow::reLocated()
{
    updateLocation();
}

void ContentWindow::onContentWidthChanged()
{
    setWidth(m_content->width());
    updateLocation();
}

void ContentWindow::onContentHeightChanged()
{
    setHeight(m_content->height());
    updateLocation();
}

void ContentWindow::updateLocation()
{
    if (m_pos == Types::NoPosition) {
        return;
    }

    QScreen *currentScreen = getScreen();
    if (!currentScreen) {
        if (parent()) {
            currentScreen = parent()->screen();
        }

        if (!currentScreen) {
            return;
        }
    }

    QRect ar = m_useAvailableGeometry? ScreenAreaUtils::instance()->getAvailableGeometry(currentScreen) : currentScreen->geometry();
    int nx = 0, ny = 0;
    switch (m_pos) {
        case Types::TopLeft:
            nx = ar.left() + m_margin->left();
            ny = ar.top() + m_margin->top();
            break;
        case Types::TopRight:
            nx = ar.right() - width() - m_margin->right();
            ny = ar.top() + m_margin->top();
            break;
        case Types::BottomRight:
            nx = ar.right() - width() - m_margin->right();
            ny = ar.bottom() - height() - m_margin->bottom();
            break;
        case Types::BottomLeft:
            nx = ar.left() + m_margin->left();
            ny = ar.bottom() - height() - m_margin->bottom();
            break;
        case Types::Left:
            nx = ar.left() + m_margin->left();
            ny = ar.top() + ar.height()/2 - height()/2;
            break;
        case Types::Top:
            nx = ar.left() + ar.width()/2 - width()/2;
            ny = ar.top() + m_margin->top();
            break;
        case Types::Right:
            nx = ar.right() - width() - m_margin->right();
            ny = ar.top() + ar.height()/2 - height()/2;
            break;
        case Types::Bottom:
            nx = ar.left() + ar.width()/2 - width()/2;
            ny = ar.bottom() - height() - m_margin->bottom();
            break;
        case Types::Center:
            nx = ar.left() + ar.width()/2 - width()/2;
            ny = ar.top() + ar.height()/2 - height()/2;
            break;
        case Types::LeftCenter:
            nx = ar.left() + m_margin->left();
            ny = ar.top() + ar.height()/2 - height()/2;
            break;
        case Types::TopCenter:
            nx = ar.left() + ar.width()/2 - width()/2;
            ny = ar.top() + m_margin->top();
            break;
        case Types::RightCenter:
            nx = ar.right() - width() - m_margin->right();
            ny = ar.top() + ar.height()/2 - height()/2;
            break;
        case Types::BottomCenter:
            nx = ar.left() + ar.width()/2 - width()/2;
            ny = ar.bottom() - height() - m_margin->bottom();
            break;
        default:
            nx = ar.left();
            ny = ar.top();
            break;
    }

    setWindowPosition({nx, ny});
}

} // UkuiQuick
