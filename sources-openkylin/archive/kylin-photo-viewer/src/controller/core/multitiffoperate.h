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
#ifndef MULTITIFFOPERATE_H
#define MULTITIFFOPERATE_H

#include <QObject>
#include <QThread>
#include "model/processing/processing.h"
class MultiTiffOperate : public QThread
{
    Q_OBJECT
public:
    MultiTiffOperate(const QString &path, const QList<Processing::FlipWay> operateList);
Q_SIGNALS:
    void tiffOPerateFinish(QString path);

public:
    void run();

private:
    QString m_path;
    Processing::FlipWay m_operateWay;
    QList<FIBITMAP *> m_fibilist;
    QList<Processing::FlipWay> m_operateList;
    void finishOperate();
    //    QPixmap changImageSize(const QPixmap &pix);
};

#endif // MULTITIFFOPERATE_H
