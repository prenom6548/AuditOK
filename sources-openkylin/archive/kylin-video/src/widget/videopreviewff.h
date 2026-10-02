#ifndef VIDEOPREVIEWFF_H
#define VIDEOPREVIEWFF_H

#include <QDialog>
#include <QLabel>
#include <QResizeEvent>
#include <QSharedPointer>

#include "getpreviewwithffmpegthumbnailer.h"

class VideoPreviewFf : public QDialog
{
    Q_OBJECT
public:
    VideoPreviewFf(QWidget* parent);
    ~VideoPreviewFf();

    void setTime(qint64 msec);

    void setShowFile(QString fileName);

    void getPreview();

signals:
    void createPreview(qint64 time, QString file, QPixmap img);
private:
    QSharedPointer<GetPreviewWithFfmpegthumbnailer> m_ffmpegthumbnailer { nullptr };

    QWidget* m_window { nullptr };
    QLabel* m_pic { nullptr };
    QLabel* m_timeLab { nullptr };

    QString m_fileName;
    qint64 m_showTime;

    QString m_lastFileName;
    qint64 m_lastShowTime;
    QPixmap m_lastMap;

    PreviewStruct m_lastPreview;

protected:
    void resizeEvent(QResizeEvent *e) override;
    void hideEvent(QHideEvent *e) override;
};

#endif // VIDEOPREVIEWFF_H
