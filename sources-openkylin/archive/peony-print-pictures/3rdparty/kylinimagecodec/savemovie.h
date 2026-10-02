/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
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


#ifndef SAVEMOVIE_H
#define SAVEMOVIE_H

#include <QThread>
#include <QFileInfo>
#include <QProcess>
#include <QDir>
#include <QSemaphore>
#include "kylinimagecodec_global.h"

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
    static const QString TEMP_PATH;       //临时文件路径
    static const QString creatTempPath(); //创建临时目录
};

#endif // SAVEMOVIE_H
