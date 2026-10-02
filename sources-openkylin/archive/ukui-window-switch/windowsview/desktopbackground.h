/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef DESKTOPBACKGROUND_H
#define DESKTOPBACKGROUND_H

#include <QQuickItem>
#include <QQuickPaintedItem>
#include <QImage>

#include <functional>



class QGSettings;
class QColor;

/*!
 * \brief   背景绘制的Item，导出由QML使用进行背景的绘制
 * \details 方便前端进行背景绘制，并提供虚化、背景图片拉伸的能力
 */

using ImageTask = std::function<QImage()>;
using ImageTaskCallBack = std::function<void(const QImage&)>;

class BackgroundImageHandler : public QObject
{
    Q_OBJECT
public:
    ~BackgroundImageHandler() = default;

    static void loadBackgroundImageAsync(const QString filePath, double scalingFactor, const QString fillStyle,
                                  QObject* context, ImageTaskCallBack loadFinishedCallback);
    static void blurImageAsync(const QImage &image, qreal radius,
                        QObject* context, ImageTaskCallBack blurFinishedCallback);
    static QImage blurImage(int blur_radius, const QImage &blurImage);

private:
    BackgroundImageHandler(ImageTask taskFunc, QObject* context, ImageTaskCallBack callback);
    static QImage loadBackgroundImage(const QString &filePath, double scalingFactor, const QString &fillStyle);
    static int edge(int x, int i, int w);
    static float gaussFunc1D(int x, float sigma);
    static void getKernal(int radius, float* weight);

Q_SIGNALS:
    void taskFinished(const QImage &image);
};

class DesktopBackground : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(qreal blurAlpha READ blurAlpha WRITE setBlurAlpha NOTIFY blurAlphaChanged)
public:
    explicit DesktopBackground(QQuickPaintedItem *parent = nullptr);
    ~DesktopBackground();
    void paint(QPainter *painter);
    qreal blurAlpha() { return m_blurAlpha; }
    void setBlurAlpha(qreal alpha);
    QRect scaledRect(const QImage &image);
    QRect getDestRect(const QImage &image, const QRectF &desktopRect);

    static void setNonBlurBackground(const QImage &image);
    static void setBlurBackground(const QImage &originImage, qreal blurAlpha);
    static double getSystemScalingFactor();
    static QString getBackgroundFile();
    static QString getBackgroundFillStyle();

signals:
    void loadPicture(const QString &fileName);
    void blurAlphaChanged();
    void blurImage(int radius, const QImage &blurImage);

private slots:
    void onBlurAlphaChanged();

private:
    void init();
    static QString getBackgroundFileFromAccount();
    QColor getBackgroundPrimaryColor();
    void updateBlurBackground();

private:
    QString m_backgroundFile;
    QString m_backgroundFillStyle = "scaled";
    double m_scalingFactor = 1.5;
    static QImage m_blurBackground;
    static QImage m_nonBlurBackground;
    qreal m_blurAlpha;
    static quint32 m_blurRadius;
    bool m_usePureColor = false;
    QColor m_pureColor = Qt::black;
};

#endif // DESKTOPBACKGROUND_H
