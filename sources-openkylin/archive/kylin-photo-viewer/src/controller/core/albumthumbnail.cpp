/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */
#include "albumthumbnail.h"
#include <convenient-utils.h>

AlbumThumbnail::AlbumThumbnail(const QString &path, const QString &realFormat)
{
    m_path = path;
    m_realFormat = realFormat;
}

void AlbumThumbnail::run()
{
    ImageAndInfo thumbnail;
    QVariant varThumbnail, varImageCache, varBigImageCache;
    if(Variable::g_exitApp.load() || isInterruptionRequested()){
        return;
    }
    QPair<ImageAndFileinfo, QImage> imagePix;
    ImageAndInfo bigImagePix;
    ImageAndFileinfo maf = File::loadImage(m_path, m_realFormat);
    if(Variable::g_exitApp.load() || isInterruptionRequested()){
        return;
    }
    imagePix.first = maf;
    if (!maf.image.isNull()) {
        QPixmap pix = Processing::converFormat(maf.image);
        if(Variable::g_exitApp.load() || isInterruptionRequested()){
            return;
        }
        imagePix.second = pix.toImage();
        if(Variable::g_exitApp.load() || isInterruptionRequested()){
            return;
        }
        if (pix.width() >= Variable::BIGRESOLUTION_IMAGE || pix.height() >= Variable::BIGRESOLUTION_IMAGE) {
            QPixmap tmpPix = Processing::resizePix(pix, Variable::g_widSize);
            bigImagePix.image = tmpPix.toImage();
        }
        pix = pix.scaled(Variable::ALBUM_IMAGE_SIZE, Qt::KeepAspectRatio);  // 采用快速缩放
        if(Variable::g_exitApp.load() || isInterruptionRequested()){
            return;
        }
        thumbnail.image = changImageSize(pix);
    }
    thumbnail.info = maf.info;
    bigImagePix.info = maf.info;

    varThumbnail.setValue<ImageAndInfo>(thumbnail);
    if(Variable::g_exitApp.load() || isInterruptionRequested()){
        return;
    }
    varImageCache.setValue<QPair<ImageAndFileinfo, QImage>>(imagePix);
    varBigImageCache.setValue<ImageAndInfo>(bigImagePix);
    Q_EMIT albumFinish(varThumbnail, varImageCache, varBigImageCache);
}

QImage AlbumThumbnail::changImageSize(const QPixmap &pix)
{
    if (pix.size() == QSize(Variable::ALBUM_IMAGE_SIZE.width(), Variable::ALBUM_IMAGE_SIZE.height() - 2)) {
        return pix.toImage();
    }

    int w = 0;
    int h = 0;

    w = Variable::ALBUM_IMAGE_SIZE.width() - pix.width();
    h = Variable::ALBUM_IMAGE_SIZE.height() - pix.height();

    QPixmap newPix(QSize(Variable::ALBUM_IMAGE_SIZE.width(), Variable::ALBUM_IMAGE_SIZE.height() - 2));
    newPix.fill(Qt::transparent);
    QPainter painter(&newPix);
    painter.drawPixmap(w / 2, h / 2, pix);
    return newPix.toImage();
}

MainImageLoad::MainImageLoad(const QString &path, const QString &realFormat)
{
    m_path = path;
    m_realFormat = realFormat;
}

void MainImageLoad::run()
{
    ImageAndFileinfo maf = File::loadImage(m_path, m_realFormat);
    QVariant varMainImage, varBigImage;
    ImageAndInfo bigImagePix;
    if (!maf.image.isNull()) {
        QPixmap pix = Processing::converFormat(maf.image);
        if (pix.width() >= Variable::BIGRESOLUTION_IMAGE || pix.height() >= Variable::BIGRESOLUTION_IMAGE) {
            QPixmap tmpPix = Processing::resizePix(pix, Variable::g_widSize);
            bigImagePix.image = tmpPix.toImage();
        }
    }
    bigImagePix.info = maf.info;
    varMainImage.setValue<ImageAndFileinfo>(maf);
    varBigImage.setValue<ImageAndInfo>(bigImagePix);
    Q_EMIT mainImageLoadFinish(m_path, varMainImage, varBigImage);
}

FindImageFromDir::FindImageFromDir(const QString &path)
{
    m_path = path;
}
void FindImageFromDir::run()
{
    QStringList images;
    QFileInfo info(m_path);
    QString path = info.absolutePath();//转成绝对路径
    QString uri = "file://" + path;

    QStringList fileList = Peony::ConvenientUtils::getInstance()->getFileUrisInSequence(uri);
    for (QString &filePath : fileList) {
        QString tmp = filePath;
        if (tmp.startsWith("file:///")) {
            tmp.replace("file:///", "/");
        }
        QString decodedPath = QUrl::fromPercentEncoding(tmp.toUtf8());
        if (QFileInfo::exists(decodedPath)) {
            QString format = decodedPath;
            format = format.split(".").last();
            if (Variable::SUPPORT_FORMATS.contains(format.toLower())) {
                images << decodedPath;
            }
        }
    }

    Q_EMIT findFinish(images, m_path);
}
