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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef UKUI_QUICK_UKUI_WINDOW_H
#define UKUI_QUICK_UKUI_WINDOW_H

#include <QQuickWindow>

#include "window-helper.h"
#include "region.h"

namespace UkuiQuick {
class Region;
/**
 * @class UKUIWindow
 *
 * integration ukui-shell
 *
 * 集成ukui-shell
 * 1.设置窗口类型
 * 2.设置位置
 * 3.跳过任务栏，模糊特效等
 *
 */
class UKUIWindow : public QQuickWindow
{
    Q_OBJECT
    Q_PROPERTY(bool skipTaskBar READ skipTaskBar WRITE setSkipTaskBar NOTIFY skipTaskBarChanged)
    Q_PROPERTY(bool skipSwitcher READ skipSwitcher WRITE setSkipSwitcher NOTIFY skipSwitcherChanged)
    Q_PROPERTY(bool removeTitleBar READ removeTitleBar WRITE setRemoveTitleBar NOTIFY removeTitleBarChanged)
    Q_PROPERTY(UkuiQuick::WindowType::Type windowType READ windowType WRITE setWindowType NOTIFY windowTypeChanged)
    Q_PROPERTY(bool enableBlurEffect READ enableBlurEffect WRITE setEnableBlurEffect NOTIFY enableBlurEffectChanged)
    Q_PROPERTY(Region* blurRegion READ blurRegion CONSTANT FINAL)
    Q_PROPERTY(Region* maskRegion READ maskRegion CONSTANT FINAL)

public:
    explicit UKUIWindow(QWindow *parent = nullptr);
    ~UKUIWindow() override;

    WindowType::Type windowType() const;
    void setWindowType(WindowType::Type type);

    bool skipTaskBar() const;
    bool skipSwitcher() const;
    bool removeTitleBar() const;

    void setSkipTaskBar(bool skip);
    void setSkipSwitcher(bool skip);
    void setRemoveTitleBar(bool remove);
    QPoint windowPosition() const;
    void setWindowPosition(const QPoint &pos);

    bool enableBlurEffect() const;
    void setEnableBlurEffect(bool enable);

    Region* blurRegion() const;
    Region* maskRegion() const;

Q_SIGNALS:
    void windowTypeChanged();
    void skipTaskBarChanged();
    void skipSwitcherChanged();
    void removeTitleBarChanged();
    void enableBlurEffectChanged();

protected:
    void moveEvent(QMoveEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private Q_SLOTS:
    void blurRegionChanged();
    void maskRegionChanged();

private:
    class Private;
    Private *d {nullptr};
};

/**
 * @class UKUIWindowExtension
 *
 * 向qml中注册UKUIWindow,并为UKUIWindow设置x,y属性，适配wayland环境
 *
 * Usage:
     UKUIWindow {
        id: window
        x: 10
        y: 10
    }
 *
 */
class UKUIWindowExtension : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(int y READ y WRITE setY NOTIFY yChanged)
public:
    explicit UKUIWindowExtension(QObject *parent);

    int x() const;
    int y() const;

    void setX(int x);
    void setY(int y);

Q_SIGNALS:
    void xChanged(int x);
    void yChanged(int y);

private:
    UKUIWindow *m_window {nullptr};
};

} // UkuiQuick

#endif //UKUI_QUICK_UKUI_WINDOW_H
