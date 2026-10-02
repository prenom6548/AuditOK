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

// 统一运行 FileFilterModel、LogicBurnImage、LogicCopyDisc、LogicMain 四个单元测试
#define RUN_MODEL_UNIFIED_TESTS

#include <QtTest>

#include "filefiltermodel_test.cpp"
#include "logicburnimage_test.cpp"
#include "logiccopydisc_test.cpp"
#include "logicmain_test.cpp"
#include "menufactory_test.cpp"
#include "model_test.cpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    int status = 0;
    FileFilterModelTest fileFilterModelTest;
    status |= QTest::qExec(&fileFilterModelTest, argc, argv);

    LogicBurnImageTest logicBurnImageTest;
    status |= QTest::qExec(&logicBurnImageTest, argc, argv);

    LogicCopyDiscTest logicCopyDiscTest;
    status |= QTest::qExec(&logicCopyDiscTest, argc, argv);

    LogicMainTest logicMainTest;
    status |= QTest::qExec(&logicMainTest, argc, argv);

    MenuFactoryTest meufactoryTest;
    status |= QTest::qExec(&meufactoryTest, argc, argv);

    ModelTest modelTest;
    status |= QTest::qExec(&modelTest, argc, argv);

    return status;
}
