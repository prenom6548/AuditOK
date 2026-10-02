/*
 * libkysdk-base's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#ifndef KYLIN_IMAGE_CODEC_SAVEMOVIE_H_
#define KYLIN_IMAGE_CODEC_SAVEMOVIE_H_

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QSemaphore>
#include <QThread>

#include "kylin_image_codec/kylinimagecodec_global.hpp"

/**
 * @brief kabase模块应用通用函数
 * @since 1.2.0
 */

namespace kdk
{
namespace kabase
{

using namespace cv;

class SaveMovie : public QThread
{
    Q_OBJECT

Q_SIGNALS:
    void saveMovieFinish(const QString &path);

public:
    SaveMovie(QList<Mat> *list, const int &fps, const QString &savepath, QStringList *savelist = nullptr);

protected:
    void run();

private:
    QList<Mat> *m_list;
    QStringList *m_savelist;
    int m_fps;
    QString m_savepath;
    bool m_special;
    QProcess *m_process = nullptr;
    void saveFinish();
    void processLog();
    static const QString TEMP_PATH; // 临时文件路径
    static const QString creatTempPath(); // 创建临时目录
};

} // namespace kabase
} // namespace kdk

#endif // SAVEMOVIE_H
