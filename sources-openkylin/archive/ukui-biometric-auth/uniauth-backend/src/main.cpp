/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
#include "personalizeddata.h"
#include <ukui-log4qt.h>
#include <syslog.h>

int main(int argc, char *argv[])
{
    syslog(LOG_INFO, "[uniauth-backend] start begin!");
    initUkuiLog4qt("ukui-biometric-uniauth");
    QCoreApplication a(argc, argv);
    syslog(LOG_INFO, "[uniauth-backend] KYLINUSERDATAMNG initing!");
    KYLINUSERDATAMNG::instance();
    syslog(LOG_INFO, "[uniauth-backend] KYLINUSERDATAMNG inited!");
    ServiceInterface serviveInterface;
    Q_UNUSED(serviveInterface);
    syslog(LOG_INFO, "[uniauth-backend] qcoreapplication eventloop begin!");
    return a.exec();
}
