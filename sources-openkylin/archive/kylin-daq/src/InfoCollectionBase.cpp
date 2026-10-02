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
#include "InfoCollectionBase.h"

InfoCollectionBase::InfoCollectionBase(QObject *parent)
    :QObject(parent)
{

}

void InfoCollectionBase::getOnePopenResult(const QString &shellCmd, QString &result)
{
    QList<QString> resultList;
    getPopenResult(shellCmd, resultList);
    if (!resultList.isEmpty()) {
        result = resultList.first();
    } else {
        result = "";
    }
    result.simplified();
}

void InfoCollectionBase::getPopenResult(const QString &shellCmd, QList<QString> &result)
{
    QStringList options;
    options << "-c"<< shellCmd;
    QProcess proc;
    proc.start("/bin/bash", options);
    proc.waitForFinished(10 * 1000);
    QByteArray arr = proc.readAllStandardOutput();

    QString tmp = QString(arr);

    if (tmp.contains("\n")){
        result = tmp.split("\n");
    }else {
        result.append(tmp);
    }
}
