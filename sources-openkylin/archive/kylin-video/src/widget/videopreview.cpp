/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
//#include "videopreview.h"

//#include <QResizeEvent>
//#include <QtAVWidgets/global.h>

//#include "global/global.h"
//#include "global/functions.h"

//VideoPreview::VideoPreview(QWidget *parent) :
//    QDialog(parent)
//{
//    m_dock = new QWidget(this);
//    m_dock->setAccessibleName("kylin-video_QWidget_m_dock");
//    setWindowFlags(windowFlags() | Qt::WindowDoesNotAcceptFocus);

//    kdk::UkuiStyleHelper::self()->removeHeader(this);

//    m_extractor = new VideoFrameExtractor(this);
//    m_extractor->setObjectName("kylin-video_VideoFrameExtractor_m_extractor");
//    connect(m_extractor, SIGNAL(frameExtracted(QtAV::VideoFrame)), SLOT(displayFrame(QtAV::VideoFrame)));
//    connect(m_extractor, SIGNAL(error()), SLOT(displayNoFrame()));
//    connect(this, SIGNAL(fileChanged()), SLOT(displayNoFrame()));
//    m_extractor->setAutoExtract(false);

//    m_out = new VideoOutput(VideoRendererId_Widget, this);
//    m_out->setObjectName("kylin-video_VideoOutput_m_out");
//    m_out->widget()->setParent(m_dock);
//    m_timeLab = new QLabel(m_dock);
//    m_timeLab->setAccessibleName("kylin-video_QLabel_m_timeLab");
//    m_timeLab->setStyleSheet("background-color:qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255, 255, 255, 0), stop:1 rgba(19, 19, 20, 200));color:rgb(222,222,222)");
//    m_timeLab->setFixedHeight(25);
//    QFont f("Noto Sans CJK SC Regular");
//    f.setPixelSize(16);
//    m_timeLab->setFont(f);
//    m_timeLab->setAlignment(Qt::AlignCenter);

//    m_dock->setStyleSheet("border-radius:8px;");
//}

//void VideoPreview::setFile(QString &file)
//{
//    if (file == m_file)
//        return;
//    m_file = file;
//    m_extractor->setSource(m_file);
//}

//void VideoPreview::setTime(qint64 msec)
//{
//    m_extractor->setPosition(msec);
//    QString time_str = Functions::timeToStr((double)msec/1000);
//    m_timeLab->setText(time_str);
//}

//void VideoPreview::display()
//{
//    m_extractor->extract();
//}

//void VideoPreview::displayFrame(const VideoFrame &frame)
//{
//    int diff = qAbs(qint64(frame.timestamp()*1000.0) - m_extractor->position());
//    if (diff > m_extractor->precision()) {
//        //qWarning("timestamp difference (%d/%lld) is too large! ignore", diff);
//    }
//    if (m_out->isSupported(frame.format().pixelFormat())) {
//        m_out->receive(frame);
//        return;
//    }
//    QSize s = m_out->widget()->rect().size();
//    VideoFrame f(frame.to(VideoFormat::Format_RGB32, s));
//    if (!f.isValid())
//        return;
//    m_out->receive(f);
//}

//void VideoPreview::displayNoFrame()
//{
//    m_out->receive(VideoFrame());
//}

//void VideoPreview::resizeEvent(QResizeEvent *e)
//{
//    Q_UNUSED(e);
//    m_dock->setGeometry(QRect(QPoint(0, 0), e->size()));
//    m_out->widget()->setGeometry(QRect(QPoint(0, 0), e->size()));

//    m_timeLab->setGeometry(QRect(QPoint(0, height()-25), QSize(width(), 25)));
//    m_timeLab->raise();
//}
