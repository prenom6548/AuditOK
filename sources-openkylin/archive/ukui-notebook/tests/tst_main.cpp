#include <QtTest>
#include "tst_notemodel.h"
#include "tst_dbmanager.h"
#include "tst_notedata.h"
#include "tst_adaptscreeninfo.h"
#include "tst_editpage.h"
#include "tst_iconviewmodedelegate.h"
#include "tst_listviewmodedelegate.h"
#include "tst_widget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    int status = 0;

    // 创建并运行 NoteModel 测试
    TestNoteModel noteModelTest;
    status |= QTest::qExec(&noteModelTest, argc, argv);
    
    // 创建并运行 DBManager 测试
    TestDBManager dbManagerTest;
    status |= QTest::qExec(&dbManagerTest, argc, argv);

    TestNoteData noteDataTest;
    status |= QTest::qExec(&noteDataTest, argc, argv);

    TestAdaptScreenInfo adaptScreenInfoTest;
    status |= QTest::qExec(&adaptScreenInfoTest, argc, argv);

    TestEditPage editPageTest;
    status |= QTest::qExec(&editPageTest, argc, argv);

    TestIconViewModeDelegate iconViewModeDelegateTest;
    status |= QTest::qExec(&iconViewModeDelegateTest, argc, argv);

    TestListViewModeDelegate listViewModeDelegateTest;
    status |= QTest::qExec(&listViewModeDelegateTest, argc, argv);

    TestWidget widgetTest;
    status |= QTest::qExec(&widgetTest, argc, argv);
    
    return status;
}
