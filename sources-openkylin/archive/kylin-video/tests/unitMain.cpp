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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <QApplication>

#include "unitcontruct.h"
#include "playlisttest.h"
#include "qthelpertest.h"
#include "sqlitehandletest.h"
#include "extensionstest.h"
#include "functionstest.h"
#include "pathtest.h"
#include "shortcutsettingtest.h"
#include "filletwidgettest.h"
#include "imagelabeltest.h"
#include "kactiontest.h"
#include "setupdialogtest.h"
#include "setupscreenshottest.h"
#include "global/global.h"


int main(int argc, char** argv)
{
    if(QString(qgetenv("XDG_SESSION_TYPE")) == "wayland")
    {
        qputenv("QT_QPA_PLATFORM", "wayland");
    }
    Global::initGlobal();
    QApplication app(argc, argv);
    unitConstruct unit;
    QTest::qExec(&unit);
    
    
    // 执行PlayList测试
    printf("Creating PlayListTest instance...\n");
    PlayListTest playListTest;
    printf("Running PlayListTest...\n");
    QTest::qExec(&playListTest);
    printf("PlayListTest completed.\n");
    
    // 执行QTHelper测试
    printf("Creating QTHelperTest instance...\n");
    QTHelperTest qthelperTest;
    printf("Running QTHelperTest...\n");
    QTest::qExec(&qthelperTest);
    printf("QTHelperTest completed.\n");
    
    // 执行SqliteHandle测试
    printf("Creating SqliteHandleTest instance...\n");
    SqliteHandleTest sqliteHandleTest;
    printf("Running SqliteHandleTest...\n");
    QTest::qExec(&sqliteHandleTest);
    printf("SqliteHandleTest completed.\n");
    
    // 执行Extensions测试
    printf("Creating ExtensionsTest instance...\n");
    ExtensionsTest extensionsTest;
    printf("Running ExtensionsTest...\n");
    QTest::qExec(&extensionsTest);
    printf("ExtensionsTest completed.\n");
    
    // 执行Functions测试
    printf("Creating FunctionsTest instance...\n");
    FunctionsTest functionsTest;
    printf("Running FunctionsTest...\n");
    QTest::qExec(&functionsTest);
    printf("FunctionsTest completed.\n");
    
    // 执行Paths测试
    printf("Creating PathsTest instance...\n");
    PathsTest pathsTest;
    printf("Running PathsTest...\n");
    QTest::qExec(&pathsTest);
    printf("PathsTest completed.\n");
    
//     执行ShortCutSetting测试
    printf("Creating ShortCutSettingTest instance...\n");
    ShortCutSettingTest shortcutSettingTest;
    printf("Running ShortCutSettingTest...\n");
    QTest::qExec(&shortcutSettingTest);
    printf("ShortCutSettingTest completed.\n");
    
    // 执行FilletWidget测试
    printf("Creating FilletWidgetTest instance...\n");
    FilletWidgetTest filletWidgetTest;
    printf("Running FilletWidgetTest...\n");
    QTest::qExec(&filletWidgetTest);
    printf("FilletWidgetTest completed.\n");
    
    // 执行ImageLabel测试
    printf("Creating ImageLabelTest instance...\n");
    ImageLabelTest imageLabelTest;
    printf("Running ImageLabelTest...\n");
    QTest::qExec(&imageLabelTest);
    printf("ImageLabelTest completed.\n");
    
    // 执行KAction测试
    printf("Creating KActionTest instance...\n");
    KActionTest kactionTest;
    printf("Running KActionTest...\n");
    QTest::qExec(&kactionTest);
    printf("KActionTest completed.\n");
    
    // 执行SetupScreenshot测试
    printf("Creating SetupScreenshotTest instance...\n");
    SetupScreenshotTest setupScreenshotTest;
    printf("Running SetupScreenshotTest...\n");
    QTest::qExec(&setupScreenshotTest);
    printf("SetupScreenshotTest completed.\n");

    return 0;
}
