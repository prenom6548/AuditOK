/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#ifndef BACKGROUNDMANAGER_H
#define BACKGROUNDMANAGER_H

#include <QObject>
#include <QGSettings>
#include <QColor>
#include <QRect>
#include <QQmlEngine>
#include <QHash>
#include <QTimer>

#define BACKGROUND_MATE_SETTINGS "org.mate.background"
#define BACKGROUND_MATE_PATH "/org/mate/desktop/background/"
#define BACKGROUND_SETTINGS "org.ukui.interface"
#define BACKGROUND_PATH "/org/ukui/interface/"
#define ACCOUNT_SETTINGS "org.freedesktop.Accounts"
#define ACCOUNT_PATH "/org/freedesktop/Accounts"
#define PICTUREFILENAME "pictureFilename"
#define PICTUREOPTIONS "pictureOptions"
#define PRIMARYCOLOR "primaryColor"
#define PICTURE_FILENAME "picture-filename"
#define DEFAULTBACKGROUND "/usr/share/backgrounds/ubuntukylin-default-settings.jpg"

class QScreen;
class BackgroundManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString backgroundOption READ getBackgroundOption WRITE setBackgroundOption NOTIFY backgroundOptionChanged)
    Q_PROPERTY(QString accountBackground READ getAccountBackground WRITE setAccountBackground NOTIFY accountBackgroundChanged)
    Q_PROPERTY(bool usePureColor READ getUsePureColor WRITE setUsePureColor NOTIFY usePureColorChanged)
    Q_PROPERTY(QColor color READ getColor WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QString backImagePath READ getBackImagePath NOTIFY backImagePathChanged)
    Q_PROPERTY(QString frontImagePath READ getFrontImagePath NOTIFY frontImagePathChanged)
    Q_PROPERTY(QString videoWallpaperPath READ getVideoWallpaperPath NOTIFY videoWallpaperChanged)
    Q_PROPERTY(QSize frontImageSourceSize READ getFrontImageSourceSize NOTIFY frontImageSourceSizeChanged)
    Q_PROPERTY(QSize backImageSourceSize READ getBackImageSourceSize NOTIFY backImageSourceSizeChanged)

public:
    static BackgroundManager *globalInstance();

    Q_INVOKABLE QString getBackImagePath() const;
    Q_INVOKABLE QString getFrontImagePath() const;
    Q_INVOKABLE QString getVideoWallpaperPath() const;

    Q_INVOKABLE QString getBackgroundOption() const;
    Q_INVOKABLE void setBackgroundOption(const QString &backgroundOption);

    Q_INVOKABLE QString getAccountBackground() const;
    Q_INVOKABLE void setAccountBackground(const QString &backgroundPath);

    Q_INVOKABLE bool getUsePureColor() const;
    Q_INVOKABLE void setUsePureColor(const bool usePureColor);

    Q_INVOKABLE QColor getColor() const;
    Q_INVOKABLE void setColor(const QColor &color);

    Q_INVOKABLE QSize getFrontImageSourceSize() const;
    Q_INVOKABLE QSize getBackImageSourceSize() const;

    Q_INVOKABLE QRect calculateSpannedSourceRect(const QRect &screenGeometry, const QString &imagePath) const;
    Q_INVOKABLE QSize calculateOptimalSourceSize(const QString &imagePath) const;
    Q_INVOKABLE bool fileExists(const QString &path) const;

    void forceUpdateBackground();

private:
    explicit BackgroundManager(QObject *parent = nullptr);
    void initGSettings();
    void initBackground();
    void switchBackground();
    void initScreenMonitoring();
    void connectScreenDpiSignals(QScreen *screen);
    bool shouldUpdateForScreenChange() const;
    void scheduleUpdateSourceSizes();
    
    QSize maxScreenSize() const;
    QSize calculateWallpaperTargetSize(const QSize &originalSize, const QSize &screenSize) const;
    bool isLeftGreaterThanRightSize(const QSize &left, const QSize &right) const;
    QString normalizeImagePath(const QString &imagePath) const;
    void setVideoWallpaperPath(const QString &path);
    void updateSourceSizes();
    void updateImagePaths(const QString &newBackPath, const QString &newFrontPath);
    
    // Caching mechanism
    struct SourceSizeInfo {
        QSize size;
        qint64 lastModified;
        QSize maxScreenSize;
        QSize originalImageSize;
        bool operator==(const SourceSizeInfo& other) const {
            return size == other.size && 
                   lastModified == other.lastModified && 
                   maxScreenSize == other.maxScreenSize &&
                   originalImageSize == other.originalImageSize;
        }
    };
    
    mutable QHash<QString, SourceSizeInfo> m_sourceSizeCache;
    QSize getCachedOptimalSourceSize(const QString &imagePath) const;
    void clearSourceSizeCache();

public Q_SLOTS:
    void onDelayedUpdateSourceSizes();

Q_SIGNALS:
    void backgroundOptionChanged();
    void accountBackgroundChanged();
    void usePureColorChanged();
    void colorChanged();
    void backImagePathChanged();
    void frontImagePathChanged();
    void forceUpdated();
    void videoWallpaperChanged();
    void frontImageSourceSizeChanged();
    void backImageSourceSizeChanged();

private:
    QString m_backImagePath;
    QString m_frontImagePath;
    QString m_videoWallpaperPath;
    bool m_usePureColor = false;
    QColor m_color = Qt::black;
    QString m_backgroundOption;
    QGSettings *m_backgroundSettings = nullptr;
    bool m_shouldSyncAccountBackground = true;
    QSize m_frontImageSourceSize;
    QSize m_backImageSourceSize;
    mutable QSize m_lastMaxScreenSize;
    QTimer* m_delayedUpdateTimer = nullptr;
};

#endif // BACKGROUNDMANAGER_H
