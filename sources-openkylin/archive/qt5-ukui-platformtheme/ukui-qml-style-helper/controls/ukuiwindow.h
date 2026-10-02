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
#ifndef UKUIWINDOW_H
#define UKUIWINDOW_H

#include <QObject>
#include <QColor>
#include <QFont>
#include <QPalette>
#include <QQuickItem>
#include <QQuickWindow>
#include <QDBusInterface>
#include "ukuiwindowmanager.h"
#include "ukuiwindowhelper.h"

namespace UKUISDKStyle {

class UKUIWindow : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(bool useStyleWindowManager READ useStyleWindowManager WRITE setUseStyleWindowManager NOTIFY useStyleWindowManagerChanged)

    /**
     * The enabled state of the window blur effect
     */
    Q_PROPERTY(bool blurEnabled READ blurEnabled WRITE setBlurEnabled NOTIFY blurEnabledChanged)

    Q_PROPERTY(bool shadowEnabled READ shadowEnabled WRITE setShadowEnabled NOTIFY shadowEnabledChanged)

    Q_PROPERTY(bool borderEnabled READ borderEnabled WRITE setBorderEnabled NOTIFY borderEnabledChanged)

    Q_PROPERTY(bool roundCornerEnabled READ roundCornerEnabled WRITE setRoundCornerEnabled NOTIFY roundCornerEnabledChanged)

    Q_PROPERTY(bool removeTitleBar READ removeTitleBar WRITE setRemoveTitleBar NOTIFY removeTitleBarChanged)

    /// 关联 QQuickWindow 是否处于激活态（获得焦点）
    Q_PROPERTY(bool windowActive READ windowActive NOTIFY windowActiveChanged)

    /// 为 true 时清除 transientParent，使窗口成为独立顶层窗（可正常激活/失焦）
    Q_PROPERTY(bool standaloneWindow READ standaloneWindow WRITE setStandaloneWindow NOTIFY standaloneWindowChanged)

public:
    explicit UKUIWindow(QQuickItem *parent = nullptr);
    ~UKUIWindow();

    static UKUIWindow* qmlAttachedProperties(QObject* parent);


    bool useStyleWindowManager() const;
    Q_INVOKABLE void setUseStyleWindowManager(bool newUseStyleWindowManager);
    Q_INVOKABLE void clearTransientParent();

    bool blurEnabled() const;
    void setBlurEnabled(bool newBlur);

    bool shadowEnabled() const;
    void setShadowEnabled(bool newShadow);

    bool borderEnabled() const;
    void setBorderEnabled(bool newBorder);

    bool roundCornerEnabled() const;
    void setRoundCornerEnabled(bool newRoundCorner);

    bool removeTitleBar() const;
    void setRemoveTitleBar(bool newRemoveTitleBar);

    bool windowActive() const;

    bool standaloneWindow() const;
    void setStandaloneWindow(bool standalone);

public slots:


signals:

    void useStyleWindowManagerChanged();
    void blurEnabledChanged();
    void shadowEnabledChanged();
    void borderEnabledChanged();
    void roundCornerEnabledChanged();
    void removeTitleBarChanged();
    void windowActiveChanged();
    void standaloneWindowChanged();

private:
    void updateBlurEffect();
    void applyStandaloneWindow();

    bool m_useStyleWindowManager = false;
    QQuickWindow *m_quickWindow = nullptr;
    bool m_blurEnabled = false;
    bool m_shadowEnabled = true;
    bool m_borderEnabled = true;
    bool m_roundCornerEnabled = true;
    UkuiWindowHelper *m_windowHelper = nullptr;
    QDBusInterface* m_wlcomDBus = nullptr;
    bool m_isRendering = false;

    bool m_removeTitleBar = true;
    bool m_standaloneWindow = false;
};

}
QML_DECLARE_TYPEINFO(UKUISDKStyle::UKUIWindow, QML_HAS_ATTACHED_PROPERTIES)

#endif // APPPARAMETER_H
