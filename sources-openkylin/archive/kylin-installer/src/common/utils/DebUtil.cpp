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
#include "DebUtil.h"

QString DebUtil::getDebInfo(QString debPath, QString type)
{
    debPath = QDir::toNativeSeparators(debPath);
    qWarning() << "debPath" <<debPath;

    QApt::DebFile debfile(debPath);
    if (debfile.isValid()){
        if (type == "Package"){
            return debfile.packageName();}
        else if (type == "Version"){
            return debfile.version();
        }
        else{
            return debfile.architecture();
        }
    }
    else{
        return "The software package is damaged";
    }
}
