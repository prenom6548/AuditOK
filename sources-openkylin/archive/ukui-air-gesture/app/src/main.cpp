/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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

#include <QDebug>
#include "ukui-log4qt.h"

#include "singleapplication.h"

int main(int argc, char *argv[])
{
    initUkuiLog4qt("ukui-air-gesture");

    SingleApplication a(argc, argv);

    const SingleApplication::InitResult result = a.getInitResult();
    switch (result) {
    case SingleApplication::InitResult::Success:
        break;
    case SingleApplication::InitResult::AlreadyRunning:
        return 0;
    case SingleApplication::InitResult::DBusFailed:
        qWarning() << "Application exiting: D-Bus registration failed.";
        return 1;
    case SingleApplication::InitResult::PythonFailed:
        qWarning() << "Application exiting: Python subprocess failed to start.";
        return 1;
    }

    return a.exec();
}