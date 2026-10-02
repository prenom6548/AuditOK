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

#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QObject>
#include <QQuickImageProvider>
#include <QIcon>
#include <QUrl>
#include <QPixmap>
#include <QString>
#include "icon.h"

namespace UKUIQQC2Style {

class IconHelper
{
public:
    // 判断函数
    // 是否存在主题图标
    static bool isThemeIcon(const QString &name);
    // 是否本地文件
    static bool isLocalFile(const QUrl &url);
    // 远程文件： http or https
    static bool isRemoteServerFile(const QUrl &url);

    // 功能函数
    /**
     * Url转换为本地文件
     * 判断一个url是否本地文件。包括qrc
     * @param url
     * @return 返回可用于load的path，如果不是本地文件，返回空
     */
    static QString toLocalPath(const QUrl &url);

    // 图标相关
    // 从某个路径加载图标，并存入引用中，返回加载是否成功
    static bool initIcon(const QString &path, QIcon &icon);

    // 从路径或者主题加载图标
    static QIcon loadIcon(const QString &id);

    // 默认图标
    static QIcon getDefaultIcon();

    static QPixmap generatePixMap(const QString &id, Icon::Mode mode, const QSize &requestSize, QSize defaultSize);


private:
    static bool loadThemeIcon(const QString &name, QIcon &icon);
    static void loadDefaultIcon(QIcon &icon);
    static QPixmap generatedDisablePixmap(QPixmap pixmap);
    static QPixmap generatedHightlightPixmap(QPixmap pixmap);
    static QPixmap generatedNormalPixmap(QPixmap pixmap);

};


class ImageProvider : public QQuickImageProvider
{
public:
    explicit ImageProvider();

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);  //重写requestPixmap函数
};
}
#endif // IMAGEPROVIDER_H
