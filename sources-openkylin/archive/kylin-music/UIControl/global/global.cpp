/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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

#include "global.h"

#include <QSettings>
#include <QGSettings>
#include <QApplication>
#include <QFile>
#include <QDebug>

bool Global::isWayland = false;

using namespace Global;

void Global::global_init() {

    // 运行环境
    if(QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
        qputenv("QT_QPA_PLATFORM", "wayland");
        isWayland = true;
    } else {
        isWayland = false;
    }

}

void Global::global_end() {

}
