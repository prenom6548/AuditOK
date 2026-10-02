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

#ifndef CLOUDPLATFORM_H
#define CLOUDPLATFORM_H

#include <QObject>
#include <kysdk/kysdk-system/libkysysinfo.h>

class CloudPlatform : public QObject
{
    Q_OBJECT
public:
    explicit CloudPlatform(QObject *parent = nullptr);
    static CloudPlatform* self();

private:
    bool m_isCloudPlatform = false;
public:
    bool isCloudPlatform();
signals:

};

#endif // CLOUDPLATFORM_H
