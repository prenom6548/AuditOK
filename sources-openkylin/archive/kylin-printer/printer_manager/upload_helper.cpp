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
#include "upload_helper.h"
#include "common.h"
#include <QThread>
#include <QTimer>

UploadHelper::UploadHelper(DeviceInformation info, int ppdRank, QObject *parent)
    : m_info(info), m_ppdRank(ppdRank), QObject(parent)
{
    m_thread = new QThread();
    moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &UploadHelper::dowork);

    connect(this, &UploadHelper::finished, m_thread, &QThread::quit);
    connect(this, &UploadHelper::finished, this, &UploadHelper::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

UploadHelper::~UploadHelper()
{

}

void UploadHelper::dowork()
{
    postDeviceInfo(m_info, m_ppdRank);


    finished();
}
