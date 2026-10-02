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

#include "pixmap-provider.h"

#include <QImage>
#include <QFile>
#include <QVariant>
#include <QPainter>
#include <QThreadPool>
#include <QMetaObject>
#include <QMutexLocker>

//opencv
#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>

#define BACKGROUND_SETTING_SCHEMA  "org.mate.background"
#define BACKGROUND_FILENAME        "pictureFilename"
#define PRIMARY_COLOR              "primaryColor"
#define PICTURE_OPTIONS            "pictureOptions"
#define DEFAULT_SIZE               {1920, 1080}

#define SCREENSAVER_SETTING_SCHEMA "org.ukui.screensaver"
#define SCREENSAVER_FILENAME       "background"

static PixmapProvider *globalInstance = nullptr;

PixmapProvider *PixmapProvider::instance()
{
    if (!globalInstance) {
        globalInstance = new PixmapProvider(nullptr);
    }
    return globalInstance;
}

PixmapProvider::PixmapProvider(QObject *parent) : QObject(parent)
{

}

void PixmapProvider::loadDesktopBackground()
{
    if (m_backgroundGSetting) {
        return;
    }

    QByteArray schema = QByteArray(BACKGROUND_SETTING_SCHEMA);
    if (QGSettings::isSchemaInstalled(schema)) {
        m_backgroundGSetting = new QGSettings(schema, "", this);
        if (m_backgroundGSetting->keys().contains(BACKGROUND_FILENAME)) {
            m_desktopFileName = m_backgroundGSetting->get(BACKGROUND_FILENAME).toString();
        }

        if (m_backgroundGSetting->keys().contains(PRIMARY_COLOR)) {
            m_primaryColor = m_backgroundGSetting->get(PRIMARY_COLOR).toString();
        }

        if (m_backgroundGSetting->keys().contains(PICTURE_OPTIONS)) {
            m_pictureOptions = m_backgroundGSetting->get(PICTURE_OPTIONS).toString();
        }

        connect(m_backgroundGSetting, &QGSettings::changed,
                this, &PixmapProvider::desktopPicChangedSlot);
    }

    loadBlurPixmapWorker(m_desktopFileName, BackgroundType::DesktopPic, m_primaryColor);
}

void PixmapProvider::loadScreensaverPic()
{
    if (m_screensaverGSetting) {
        return;
    }

    QByteArray schema = QByteArray(SCREENSAVER_SETTING_SCHEMA);
    if (QGSettings::isSchemaInstalled(schema)) {
        m_screensaverGSetting = new QGSettings(schema, "", this);
        if (m_screensaverGSetting->keys().contains(SCREENSAVER_FILENAME)) {
            m_screensaverFileName = m_screensaverGSetting->get(SCREENSAVER_FILENAME).toString();
        }

        connect(m_screensaverGSetting, &QGSettings::changed,
                this, &PixmapProvider::screensaverPicChangedSlot);
    }

    loadBlurPixmapWorker(m_screensaverFileName, BackgroundType::ScreensaverPic);
}

void PixmapProvider::desktopPicChangedSlot(const QString& key)
{
    if (key == BACKGROUND_FILENAME || key == PRIMARY_COLOR || key == PICTURE_OPTIONS) {
        if (key == BACKGROUND_FILENAME) {
            m_desktopFileName = m_backgroundGSetting->get(BACKGROUND_FILENAME).toString();
        } else if (key == PICTURE_OPTIONS) {
            m_pictureOptions = m_backgroundGSetting->get(PICTURE_OPTIONS).toString();
        } else {
            m_primaryColor = m_backgroundGSetting->get(PRIMARY_COLOR).toString();
        }

        loadBlurPixmapWorker(m_desktopFileName, BackgroundType::DesktopPic, m_primaryColor);
    }
}

void PixmapProvider::screensaverPicChangedSlot(const QString& key)
{
    if (key == SCREENSAVER_FILENAME) {
        m_screensaverFileName = m_screensaverGSetting->get(SCREENSAVER_FILENAME).toString();

        loadBlurPixmapWorker(m_screensaverFileName, BackgroundType::ScreensaverPic);
    }
}

QPixmap &PixmapProvider::getPixmap(BackgroundType::Type type)
{
    QMutexLocker locker(&m_mutex);
    switch (type) {
        default:
        case BackgroundType::DesktopPic: {
            return m_desktopPixmap;
        }
        case BackgroundType::ScreensaverPic: {
            return m_screensaverPixmap;
        }
    }
}

const QString &PixmapProvider::pictureOption()
{
    return m_pictureOptions;
}

void PixmapProvider::loadPixmap(BackgroundType::Type type)
{
    switch (type) {
        default:
        case BackgroundType::DesktopPic: {
            loadDesktopBackground();
            break;
        }
        case BackgroundType::ScreensaverPic: {
            loadScreensaverPic();
            break;
        }
    }
}

void PixmapProvider::setBlurPixmap(const QPixmap &saveToPixmap, const QString &fileName, const BackgroundType::Type &type)
{
    {
        QMutexLocker locker(&m_mutex);
        switch (type) {
            default:
            case BackgroundType::DesktopPic: {
                if (fileName != m_desktopFileName) {
                    return;
                }
                m_desktopPixmap = saveToPixmap;
                break;
            }

            case BackgroundType::ScreensaverPic: {
                if (fileName != m_screensaverFileName) {
                    return;
                }
                m_screensaverPixmap = saveToPixmap;
                break;
            }
        }
    }

    Q_EMIT pixmapChanged(type, saveToPixmap);
}

void PixmapProvider::loadBlurPixmapWorker(const QString &imageFile, BackgroundType::Type type, const QString &color)
{
    if (imageFile.isEmpty() || !QFile::exists(imageFile)) {
        QImage image;
        image = QImage(DEFAULT_SIZE, QImage::Format_ARGB32_Premultiplied);

        if (type == BackgroundType::DesktopPic) {
            image.fill(color.isEmpty() ? DEFAULT_COLOR : color);
        } else if (type == BackgroundType::ScreensaverPic) {
            image.fill(DEFAULT_COLOR);
        }

        setBlurPixmap(QPixmap::fromImage(image), imageFile, type);
        return;
    }

    auto *blurPixmapWorker = new BlurPixmapWorker(imageFile, type);
    QThreadPool::globalInstance()->start(blurPixmapWorker);
}

void BlurPixmapWorker::run()
{
    QImage image;
    image = QImage(m_imageFileName);

    int imageWidth = image.width(), imageHeight = image.height();

    image = image.scaled(1920, 1080, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    cv::Mat input = cv::Mat(image.height(), image.width(), CV_8UC4,
                            (void*)image.constBits(), image.bytesPerLine());
    cv::Mat output;
    //执行高斯模糊
    cv::GaussianBlur(input, output, cv::Size(31, 31), 0, 0);

    QImage outputImage(output.data, output.cols, output.rows,
                       static_cast<int>(output.step), QImage::Format_ARGB32_Premultiplied);

    outputImage = outputImage.scaled(imageWidth, imageHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QPixmap saveToPixmap = QPixmap::fromImage(outputImage.copy());

    if (saveToPixmap.isNull()) {
        saveToPixmap = QPixmap::fromImage(image);
    }

    QMetaObject::invokeMethod(PixmapProvider::instance(), "setBlurPixmap",
                              Qt::DirectConnection, Q_ARG(QPixmap, saveToPixmap),
                              Q_ARG(QString, m_imageFileName), Q_ARG(BackgroundType::Type, m_type));
}

BlurPixmapWorker::BlurPixmapWorker(const QString &imageFile, BackgroundType::Type type)
{
    m_imageFileName = imageFile;
    m_type = type;
}
