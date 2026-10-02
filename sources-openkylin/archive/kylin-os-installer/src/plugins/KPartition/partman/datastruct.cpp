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

#include "datastruct.h"
#include <QMap>

namespace KInstaller{
namespace Partman {
QString GetBiByateValue(QString str)
{
    QString tmpstr = str;
    qint64 tmp = str.toLongLong();
    int length = tmpstr.length();
/*    if(length < 3 && length >= 0) {
       return GetByateValue(tmp);
    } else if(length >= 3 && length < 7) {
       return GetKibiByateValue(tmp);
    } else*/ if(/*length >= 7 && */length < 11) {
       return GetMebiByateValue(tmp);
    } else if(length >= 11 && length < 15) {
       return GetGibiByateValue(tmp);
    }  else if(length >= 15 && length < 19) {
       return GetTebiByateValue(tmp);
    } else if(length >= 19 && length < 23) {
       return GetPebiByateValue(tmp);
    } else if(length >= 23 && length < 27) {
       return GetExbiByateValue(tmp);
    }

}

QString GetBiByateValueStartingG(QString str)
{
    QString tmpstr = str;
    qint64 tmp = str.toLongLong();
    int length = tmpstr.length();
    if(length < 15) {
       return GetGibiByateValue(tmp);
    }  else if(length >= 15 && length < 19) {
       return GetTebiByateValue(tmp);
    } else if(length >= 19 && length < 23) {
       return GetPebiByateValue(tmp);
    } else if(length >= 23 && length < 27) {
       return GetExbiByateValue(tmp);
    }
}

QString GetByateValue(qint64 tmp)
{
    QString reslult = "";
    reslult = QString("%1Byte").arg(tmp);
    return (QString)reslult;
}

QString GetKibiByateValue(qint64 tmp)
{
    double tmp1 = 1.0 * tmp / 1024;
    QString reslult = "";
    reslult = QString("%1KB").arg(QString::number(tmp1, 'f', 1));
    return (QString)reslult;
}
QString GetMebiByateValue(qint64 tmp)
{
    double tmp1 = 1.0 *  tmp / 1024 / 1024;
    QString reslult = "";
    reslult = QString("%1MB").arg(QString::number(tmp1, 'f', 1));
    return (QString)reslult;
}
QString GetGibiByateValue(qint64 tmp)
{
    double tmp1 = 1.0 *  tmp / 1024 / 1024 / 1024;
    QString reslult = "";
    reslult = QString("%1GB").arg(QString::number(tmp1, 'f', 1));
    return (QString)reslult;
}
QString GetTebiByateValue(qint64 tmp)
{
    double tmp1 = 1.0 * tmp / 1024 / 1024 / 1024 / 1024;
    QString reslult = "";
    reslult = QString("%1TB").arg(QString::number(tmp1, 'f', 1));
    return (QString)reslult;
}
QString GetPebiByateValue(qint64 tmp)
{
    double tmp1 = 1.0 * tmp / 1024 / 1024 / 1024 / 1024 / 1024;
    QString reslult = "";
    reslult = QString("%1PB").arg(QString::number(tmp1, 'f', 1));
    return (QString)reslult;
}
QString GetExbiByateValue(qint64 tmp)
{
    double tmp1 = 1.0 * tmp / 1024 / 1024 / 1024 / 1024 / 1024 / 1024;
    QString reslult = "";
    reslult = QString("%1EB").arg(QString::number(tmp1, 'f', 1));
    return (QString)reslult;
}


QString GetPartTableNameByType(PartTableType type)
{
    switch (type) {
    case PartTableType::GPT:
        return "gpt";
    case PartTableType::Empty:
        return "Empty";
    case PartTableType::MsDos:
        return "msdos";
    case PartTableType::NONE:
    case PartTableType::Others:
    case PartTableType::UnKown:

    default:
        return "unkown";
    }
}


}

}
