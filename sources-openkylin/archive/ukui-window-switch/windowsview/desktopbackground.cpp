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

#include "desktopbackground.h"

#include <QSGSimpleRectNode>
#include <QPainter>
#include <QtMath>
#include <QGSettings>
#include <QElapsedTimer>
#include <QFile>
#include <QDBusInterface>
#include <QDBusReply>
#include <QApplication>
#include <QScreen>
#include <QThread>
#include <QBrush>
#include <QImageReader>

#include <algorithm>

#include <unistd.h>
#include <sys/types.h>

//! 默认的最大虚化程度
#define MAX_BLUR_ALPHA  20

static const QString backgroundSchemaUrl   = "org.mate.background";
static const QString fillStyleKeyName      = "pictureOptions";
static const QString fileNameKeyName       = "pictureFilename";

QImage DesktopBackground::m_blurBackground;
QImage DesktopBackground::m_nonBlurBackground;
quint32 DesktopBackground::m_blurRadius = 0;
static int backgroundNum = 0;

/*!
 * \brief 加载图片，支持 DIB 格式
 * \param filePath 图片文件路径
 * \return 加载成功返回图片，失败返回空图片
 */
static QImage loadImageWithImageReader(const QString &filePath)
{
    // 先尝试直接加载（支持大多数常见格式）
    QImage image(filePath);
    if (!image.isNull()) {
        return image;
    }

    // 如果直接加载失败，尝试使用 QImageReader 加载 BMP 格式
    // Qt 默认不支持直接加载纯 DIB 格式的文件‌，或者文件头信息不符合标准 BMP 规范，尝试指定格式
    QImageReader readerBmp(filePath, "BMP");
    if (readerBmp.canRead()) {
        image = readerBmp.read();
        if (!image.isNull()) {
            return image;
        } else {
            qDebug() << "Failed to read image:" << filePath << readerBmp.format() << readerBmp.errorString();
        }
    }

    return QImage();
}

DesktopBackground::DesktopBackground(QQuickPaintedItem *parent)
    : QQuickPaintedItem(parent)
    , m_blurAlpha(0)
{
    backgroundNum++;
    QElapsedTimer timer;
    timer.start();
    setFlag(ItemHasContents);
    init();
    qDebug() << "The DesktopBackground Constructor took" << timer.elapsed() << "milliseconds";
}

DesktopBackground::~DesktopBackground()
{
    backgroundNum --;
    m_blurRadius = 0;
}

void DesktopBackground::init()
{
    connect(this, &DesktopBackground::blurAlphaChanged, this, &DesktopBackground::onBlurAlphaChanged);

    const QByteArray id("org.mate.background");
    if(QGSettings::isSchemaInstalled(id)){
        QGSettings * backgSetting = new QGSettings(id, QByteArray(), this);
        connect(backgSetting, &QGSettings::changed,[=](QString key) {
            if ("pictureFilename" == key || "primaryColor" == key) {
                m_usePureColor = getBackgroundFile().isEmpty();
                if (!m_usePureColor) {
                    return;
                }
                m_pureColor = getBackgroundPrimaryColor();
                update();
            }
            if ("pictureOptions" == key) {
                m_backgroundFillStyle = getBackgroundFillStyle();
                update();
            }
        });
    }

    m_backgroundFile = getBackgroundFile();
    m_backgroundFillStyle = getBackgroundFillStyle();

    if (m_backgroundFile == "") {
        // no background file, use pure color
        qWarning() << "[Multitask]: 未获取到背景图片!";
        m_usePureColor = true;
        m_pureColor = getBackgroundPrimaryColor();
        return;
    }

    m_scalingFactor = getSystemScalingFactor();

    if (m_nonBlurBackground.isNull()) {
        QImage image = loadImageWithImageReader(m_backgroundFile);
        if (image.isNull()) {
            // 异常处理：加载背景图失败，则显示纯色背景
            m_usePureColor = true;
            return;
        } else {
            m_usePureColor = false;
        }

        m_nonBlurBackground = image;
        if (m_backgroundFillStyle == "centered" || m_backgroundFillStyle == "wallpaper") {
            // 居中 或 平铺
            m_nonBlurBackground = m_nonBlurBackground.scaled(m_nonBlurBackground.width() * m_scalingFactor,
                                                             m_nonBlurBackground.height() * m_scalingFactor);
        }
    }
}

void DesktopBackground::setBlurAlpha(qreal alpha)
{
    if(alpha < 0 || alpha > 1) {
        return;
    }
    m_blurAlpha = alpha;
    quint32 blur_radius = qRound(m_blurAlpha * MAX_BLUR_ALPHA);
    if (blur_radius != 0  && m_blurRadius != blur_radius) {
        m_blurRadius = qRound(m_blurAlpha * MAX_BLUR_ALPHA);
        emit blurAlphaChanged();
    }
}

QRect DesktopBackground::scaledRect(const QImage &image)
{
    qreal scaledAspectRatio = qreal(boundingRect().width()) / qreal(boundingRect().height());
    qreal width = image.width();
    qreal height = image.height();

    if ((width / height) == scaledAspectRatio) {
        return image.rect();
    }

    bool widthShorter = (width <= height);
    // scaledAspectRatio是一个必大于1的实数
    // 值为qMax(boundingRect().height(), boundingRect().width()) / qMax(boundingRect().width(), boundingRect().height())
    if (widthShorter) {
        scaledAspectRatio = qreal(boundingRect().height()) / qreal(boundingRect().width());
    }
    qreal shorterEdge = widthShorter ? width : height;
    qreal longerEdge = widthShorter ? height : width;
    while (shorterEdge > 1) {
        qint32 temp = qFloor(shorterEdge * scaledAspectRatio);
        if (temp <= longerEdge) {
            longerEdge = temp;
            break;
        }
        qint32 spacing = qRound(shorterEdge / 20);
        if (spacing <= 0) {
            spacing = 1;
        }
        shorterEdge -= spacing;
    }

    QSize scaledSize = image.size();
    if (shorterEdge > 1 && longerEdge > 1) {
        scaledSize.setWidth(widthShorter ? shorterEdge : longerEdge);
        scaledSize.setHeight(widthShorter ? longerEdge : shorterEdge);
    }
    qint32 offsetX = 0;
    qint32 offsetY = 0;
    if (image.width() > scaledSize.width()) {
        offsetX = (image.width() - scaledSize.width()) / 2;
    }
    if (image.height() > scaledSize.height()) {
        offsetY = (image.height() - scaledSize.height()) / 2;
    }
    QPoint offsetPoint = image.rect().topLeft();
    offsetPoint += QPoint(offsetX, offsetY);
    return QRect(offsetPoint, scaledSize);
}

QRect DesktopBackground::getDestRect(const QImage &image, const QRectF &desktopRect)
{
    qreal screenScale = qreal(desktopRect.width()) / desktopRect.height();
    qreal pixmapScale = qreal(image.width() / image.height());
    qreal width = image.width();
    qreal height = image.height();
    if (qFuzzyCompare(pixmapScale, screenScale)) {
        return QRect(0, 0, desktopRect.width(), desktopRect.height());
    }
    qreal scaleWidth = desktopRect.width() / width;
    qreal scaleHeight = desktopRect.height() / height;
    qreal realPixmapWidth = 0;
    qreal realPixmapHeight = 0;
    if (pixmapScale < screenScale) {
        //图片比例小于屏幕比例时，按照图片和屏幕高度比进行缩放
        realPixmapWidth = width * scaleHeight;
        realPixmapHeight = desktopRect.height();
    } else {
        //图片比例大于屏幕比例时，按照图片与屏幕宽度比进行缩放
        realPixmapWidth = desktopRect.width();
        realPixmapHeight = height * scaleWidth;
    }

    QSize sourceSize(desktopRect.width(), desktopRect.height());
    qint32 offsetX = 0;
    qint32 offsetY = 0;
    if (desktopRect.width() == realPixmapWidth) {
        offsetY = (desktopRect.height() - realPixmapHeight) / 2;
        sourceSize.setHeight(realPixmapHeight);
    } else if (desktopRect.height() == realPixmapHeight) {
        offsetX = (desktopRect.width() - realPixmapWidth) / 2;
        sourceSize.setWidth(realPixmapWidth);
    }
    // 规避xcb下闪线的问题
    sourceSize = sourceSize - QSize(1, 1);
    QPoint offsetPoint(0, 0);
    offsetPoint += QPoint(offsetX, offsetY);
    return QRect(offsetPoint, sourceSize);
}

void DesktopBackground::setNonBlurBackground(const QImage &image)
{
    m_nonBlurBackground = image;
}

void DesktopBackground::setBlurBackground(const QImage &originImage, qreal blurAlpha)
{
    m_blurBackground = BackgroundImageHandler::blurImage(blurAlpha * MAX_BLUR_ALPHA, originImage);
}

QString DesktopBackground::getBackgroundFile()
{
    if (!QGSettings::isSchemaInstalled(backgroundSchemaUrl.toUtf8())) {
        return QString();
    }

    QString filePath;
    QGSettings gsettings(backgroundSchemaUrl.toUtf8());
    const QStringList keyList = gsettings.keys();
    if (keyList.contains(fileNameKeyName)) {
        filePath = gsettings.get(fileNameKeyName).toString();
    } else {
        qWarning() << "[Multitask]: inValid key name" << fileNameKeyName;
        filePath = "";
    }
    if (!QFile::exists(filePath)) {
        // maybe desktop background picture was deleted, try AccountsService
        filePath = getBackgroundFileFromAccount();
    }
    return filePath;
}

QString DesktopBackground::getBackgroundFileFromAccount()
{
    uid_t uid = getuid();
    QDBusInterface iface("org.freedesktop.Accounts", "/org/freedesktop/Accounts",
                         "org.freedesktop.Accounts", QDBusConnection::systemBus());

    QDBusReply<QDBusObjectPath> userPath = iface.call("FindUserById", (qint64)uid);
    if(!userPath.isValid()) {
        qWarning() << "[Multitask]: UserPath error" << userPath.error();
    } else {
        QDBusInterface userIface("org.freedesktop.Accounts", userPath.value().path(),
                                 "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
        QDBusReply<QDBusVariant> backgroundReply = userIface.call("Get", "org.freedesktop.Accounts.User", "BackgroundFile");
        if(backgroundReply.isValid()) {
            return backgroundReply.value().variant().toString();
        }
    }
    return "";
}

QString DesktopBackground::getBackgroundFillStyle()
{
    if (!QGSettings::isSchemaInstalled(backgroundSchemaUrl.toUtf8())) {
        return QString();
    }

    QString fillStyle;
    QGSettings gsettings(backgroundSchemaUrl.toUtf8());
    const QStringList keyList = gsettings.keys();
    if (keyList.contains(fillStyleKeyName)) {
        fillStyle = gsettings.get(fillStyleKeyName).toString();
    } else {
        qWarning() << "[Multitask]: inValid key name" << fillStyleKeyName;
        fillStyle = "scaled";
    }
    return fillStyle;
}

QColor DesktopBackground::getBackgroundPrimaryColor()
{
    if (!QGSettings::isSchemaInstalled(backgroundSchemaUrl.toUtf8())) {
        return QColor();
    }

    QGSettings gsettings(backgroundSchemaUrl.toUtf8());
    const QStringList keyList = gsettings.keys();
    if (keyList.contains("primaryColor")) {
        auto color = gsettings.get("primaryColor").toString();
        return QColor(color);
    } else {
        qWarning() << "[Multitask]: inValid key name primaryColor";
        return QColor(Qt::color1) ;
    }
}

void DesktopBackground::updateBlurBackground()
{
    QElapsedTimer timer;
    timer.start();
    if (m_nonBlurBackground.isNull()) {
        return;
    }
    if (m_blurRadius > 0 && m_blurBackground.isNull()) {
        // 重新模糊背景图片
        m_blurBackground = BackgroundImageHandler::blurImage(m_blurAlpha * MAX_BLUR_ALPHA, m_nonBlurBackground);
    }
    qDebug() << "The DesktopBackground blur took" << timer.elapsed() << "milliseconds";
    //    m_blurBackground.save("/home/pangyi/backgroundImage.png", "PNG");
}

double DesktopBackground::getSystemScalingFactor()
{
    double scalingFactor = 1.5;
    QString schemaUrl = "org.ukui.SettingsDaemon.plugins.xsettings";
    QString keyName   = "scalingFactor";

    if (QGSettings::isSchemaInstalled(schemaUrl.toUtf8())) {
        QGSettings gsettings(schemaUrl.toUtf8());

        const QStringList keyList = gsettings.keys();
        if (keyList.contains(keyName)) {
            scalingFactor = gsettings.get(keyName).toDouble();
            //            qDebug() << "MMMMMM scalingFactor = " << scalingFactor;
        } else {
            qWarning() << QString("未获取到Gesetting的键值(%1) value").arg(keyName);
            // default
            return scalingFactor;
        }
    }
    return scalingFactor;
}

void DesktopBackground::onBlurAlphaChanged()
{
    if(m_nonBlurBackground.isNull()) {
        return;
    }
    updateBlurBackground();
    update();
}

void DesktopBackground::paint(QPainter *painter)
{
    if (m_usePureColor) {
        painter->fillRect(boundingRect(), m_pureColor);
        return;
    }
    QImage tmpImage;
    if (qRound(m_blurAlpha * MAX_BLUR_ALPHA) > 0) {
        tmpImage = m_blurBackground;
    } else {
        tmpImage = m_nonBlurBackground;
    }
    if (m_backgroundFillStyle == "centered") {
        //居中
        // scale tmpImage to fitin Desktop Preview Window
        if (tmpImage.width() > boundingRect().width() || tmpImage.height() > boundingRect().height()) {
            tmpImage = tmpImage.scaled(boundingRect().width(), boundingRect().height(), Qt::KeepAspectRatioByExpanding);
        }
        painter->drawImage((boundingRect().width() - tmpImage.width()) / 2,
                           (boundingRect().height() - tmpImage.height()) / 2,
                           tmpImage);
    } else if (m_backgroundFillStyle == "wallpaper") {
        //平铺
        // scale tmpImage to fitin Desktop Preview Window
        if (tmpImage.width() > boundingRect().width() || tmpImage.height() > boundingRect().height()) {
            tmpImage = tmpImage.scaled(boundingRect().width(), boundingRect().height(), Qt::KeepAspectRatioByExpanding);
        }
        // 1. 创建基于 tmpImage 的画刷
        // Qt::TexturePattern 告诉 Qt 这是一个纹理，需要平铺
        QBrush brush(tmpImage);
        brush.setStyle(Qt::BrushStyle::TexturePattern);
        // 2. 设置画刷
        painter->setBrush(brush);
        // 3. 设置笔为空，避免绘制边框
        painter->setPen(Qt::NoPen);
        // 4. 直接绘制整个边界矩形，Qt 会自动高效地平铺图片
        painter->drawRect(boundingRect());

    } else if (m_backgroundFillStyle == "scaled") {
        //填充
        painter->drawImage(boundingRect(), tmpImage, scaledRect(tmpImage));
    } else if (m_backgroundFillStyle == "stretched") {
        //拉伸
        painter->drawImage(boundingRect(), tmpImage);
    } else if (m_backgroundFillStyle == "zoom") {
        //适应
        painter->drawImage(getDestRect(tmpImage, boundingRect()), tmpImage, tmpImage.rect());
    } else if (m_backgroundFillStyle == "spanned") {
        //跨区
        painter->drawImage(boundingRect(), tmpImage, tmpImage.rect());
    } else {
        // default to scaled
        painter->drawImage(boundingRect(), tmpImage, scaledRect(tmpImage));
    }
}



int BackgroundImageHandler::edge(int x, int i, int w)
{
    // x 为中心位置, i 为偏移量, w 为图片的宽度/高度
    int inx = x + i;
    if(inx < 0 || inx >= w) {
        return x - i;
    } else {
        return inx;
    }
}

// 一维高斯函数
float BackgroundImageHandler::gaussFunc1D(int x, float sigma)
{
    float A = 1.0f / (sigma * sqrt(2 * 3.1415f));
    float index = -1.0 * (x * x) / (2 * sigma * sigma);
    return A * qExp(index);
}

// 获取线性的权值空间
void BackgroundImageHandler::getKernal(int radius, float* weight)
{
    float sigma = radius / 3.0f;
    float sum = 0.0f;
    // 获取权值空间weight[]
    for(int i = 0; i < 2 * radius + 1; i++) {
        weight[i] = gaussFunc1D(i - radius, sigma);
        sum += weight[i];
    }
    // 归一化
    for(int i = 0; i < 2 * radius + 1; i++) {
        weight[i] /= sum;
    }
}

// 获取最终的模糊图像
QImage BackgroundImageHandler::blurImage(int blur_radius, const QImage &image)
{
    QImage blurImage = image;
    int imgOriginWidth = blurImage.width();
    int imgOriginHeight = blurImage.height();
    // 模糊算法优化点：对原图进行缩放
    int maxImgWidth = 1920;
    if (blurImage.width() > maxImgWidth) {
        int scaleRatio = qRound((qreal)blurImage.width()/maxImgWidth * 2);
        blurImage = blurImage.scaled(blurImage.width()/scaleRatio, blurImage.height()/scaleRatio, Qt::KeepAspectRatio, Qt::FastTransformation);
    }

    // 保证图片格式可用
    if (blurImage.format() == QImage::Format::Format_Mono || blurImage.format() == QImage::Format::Format_MonoLSB) {
        blurImage.setColorCount(256); // set the color count to 256, which is 8-bit
        blurImage = blurImage.convertToFormat(QImage::Format_RGB32);
    }

    // 原图为 originImg
    // 第一个方向处理的图像为tmpImg
    int originWidth = blurImage.width();
    int originHeight = blurImage.height();
    int divisor = std::__gcd(originWidth, originHeight);
    int blurWidth = originWidth / divisor;
    int blurHeight = originHeight / divisor;
    int value = 1;
    if (blurWidth < 300) {
        while (true) {
            if ( ( value + 1 ) * blurWidth > 300 ) {
                break;
            } else {
                ++value;
            }
        }
        blurImage = blurImage.scaled(blurWidth * value, blurHeight * value, Qt::KeepAspectRatio);
    } else {   // 说明背景图没有找到足够大的最大公约数
        blurImage = blurImage.scaledToWidth(300);
    }
    //    qDebug() << __func__ << originWidth << originHeight << blurImage.width() << blurImage.height() << value << divisor;
    QImage tmpImg = blurImage;
    // 经过第二次处理的最终结果bluredImage
    float weight[2 * blur_radius + 1];
    getKernal(blur_radius, weight);
    int height = blurImage.height();
    int width = blurImage.width();
    float rgb[3];
    unsigned char *p, *p1, *ptmp;
    // 在横向进行一次相加
    for(int y = 0; y < height; y++) {
        p = blurImage.scanLine(y);
        for(int x = 0; x < width; x++) {
            rgb[0] = rgb[1] = rgb[2] = 0.0f;
            for(int i = -blur_radius; i <= blur_radius; i++) {
                // 边界处理后的对应的权值矩阵实际值
                int inx = edge(x, i, width);
                p1 = p + inx * 4;
                rgb[0] += p1[0] * weight[blur_radius + i];
                rgb[1] += p1[1] * weight[blur_radius + i];
                rgb[2] += p1[2] * weight[blur_radius + i];
            }
            ptmp = tmpImg.scanLine(y) + 4 * x;
            ptmp[0] = rgb[0];
            ptmp[1] = rgb[1];
            ptmp[2] = rgb[2];
        }
    }
    // 在纵方向对第一次的结果重新进行一次，先进行逆时针旋转90度进行高斯模糊，减少 scanLine() 函数的计算量
    QTransform matrix;
    matrix.rotate(-90.0);
    tmpImg = tmpImg.transformed(matrix, Qt::FastTransformation);
    blurImage = blurImage.transformed(matrix, Qt::FastTransformation);
    int height_rotate = blurImage.height();
    int width_rotate = blurImage.width();
    for(int y = 0; y < height_rotate; y++) {
        p = tmpImg.scanLine(y);
        for(int x = 0; x < width_rotate; x++) {
            rgb[0] = rgb[1] = rgb[2] = 0.0f;
            for(int i = -blur_radius; i <= blur_radius; i++) {
                // 边界处理后的对应的权值矩阵实际值
                int inx = edge(x, i, width_rotate);
                p1 = p + inx * 4;
                rgb[0] += p1[0] * weight[blur_radius + i];
                rgb[1] += p1[1] * weight[blur_radius + i];
                rgb[2] += p1[2] * weight[blur_radius + i];
            }
            ptmp = blurImage.scanLine(y) + 4 * x;
            ptmp[0] = rgb[0];
            ptmp[1] = rgb[1];
            ptmp[2] = rgb[2];
        }
    }
    matrix.rotate(180.0);
    blurImage = blurImage.transformed(matrix, Qt::FastTransformation);

    // smooth scale是耗时大户啊（将近90ms），暂时没想到优化方法！
    blurImage = blurImage.scaled(originWidth, originHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    // 将模糊图片缩放回原图尺寸
    blurImage = blurImage.scaled(imgOriginWidth, imgOriginHeight, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    return blurImage;
}

void BackgroundImageHandler::loadBackgroundImageAsync(const QString filePath, double scalingFactor, const QString fillStyle, QObject *context, ImageTaskCallBack loadFinishedCallback)
{
    auto taskFunc = [filePath, scalingFactor, fillStyle]() {
        return loadBackgroundImage(filePath, scalingFactor, fillStyle);
    };

    new BackgroundImageHandler(taskFunc, context, loadFinishedCallback);
}

void BackgroundImageHandler::blurImageAsync(const QImage &image, qreal radius, QObject *context, ImageTaskCallBack blurFinishedCallback)
{
    auto taskFunc = [radius, image]() {
        return blurImage(radius, image);
    };
    new BackgroundImageHandler(taskFunc, context, blurFinishedCallback);
}

BackgroundImageHandler::BackgroundImageHandler(ImageTask taskFunc, QObject *context, ImageTaskCallBack callback)
{
    auto* thread = QThread::create([this, taskFunc](){
        auto image = taskFunc();
        Q_EMIT taskFinished(image);
    });

    moveToThread(thread);

    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(thread, &QThread::finished, this, &QObject::deleteLater);
    connect(this, &BackgroundImageHandler::taskFinished, context, [callback, thread](const QImage image) {
        callback(image);
        if (thread->isRunning()) {
            thread->quit();
            thread->wait();
        }
    });

    thread->start();
}

QImage BackgroundImageHandler::loadBackgroundImage(const QString& filePath, double scalingFactor, const QString& fillStyle)
{
    QImage image = loadImageWithImageReader(filePath);
    if (scalingFactor != 1.0) {
        if (fillStyle == "centered" || fillStyle == "wallpaper") {
            // 居中 或 平铺
            image = image.scaled(image.width() * scalingFactor, image.height() * scalingFactor);
        }
    }
    return image;
}
