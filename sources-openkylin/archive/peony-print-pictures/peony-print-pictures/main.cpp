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

#include "print-pictures-plugin.h"
#include "custom_log.h"
#include <QApplication>
#include <QDebug>
int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    Peony::PrintPicturesPlugin p;
    if (argc == 0)
        return 0;
    p.setSelectionUris({argv[1]});
    p.doPrintAction();
    a.exec();
}
