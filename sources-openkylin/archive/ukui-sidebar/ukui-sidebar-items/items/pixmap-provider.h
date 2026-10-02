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
// Created by hxf on 22-11-9.
//

#ifndef UKUI_SIDEBAR_PIXMAP_PROVIDER_H
#define UKUI_SIDEBAR_PIXMAP_PROVIDER_H

#include <QString>
#include <QPixmap>
#include <QRunnable>
#include <QGSettings>
#include <QMutex>

#include "commons.h"

class PixmapProvider : public QObject
{
    Q_OBJECT
public:
    static PixmapProvider *instance();
    const QString &pictureOption();
    QPixmap &getPixmap(BackgroundType::Type type);
    void loadPixmap(BackgroundType::Type type);
    Q_INVOKABLE void setBlurPixmap(const QPixmap &saveToPixmap, const QString &fileName, const BackgroundType::Type &type);

private:
    explicit PixmapProvider(QObject *parent = nullptr);

    void loadDesktopBackground();
    void loadScreensaverPic();
    void loadBlurPixmapWorker(const QString &imageFile, BackgroundType::Type type, const QString &color = "");

private Q_SLOTS:
    void desktopPicChangedSlot(const QString& key);
    void screensaverPicChangedSlot(const QString& key);

Q_SIGNALS:
    void pixmapChanged(BackgroundType::Type type, const QPixmap &saveToPixmap);

private:
    QString m_primaryColor;
    QString m_pictureOptions;
    QString m_desktopFileName;
    QString m_screensaverFileName;

    QGSettings *m_backgroundGSetting = nullptr;
    QGSettings *m_screensaverGSetting = nullptr;

    QPixmap m_desktopPixmap;
    QPixmap m_screensaverPixmap;
    QMutex m_mutex;
};

class BlurPixmapWorker : public QRunnable
{
public:
    BlurPixmapWorker(const QString &imageFile, BackgroundType::Type type);
    void run() override;

private:
    QString m_imageFileName;
    BackgroundType::Type m_type;
};

#endif //UKUI_SIDEBAR_PIXMAP_PROVIDER_H
