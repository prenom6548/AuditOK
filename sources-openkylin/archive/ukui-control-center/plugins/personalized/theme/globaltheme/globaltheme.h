/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef GLOBALTHEME_H
#define GLOBALTHEME_H

#include <QObject>

class GlobalThemePrivate;

/*!
 * \brief The GlobalTheme class
 * 一个globaltheme ukui上是集成了各种主题的统一配置接口，目前它主要包含以下配置
 * 1、qt，qt控件的主题
 * 2、gtk，gtk控件的主题
 * 3、xdg，光标、图标、声音等主题
 * 4、wallpaper，壁纸
 * 5、effect，特效配置
 * 6、light-dark-mode，主题的亮色与暗色
 * 这些配置都写在了一个theme.conf文件中，有一些配置可能会在conf中声明支持与否，比如透明、毛玻璃等，
 * 如果不支持，则控制面板中关于这一块的自定义设置应该禁用
 */
class GlobalTheme : public QObject
{
    Q_OBJECT
public:
    explicit GlobalTheme(const QString &theme, QObject *parent = nullptr);
    explicit GlobalTheme(QObject *parent = nullptr); //custom globaltheme 使用

    Q_INVOKABLE virtual QString getThemeName();
    Q_INVOKABLE virtual QString getLocaleThemeName();

    Q_INVOKABLE virtual bool getSupportLightDarkMode();
    Q_INVOKABLE virtual QString getDefaultLightDarkMode();

    Q_INVOKABLE virtual QString getWidgetStyleName();
    Q_INVOKABLE virtual QString getQtLightThemeName();
    Q_INVOKABLE virtual QString getQtDarkThemeName();
    Q_INVOKABLE virtual QString getQtMixedThemeName();
    Q_INVOKABLE virtual QString getQtAccentRole();

    Q_INVOKABLE virtual QString getGtkThemeName();
    Q_INVOKABLE virtual QString getGtkLightThemeName();
    Q_INVOKABLE virtual QString getGtkDarkThemeName();
    Q_INVOKABLE virtual QString getGtkAccent();

    Q_INVOKABLE virtual QString getIconThemeName();
    Q_INVOKABLE virtual QString getCursorThemeName();
    Q_INVOKABLE virtual QString getSoundThemeName();
    Q_INVOKABLE virtual QString getWallPaperPath();

    Q_INVOKABLE virtual bool getSupportBlur();
    Q_INVOKABLE virtual bool getBlurEnabled();

    Q_INVOKABLE virtual bool getSupportTransparency();
    Q_INVOKABLE virtual int getTransparencyBlur();
    Q_INVOKABLE virtual int getTransparencyNoBlur();

    Q_INVOKABLE virtual bool getSupportAnimation();
    Q_INVOKABLE virtual int getAnimationDuration();

    Q_INVOKABLE virtual bool getSupportRadius();
    Q_INVOKABLE virtual QString  getRadiusValue();

    virtual void loadThumbnail();
    virtual QPixmap getThumbnail();

    /*!
     * \brief realPath
     * \return 主题配置所在的路径，用这个方法可以区分是系统安装的主题还是用户安装的主题
     */
    virtual QString realPath();

Q_SIGNALS:
    void thumbnailLoaded();

public Q_SLOTS:
    virtual void updateThumbnail(const QPixmap &pixmap);

private:
    Q_DECLARE_PRIVATE(GlobalTheme)
    GlobalThemePrivate *d_ptr;
};

#endif // GLOBALTHEME_H
