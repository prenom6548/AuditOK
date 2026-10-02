/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#pragma once


#include <QtQml>
#include <QQuickWindow>
#include <QQuickItem>
#include <QColor>
#include <QPainterPath>
#include <QPixmap>
//#include <KWindowShadow>
//#include <KWindowShadowTile>
#include "ukuiwindowhelper.h"
#include "tokenparameter.h"
#include <QDBusInterface>

namespace UKUIQQC2Style {

class UKUIPopupWindowHandle : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(bool posFollwMouse READ posFollwMouse WRITE setPosFollwMouse)

    /**
     * The enabled state of the window blur effect (set once at construction).
     */
    Q_PROPERTY(bool blur READ blur CONSTANT)
    /**
     * The strength of the window blur effect
     */
    Q_PROPERTY(int blurStrength WRITE setBlurStrength)

    /**
     *  The visible of server-side titlebar
     */
    Q_PROPERTY(bool titlebarVisible WRITE setTitlebarVisible)


    Q_PROPERTY(int radius WRITE setRadius)

    Q_PROPERTY(bool windowStaysOnTopHint WRITE setWindowStaysOnTopHint)


//    QML_NAMED_ELEMENT(PopupHandle)
//    QML_ATTACHED(UKUIPopupWindowHandle)

public:
    explicit UKUIPopupWindowHandle(QQuickItem *parent = nullptr);
    ~UKUIPopupWindowHandle() override;

    Q_INVOKABLE QPoint posByCursor(int width, int height);
    Q_INVOKABLE QSize getScreenSize();
    Q_INVOKABLE QPoint getScreenPoint();

    bool blur() const;
    bool posFollwMouse() const;
    void setPosFollwMouse(bool newPosFollwMouse);
    void setBlurStrength(int strength);
    void setBlurEnabled(bool enabled);
    void setTitlebarVisible(bool visible);
    void setRadius(int newRadius);
    void setWindowStaysOnTopHint(bool newWindowStaysOnTopHint);

protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    bool m_posFollwMouse = false;
    int m_blurStrength = 0;
    bool m_blurEnabled = true;
    bool m_titlebarVisible = true;
    bool m_isXCB = false;

    UkuiWindowHelper *m_windowHelper = nullptr;

    int m_radius = 0;
    bool m_windowStaysOnTopHint = false;

    QDBusInterface* m_wlcomDBus = nullptr;
    bool m_isHardwareRendering = true;

};
}

QML_DECLARE_TYPEINFO(UKUIQQC2Style::UKUIPopupWindowHandle, QML_HAS_ATTACHED_PROPERTIES)
