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

#ifndef ADDRSET_H
#define ADDRSET_H

#include <QWidget>

class AddrSet : public QWidget
{
    Q_OBJECT
public:
    explicit AddrSet(QWidget *parent = nullptr);
    ~AddrSet();
    QString getHostIpAddress();  //获取IP
    QString getHostMacAddress(); //获取Mac
    QString getLocalHost();      //获取主机名

    static AddrSet *getInstance();

private:
    AddrSet *m_addrset;
};

#endif // ADDRSET_H
