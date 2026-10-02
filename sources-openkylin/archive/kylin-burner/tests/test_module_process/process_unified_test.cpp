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

// 统一运行 ProcessMd5Sum、ProcessOpenFile、ProcessLsof 三个单元测试
#define RUN_PROCESS_UNIFIED_TESTS

#include <QtTest>

#include "processmd5sum_test.cpp"
#include "processopenfile_test.cpp"
#include "processlsof_test.cpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    int status = 0;
    ProcessMd5SumTest processMd5SumTest;
    status |= QTest::qExec(&processMd5SumTest, argc, argv);

    ProcessOpenFileTest processOpenFileTest;
    status |= QTest::qExec(&processOpenFileTest, argc, argv);

    ProcessLsofTest processLsofTest;
    status |= QTest::qExec(&processLsofTest, argc, argv);

    return status;
}
