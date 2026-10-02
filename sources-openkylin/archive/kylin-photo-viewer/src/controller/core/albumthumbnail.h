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
#ifndef ALBUMTHUMBNAIL_H
#define ALBUMTHUMBNAIL_H

#include <QThread>
#include "global/variable.h"
#include "model/processing/processing.h"
#include "model/file/file.h"

class AlbumThumbnail : public QThread
{
    Q_OBJECT

Q_SIGNALS:
    void albumFinish(QVariant var1, QVariant var2, QVariant var3);

public:
    AlbumThumbnail(const QString &path, const QString &realFormat);
    void run();

private:
    QString m_path= "";
    QString m_realFormat = "";
    QImage changImageSize(const QPixmap &pix);
};

class MainImageLoad : public QThread
{
    Q_OBJECT

Q_SIGNALS:
    void mainImageLoadFinish(QString path, QVariant var1, QVariant var2);
public:
    MainImageLoad(const QString &path, const QString &realFormat);
    void run();

private:
    QString m_path= "";
    QString m_realFormat = "";
};

class FindImageFromDir : public QThread
{
    Q_OBJECT

Q_SIGNALS:
    void findFinish(QStringList images, QString path);
public:
    FindImageFromDir(const QString &path);
    void run();

private:
    QString m_path= "";
};

#endif // ALBUMTHUMBNAIL_H
