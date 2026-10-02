/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include "shredqthread.h"
#include <QByteArray>
#include <QDebug>

ShredQThread::ShredQThread(QString strFileName)
{
    this->m_strFileName = strFileName;
}

ShredQThread::~ShredQThread() {}

void ShredQThread::run()
{
    QByteArray ba = m_strFileName.toUtf8();
    int result = do_file(ba.data());
    if (result == 0) {
        // success
        emit success();
    } else {
        // failed
        qDebug() << Q_FUNC_INFO << result;
        emit failed();
    }
}
