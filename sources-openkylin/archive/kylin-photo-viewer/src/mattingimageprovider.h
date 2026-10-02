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
#ifndef MATTINGIMAGEPROVIDER_H
#define MATTINGIMAGEPROVIDER_H

#include <QQuickAsyncImageProvider>
#include "./view/sizedate.h"

class MattingImageProvider : public QQuickImageProvider
{
public:
    MattingImageProvider();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
    void setImage(QImage &img);
private:
    QImage m_image;
};

class ShowMattingImage : public QObject
{
    Q_OBJECT
public:
    explicit ShowMattingImage(QObject *parent = 0);
    MattingImageProvider *getImgProvider();

public Q_SLOTS:
    void setImage(QImage &img);
Q_SIGNALS:
    void callQmlRefeshImg();

private:
    MattingImageProvider *m_pImgProvider = nullptr;
};

#endif // MATTINGIMAGEPROVIDER_H
