#include <QImage>
#include <QDebug>
#include <QTime>

#include "getpreviewwithffmpegthumbnailer.h"

GetPreviewWithFfmpegthumbnailer::GetPreviewWithFfmpegthumbnailer()
{
    qRegisterMetaType<PreviewStruct>("PreviewStruct");

    m_thumbnailer = video_thumbnailer_create();
    m_imageData = video_thumbnailer_create_image_data();
    start();
}

GetPreviewWithFfmpegthumbnailer::~GetPreviewWithFfmpegthumbnailer()
{
    close();
    video_thumbnailer_destroy(m_thumbnailer);
    video_thumbnailer_destroy_image_data(m_imageData);
}

void GetPreviewWithFfmpegthumbnailer::close()
{
    m_running = false;
    m_waitCondition.wakeAll(); // 唤醒线程退出
}

void GetPreviewWithFfmpegthumbnailer::setParams(int width, int height, QString fileName, qint64 pos, int quality)
{
    m_vectorMutex.lock();
    m_previewInfo.file = fileName;
    m_previewInfo.height = height;
    m_previewInfo.wid = width;
    m_previewInfo.pos = pos;
    m_previewInfo.quality = quality;
    m_previewInfo.isValid = true;
    m_vectorMutex.unlock();

    m_waitCondition.notify_all();
}

QPixmap GetPreviewWithFfmpegthumbnailer::getImage()
{

}

void GetPreviewWithFfmpegthumbnailer::run()
{
    while (m_running) {
        PreviewStruct pre;

        m_vectorMutex.lock();
        pre = m_previewInfo;
        m_previewInfo.isValid = false;
        m_vectorMutex.unlock();

        if (!pre.isValid) {
//           QMutexLocker locker(&m_conditionMutex);
           m_waitCondition.wait(&m_conditionMutex);

        }

        if (pre.isValid) {
            qInfo() << "create preview pos:" << pre.pos;
            // 初始化缩略图对象
            m_thumbnailer->thumbnail_size = pre.height > pre.wid ? pre.height : pre.wid;

            QString showTime;
            QTime time = QTime::fromMSecsSinceStartOfDay(pre.pos);
            if(pre.pos >= 3600)    // hours
                showTime = time.toString("h:mm:ss.zzz");
            else if(pre.pos >= 60) // minutes
                showTime = time.toString("mm:ss.zzz");
            else
                showTime = time.toString("0:ss.zzz");
            qInfo() << "showTime:" << showTime;

            m_thumbnailer->seek_time = showTime.toUtf8().data();
            m_thumbnailer->thumbnail_image_quality = pre.quality;
            // 获取缩略图
            video_thumbnailer_generate_thumbnail_to_buffer(m_thumbnailer, pre.file.toUtf8().data(), m_imageData);

            // 缩略图转成qt格式
            QImage image = QImage::fromData(m_imageData->image_data_ptr, m_imageData->image_data_size);
            // 缩略图尺寸转换
            if (pre.wid != pre.height) {
                image = image.scaled(QSize(pre.wid, pre.height));
            }

            pre.img = QPixmap::fromImage(image);

            m_lastPreview = pre;

            emit finishedPreview(pre);
        }
    }
}
