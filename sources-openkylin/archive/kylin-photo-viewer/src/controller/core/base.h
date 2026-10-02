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
#ifndef BASE_H
#define BASE_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QPixmap>
#include <QPrinter>
#include <QColor>
#include <QTimer>
#include <QGSettings>
#include <QStandardItemModel>
#include <QFileSystemWatcher>
#include "model/dbus.h"
#include "model/file/file.h"
#include "model/processing/processing.h"
#include "albumthumbnail.h"
#include "multitiffoperate.h"
#include <libkydatacollect.h>
#include "global/interactiveqml.h"
#include "model/marktool/marktool.h"
#ifdef USE_ONNXRUNTIME
#include <onnxruntime/onnxruntime_cxx_api.h>
#endif
#ifdef SUPPORTSCANNER
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/imgproc.hpp>
using namespace cv;
#endif
typedef std::vector<std::string> (*GetOcrFunc)(const std::string &, int);

class Enums
{
public:
    enum ChamgeImageType { NEXT_IMAGE = -1, BACK_IMAGE = -2, ERROR_IMAGE = -9 };
    enum ChangeShowSizeType { BIG = 0, SMALL, ORIGIN, AUTO }; //放大、缩小、原图、自适应
    //成功，同名，文件不存在，没有权限，其他错误，以.开头，名字为空
    enum RenameState {
        SUCCESS = 0,
        SAME_NAME = -1,
        NOT_EXITS = -2,
        NO_PEIMISSION = -3,
        UNKNOWN_ERROR = -4,
        START_WITH_POINT = -5,
        EMPTY_NONE = -6
    };
};

class ImageShowStatus : public Enums //显示图片细节相关，记录显示图片状态
{
public:
    QSize m_size; //记录当前窗口大小
    QFileInfo m_info;
    QString m_nowpath;   //记录当前路径
    QString m_backpath;  //记录上次路径
    QString m_failImage; //记录当m_nowpath和m_backpath都为空时的上一次的失败路径
    QImage m_nowImage;        //记录当前打开
    QImage m_backImage;       //记录上次打开
    QImage m_cutImage;        //裁剪时使用的显示图片
    QImage m_nowPixmap;
    int m_maxFrame = 0;                           //图片最大帧数
    QList<ProcessingBase::FlipWay> m_willProcess; //存操作方式
    QList<ProcessingBase::FlipWay> m_tiffProcess; // tiff的操作方式
    QString m_tiffProcessPath = "";               //正在操作的tiff图片
    int m_willProcessNum = 0;                     //存当前加载出来的帧数
    double m_proportion;                          //图片显示比例
    double m_origProportion;                        //只针对大图片的原图比例
    QSize m_tmpSize;                              //按比例缩放后的图片大小
    int m_lastTimeProportion = 0;                 //上次缩放图片显示比例
    QString m_imageSize;                          //原图大小
    QString m_colorSpace;                         //颜色域
    QTimer *m_playMovieTimer;                     //播放动图的计时器
    QList<QImage> *m_imageList;                        //存储动图的每一帧
    int m_imageListIndex = 0;                       //当前播放到哪一帧
    QPoint m_startShowPoint;
    int m_fps = 0;                              //帧率
    bool m_processed = false;                   //此图被操作过
    bool m_thisImageIsSaving = false;           //此图片正在储存
    bool m_openSuccess = true;                  //默认打开图片成功，设置为true
    QStandardItemModel *m_albumModel = nullptr; //相册model
    QPixmap m_defultPixmap = QPixmap(":/res/res/kyview_logo.png");
    QPixmap m_item0White = QPixmap(":/res/res/addalbum_white.png");
    QPixmap m_item0Black = QPixmap(":/res/res/1addalbum_black.png");
    QPixmap m_openImage = QPixmap(":/res/res/damaged_img.png");
    //    QPixmap m_openImageB = QPixmap(":/res/res/damaged_imgB.png");
};

#endif // BASE_H
