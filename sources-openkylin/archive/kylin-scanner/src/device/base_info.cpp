/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#include "base_info.h"
#include "common.h"

BaseInfo::BaseInfo()
{
    init();
}

BaseInfo::~BaseInfo()
{

}

void BaseInfo::init()
{
    m_appDisplayName = tr("Scanner");
    m_debianArchitecture = getRetFromCommand(QStringList{"dpkg", "--print-architecture"});
    m_serverAddress = "api.kylinos.cn";
}

QString BaseInfo::getAppDisplayName()
{
    return m_appDisplayName;
}

QString BaseInfo::getDebianArchitecture()
{
    return m_debianArchitecture;
}

QString BaseInfo::getServerAddress()
{
    return m_serverAddress;
}
