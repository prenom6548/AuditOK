/*
 * KWin Style UKUI
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#ifndef PDEBUG_H
#define PDEBUG_H
#include <QDebug>
#include <QDateTime>

#define SERVICE "com.kylin.statusmanager.interface"
#define PATH "/"
#define INTERFACE "com.kylin.statusmanager.interface"

#define UKUI_SEARCH_SCHEMAS          "org.ukui.search.settings"
#define SEARCH_METHOD_KEY            "fileIndexEnable"

namespace UKUIFileDialog {

class NullDebug

{

public:

template<typename T>

NullDebug & operator <<(const T&){return *this;}

};
inline NullDebug nullDebug(){return NullDebug();}
}

//#define LOG_TIMI

#ifdef LOG_TIMI

#    define pDebug qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz") << " platformtheme input:"

#else

#    define pDebug UKUIFileDialog::nullDebug()

#endif

#endif // KYFILEDIALOG_H
