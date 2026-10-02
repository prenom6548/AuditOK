/*
 * libkysdk-thememanager's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#include <QCoreApplication>
#include <QDebug>
#include "kwatermark.h"
// #include "../src/kwatermark.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    kdk::KWatermark mark;
    kdk::WatermarkData data = mark.createWatermark("/home/kylin/111.png", 0.3, 200, 200, 3, 0);
    qDebug() << QString("%1%2").arg("Create watermark:").arg(data.handle.isEmpty() ? data.path : data.handle);

    qDebug() << "Update watermark:" << mark.updateWatermark(data, "/home/kylin/111.png", 0.3, 200, 200, 2, 1);

    mark.destroyWatermark(data);

    return a.exec();
}
