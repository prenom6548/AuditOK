/*
 * UKUI Framework
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef EFFECT_H
#define EFFECT_H

#include <QObject>
#include <QDBusContext>

#include "gsettings_manager.h"
class Effect : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.UI.Effect")
public:
    explicit Effect(QObject *parent = nullptr);
    ~Effect();

public slots:
    /**
     * @brief GetWindowOpacity
     * 获取系统应用窗口透明部分的透明度
     * @return
     */
    Q_INVOKABLE int GetWindowOpacity() const;

    /**
     * @brief SetWindowOpacity
     * 设置系统应用窗口透明部分的透明度
     * @param opacity
     * @return
     */
    Q_INVOKABLE bool SetWindowOpacity(int opacity);

    /**
     * @brief GetWindowBlur
     * 获取当前窗口背景模糊特效开启状态
     * @return
     * true为开启，false为关闭
     */
    Q_INVOKABLE bool GetWindowBlur() const;

    /**
     * @brief SetWindowBlur
     * 设置当前窗口背景模糊特效是否开启
     * @param blur
     * @return
     */
    Q_INVOKABLE bool SetWindowBlur(bool blur);

signals:
    void WindowOpacityChanged();
    void WindowBlurChanged();

private:
    QHash<QString, OnKeyChanged> setChangedHandler();
    QHash<QString, OnKeyChanged> m_settingsChangedHandlers;

    GsettingsManager *m_pGsettingsManager = nullptr;
    const QString C_transparency = "transparency";
    const QString C_effect = "effect";


    int WindowOpacity = 100;
    bool WindowBlur = false;

    Q_PROPERTY(int WindowOpacity READ GetWindowOpacity WRITE SetWindowOpacity NOTIFY WindowOpacityChanged)
    Q_PROPERTY(bool WindowBlur READ GetWindowBlur WRITE SetWindowBlur NOTIFY WindowBlurChanged)
};

#endif // EFFECT_H
