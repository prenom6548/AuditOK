/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "kernelfilesystemfs.h"
#include <QString>
#include <QDebug>
#include "../ksystemsetting_unit.h"


QStringList getKernelFs()
{
    QStringList list = {};
    QStringList list1 = {};
    QStringList res = {};
    const QString cmd{"/bin/bash"};
    QString arg{"cat /proc/filesystems | grep -v nodev"};
    QString output, error, dir;
    int exitcode = 0;
    if(KServer::KCommand::getInstance()->RunScripCommand(cmd, {"-c" ,arg}, dir, output, error, exitcode)){
        output.remove(0, 1);
        output.remove(output.size() -1, 1);
        list = output.replace("\n\t", " ").split(" ", QString::SkipEmptyParts );
        qDebug() << list;
    } else {
        qWarning() << QString("cat /proc/filesystems: Failed:%1").arg(error);
    }
    list1 = KServer::ReadSettingIni("setting", "FileSystem").split(";");
    for (int j = 0; j < list.count(); ++j) {
        for (int i = 0; i < list1.count(); ++i) {
            if(list.at(j) == list1.at(i)) {
                res.append(list.at(j));
                break;
            }
        }
    }
    res.append("kylin-data");
    res.append("efi");
    res.append("linux-swap");
    res.append("lvm-pv");
    res.append("unused");

    if(res.contains("ext4"))//保证第一个是ext4分区
    {
        if(res.removeOne("ext4"))
        {
            res.insert(0,"ext4");
        }
    }
    if(res.contains("efi"))//保证第二个是ext4分区
    {
        if(res.removeOne("efi"))
        {
            res.insert(1,"efi");
        }
    }
    if(res.contains("kylin-data"))//保证第三个是ext4分区
    {
        if(res.removeOne("kylin-data"))
        {
            res.insert(2,"kylin-data");
        }
    }
    if(res.contains("linux-swap"))//保证第四个是ext4分区
    {
        if(res.removeOne("linux-swap"))
        {
            res.insert(3,"linux-swap");
        }
    }
    if(res.contains("lvm-pv"))//保证第四个是ext4分区
    {
        if(res.removeOne("lvm-pv"))
        {
            res.insert(4,"lvm-pv");
        }
    }


    return res;
}

