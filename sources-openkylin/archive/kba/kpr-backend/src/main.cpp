/*
 * Copyright (C) 2022 Tianjin KYLIN Information Technology Co., Ltd.
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
#include <QCoreApplication>
#include "serviceinterface.h"
//#include <ukui-log4qt.h>

int main(int argc, char *argv[])
{
    //initUkuiLog4qt("ukui-biometric-uniauth");
    QCoreApplication a(argc, argv);

    ServiceInterface serviveInterface;
    Q_UNUSED(serviveInterface);

    return a.exec();
}
