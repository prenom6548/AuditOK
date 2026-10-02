/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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

#ifndef UKUI_QUICK_WINDOW_BLUR_BEHIND_H
#define UKUI_QUICK_WINDOW_BLUR_BEHIND_H
#include <QObject>
#include <QQmlListProperty>
#include <QQuickItem>
#include <window-helper.h>

#include "windows/region.h"

namespace UkuiQuick {

/**
 * @class WindowBlurBehind
 * 注意：使用这个类需要为目标window初始化WindowProxy类，如果未曾手动初始化，会自动初始化一个默认的WindowProxy类
        WindowBlurBehind {
            rectRegions: [
                RectRegion {
                    x: 0
                    y: 0
                    width: 100
                    height: 200
                    radius: 8
                },
                RectRegion {
                    x: 100
                    y: 200
                    width: 100
                    height: 200
                    radius: 0
                }
            ]
            enable: true
        }
 *
 *
 */
class WindowBlurBehind : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<UkuiQuick::RectRegion> rectRegions READ rectRegions)
    Q_PROPERTY(bool enable READ enable WRITE setEnable NOTIFY enableChanged)
    Q_PROPERTY(QWindow* window READ window WRITE setWindow NOTIFY windowChanged)
    Q_CLASSINFO("DefaultProperty", "rectRegions")

public:
    explicit WindowBlurBehind(QQuickItem *parent = nullptr);
    QQmlListProperty<RectRegion> rectRegions();

    void setEnable(bool enable);
    bool enable() const;

    void setWindow(QWindow *window);
    QWindow* window();

    /**
     * You cannot append elements to or clear a QQmlListProperty directly in QML.
     * To edit QQmlListProperty in QML, assign a new list to it.
     *
     * Note that objects cannot be individually added to or removed from the list once created;
     * to modify the contents of a list, it must be reassigned to a new list.
     */
    static void appendRect(QQmlListProperty<RectRegion> *list, RectRegion *rect);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    static qsizetype rectCount(QQmlListProperty<RectRegion> *list);
    static RectRegion *rectAt(QQmlListProperty<RectRegion> *list, qsizetype index);
    static void replaceRect(QQmlListProperty<RectRegion> *list, qsizetype index, RectRegion *rect);
#else
    static int rectCount(QQmlListProperty<RectRegion> *list);
    static RectRegion *rectAt(QQmlListProperty<RectRegion> *list, int index);
    static void replaceRect(QQmlListProperty<RectRegion> *list, int index, RectRegion *rect);
#endif
    static void clearRect(QQmlListProperty<RectRegion> *list);
    static void removeLastRect(QQmlListProperty<RectRegion> *list);

    Q_INVOKABLE void addRectRegion(RectRegion *rect);
    Q_INVOKABLE void clearRectRegion();

Q_SIGNALS:
    void enableChanged();
    void windowChanged();

private:
    void updateBlurRegion();
    QList<RectRegion *> m_rects;
    bool m_enable = false;
    QWindow *m_window = nullptr;
    QPointer<WindowProxy> m_windowProxy;
};

} // UkuiQuick

#endif //UKUI_QUICK_WINDOW_BLUR_BEHIND_H
