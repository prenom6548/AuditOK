/*
 * UKUI Intelligent Data Manager
 *
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef WINDOWHELPER_H
#define WINDOWHELPER_H

#include <QObject>
#include <QWindow>

namespace UkuiQuick {
class WindowProxy2;
}
class QGSettings;

class WindowHelper : public QObject
{
    Q_OBJECT
public:
    explicit WindowHelper(QWindow *parent = nullptr);

    QWindow *getRootWindow() const;

    bool getIsWayland() const;

    bool getAutoHide() const;
    void setAutoHide(bool newAutoHide);

    bool eventFilter(QObject *watched, QEvent *event);

public slots:
    void handleSystemMoveRequest(QPointF pos);
    void handleOptionMenuRequest(QPointF pos);

    void activeWindow();
    void enableBlur();

    void removeHeaderBarX11();
    void removeHeaderBarWayland();

protected:
    void updateSettings();
    void updateGeometry(int panelPos);

private slots:
    void switchToWindowMode();
    void resetWindowState();

private:
    QWindow *rootWindow;
    bool isX11 = false;
    bool isWayland = false;

    UkuiQuick::WindowProxy2 *proxy = nullptr;
    QGSettings *settings = nullptr;
    QRect geometry = QRect(0, 0, 1096, 688);
    int panelPos = 0;
    int panelSize = 46;

    bool autoHide = true;
    bool isInslandMode = false;
};

#endif // WINDOWHELPER_H
