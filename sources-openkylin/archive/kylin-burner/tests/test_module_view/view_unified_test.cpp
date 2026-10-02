/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

// 统一运行 BurnData、CopyDisc、BurnImage、MainWindow、DiskCheck、BusnessSetting 六个单元测试
#define RUN_VIEW_UNIFIED_TESTS

#include <QtTest>

#include "burndata_test.cpp"
#include "copydisc_test.cpp"
#include "burnimage_test.cpp"
#include "mainwindow_test.cpp"
#include "diskcheck_test.cpp"
#include "busnesssetting_test.cpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    int status = 0;
    BurnDataTest burnDataTest;
    status |= QTest::qExec(&burnDataTest, argc, argv);

    CopyDiscTest copyDiscTest;
    status |= QTest::qExec(&copyDiscTest, argc, argv);

    BurnImageTest burnImageTest;
    status |= QTest::qExec(&burnImageTest, argc, argv);

    MainWindowTest mainWindowTest;
    status |= QTest::qExec(&mainWindowTest, argc, argv);

    DiskCheckTest diskCheckTest;
    status |= QTest::qExec(&diskCheckTest, argc, argv);

    BusnessSettingTest busnessSettingTest;
    status |= QTest::qExec(&busnessSettingTest, argc, argv);

    return status;
}
