#ifndef GETPREVIEWWITHFFMPEGTHUMBNAILER_H
#define GETPREVIEWWITHFFMPEGTHUMBNAILER_H
#include <QString>
#include <QThread>
#include <QVector>
#include <QMutex>
#include <QPixmap>
#include <QWaitCondition>

#include <libffmpegthumbnailer/videothumbnailerc.h>

struct PreviewStruct {
    int wid;
    int height;
    qint64 pos {-1000};
    QString file;
    QPixmap img;
    int quality { 0 };
    bool isValid { false };
};

class GetPreviewWithFfmpegthumbnailer : public QThread
{
    Q_OBJECT
public:
    GetPreviewWithFfmpegthumbnailer();

    ~GetPreviewWithFfmpegthumbnailer();

    void close();

    void setParams(int width, int height, QString fileName, qint64 pos, int quality = 0);

    QPixmap getImage();

private:
    video_thumbnailer* m_thumbnailer { nullptr };
    image_data* m_imageData { nullptr };

    QString m_fileName;
    int m_width;
    int m_height;

    bool m_running { true };

    QMutex m_vectorMutex;
    PreviewStruct m_previewInfo;
    QVector<PreviewStruct> m_previewVec;

    QMutex m_conditionMutex;
    QWaitCondition m_waitCondition;

    PreviewStruct m_lastPreview;

signals:
    void finishedPreview(PreviewStruct pre);
protected:
    void run() override;
};

#endif // GETPREVIEWWITHFFMPEGTHUMBNAILER_H
