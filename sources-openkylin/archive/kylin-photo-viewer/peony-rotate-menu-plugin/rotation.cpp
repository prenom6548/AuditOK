/* Copyright (C) 2020, KylinSoft Co., Ltd.
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
#include "rotation.h"
#include <dlfcn.h>
typedef char** (*GetFileProtectedFunc)(int*);

typedef char** (*GetFileProtectedFunc)(int*);

Rotation::Rotation(QObject *parent) : QObject(parent)
{

}

bool Rotation::rotateAndSaveImage(QStringList imagePaths, bool way)
{
    for(QString &imagePath :imagePaths){
        if(m_isClosed){
            Q_EMIT allImageFinishSig();     //提前终止
            break;
        }
        m_imageFormat = realFormat(imagePath);
        bool success  = rotateFun(imagePath, way);
        if( success && m_imageFormat != "gif" && m_imageFormat != "apng" ){
            Q_EMIT imageFinishSig();
        }
        if( !success ){
            qDebug()<<"rotate - 旋转保存失败"<<imagePath;
            Q_EMIT imageFinishSig();
        }
    }
    Q_EMIT allImageFinishSig();
    return true;
}

bool Rotation::rotateFun(QString imagePath, bool way)
{
    if(!is_write(imagePath)){ //不可写直接返回进度条+1
        return true;
    }

    bool saveFinish = false;
    //加载
    ImageResult maf = KylinImageCodec::loadImageFromFile(imagePath, m_imageFormat);
    if(!maf.openSuccess){
        qDebug()<<"打开失败";
        return false;
    }

    int m_tiffPage ;
    if(m_imageFormat == "tiff" ){
        QImageReader reader(imagePath);
        m_tiffPage = reader.imageCount();
    }

    QList<QImage> *m_imageList = maf.imageList;
    int m_fps = maf.fps;
    int m_maxFrame = maf.maxFrame;

    //旋转
    if(m_imageFormat == "tiff" && m_tiffPage > 1 ){
        const char* inputPath =  imagePath.toLocal8Bit().constData();
        FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(inputPath );
        FIMULTIBITMAP *inputBitmap = FreeImage_OpenMultiBitmap(fif, inputPath, false, true, true, 0);
        int pages = FreeImage_GetPageCount(inputBitmap);

        QList<FIBITMAP *> m_fibilist;
        FIBITMAP *pFrame;
        for (int i = 0; i < pages; i++) {
            pFrame = FreeImage_LockPage(inputBitmap, i);
            FIBITMAP *pFrame1;
            if(way == true){
                pFrame1 = FreeImage_Rotate(pFrame, -90);
            }else{
                pFrame1 = FreeImage_Rotate(pFrame, 90);
            }
            m_fibilist.append(pFrame1);
            FreeImage_UnlockPage(inputBitmap, pFrame, true);
        }
        FreeImage_CloseMultiBitmap(inputBitmap);

        if (m_fibilist.length() > 0) {
            FIMULTIBITMAP *outputBitmap = FreeImage_OpenMultiBitmap(fif, inputPath, true, false);
            for (int i = 0; i < m_fibilist.length(); i++) {
                FreeImage_AppendPage(outputBitmap, m_fibilist.at(i));
            }
            saveFinish = FreeImage_CloseMultiBitmap(outputBitmap);
        }

        for(auto fib : m_fibilist){
            if(fib != nullptr){
                FreeImage_Unload(fib);
            }
        }
        m_fibilist.clear();
        if (!m_imageList->isEmpty()){
            m_imageList->clear();
        }

        return saveFinish;
    }

    if(m_imageFormat == "gif" || m_imageFormat == "apng"){
        m_movieRotate = false;
        while(m_maxFrame > m_imageList->length()){
            QThread::usleep(10);
        }
        for (int i = 0; i < m_imageList->length(); i++) {
            QImage tmp = processImage(m_imageList->at(i), way);
            m_imageList->replace(i, tmp);
        }
        saveFinish = KylinImageCodec::saveImage(m_imageList, m_fps, imagePath, m_imageFormat, true);

        if (!m_imageList->isEmpty()){
            m_imageList->clear();
        }
        while(!m_movieRotate){
            QThread::usleep(10);
        }
        return saveFinish;
    }

    QImage m_imageMat = processImage(maf.image,way);
    if(!m_imageMat.isNull()){
        saveFinish =  KylinImageCodec::saveImage(m_imageMat,imagePath, m_imageFormat,true);
    }



    return saveFinish;
}

bool Rotation::is_write(QString imagePath)
{
    QFileInfo fileInfo(imagePath);
    if(!fileInfo.exists()){
        return false;
    }
    if (!fileInfo.isWritable()){
        return false;
    }
#ifndef UNUI
    if(!is_pic_file(imagePath)){
        return false;
    }
#endif

    //判断图片是否被添加到防篡改列表
    if (!m_securityFd) {
        m_securityFd = dlopen("/usr/lib/kysdk/kysdk-security/libkyfilesec.so", RTLD_LAZY);
    }
    if (m_securityFd) {
        GetFileProtectedFunc kdk_get_file_protected = (GetFileProtectedFunc)dlsym(m_securityFd, "kdk_get_file_protected");
        if (kdk_get_file_protected == NULL) {
            qDebug()<< "找不到符号:kdk_get_file_protected";
        } else {
            int len;
            char** p = kdk_get_file_protected(&len);
            if (p != nullptr) {
                for (int i = 0; i < len; i++) {
                    std::string str = p[i];
                    if (imagePath.toStdString().data() == str) {
                        return false;
                    }
                }
                for (int i = 0; i < len; i++) {
                    free(p[i]);
                }
                free(p);
            }
        }
    }

    return true;
}

bool Rotation::is_pic_file(QString imagepath)
{
    QFileInfo file (imagepath);
    if(m_file_type_list.contains(file.suffix())){
        return true;
    }
    return false;
}

QImage Rotation::processImage(const QImage &image, bool &way)
{
    if(way){
        return rotateRightFun(image);
    }else{
        return rotateLeftFun(image);
    }
}

QImage Rotation::rotateRightFun(QImage image)
{  
    if (image.isNull()) {
        return QImage();
    }
    QImage image_copy(image);
    if (!image_copy.isNull()) {
        QTransform rotatematrix;
        rotatematrix.rotate(90);
        image = image_copy.transformed(rotatematrix, Qt::SmoothTransformation);
        return image;
    }
    qDebug()<<"旋转失败";
    return QImage();
}

QImage Rotation::rotateLeftFun(QImage image)
{
    if (image.isNull()) {
        return QImage();
    }
    QImage image_copy(image);
    if (!image_copy.isNull()) {
        QTransform rotatematrix;
        rotatematrix.rotate(270);
        image = image_copy.transformed(rotatematrix, Qt::SmoothTransformation);
        return image;
    }
    return QImage();
}

QString Rotation::realFormat(const QString &path)
{
    m_imageRealFormat = "";
    m_imageRealFormat = KylinImageCodec::m_formats.key(KylinImageCodec::get_real_format(path));
    if (m_imageRealFormat == "") {
        processStart("file", QStringList() << "-i" << path);
        if (m_imageRealFormat != "") {
            if (m_imageRealFormat.contains("image/")) {
                m_imageRealFormat = m_imageRealFormat.mid(m_imageRealFormat.indexOf(":") + 2);
                m_imageRealFormat = m_imageRealFormat.left(m_imageRealFormat.indexOf(";"));
            } else {
                m_imageRealFormat = "";
            }
        }
    }
    if (m_imageRealFormat.startsWith("image/")) {
        if ("image/svg+xml" == m_imageRealFormat) {
            m_imageRealFormat = "svg";
        } else if ("image/x-sun-raster" == m_imageRealFormat) {
            m_imageRealFormat = "sr";
        } else if ("image/x-portable-anymap" == m_imageRealFormat) {
            m_imageRealFormat = "pnm";
        } else {
            qDebug() << "不支持的format";
        }
    } else {
        m_imageRealFormat = m_imageRealFormat.toLower();
    }
    //校准
    if ("" != m_imageRealFormat) {
        QFileInfo file(path);
        QString fileFormat = file.suffix().toLower();
        // jpeg
        if (KylinImageCodec::g_jpegList.contains(m_imageRealFormat)) {
            if (KylinImageCodec::g_jpegList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "jpeg";
            }
        }
        //{"bmp", "dib"};
        if (KylinImageCodec::g_bmpList.contains(m_imageRealFormat)) {
            if (KylinImageCodec::g_bmpList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "bmp";
            }
        }
        //{"png", "apng"}
        if (KylinImageCodec::g_pngList.contains(m_imageRealFormat)) {
            if (KylinImageCodec::g_pngList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "png";
            }
        }
        //{"fax", "g3"}
        if (KylinImageCodec::g_faxList.contains(m_imageRealFormat)) {
            if (KylinImageCodec::g_faxList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "fax";
            }
        }
        //{"koa", "koala"}
        if (KylinImageCodec::g_koaList.contains(m_imageRealFormat)) {
            if (KylinImageCodec::g_koaList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "koa";
            }
        }
        //{"lbm", "iff"}
        if (KylinImageCodec::g_lbmList.contains(m_imageRealFormat)) {
            if (KylinImageCodec::g_lbmList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "lbm";
            }
        }
        //{"tga", "targa"}
        if (KylinImageCodec::g_tgaList.contains(m_imageRealFormat)) {
            if (KylinImageCodec::g_tgaList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "tga";
            }
        }
        //{"tiff", "tif"}
        if (KylinImageCodec::g_tiffList.contains(m_imageRealFormat)) {
            if (KylinImageCodec::g_tiffList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "tiff";
            }
        }
        //{"pct", "pic", "pict"}
        if (KylinImageCodec::g_pictList.contains(m_imageRealFormat)) {
            if (KylinImageCodec::g_pictList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "pict";
            }
        }
        //{"ppm", "pbm", "pgm"}
        if (KylinImageCodec::g_pbmList.contains(m_imageRealFormat)) {
            if (KylinImageCodec::g_pbmList.contains(fileFormat) || fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            } else {
                m_imageRealFormat = "pbm";
            }
        }
        if (KylinImageCodec::g_otherFormatList.contains(m_imageRealFormat)) {
            if (KylinImageCodec::g_otherFormatList.contains(fileFormat) && fileFormat == m_imageRealFormat) {
                m_imageRealFormat = fileFormat;
            }
        }
    }

    return m_imageRealFormat;
}

void Rotation::processStart(const QString &cmd, QStringList arguments)
{
#ifndef UNUI
    QProcess *m_process = new QProcess;
        connect(m_process, &QProcess::readyReadStandardError, [=] {
            qDebug() << "*******process error*******\n"
                     << QString::fromLocal8Bit(m_process->readAllStandardError())
                     << "\n*******process error*******";
        });
        connect(m_process, &QProcess::readyReadStandardOutput, [=] {
            m_imageRealFormat = QString::fromLocal8Bit(m_process->readAll());
        });
        m_process->setReadChannel(QProcess::StandardOutput); //标准输出读取内容必须设置该属性
    m_process->start(cmd, arguments);
    m_process->waitForStarted();
    m_process->waitForFinished();
    m_process->waitForReadyRead();
    m_process->deleteLater();
#endif
}
