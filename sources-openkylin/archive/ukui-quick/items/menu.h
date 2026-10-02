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

#ifndef UKUI_QUICK_ITEMS_MENU_H
#define UKUI_QUICK_ITEMS_MENU_H

#include "menu-item.h"
#include <QMenu>
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QQmlListProperty>
#include <QQuickItem>
#include <QSize>

namespace UkuiQuick {

/**
 * Example :
 * import org.ukui.quick.items 1.0
 * MouseArea {
        Menu {
            id: menu
            transientParent: parent
            anchoredPositioningEnabled: true
            verticalAnchor: Menu.Bottom
            horizontalAlignment: Menu.AlignRight
            margin: 8
            content: [
                MenuItem {
                    text: "Click!!!!"
                    onClicked: console.log("menu:Clicked!!")
                }
            ]
        }
        onClicked: {
            menu.open();
        }
  }
 */

class Menu : public QObject
{
    Q_OBJECT
    // 错误
    // Q_PROPERTY(QQmlListProperty<MenuItem> content READ content)
    // 正确
    Q_PROPERTY(QQmlListProperty<UkuiQuick::MenuItem> content READ content)
    Q_PROPERTY(bool visible READ visible NOTIFY visibleChanged)
    // 仅作为菜单窗口的 transient parent；是否同时作为默认定位参考项由 anchoredPositioningEnabled 控制。
    Q_PROPERTY(QQuickItem* transientParent MEMBER m_transientParent WRITE setTransientParent NOTIFY transientParentChanged)
    // 控制是否启用基于 transientParent 的默认锚点定位；默认关闭以兼容历史鼠标弹出行为。
    Q_PROPERTY(bool anchoredPositioningEnabled READ anchoredPositioningEnabled WRITE setAnchoredPositioningEnabled NOTIFY anchoredPositioningEnabledChanged)
    // 控制菜单显示在 transientParent 的上方还是下方。
    Q_PROPERTY(VerticalAnchor verticalAnchor READ verticalAnchor WRITE setVerticalAnchor NOTIFY verticalAnchorChanged)
    // 控制菜单在水平方向相对 transientParent 的对齐方式。
    Q_PROPERTY(HorizontalAlignment horizontalAlignment READ horizontalAlignment WRITE setHorizontalAlignment NOTIFY horizontalAlignmentChanged)
    // 仅控制菜单与 transientParent 在主方向（上下方向）的间距，不影响左右对齐结果。
    Q_PROPERTY(int margin READ margin WRITE setMargin NOTIFY marginChanged)

public:
    enum VerticalAnchor {
        Top,
        Bottom
    };
    Q_ENUM(VerticalAnchor)

    enum HorizontalAlignment {
        AlignLeft,
        AlignCenter,
        AlignRight
    };
    Q_ENUM(HorizontalAlignment)

    explicit Menu(QObject *parent = nullptr);
    ~Menu();
    QQmlListProperty<MenuItem> content();
    bool visible() const;
    bool anchoredPositioningEnabled() const;
    void setAnchoredPositioningEnabled(bool enabled);
    VerticalAnchor verticalAnchor() const;
    void setVerticalAnchor(VerticalAnchor verticalAnchor);
    HorizontalAlignment horizontalAlignment() const;
    void setHorizontalAlignment(HorizontalAlignment horizontalAlignment);
    int margin() const;
    void setMargin(int margin);

    Q_INVOKABLE void open(int x = -1, int y = -1);
    Q_INVOKABLE void close();
    void setTransientParent(QQuickItem* item);

private:
    QPoint anchoredPopupPosition(const QRect &parentRect, const QSize &menuSize) const;
    QRect transientParentGlobalRect() const;

    QList<MenuItem *> m_items;
    QMenu *m_menu = nullptr;
    QQuickItem *m_transientParent = nullptr;
    bool m_visible = false;
    bool m_anchoredPositioningEnabled = false;
    VerticalAnchor m_verticalAnchor = Bottom;
    HorizontalAlignment m_horizontalAlignment = AlignLeft;
    int m_margin = 0;

Q_SIGNALS:
    void visibleChanged();
    void transientParentChanged();
    void anchoredPositioningEnabledChanged();
    void verticalAnchorChanged();
    void horizontalAlignmentChanged();
    void marginChanged();
};

}

#endif //UKUI_QUICK_ITEMS_MENU_H
