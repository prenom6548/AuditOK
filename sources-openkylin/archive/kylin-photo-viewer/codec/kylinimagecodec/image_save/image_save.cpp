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
#include "kylinimagecodec.h"

QStringList *KylinImageCodec::m_list = new QStringList;
QString KylinImageCodec::m_savePath = "";
bool KylinImageCodec::saveImage(const QImage &image, const QString &savepath, const QString &saveFormat, bool replace)
{
    return save(image, saveWay(savepath, replace), saveFormat);
}

bool KylinImageCodec::saveImage(QList<QImage> *list, const int &fps, const QString &savepath, const QString &saveFormat, bool replace)
{
    return save(list, fps, saveWay(savepath, replace), saveFormat);
}
//原格式保存
bool KylinImageCodec::save(const QImage &image, const QString &savepath, const QString &suffix)
{
    m_savePath = savepath;
    // 1. 特殊格式
    if (suffix == "svg") {
        // 1. 创建 Cairo SVG 表面
        cairo_surface_t* surface = cairo_svg_surface_create(
            savepath.toUtf8().constData(),
            image.width(),
            image.height()
        );

        if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
            qWarning() << "Failed to create SVG surface:"
                       << cairo_status_to_string(cairo_surface_status(surface));
            return false;
        }

        // 2. 创建 Cairo 上下文
        cairo_t* cr = cairo_create(surface);
        if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
            cairo_surface_destroy(surface);
            qWarning() << "Failed to create Cairo context:"
                       << cairo_status_to_string(cairo_status(cr));
            return false;
        }

        // 3. 转换 QImage 为 Cairo 兼容格式
        QImage convertedImage = image.convertToFormat(QImage::Format_ARGB32);
        if (convertedImage.isNull()) {
            cairo_destroy(cr);
            cairo_surface_destroy(surface);
            qWarning() << "Image conversion failed";
            return false;
        }

        // 4. 创建 Cairo 图像表面
        cairo_surface_t* imageSurface = cairo_image_surface_create_for_data(
                    convertedImage.bits(),
                    CAIRO_FORMAT_ARGB32,
                    convertedImage.width(),
                    convertedImage.height(),
                    convertedImage.bytesPerLine()
        );

        if (cairo_surface_status(imageSurface) != CAIRO_STATUS_SUCCESS) {
            cairo_destroy(cr);
            cairo_surface_destroy(surface);
            qWarning() << "Failed to create image surface";
            return false;
        }

        // 5. 绘制图像到 SVG
        cairo_set_source_surface(cr, imageSurface, 0, 0);
        cairo_paint(cr);

        // 6. 添加元数据 (可选)
        cairo_svg_surface_restrict_to_version(surface, CAIRO_SVG_VERSION_1_2);
        cairo_svg_surface_set_document_unit(surface, CAIRO_SVG_UNIT_PX);

        // 7. 清理资源
        cairo_surface_destroy(imageSurface);
        cairo_destroy(cr);
        cairo_surface_flush(surface);
        cairo_surface_destroy(surface);

        return true;
    }
    if (suffix == "ras") {
        QImage rgbImage = image.convertToFormat(QImage::Format_RGB888);
        QFile file(savepath);
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "无法打开文件:" << savepath;
            return false;
        }

        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::BigEndian);  // RAS 使用大端字节序

        // RAS 文件头 (28 字节)
        const quint32 magicNumber = 0x59a66a95;     // RAS 魔数
        const quint32 width = rgbImage.width();
        const quint32 height = rgbImage.height();
        const quint32 depth = 24;                   // 24位彩色
        const quint32 length = width * height * 3;  // 图像数据长度 (字节)
        const quint32 type = 0;                     // 原始 RAS 格式
        const quint32 mapType = 0;                  // 无颜色映射
        const quint32 mapLength = 0;                // 无颜色映射

        stream << magicNumber;
        stream << width;
        stream << height;
        stream << depth;
        stream << length;
        stream << type;
        stream << mapType;
        stream << mapLength;

        // 写入图像数据 (从上到下，BGR 顺序)
        for (int y = 0; y < height; y++) {  // 改为从上到下遍历
             const uchar *scanLine = rgbImage.scanLine(y);
             for (int x = 0; x < width; x++) {
                 // 转换 RGB 到 BGR
                 uchar r = scanLine[x * 3];
                 uchar g = scanLine[x * 3 + 1];
                 uchar b = scanLine[x * 3 + 2];

                 stream << b << g << r;  // BGR 顺序写入
             }

             // 行对齐到 4 字节边界
             int padding = (4 - (width * 3) % 4) % 4;
             for (int i = 0; i < padding; i++) {
                 stream << (uchar)0;
             }
        }
        file.close();
        return true;
    }
    //单帧gif的场景
    if (suffix == "gif") {
        QList<QImage> *list = new QList<QImage>;
        list->append(image);
        return save(list, 0, savepath, suffix);
    }
    // 2. 可以qt直接保存的
    if(m_supportFormatsSaveOnQt.contains(suffix)){
        bool result = image.save(savepath, suffix.toUtf8().constData(), SAVE_QUAITY_VALUE); //质量需要
        qDebug()<<"qt 保存 "<<result <<suffix ;
        return result;
    }
    // 3. freeimage 保存
    //如果是freeimage库的图片
    if (m_supportFormatsSaveOnfreeimage.contains(suffix) && !g_needSaveAs) {
        //获取fif
        FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
        fif = get_real_format(QString(savepath));
        int formatCode;
        if (m_formats.contains(suffix.toUpper())) {
            formatCode = m_formats.value(suffix.toUpper());
            fif = FREE_IMAGE_FORMAT(formatCode);
        } else {
            if (fif == FIF_UNKNOWN) {
                fif = get_real_format(QString(savepath) + "." + suffix);
            }
        }
        //保存
        bool result = saveOnFreeImage(savepath, image, fif);
        qDebug() << "freeimage 保存"<< result << suffix <<fif ;
        return result;
    }

    qDebug() << "不支持该图片的保存，"<< suffix;
    return false;
}

bool KylinImageCodec::save(QList<QImage> *list, const int &fps, const QString &savepath, const QString &suffix)
{
    if (list->length() < 1) {
        return false;
    }

    return saveMovie(list, fps, savepath, suffix);
}

bool KylinImageCodec::saveOnFreeImage(const QString &path, const QImage &image, FREE_IMAGE_FORMAT fif)
{
    FIBITMAP * dib = QImge2FIBitMap(image,fif);
    return writeFIBITMAPToFile(dib, path);
}

bool KylinImageCodec::saveMovie(QList<QImage> *list, const int &fps, const QString &savepath, const QString &suffix)
{
    SaveMovie *saveMovie = new SaveMovie(list, fps, savepath, suffix, m_list);
    connect(saveMovie, &SaveMovie::saveMovieFinish, getSignalObj(), &KylinImageCodecSignals::saveMovieFinish);
    saveMovie->start();
    //    Gif_H m_Gif;
    //    Gif_H::GifWriter m_GifWriter;
    //    for (int i=0; i<list->length(); ++i){
    //        Mat tmpMat;
    //        cvtColor(list->at(i),tmpMat,cv::COLOR_RGBA2BGRA);
    //        if(i == 0)
    //            if (!m_Gif.GifBegin(&m_GifWriter, savepath.toLocal8Bit().data(), tmpMat.cols, tmpMat.rows, fps))
    //                return false;
    //        // 写入图片信息
    //        m_Gif.GifWriteFrame(&m_GifWriter, tmpMat.data, tmpMat.cols, tmpMat.rows, fps);
    //    }
    //    m_Gif.GifEnd(&m_GifWriter);
    return true;
}

QString KylinImageCodec::saveWay(const QString &savepath, bool replace)
{
    //覆盖保存
    if (replace) {
        return savepath;
    }

    //备份保存
    QString tmpPath = savepath;
    int num = 1;
    while (QFileInfo::exists(tmpPath)) {
        tmpPath = tmpPath.mid(0, tmpPath.lastIndexOf(".")) + "(" + QString::number(num) + ")"
                  + tmpPath.mid(tmpPath.lastIndexOf("."), -1);
        num++;
    }
    return tmpPath;
}


bool KylinImageCodec::isSaving(const QString &path)
{
    return m_list->contains(path);
}

bool KylinImageCodec::allSaveFinish()
{
    return m_list->isEmpty();
}

QString KylinImageCodec::savePath()
{
    return m_savePath;
}

