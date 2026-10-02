/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include "greeterutils.h"
#include <QSettings>
#include <QFile>

namespace UKUIGreeter::Utils {

bool isCommunity()
{
    static int sIsCommunity = -1;
    if (sIsCommunity == -1) {
        QString filename = "/etc/os-release";
        QSettings osSettings(filename, QSettings::IniFormat);

        QString versionID = osSettings.value("VERSION_ID").toString();

        if (versionID.compare("22.04", Qt::CaseSensitive)) {
            sIsCommunity = 0;
        } else {
            sIsCommunity = 1;
        }
    }
    return (bool)(sIsCommunity);
}

QString getDefaultFace()
{
    if (isCommunity()) {
        return ":/images/01-default-community.png";
    } else {
	//优先读取控制面板的默认头像文件，保持登录锁屏默认头像和控制面板一致
	QFile faceFile("/usr/share/ukui/faces/default.png");
	if(faceFile.exists()){
	    return "/usr/share/ukui/faces/default.png";
	}
        return ":/images/01-default-commercial.png";
    }
}

}
