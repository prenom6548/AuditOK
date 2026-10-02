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
#ifndef ROTATION_H
#define ROTATION_H

#include <QObject>
#include <QMessageBox>
#include <dlfcn.h>

#include "../codec/kylinimagecodec/kylinimagecodec.h"

class Rotation : public QObject
{
    Q_OBJECT
public:
    explicit Rotation(QObject *parent = nullptr);

    bool rotateAndSaveImage(QStringList imagePaths, bool way);  //图片旋转保存操作

    bool m_movieRotate = true;      //是否可以重新进行动图旋转
    bool m_isClosed = false;        //进度条是否关闭

Q_SIGNALS:
    void imageFinishSig();                                      //单张图片处理完成信号
    void allImageFinishSig();                                   //所有图片处理完成信号

private:

    QStringList m_file_type_list = {"exr", "jfi", "jif", "jp2", "j2k", "jng", "wbmp", "xbm",
                                    "pnm", "tiff","tif", "webp", "bmp", "dib", "png", "jpg", "jpe",
                                    "jpeg", "pbm", "pgm", "ppm", "svg",  "ico", "xpm", "sr",
                                     "tga"};    // "psd", "ras",暂不支持  //"gif", "apng", 和看图同步 暂不支持
    //图片信息
    QString m_imageFormat;                          //图片格式
    //获取图片格式
    QString m_imageRealFormat;

    void *m_securityFd = nullptr;

    bool rotateFun(QString imagePath, bool way);    //图片旋转操作
    bool is_write( QString imagePath);              //是否可写、是否是图片
    bool is_pic_file(QString imagepath);            //是否是支持的图片格式
    QImage processImage(const QImage &image,  bool &way);
    QImage rotateRightFun(QImage image);
    QImage rotateLeftFun(QImage image);
    QString realFormat(const QString &path);        //获取图片类型
    void processStart(const QString &cmd, QStringList arguments);





};

#endif // ROTATION_H
