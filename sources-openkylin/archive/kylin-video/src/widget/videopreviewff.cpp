#include <QPixmap>
#include <QThread>
#include <QDebug>

#include "videopreviewff.h"
#include "global/functions.h"


VideoPreviewFf::VideoPreviewFf(QWidget* parent) : QDialog(parent)
{
    m_window = new QWidget(this);

    m_pic = new QLabel(m_window);

    setWindowFlags(windowFlags() | Qt::WindowDoesNotAcceptFocus);

    m_timeLab = new QLabel(m_window);

    m_timeLab->setFixedHeight(25);
    QFont f("Noto Sans CJK SC Regular");
    f.setPixelSize(16);
    m_timeLab->setFont(f);
    m_timeLab->setAlignment(Qt::AlignCenter);
    m_timeLab->setStyleSheet("background-color:qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255, 255, 255, 0), stop:1 rgba(19, 19, 20, 200));color:rgb(222,222,222)");

    m_ffmpegthumbnailer =  QSharedPointer<GetPreviewWithFfmpegthumbnailer>(new GetPreviewWithFfmpegthumbnailer);
    connect(m_ffmpegthumbnailer.data(), &GetPreviewWithFfmpegthumbnailer::finishedPreview, [this](PreviewStruct preview) {
        m_lastPreview = preview;
        if (!isHidden() && preview.pos == m_showTime) {
            m_pic->setPixmap(preview.img);
        }
    });

}

VideoPreviewFf::~VideoPreviewFf()
{
}

void VideoPreviewFf::setTime(qint64 msec)
{
    m_showTime = msec;
    QString time_str = Functions::timeToStr((double)msec/1000);
    m_timeLab->setText(time_str);
}

void VideoPreviewFf::setShowFile(QString fileName)
{
    m_fileName = fileName;
}

void VideoPreviewFf::getPreview()
{

    qint64 inputTime = this->m_showTime;
    QString inputFile = this->m_fileName;

    m_lastShowTime = inputTime;
    if (abs(m_lastPreview.pos / 1000 - inputTime / 1000) == 0) {
        m_pic->setPixmap(m_lastPreview.img);
    } else {
        m_ffmpegthumbnailer->setParams(width(), height(), inputFile, inputTime);
    }
}


void VideoPreviewFf::resizeEvent(QResizeEvent *e)
{
    m_pic->setGeometry(QRect(QPoint(0, 0), e->size()));
    m_timeLab->setGeometry(QRect(QPoint(0, height()-25), QSize(width(), 25)));
    m_timeLab->raise();
    return QDialog::resizeEvent(e);
}

void VideoPreviewFf::hideEvent(QHideEvent *e)
{
    m_pic->clear();
    return QDialog::hideEvent(e);
}
