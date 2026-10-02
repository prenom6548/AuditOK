/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

//
// Created by hxf on 22-9-23.
//

#ifndef UKUI_SIDEBAR_DESKTOP_BACKGROUND_H
#define UKUI_SIDEBAR_DESKTOP_BACKGROUND_H

#include <QObject>
#include <QRect>
#include <QPixmap>
#include <QQuickPaintedItem>

#include "commons.h"

class DesktopBackground : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(bool useDesktopBackground READ useDesktopBackground WRITE setUseDesktopBackground)
    Q_PROPERTY(BackgroundType::Type backgroundType READ backgroundType WRITE setBackgroundType)

public:
    explicit DesktopBackground(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;
    bool useDesktopBackground();
    void setUseDesktopBackground(bool used);

    BackgroundType::Type backgroundType();
    void setBackgroundType(BackgroundType::Type);

private Q_SLOTS:
    void updateBackground(const BackgroundType::Type &type, QPixmap pixmap);
    void refresh();

private:
    void rebuildBackgroundImage(const QImage &rawImage);
    static QRect getSourceRect(const QRect &targetRect, const QImage &image);

private:
    BackgroundType::Type m_backgroundType = BackgroundType::Null;
    QPixmap m_backgroundPixmap;
};

#endif //UKUI_SIDEBAR_DESKTOP_BACKGROUND_H
