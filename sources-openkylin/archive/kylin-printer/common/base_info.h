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
    void initDbus();
    void setSystemVersion();
public:
    QString getAppDisplayName();
    QString getDebianArchitecture();
    QString getServerAddress();
    QString getSystemVersion();
private:
    QString m_appDisplayName;
    QString m_debianArchitecture;
    QString m_serverAddress;
    QString m_systemVersion;
public:
    QString KUM_DEST;
    QString KUM_PATH;
    QString KUM_IFACE;
};

#endif  // BASEINFO_H
