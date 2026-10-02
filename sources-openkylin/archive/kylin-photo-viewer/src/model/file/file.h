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
#ifndef FILE_H
#define FILE_H
#define THEMEDARK "ukui-dark"
#define THEMEBLACK "ukui-black"
#include <QObject>
#include <QFileInfo>
#include <QVariant>
#include "model/processing/processing.h"
#include "global/computingtime.h"
#include "gio/gio.h"
#include <gio/gfileinfo.h>
#include "global/variable.h"
#include <QUrl>
#include <dlfcn.h>
typedef ImageResult ImageAndFileinfo;
Q_DECLARE_METATYPE(ImageAndFileinfo) // QVarant 注册自定义类型
class File : public QObject
{
    Q_OBJECT

public:
    static ImageAndFileinfo loadImage(QString path, QString realFormat = "");
    static bool saveImage(const QImage &image, const QString &savepath, const QString &realFormat,
                          bool replace = true); //静态图
    static bool saveImage(QList<QImage> *list, const int &fps, const QString &savepath, const QString &realFormat,
                          bool replace = true); //动态图
    static void deleteImage(const QString &savepath);
    static bool isSaving(const QString &path);
    static bool allSaveFinish();
    static bool canDel(QString path); //判断是否可删除
    static QString savePath();        //返回保存路径--针对裁剪和另存为
    static void changeSaveSign(bool needSave);
    static QString realFormat(const QString &path); //返回图片真正的格式

private:
    static void processStart(const QString &cmd, QStringList arguments = QStringList());
    static QProcess *m_process; //操作文件
    static QString m_imageRealFormat;
    static void *m_securityFd;
};

#endif // FILE_H
