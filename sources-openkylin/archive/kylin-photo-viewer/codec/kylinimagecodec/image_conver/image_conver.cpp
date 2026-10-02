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

QImage KylinImageCodec::FIBitmap2QImage(FIBITMAP *dib)
{
    static QImage none(0, 0, QImage::Format_Invalid);
    FREE_IMAGE_TYPE fit = FreeImage_GetImageType(dib);
    if (!dib || fit == FIT_UNKNOWN) return none;

    //获取像素数据
    int width  = static_cast<int>(FreeImage_GetWidth(dib));
    int height = static_cast<int>(FreeImage_GetHeight(dib));
    int depth = static_cast<int>(FreeImage_GetBPP(dib));
    int pitch = FreeImage_GetPitch(dib);
    BYTE* bits = FreeImage_GetBits(dib);
    qDebug()<<"FIBitmap2QImage: depth="<<depth;

    switch (depth) {
    case 1: {
        QImage result(width, height, QImage::Format_Mono);
        FreeImage_ConvertToRawBits(
                    result.scanLine(0), dib, result.bytesPerLine(), 1, 0, 0, 0, true
                    );
        return result;
    }
    case 4: {
        QImage result(width, height, QImage::Format_RGB32);
        FreeImage_ConvertToRawBits(
                    result.scanLine(0), dib, result.bytesPerLine(), 32,
                    FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
                    true
                    );
        return result;
    }
    case 8: {
        QImage result(width, height, QImage::Format_Indexed8);
        FreeImage_ConvertToRawBits(
                    result.scanLine(0), dib, result.bytesPerLine(), 8, 0, 0, 0, true
                    );
        return result;
    }
    case 16:{
        if ( // 5-5-5
             (FreeImage_GetRedMask(dib)   == FI16_555_RED_MASK) &&
             (FreeImage_GetGreenMask(dib) == FI16_555_GREEN_MASK) &&
             (FreeImage_GetBlueMask(dib)  == FI16_555_BLUE_MASK)) {
            QImage result(width, height, QImage::Format_RGB555);
            FreeImage_ConvertToRawBits(
                        result.scanLine(0), dib, result.bytesPerLine(), 16,
                        FI16_555_RED_MASK, FI16_555_GREEN_MASK, FI16_555_BLUE_MASK,
                        true
                        );
            return result;
        } else { // 5-6-5
            QImage result(width, height, QImage::Format_RGB16);
            FreeImage_ConvertToRawBits(
                        result.scanLine(0), dib, result.bytesPerLine(), 16,
                        FI16_565_RED_MASK, FI16_565_GREEN_MASK, FI16_565_BLUE_MASK,
                        true
                        );
            return result;
        }
        }
    case 24: {
        QImage result(width, height, QImage::Format_RGB32);
        FreeImage_ConvertToRawBits(
                    result.scanLine(0), dib, result.bytesPerLine(), 32,
                    FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
                    true
                    );
        return result;
    }
    case 32: {
        QImage result(width, height, QImage::Format_ARGB32);
        FreeImage_ConvertToRawBits(
                    result.scanLine(0), dib, result.bytesPerLine(), 32,
                    FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
                    true
                    );
        return result;
    }
    case 48: {
        QImage image(width, height, QImage::Format_RGB32);
        for (int y = 0; y < height; y++) {
            FIRGB16* srcLine = (FIRGB16*)FreeImage_GetScanLine(dib, y);
            QRgb* dstLine = (QRgb*)image.scanLine(height - 1 - y);

            for (int x = 0; x < width; x++) {
                float r = srcLine[x].red / 65535.0f;
                float g = srcLine[x].green / 65535.0f;
                float b = srcLine[x].blue / 65535.0f;

                dstLine[x] = qRgb(
                            static_cast<int>(r * 255),
                            static_cast<int>(g * 255),
                            static_cast<int>(b * 255)
                            );
            }
        }
        return image;
    }
    case 64: {
        QImage image(width, height, QImage::Format_RGBA64_Premultiplied);
        for (int y = 0; y < height; y++) {
            FIRGBA16* srcLine = (FIRGBA16*)FreeImage_GetScanLine(dib, y);
            QRgba64* dstLine = reinterpret_cast<QRgba64*>(image.scanLine(height - 1 - y));

            for (int x = 0; x < width; x++) {
                dstLine[x] = QRgba64::fromRgba64(
                            srcLine[x].red,
                            srcLine[x].green,
                            srcLine[x].blue,
                            srcLine[x].alpha
                            );
            }
        }
        return image;
    }
    case 96:    // FIT_RGBF(3*32 float)
    case 128:{  //FIT_RGBAF(4*32 float)
        //浮点数转换
        bool haveAlpha = FreeImage_IsTransparent(dib);
        qDebug()<<"haveAlpha ="<<haveAlpha;
        if(!haveAlpha){ // 96
            QImage qImage(width, height, QImage::Format_RGB888);
            for (int y = 0; y < height; y++) {
                const float* srcPixels = reinterpret_cast<float*>(bits + (height - 1 - y) * pitch);
                uchar* dstPixels = qImage.scanLine(y); // QImage 的 RGB 数据

                for (int x = 0; x < width; x++) {
                    const float r = srcPixels[3 * x + 0]; // R
                    const float g = srcPixels[3 * x + 1]; // G
                    const float b = srcPixels[3 * x + 2]; // B

                    // 归一化到 [0, 255] 并转为 uchar
                    dstPixels[3 * x + 0] = static_cast<uchar>(qBound(r * 255.0f, 0.0f, 255.0f)); // R
                    dstPixels[3 * x + 1] = static_cast<uchar>(qBound(g * 255.0f, 0.0f, 255.0f)); // G
                    dstPixels[3 * x + 2] = static_cast<uchar>(qBound(b * 255.0f, 0.0f, 255.0f)); // B
                }
            }
            return qImage;

        }else{  //128
            QImage qImage(width, height, QImage::Format_RGBA8888);
            for (int y = 0; y < height; y++) {
                const float* srcPixels = reinterpret_cast<float*>(bits + y * pitch);
                uchar* dstPixels = qImage.scanLine(y);

                for (int x = 0; x < width; x++) {
                    const float r = srcPixels[4 * x + 0]; // R
                    const float g = srcPixels[4 * x + 1]; // G
                    const float b = srcPixels[4 * x + 2]; // B
                    const float a = srcPixels[4 * x + 3]; // Alpha

                    // 线性映射到 [0,255]
                    dstPixels[4 * x + 0] = static_cast<uchar>(qBound(r * 255.0f, 0.0f, 255.0f)); // R
                    dstPixels[4 * x + 1] = static_cast<uchar>(qBound(g * 255.0f, 0.0f, 255.0f)); // G
                    dstPixels[4 * x + 2] = static_cast<uchar>(qBound(b * 255.0f, 0.0f, 255.0f)); // B
                    dstPixels[4 * x + 3] = static_cast<uchar>(qBound(a * 255.0f, 0.0f, 255.0f)); // A
                }
            }
            return qImage;
        }
    }
    default:
        break;
    }
    return QImage();
}

FREE_IMAGE_FORMAT KylinImageCodec::get_real_format(const QString &path)
{
    /* 获取后缀 */
    QFileInfo file_info(path);
    QString file_suffix_upper = file_info.suffix().toUpper();

    /* 用库获取文件格式 */
    QByteArray temp_path;
    temp_path.append(path.toUtf8());
    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(temp_path.data()); //后缀优先

    /* 修复后缀 , 以库解析出来的为准 */
    int formatCode;
    if (m_formats.contains(file_suffix_upper)) {
        formatCode = m_formats.value(file_suffix_upper);    //文件后缀格式
    }
    if (format != FIF_UNKNOWN && format != formatCode) {
        file_suffix_upper = m_formats.key(format);
    }
    if (format == FIF_TIFF) {
        file_suffix_upper = "TIFF";
    }
    /* 不支持格式 , 重新解析 , 防止后缀错误 */
    if (!m_formats.contains(file_suffix_upper)) {
        file_suffix_upper.clear();
    }

    /* 从文件数据解析文件格式 */
    if (file_suffix_upper.isEmpty()) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return FIF_UNKNOWN;
        }
        const QByteArray data = file.read(64);

        /* Check bmp file */
        if (data.startsWith("BM")) {
            return FIF_BMP;
        }

        /* Check dds file */
        if (data.startsWith("DDS")) {
            return FIF_DDS;
        }

        /* Check gif file */
        if (data.startsWith("GIF8")) {
            return FIF_GIF;
        }

        /* Check Max OS icons file */
        if (data.startsWith("icns")) {
            return FIF_UNKNOWN;
        }

        /* Check jpeg file */
        if (data.startsWith("\xff\xd8")) {
            return FIF_JPEG;
        }

        /* Check mng file */
        if (data.startsWith("\x8a\x4d\x4e\x47\x0d\x0a\x1a\x0a")) {
            return FIF_MNG;
        }

        //        /* Check net pbm file (BitMap) */
        if (data.startsWith("P1") || data.startsWith("P4")) {
            return FIF_PBM;
        }

        //        /* Check pgm file (GrayMap) */
        if (data.startsWith("P2") || data.startsWith("P5")) {
            return FIF_PGM;
        }

        //        /* Check ppm file (PixMap) */
        if (data.startsWith("P3") || data.startsWith("P6")) {
            return FIF_PPM;
        }

        //        /* Check png file */
        if (data.startsWith("\x89PNG\x0d\x0a\x1a\x0a")) {
            return FIF_PNG;
        }

        /* Check svg file */
        if (data.indexOf("<svg") > -1) {
            return FIF_UNKNOWN;
        }

        /* Check tiff file */
        if (data.startsWith("MM\x00\x2a") || data.startsWith("II\x2a\x00")) {
            // big-endian, little-endian.
            return FIF_TIFF;
        }

        /* Check webp file */
        if (data.startsWith("RIFFr\x00\x00\x00WEBPVP")) {
            return FIF_WEBP;
        }

        /* Check xbm file */
        if (data.indexOf("#define max_width ") > -1 && data.indexOf("#define max_height ") > -1) {
            return FIF_XBM;
        }

        /* Check xpm file */
        if (data.startsWith("/* XPM */")) {
            return FIF_XPM;
        }
        return FIF_UNKNOWN;
    }

    /* 解决为空时 , 三目运算返回错误问题 */
    if (file_suffix_upper.isEmpty()) {
        return FIF_UNKNOWN;
    }

    /* 根据后缀获取对应的格式 */
    int formatCode1 = m_formats.value(file_suffix_upper);
    format = static_cast<FREE_IMAGE_FORMAT>(formatCode1);


    return format >= 0 ? format : FIF_UNKNOWN;
}

bool KylinImageCodec::writeFIBITMAPToFile(FIBITMAP *dib, const QString &path, int flag)
{
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    BOOL bSuccess = FALSE;
    const QByteArray ba = path.toUtf8();
    const char *pc = ba.data();
    // Try to guess the file format from the file extension
    fif = FreeImage_GetFIFFromFilename(pc);     //文件后缀格式
    FREE_IMAGE_FORMAT realfif = FIF_UNKNOWN;
    if (fif == FIF_UNKNOWN) {
        realfif = FreeImage_GetFileType(pc);    //文件内容格式
    }
    if (fif != FIF_UNKNOWN) {
        bSuccess = FreeImage_Save(fif, dib, pc, flag);
    } else if (realfif != FIF_UNKNOWN) {
        bSuccess = FreeImage_Save(realfif, dib, pc, flag);
    }

    if(dib){
        FreeImage_Unload(dib);
    }
    return bSuccess;
}

FIBITMAP *KylinImageCodec::QImge2FIBitMap(const QImage &image, FREE_IMAGE_FORMAT fif)
{
    if (image.isNull())  return nullptr;
    QImage img = image.copy();

    int width = img.width();
    int height = img.height();
    uint depth = static_cast<uint>(img.depth());    //包含未使用的alpha通道


    QImage::Format format = img.format();
    qDebug()<<"QImge2FIBitMap: image的format="<<format;

    switch (format) {
    //The image is invalid.
    case QImage::Format_Invalid:
        return nullptr;
    //The image is stored using 1-bit per pixel.(MSB)
    case QImage::Format_Mono:
    //The image is stored using 1-bit per pixel.(LSB)
    case QImage::Format_MonoLSB: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), depth, 0, 0, 0,
                            true);
        return res;
    }
    /* NOTE: QImage do not support 4-bit*/

    //可能会将彩色图转为灰度图
    //The image is stored using an 8-bit alpha only format.
    case QImage::Format_Alpha8:
    //The image is stored using an 8-bit grayscale format.
    case QImage::Format_Grayscale8:
    //The image is stored using 8-bit indexes into a colormap.
    case QImage::Format_Indexed8: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), depth, 0, 0, 0,
                            true);
        return res;
    }
    //The image is stored using a 16-bit RGB format (5-6-5)
    case QImage::Format_RGB16: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), 16,
                            FI16_565_RED_MASK, FI16_565_GREEN_MASK, FI16_565_BLUE_MASK,
                            true);
        return res;
    }
    //The image is stored using a 16-bit RGB format (5-5-5). The unused most significant bit is always zero.
    case QImage::Format_RGB555: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), 16,
                            FI16_555_RED_MASK, FI16_555_GREEN_MASK, FI16_555_BLUE_MASK,
                            true);
        return res;
    }

    //32-bit
    case QImage::Format_RGBA8888_Premultiplied:
    case QImage::Format_RGBA8888:{ //通道顺序不一样
        if( fif == FIF_EXR){
            qDebug()<<"FIF_EXR - Format_RGBA8888";
            FIBITMAP* dib = FreeImage_AllocateT(FIT_RGBF, width, height);
            for (int y = 0; y < height; y++) {
                uchar* srcLine = img.scanLine(y);
                FIRGBAF* dstLine = (FIRGBAF*)FreeImage_GetScanLine(dib, height - 1 - y);
                for (int x = 0; x < width; x++) {
                    dstLine[x].red  = srcLine[x * 3 + 0] / 255.0f;
                    dstLine[x].green = srcLine[x * 3 + 1] / 255.0f;
                    dstLine[x].blue   = srcLine[x * 3 + 2] / 255.0f;
                    dstLine[x].alpha = srcLine[x * 4 + 3] / 255.0f;
                }
            }
            dib = FreeImage_ConvertToRGBAF(dib); // 转为 32-bit 浮点
            return dib;
        }else{
            FIBITMAP *res = FreeImage_ConvertFromRawBits(
                                img.scanLine(0), width, height, img.bytesPerLine(), depth,
                                FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
                                true);
            return res;
        }
    }
    case QImage::Format_ARGB32_Premultiplied:
    case QImage::Format_ARGB32: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), depth,
                            FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
                            true);
        return res;
    }
    //24-bit
    case QImage::Format_RGB666:
    case QImage::Format_ARGB8555_Premultiplied:
    case QImage::Format_ARGB8565_Premultiplied:
    case QImage::Format_ARGB6666_Premultiplied:
    case QImage::Format_RGB888: {
        if( fif == FIF_EXR){
            qDebug()<<"FIF_EXR - Format_RGB888";
            FIBITMAP* dib = FreeImage_AllocateT(FIT_RGBF, width, height);
            for (int y = 0; y < height; y++) {
                uchar* srcLine = img.scanLine(y);
                FIRGBF* dstLine = (FIRGBF*)FreeImage_GetScanLine(dib, height - 1 - y);
                for (int x = 0; x < width; x++) {
                    // 交换 R 和 B 通道（BGR -> RGB）
                    dstLine[x].red  = srcLine[x * 3 + 0] / 255.0f; // B -> R
                    dstLine[x].green = srcLine[x * 3 + 1] / 255.0f; // G -> G
                    dstLine[x].blue   = srcLine[x * 3 + 2] / 255.0f; // R -> B
                }
            }
            dib = FreeImage_ConvertToRGBF(dib); // 转为 32-bit 浮点
            return dib;
        }else{
            FIBITMAP *res = FreeImage_ConvertFromRawBits(
                                img.scanLine(0), width, height, img.bytesPerLine(), depth,
                                FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
                                true);
            res = FreeImage_ConvertTo24Bits(res);
            return res;
        }
    }
    case QImage::Format_RGB32: {
        qDebug()<<"freeimage QImage::Format_RGB32 转换 ";
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), depth,
                            FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
                            true);
        res = FreeImage_ConvertTo24Bits(res);
        return res;
    }

    //FreeImage is not support
    //The image is stored using a 16-bit RGB format (4-4-4). The unused bits are always zero.
    case QImage::Format_RGB444:
    case QImage::Format_ARGB4444_Premultiplied:
    default:
        break;
    }

    return nullptr;
}
