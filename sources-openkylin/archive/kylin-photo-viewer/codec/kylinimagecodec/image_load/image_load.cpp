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
#include <QImageReader>
ImageResult KylinImageCodec::loadImage(const QString &path, QString realFormat)
{
     return loadImageFromFile(path, realFormat);
}

//以真实格式加载，原格式保存。
ImageResult KylinImageCodec::loadImageFromFile(const QString &path, const QString &realFormat)
{
    ImageResult imgr;
    QString loadFormat;
    QFileInfo info(path);
    if ("" == realFormat) {
        loadFormat = info.suffix().toLower();
    } else {
        loadFormat = realFormat;
    }

    if ((loadFormat == "gif" || loadFormat == "apng")) {
        imgr = loadMovieToImage(path,loadFormat);
    }
    else if (loadFormat == "tif" || loadFormat == "tiff") {
        FREE_IMAGE_FORMAT fif = FIF_TIFF;
        FIMULTIBITMAP *pTIF = FreeImage_OpenMultiBitmap(fif, path.toLocal8Bit().data(), false, true, true, 0);
        int gifImgCnt = FreeImage_GetPageCount(pTIF);
        FIBITMAP *pFrame;
        if (gifImgCnt > 0) {
            imgr = loadStaticToImage(path, loadFormat);
        }
        if (gifImgCnt > 1) {
            imgr.imageList = new QList<QImage>;
            imgr.fps = 1000;
            imgr.imageList->append(imgr.image);
            for (int curFrame = 1; curFrame < gifImgCnt; curFrame++) {
                pFrame = FreeImage_LockPage(pTIF, curFrame);
                QImage image = FIBitmap2QImage(pFrame);
                imgr.imageList->append(image);
                FreeImage_UnlockPage(pTIF, pFrame, curFrame);
            }
        }
        FreeImage_CloseMultiBitmap(pTIF);
    } else {
        imgr = loadStaticToImage(path,loadFormat);
    }

    imgr.info = info;
    //判断图像是否有效
    if (imgr.image.isNull()) {
        imgr.openSuccess = false;
        qDebug()<<"加载图片失败";
    }
    return imgr;
}

ImageResult KylinImageCodec::loadStaticToImage(const QString &path, const QString &loadFormat)
{ 
    ImageResult imgr;
    if(g_exitApp.load()){
        return imgr;
    }
    if (loadFormat == "svg") {
        RsvgHandle *handle = rsvg_handle_new_from_file(path.toUtf8().constData(), NULL);
        if (handle == NULL) {
            qDebug() << "加载svg图片失败！";
            return imgr;
        }

        RsvgDimensionData dimensions;
        rsvg_handle_get_dimensions(handle, &dimensions);
        int width = dimensions.width;
        int height = dimensions.height;

        //创建指定格式和尺寸的图像
        cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
        if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
            rsvg_handle_free(handle);
            qDebug() << "创建svg图像失败！";
            return imgr;
        }

        //渲染图像
        cairo_t *cr = cairo_create(surface);
        rsvg_handle_render_cairo(handle, cr);

        imgr.image = QImage(cairo_image_surface_get_data(surface),
                       width,
                       height,
                       QImage::Format_ARGB32_Premultiplied).copy();

        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        rsvg_handle_free(handle);
    } else if(m_qtSupported.contains(loadFormat)) {
        qDebug()<<"QT加载  loadImageFirstQt"<<path;
        imgr.image = loadImageFirstQt(path, loadFormat);
    } else {
        qDebug()<<"freeimage加载 loadImageOnFreeimage"<<path;
        imgr.image = loadImageOnFreeimage(path);
    }
    return imgr;
}

ImageResult KylinImageCodec::loadMovieToImage(const QString &path, const QString &suffix)
{
    ImageResult imgr;
    auto *tmpMovie = new QMovie(path, "apng");
    imgr.maxFrame = tmpMovie->frameCount();
    if(g_exitApp.load()){ //需要gui
        return imgr;
    }
    tmpMovie->jumpToFrame(0);
    imgr.image = tmpMovie->currentImage();
    //如果大于一帧
    if (tmpMovie->frameCount() > 1) {
        imgr.imageList = new QList<QImage>;
        //获取帧率
        imgr.fps = getDelay(path, suffix);
        //获取第二帧
        if(g_exitApp.load()){ //需要gui
            return imgr;
        }
        tmpMovie->jumpToFrame(1);
        QImage secondImage = tmpMovie->currentImage();
        //存放前两帧
        imgr.imageList->append(imgr.image);
        imgr.imageList->append(secondImage);
        //从第三帧开始，在线程中加载

        if(!g_exitApp.load() && m_flagGifLoad){
            LoadMovie *loadMovie = new LoadMovie(imgr.imageList, tmpMovie);
            m_LoadMovieThreads.push_back(loadMovie);
            connect(loadMovie, &LoadMovie::loadMovieFinish, getSignalObj(), &KylinImageCodecSignals::loadMovieFinish);
            connect(loadMovie,&LoadMovie::finished,loadMovie,&LoadMovie::deleteLater);
            connect(loadMovie,&LoadMovie::finished,getSignalObj(),[m_LoadMovieThreads,loadMovie](){
               auto it = std::find(m_LoadMovieThreads.begin(),m_LoadMovieThreads.end(),loadMovie);
               if(it!=m_LoadMovieThreads.end()){
               m_LoadMovieThreads.erase(it);
               }
               qDebug()<<"m_LoadMovieThreads 从列表移除 "<<m_LoadMovieThreads.length();
            });
	    loadMovie->start();
        }
        if(imgr.image.isNull() || secondImage.isNull()){
            qDebug()<<"movie加载失败，多帧";
        }
    }
    //临界值为2时，在线程中回收
    if (tmpMovie->frameCount() < 2) {
        tmpMovie->deleteLater();
        Q_EMIT getSignalObj()->loadMovieFinish(path);
    }
    return imgr;
}

QImage KylinImageCodec::loadImageOnFreeimage(const QString &path)
{
    QImage image;
    if(g_exitApp.load()){ //m_formats值
        return image;
    }
    FIBITMAP *dib = NULL;
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    fif = get_real_format(path);

    if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
        dib = FreeImage_Load(fif, path.toUtf8().constData(), 0);
        if (dib != NULL) {
            image = FIBitmap2QImage(dib);
        } else {
            image = QImage();
        }
        FreeImage_Unload(dib);
    }
    return image;

}
QImage KylinImageCodec::loadImageFirstQt(const QString &path, const QString &suffix)
{
    QImage image;
    QImageReader reader;
    reader.setFileName(path);
    reader.setFormat(suffix.toLatin1());
    reader.setAutoTransform(true);

    if(reader.imageCount() >= 0){
        image = reader.read();
        if (image.isNull()) {
            image = QImage(path);
            if (image.isNull()) {
                image = QImage();
            }
        }
    }
    return image;
}

int KylinImageCodec::getDelay(const QString &path, const QString &suffix)
{
    int fps = 0;

    if (suffix == "gif") {
        fps = gifDelay(path);
    }

    //    else if (suffix == "apng" || suffix == "png") {
    //    qDebug()<<"===此处待补充===";
    //}

    if (fps != 0) {
        return fps;
    }

    //以上都没获取到fps的保底机制
    // QMovie没有现成的方法，为求稳定，每次打开新动图的时候循环5次取最大值，此处待优化
    QMovie tmpMovie(path);
    if (fps == 0) {
        for (int i = 0; i < 5; i++) {
            tmpMovie.start();
            tmpMovie.stop();
            fps = tmpMovie.nextFrameDelay() > fps ? tmpMovie.nextFrameDelay() : fps;
        }
    }

    return fps == 0 ? 100 : fps;
}
int KylinImageCodec::gifDelay(const QString &path)
{
    int j = 0;
    int frameDelay = 0;
    SavedImage *
        frame; //这个是系统中保存gif的扩展块的信息，不太了解gif的文件结构的可以看一下：http://blog.csdn.net/wzy198852/article/details/17266507
    ExtensionBlock *ext = nullptr; //这个保存gif延时代码块的结构体

    int err = 0;
    GifFileType *gifFileType = DGifOpenFileName(path.toLocal8Bit().data(), &err);
    if (err != 0) {
        DGifCloseFile(gifFileType, &err);
        return 0;
    }
    DGifSlurp(gifFileType);
    frame = &gifFileType->SavedImages[0]; //拿到第一个图片相关信息
    for (j = 0; j < frame->ExtensionBlockCount; j++) {
        //找到含有延迟时间的代码块
        if (frame->ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
            ext = &(frame->ExtensionBlocks[j]);
        }
        //拿到延迟时间
        if (ext) {
            //延迟时间1-->10ms
            frameDelay = 10 * (ext->Bytes[2] << 8 | ext->Bytes[1]); //拿到延迟时间
            break;
        }
    }
    DGifCloseFile(gifFileType, &err);
    return frameDelay;
}

void KylinImageCodec::exitApp(){
    g_exitApp.store(true);

    while (!m_LoadMovieThreads.isEmpty()) {
       qDebug()<<"m_LoadMovieThreads 不为空，动图加载中";
    }
}
