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

#ifndef BASEINFO_H
#define BASEINFO_H

#include <QObject>
#include <QString>
#include "singleton.h"

class BaseInfo : public QObject,
                 public Singleton<BaseInfo>
{
    Q_OBJECT
    friend class Singleton<BaseInfo>;
private:
    BaseInfo();
    ~BaseInfo() override;

    void init();
public:
    QString getAppDisplayName();
    QString getDebianArchitecture();
    QString getServerAddress();
private:
    QString m_appDisplayName;
    QString m_debianArchitecture;
    QString m_serverAddress;
};

#endif  // BASEINFO_H
