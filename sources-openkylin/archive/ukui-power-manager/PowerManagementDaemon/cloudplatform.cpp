/*
 * Copyright 2023 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cloudplatform.h"

#include <QDebug>

Q_GLOBAL_STATIC(CloudPlatform, s_cloudPlatform)

CloudPlatform::CloudPlatform(QObject *parent) : QObject(parent)
{
    QString cloudPlatForm = QString(QLatin1String(kdk_system_get_hostCloudPlatform()));
    QString hostType = QString(QLatin1String(kdk_system_get_hostVirtType()));

    qDebug() << "cloud platform name:" << cloudPlatForm
             << "host type:" << hostType;
    if ((cloudPlatForm == "huawei" || cloudPlatForm == "ctyun" || hostType != "none")) {
        m_isCloudPlatform = true;
        qDebug() << "is cloud platform";
    }
}

CloudPlatform *CloudPlatform::self()
{
    return s_cloudPlatform;
}

bool CloudPlatform::isCloudPlatform()
{
    return m_isCloudPlatform;
}
