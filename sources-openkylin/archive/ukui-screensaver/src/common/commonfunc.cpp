/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "commonfunc.h"

#include <QCursor>
#include <QGuiApplication>
#include <QScreen>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QSvgRenderer>
#include <QIcon>
#include <QImageReader>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QtMath>

QT_BEGIN_NAMESPACE
extern void
qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

#define BLUR_RADIUS 300

namespace {

bool isPlaceholderScreen(const QScreen *screen)
{
    if (!screen) {
        return true;
    }
    const QString name = screen->name();
    return name.isEmpty() || name == QLatin1String("FALLBACK");
}

} // namespace

QList<QScreen *> effectiveScreens(const QList<QScreen *> &screens)
{
    if (screens.size() <= 1) {
        return screens;
    }
    QList<QScreen *> filtered;
    filtered.reserve(screens.size());
    for (QScreen *screen : screens) {
        if (!isPlaceholderScreen(screen)) {
            filtered.append(screen);
        }
    }
    if (filtered.isEmpty()) {
        return screens;
    }
    return filtered;
}

QList<QScreen *> effectiveApplicationScreens()
{
    return effectiveScreens(QGuiApplication::screens());
}

// 设置一下鼠标的居于主屏中央
void setCursorCenter()
{
    auto setCursorPos = [=](QPoint p) { QCursor::setPos(p); };

    if (!qApp->primaryScreen()) {
        QObject::connect(qApp, &QGuiApplication::primaryScreenChanged, [=] {
            static bool first = true;
            if (first) {
                setCursorPos(qApp->primaryScreen()->geometry().center());
                first = false;
            }
        });
    } else {
        setCursorPos(qApp->primaryScreen()->geometry().center());
    }
}

/*修改图片缩放机制，图片长宽不一致时，先取图片中央的部分*/
QPixmap scaledPixmap(QPixmap src)
{
    QPixmap rectPixmap;
    if (src.width() > src.height()) {
        QPixmap iconPixmap = src.copy((src.width() - src.height()) / 2, 0, src.height(), src.height());
        // 根据label高度等比例缩放图片
        rectPixmap = iconPixmap.scaledToHeight(src.height());
    } else {
        QPixmap iconPixmap = src.copy(0, (src.height() - src.width()) / 2, src.width(), src.width());
        // 根据label宽度等比例缩放图片
        rectPixmap = iconPixmap.scaledToWidth(src.width());
    }
    return rectPixmap;
}

QPixmap scaledSmoothPixmap(const QPixmap &src, int nWidth, int nHeight)
{
    if (src.isNull()) {
        return QPixmap();
    }

    QPixmap pixmapa(src);
    QPixmap pixmap(nWidth, nHeight);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.drawPixmap(0, 0, nWidth, nHeight, pixmapa);
    return pixmap;
}

QPixmap PixmapToRound(const QPixmap &src, int radius)
{
    if (src.isNull()) {
        return QPixmap();
    }

    QPixmap pixmapa(src);
    QPixmap pixmap(radius * 2, radius * 2);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addEllipse(0, 0, radius * 2, radius * 2);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, radius * 2, radius * 2, pixmapa);
    return pixmap;
}

QPixmap PixmapToRound(const QPixmap &src, int leftTop, int rightTop, int leftBottom, int rightBottom)
{
    if (src.isNull()) {
        return QPixmap();
    }
    QPixmap roundedPixmap(src.size());
    roundedPixmap.fill(Qt::transparent);

    QPainter painter(&roundedPixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath path;
    path.moveTo(leftTop, 0);
    path.arcTo(QRect(0, 0, 2 * leftTop, 2 * leftTop), 90, 90);
    path.lineTo(0, src.height() - leftBottom);
    path.arcTo(QRect(0, src.height() - 2 * leftBottom, 2 * leftBottom, 2 * leftBottom), 180, 90);
    path.lineTo(src.width() - rightBottom, src.height());
    path.arcTo(
        QRect(src.width() - 2 * rightBottom, src.height() - 2 * rightBottom, 2 * rightBottom, 2 * rightBottom),
        270,
        90);
    path.lineTo(src.width(), rightTop);
    path.arcTo(QRect(src.width() - 2 * rightTop, 0, 2 * rightTop, 2 * rightTop), 0, 90);
    path.closeSubpath();

    painter.setClipPath(path);
    painter.drawPixmap(0, 0, src);
    painter.end();

    return roundedPixmap;
}

const QPixmap loadSvg(const QString &path, const QString color, int size)
{
    int origSize = size;
    const auto ratio = qApp->devicePixelRatio();
    if (2 == ratio) {
        size += origSize;
    } else if (3 == ratio) {
        size += origSize;
    }
    QPixmap pixmap(size, size);
    QSvgRenderer renderer(path);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);
    return drawSymbolicColoredPixmap(pixmap, color);
}

QPixmap drawSymbolicColoredPixmap(const QPixmap &source, QString cgColor)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if ("white" == cgColor) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                } else if ("black" == cgColor) {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                } else if ("gray" == cgColor) {
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                } else if ("blue" == cgColor) {
                    color.setRed(61);
                    color.setGreen(107);
                    color.setBlue(229);
                    img.setPixelColor(x, y, color);
                } else {
                    return source;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

QPixmap filledSymbolicColoredPix(
    const QPixmap &source, const QColor &baseColor, const QColor &symbolicColor, int colorDifference)
{
    if (source.isNull())
        return source;
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            QColor color = img.pixelColor(x, y);
            if (color.alpha() > 0.3) {
                if (qAbs(color.red() - symbolicColor.red()) < colorDifference
                    && qAbs(color.green() - symbolicColor.green()) < colorDifference
                    && qAbs(color.blue() - symbolicColor.blue()) < colorDifference) {
                    color.setRed(baseColor.red());
                    color.setGreen(baseColor.green());
                    color.setBlue(baseColor.blue());
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

QPixmap getLoadingIcon(int size)
{
    QPixmap icon = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(size, size);
    return drawSymbolicColoredPixmap(icon, "white");
}

QPixmap loadFromFile(QString strPath)
{
    QImageReader reader;
    reader.setFileName(strPath);
    reader.setAutoTransform(true);
    reader.setDecideFormatFromContent(true);
    return QPixmap::fromImageReader(&reader);
}

QRect wallpaperCoverSourceRect(const QSize &destination, const QSize &source)
{
    if (destination.isEmpty() || source.isEmpty()) {
        return QRect();
    }

    const QRect destinationRect(QPoint(0, 0), destination);
    const QRect sourceRect(QPoint(0, 0), source);

    qreal screenScale = qreal(destination.width()) / qreal(destination.height());
    qreal width = source.width();
    qreal height = source.height();

    /* 与 rootWindowBackground::getSourceRect 一致（含整数比例相等判断） */
    if ((width / height) == screenScale) {
        return sourceRect;
    }

    bool isShortX = (width <= height);
    if (isShortX) {
        screenScale = qreal(destination.height()) / qreal(destination.width());
    }

    qreal shortEdge = isShortX ? width : height;
    qreal longEdge = isShortX ? height : width;

    while (shortEdge > 1) {
        qint32 temp = qFloor(shortEdge * screenScale);
        if (temp <= longEdge) {
            longEdge = temp;
            break;
        }

        qint32 spacing = qRound(shortEdge / 20);
        if (spacing <= 0) {
            spacing = 1;
        }
        shortEdge -= spacing;
    }

    QSize sourceSize = source;
    if (shortEdge > 1 && longEdge > 1) {
        sourceSize.setWidth(isShortX ? int(shortEdge) : int(longEdge));
        sourceSize.setHeight(isShortX ? int(longEdge) : int(shortEdge));
    }

    qint32 offsetX = 0;
    qint32 offsetY = 0;
    if (source.width() > sourceSize.width()) {
        offsetX = (source.width() - sourceSize.width()) / 2;
    }
    if (source.height() > sourceSize.height()) {
        offsetY = (source.height() - sourceSize.height()) / 2;
    }

    QPoint offsetPoint = sourceRect.topLeft();
    offsetPoint += QPoint(offsetX, offsetY);

    return QRect(offsetPoint, sourceSize);
}

void drawPixmapWallpaperFill(QPainter &painter, const QPixmap &pixmap, const QRect &destRect)
{
    if (pixmap.isNull() || !destRect.isValid()) {
        return;
    }

    const QRect srcCrop = wallpaperCoverSourceRect(destRect.size(), pixmap.size());
    if (!srcCrop.isValid()) {
        return;
    }

    painter.save();
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.drawPixmap(destRect, pixmap, srcCrop);
    painter.restore();
}

QPixmap scalePixmapWallpaperFill(const QPixmap &pixmap, int width, int height)
{
    if (pixmap.isNull() || width <= 0 || height <= 0) {
        return QPixmap();
    }

    QPixmap result(width, height);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    drawPixmapWallpaperFill(painter, pixmap, QRect(0, 0, width, height));
    return result;
}

QPixmap scaleBlurPixmap(int width, int height, QString url)
{
    QFile imgFile(url);
    if (!imgFile.exists()) {
        // qDebug()<< "pixmap file not exist!";
        return QPixmap();
    }
    QImageReader imgReader;
    imgReader.setFileName(url);
    imgReader.setAutoTransform(true);
    imgReader.setDecideFormatFromContent(true);
    QPixmap pixmap = blurPixmap(QPixmap::fromImageReader(&imgReader));
    return scalePixmapWallpaperFill(pixmap, width, height);
}

QPixmap scaleWallpaperFillFromFile(int width, int height, const QString &url)
{
    QFile imgFile(url);
    if (!imgFile.exists()) {
        return QPixmap();
    }
    QImageReader imgReader;
    imgReader.setFileName(url);
    imgReader.setAutoTransform(true);
    imgReader.setDecideFormatFromContent(true);
    const QPixmap pixmap = QPixmap::fromImageReader(&imgReader);
    return scalePixmapWallpaperFill(pixmap, width, height);
}

//QPixmap blurPixmap(QPixmap pixmap)
//{
//    QPainter painter(&pixmap);
//    QImage srcImg = pixmap.toImage();
//    qt_blurImage(&painter, srcImg, BLUR_RADIUS, false, false);

//    // 在设置Qt::WA_TranslucentBackground属性后，模糊图片会导致锁屏界面透明
//    // 因此这里修改image图形的alpha值为255.
//    for (int y = 0; y < srcImg.height(); ++y) {
//        QRgb *row = (QRgb *)srcImg.scanLine(y);
//        for (int x = 0; x < srcImg.width(); ++x) {
//            ((unsigned char *)&row[x])[3] = 255;
//        }
//    }
//    painter.end();
//    return QPixmap::fromImage(srcImg);
//}

// 模糊算法优化，之前算法得到的图片会有黑边,解决传入图片特别小时，显示异常像素点的问题
QPixmap blurPixmap(QPixmap pixmap)
{
    int blurRadius;
    if (pixmap.width() > 500) {
        blurRadius = BLUR_RADIUS;
    } else {
        blurRadius = 30;
    }
    const int expandSize = blurRadius * 2;

    // 处理无效或空图像
    if (pixmap.isNull() || pixmap.width() == 0 || pixmap.height() == 0) {
        return pixmap;
    }

    // 创建扩展画布（增加边缘区域）
    QPixmap expanded(pixmap.width() + expandSize*2,
                     pixmap.height() + expandSize*2);
    expanded.fill(Qt::transparent);

    // 使用边缘像素填充扩展区域
    {
        QPainter expandPainter(&expanded);

        // 绘制原始图像到中心位置
        expandPainter.drawPixmap(expandSize, expandSize, pixmap);

        // 生成边缘平铺图案
        const QPixmap leftEdge = pixmap.copy(0, 0, 1, pixmap.height());
        const QPixmap rightEdge = pixmap.copy(pixmap.width()-1, 0, 1, pixmap.height());
        const QPixmap topEdge = pixmap.copy(0, 0, pixmap.width(), 1);
        const QPixmap bottomEdge = pixmap.copy(0, pixmap.height()-1, pixmap.width(), 1);

        // 平铺边缘像素到扩展区域
        expandPainter.drawTiledPixmap(0, expandSize, expandSize, pixmap.height(), leftEdge);      // 左侧
        expandPainter.drawTiledPixmap(expandSize + pixmap.width(), expandSize, expandSize, pixmap.height(), rightEdge); // 右侧
        expandPainter.drawTiledPixmap(expandSize, 0, pixmap.width(), expandSize, topEdge);       // 顶部
        expandPainter.drawTiledPixmap(expandSize, expandSize + pixmap.height(), pixmap.width(), expandSize, bottomEdge); // 底部

        // 处理四个角落
        const QPixmap topLeft = pixmap.copy(0, 0, 1, 1);
        const QPixmap topRight = pixmap.copy(pixmap.width()-1, 0, 1, 1);
        const QPixmap bottomLeft = pixmap.copy(0, pixmap.height()-1, 1, 1);
        const QPixmap bottomRight = pixmap.copy(pixmap.width()-1, pixmap.height()-1, 1, 1);

        expandPainter.drawTiledPixmap(0, 0, expandSize, expandSize, topLeft);                   // 左上角
        expandPainter.drawTiledPixmap(expandSize + pixmap.width(), 0, expandSize, expandSize, topRight); // 右上角
        expandPainter.drawTiledPixmap(0, expandSize + pixmap.height(), expandSize, expandSize, bottomLeft); // 左下角
        expandPainter.drawTiledPixmap(expandSize + pixmap.width(), expandSize + pixmap.height(),
                                      expandSize, expandSize, bottomRight); // 右下角
    }

    // 执行模糊操作
    QImage blurredImg = expanded.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage resultImg(blurredImg.size(), QImage::Format_ARGB32_Premultiplied);
    {
        QPainter painter(&resultImg);
        qt_blurImage(&painter, blurredImg, blurRadius, true, false);
    }

    // 裁剪有效区域
    resultImg = resultImg.copy(expandSize, expandSize, pixmap.width(), pixmap.height());

    // 保留原始透明度信息（移除强制设置alpha）
    return QPixmap::fromImage(resultImg.convertToFormat(QImage::Format_ARGB32));
}

QStringList getAudioFiles(const QString &directoryPath)
{
    QStringList audioFiles;
    QStringList audioFilters;
    audioFilters << "*.mp3" << "*.wav" << "*.ogg" << "*.flac" << "*.aac"
                << "*.m4a" << "*.wma" << "*.opus" << "*.aiff" << "*.ape";
    QDir directory(directoryPath);
    if (!directory.exists()) {
        qWarning() << "Directory does not exist:" << directoryPath;
        return audioFiles;
    }
    directory.setNameFilters(audioFilters);
    directory.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    QFileInfoList fileList = directory.entryInfoList();
    foreach (const QFileInfo &fileInfo, fileList) {
        audioFiles.append(fileInfo.absoluteFilePath());
    }
    return audioFiles;
}

QString getFirstImageFileIterator(const QString &dirPath)
{
    QStringList imageFilters;
    imageFilters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.gif" << "*.tiff";
    QDirIterator it(dirPath, imageFilters, QDir::Files);
    if (it.hasNext()) {
        return it.next();
    }

    return QString();
}

QString mapToThumbnailPath(const QString& originalPath)
{
    if (!QFileInfo::exists(originalPath)) {
        qWarning() << "原始壁纸文件不存在:" << originalPath;
        return QString();
    }

    QFileInfo originalFileInfo(originalPath);
    QString originalFileName = originalFileInfo.fileName();

    QString thumbnailDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                           + "/ukui-control-center/wallpaperData/";

    QString pattern = QString("\\d+-usr-share-backgrounds-%1").arg(originalFileName);
    QRegularExpression regex(pattern, QRegularExpression::CaseInsensitiveOption);

    QDir dir(thumbnailDir);
    QFileInfoList files = dir.entryInfoList(QDir::Files);

    foreach (const QFileInfo &file, files) {
        if (regex.match(file.fileName()).hasMatch()) {
            return file.absoluteFilePath();
        }
    }

    qWarning() << "未找到对应的缩略图文件，原始文件:" << originalFileName;
    return originalPath;
}
