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
#include "package_helper.h"
#include "common.h"
#include <QThread>
#include <QTimer>






PackageHelper::PackageHelper(DeviceInformation &info, QObject *parent)
    : m_info(info), QObject(parent)
{
    m_thread = new QThread();
    moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &PackageHelper::dowork);

    connect(this, &PackageHelper::finished, m_thread, &QThread::quit);
    connect(this, &PackageHelper::finished, this, &PackageHelper::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

PackageHelper::~PackageHelper()
{

}

void PackageHelper::dowork()
{
    getPackagesNameFromLocal(m_info);

    if (m_info.packageInfoList.size() == 0) {
        getPackagesNameFromHttp(m_info);
        if (m_info.packageInfoList.size() == 0) {
            failed();
            return;
        }
    }

    int needInstall = packageListNeedInstall(m_info.packageInfoList);

    succeed(needInstall);
}
