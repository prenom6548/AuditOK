/*
 * libkysdk-thememanager's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#ifndef KTHEMEMANAGER_H
#define KTHEMEMANAGER_H

#include <QObject>
#include <memory>

namespace kdk {

class KThemeManagerPrivate;

/**
 * @defgroup ThemeModule
 */

/**
 * @brief 主题控制类， 提供对主题的设置和获取功能
 * @since 3.0.1.0
 */

class KThemeManager :  public QObject
{
    Q_OBJECT

public:
    KThemeManager(QObject *parent = nullptr);
    virtual ~KThemeManager();

public:

    /**
     * @brief getUkccGlobalTheme 获取控制面板当前的公共主题类型
     * @return 控制面板的公共主题类型，如Light-Seeking、Heyin、custom
     * @since 3.0.1.0
     */
    QString     getUkccGlobalTheme();

    /**
     * @brief getSupportSystemStyle 获取系统支持的主题列表
     * @return 主题列表
     * @since 3.0.1.0
     */
    QStringList getSupportSystemStyle();

    /**
     * @brief getSystemThemeColor 获取当前系统强调色
     * @return 当前系统强调色。
     * @since 3.0.1.0
     */
    QString     getSystemThemeColor();

    /**
     * @brief getSystemFont 获取系统当前字体类型
     * @return 系统当前字体类型
     * @since 3.0.1.0
     */
    QString     getSystemFont();

    /**
     * @brief getSystemFontSize 获取系统当前字体大小
     * @return 字体大小
     * @since 3.0.1.0
     */
    int         getSystemFontSize();

    /**
     * @brief setSystemStyle 设置系统主题
     * @param style 系统主题，getSupportSystemStyle返回值之一
     * @return  成功返回true;失败返回false
     * @since 3.0.1.0
     */
    bool        setSystemStyle(const QString &style);

    /**
     * @brief setSystemFont 设置系统字体
     * @param font 字体全名，如"Noto Sans CJK SC"
     * @return  成功返回true;失败返回false
     * @since 3.0.1.0
     */
    bool        setSystemFont(const QString &font);

    /**
     * @brief setSystemFontSize 设置系统字体大小
     * @param size 字体大小
     * @return  成功返回true;失败返回false
     * @since 3.0.1.0
     */
    bool        setSystemFontSize(int size);

    /**
     * @brief setSystemThemeColor 设置系统强调色
     * @param color 系统强调色,从以下选项中选择一个
     * {"default","daybreakBlue","jamPurple","magenta","sunRed","sunsetOrange","dustGold","polarGreen"}
     * @return  成功返回true;失败返回false
     * @since 3.0.1.0
     */
    bool        setSystemThemeColor(const QString &color);

    /**
     * @brief setPanelSpecialEffect 设置窗口特效模式
     * @param effect true打开;false关闭
     * @return 成功返回true;失败返回false
     * @since 3.0.1.0
     */
    bool        setPanelSpecialEffect(bool effect);

    /**
     * @brief setTransparency 设置窗口透明度
     * @param tran 透明度
     * @return 成功返回true;失败返回false
     * @since 3.0.1.0
     */
    bool        setTransparency(double tran);

signals:
    void changed(const QString property, const QVariant value);

private:
    Q_DECLARE_PRIVATE(KThemeManager)
    std::unique_ptr<KThemeManagerPrivate> const d_ptr;
};

}
#endif // KTHEMEMANAGER_H
