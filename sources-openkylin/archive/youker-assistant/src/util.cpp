/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "util.h"
#include <QFile>
#include <QProcess>

KDKVolumeBaseType Util::getVolumeUnit(const QString &unit)
{
    qDebug()<<Q_FUNC_INFO;
    if(unit.contains("M",Qt::CaseInsensitive)){
        return KDK_MEGABYTE;
    }else if(unit.contains("G",Qt::CaseInsensitive)){
        return KDK_GIGABYTE;
    }else if(unit.contains("T",Qt::CaseInsensitive)){
        return KDK_TERABYTE;
    }else if(unit.contains("P",Qt::CaseInsensitive)){
        return KDK_PETABYTE;
    }
    return KDK_KILOBYTE;
}

char* Util::getVolume(const QString &volume, KDKVolumeBaseType unit){
    qDebug()<<Q_FUNC_INFO;
    char* ch;
    QByteArray ba = volume.toLatin1();
    ch = ba.data();

    char buf[128] = {0};
    sscanf(ch,"%[0-9]",buf);

    long long val = atoi(buf);

    if(0 == val){
        return ch;
    }

    // 转换
    if(unit == KDK_MEGABYTE){
        val = (val<<20);
    }else if(unit == KDK_GIGABYTE){
        val = (val<<30);
    }else if(unit == KDK_TERABYTE){
        val = (val<<40);
    }else if(unit == KDK_PETABYTE){
        val = (val<<50);
    }else if(unit == KDK_KILOBYTE){
        val = (val<<10);
    }

    static char data[128] = {0};
    sprintf(data,"%lld",val);

    return data;
}

bool Util::isVolumeUnit(const QString &str)
{
    qDebug()<<Q_FUNC_INFO;
    QStringList volumeUnitList = {"B","KB","MB","GB","TB","PB","BIT","KIB","MIB","GIB","TIB","PIB"};
    return volumeUnitList.contains(str.toUpper());
}

bool Util::judgmentHW()
{
    // 990、9A0
    QFile file("/proc/hardware"); 
    if(file.exists()){
        return true;
    }else{
        // panguw
        QProcess proc;
        proc.start("cat /proc/cpuinfo");
        proc.waitForFinished();
        QString cpuInfo = proc.readAll();
        QStringList infoList = cpuInfo.split("\n");
        for (int n = 0; n < infoList.size(); n++) {
            QString strInfoLine = infoList[n];
            if (strInfoLine.contains("PANGU")) {
                return true;
            }
        } 
    }
    // Close the file
    file.close();
    return false;
}

bool Util::isValidDateFormat(const QString &dateString) {
    QRegularExpression dateRegex("^(0[1-9]|1[0-2])/(0[1-9]|[12][0-9]|3[01])/([0-9]{4})$");
    QRegularExpressionMatch match = dateRegex.match(dateString);
    return match.hasMatch();
}
