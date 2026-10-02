/*
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
 */
#include <QApplication>
#include "test_horscreen.h"

// Google Mock的main函数
using ::testing::InitGoogleMock;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 初始化Google Mock
    InitGoogleMock(&argc, argv);
    
    TestHorscreen testHorscreen;
    QTest::qExec(&testHorscreen);
    
    // 运行Google Mock测试
    return RUN_ALL_TESTS();
}