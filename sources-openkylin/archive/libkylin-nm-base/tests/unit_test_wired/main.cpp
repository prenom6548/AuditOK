/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QThread>

int main(int argc, char **argv)
{
    // 创建 QCoreApplication 实例以支持 QEventLoop 和信号槽
    QCoreApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);

    // 确保 QCoreApplication 实例存在，避免 QEventLoop 报错
    // 这对于测试中使用的信号槽连接是必需的

    testing::InitGoogleTest(&argc, argv);

    int ret = RUN_ALL_TESTS();

    // 确保在退出前等待所有线程完成
    QThread::sleep(1);

    return ret;
}
