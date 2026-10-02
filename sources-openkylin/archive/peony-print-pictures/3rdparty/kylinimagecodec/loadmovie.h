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


#ifndef LOADMOVIE_H
#define LOADMOVIE_H

#include <QThread>
#include <QMovie>
#include "kylinimagecodec_global.h"

using namespace cv;

class LoadMovie : public QThread
{
    Q_OBJECT

Q_SIGNALS:
    void loadMovieFinish(const QString &path);

public:
    LoadMovie(QList<Mat> *list, QMovie *movie);

protected:
    void run();

private:
    QList<Mat> *m_list = nullptr;
    QMovie *m_movie = nullptr;
};

#endif // LOADMOVIE_H
