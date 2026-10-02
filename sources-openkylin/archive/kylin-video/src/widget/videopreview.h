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
//#ifndef VIDEOPREVIEW_H
//#define VIDEOPREVIEW_H

//#include <QLabel>
//#include <QDialog>
//#include <QtAV/VideoOutput.h>
//#include <QtAV/VideoFrameExtractor.h>

//using namespace QtAV;

//class VideoPreview : public QDialog
//{
//    Q_OBJECT
//public:
//    explicit VideoPreview(QWidget *parent = nullptr);

//    void setFile(QString &file);
//    void setTime(qint64 msec);
//    void display();
//    void setFont(const QFont& f) {
//        m_timeLab->setFont(f);
//    }
//signals:

//public slots:
//    void displayFrame(const QtAV::VideoFrame& frame);
//    void displayNoFrame();

//private:
//    QString m_file;
//    VideoFrameExtractor *m_extractor;
//    QWidget *m_dock;

//    VideoOutput *m_out;
//    QLabel *m_timeLab;
//    QLabel *m_imageLab;

//    void resizeEvent(QResizeEvent *e) override;
//};

//#endif // VIDEOPREVIEW_H
