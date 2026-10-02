/*
 * Copyright (C) 2025 KylinSoft Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ukui-log4qt.h"
#include <QCoreApplication>

int main(int argc, char **argv)
{
    if (argc < 2) {
        puts("I need more args");
        return 0;
    }
    initUkuiLog4qt(argv[1]);
    QCoreApplication app(argc, argv);
    KyDebug("debug");
    KyInfo("info");
    KyWarning("warning");
    KyCritical("critical");
    return app.exec();
}
