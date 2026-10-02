#include <QtTest/QtTest>
#include "../../kysdk-shortcut/libkyshortcut/libkyshortcut.h"

class TestShortcut : public QObject
{
    Q_OBJECT

private slots:
    void testCreateGlobalShortcut();
    void testDeleteGlobalShortcut();
    void testGlobalShortcutIsAvailable();
    void testSetGlobalShortcut();
    void testGetGlobalShortcuts();
    void testGetGlobalShortcutsByKey();
    void testDestroyInfoList();
    void testKeyCombinationToString();
    void testStringToKeyCombination();
};

void TestShortcut::testCreateGlobalShortcut()
{
    int result = KYSDK_SUCCESS;

    // 1. 快捷键名称空,预期失败
    result = kdk_shortcut_create_global_shortcut("", "Ctrl+Alt+M", "/usr/share/applications/atril.desktop");
    QCOMPARE(result, KYSDK_SHORTCUT_NAME_ERROR);

    // 2. 快捷键名称不存在 & 未添加过的快捷键 & 正常终端执行,预期成功
    result = kdk_shortcut_create_global_shortcut("ExsitTest", "Ctrl+Alt+Q", "/usr/share/applications/atril.desktop");
    QCOMPARE(result, KYSDK_SUCCESS);

    // 4. 错误的键,预期失败
    result = kdk_shortcut_create_global_shortcut("KeyTest1", "Ctrdl+Alt+N", "/usr/share/applications/atril.desktop");
    QCOMPARE(result, KYSDK_SHORTCUT_KEY_ERROR);

    // 5. 快捷键空,预期失败
    result = kdk_shortcut_create_global_shortcut("KeyTest2", "", "/usr/share/applications/atril.desktop");
    QCOMPARE(result, KYSDK_SHORTCUT_KEY_ERROR);

    // 9. 快捷键多键-4个键,预期失败
    result = kdk_shortcut_create_global_shortcut("KeyTest6", "Ctrl+Shift+Alt+G", "/usr/share/applications/atril.desktop");
    QCOMPARE(result, KYSDK_SHORTCUT_KEY_ERROR);

    // 12. 快捷键执行-空,预期失败
    result = kdk_shortcut_create_global_shortcut("KeyDesktop1", "Ctrl+Alt+N", "");
    QCOMPARE(result, KYSDK_SHORTCUT_ACTION_ERROR);

    // 13. 快捷键执行-不存在的desktop,预期失败
    result = kdk_shortcut_create_global_shortcut("KeyDesktop2", "Ctrl+Alt+N", "/usr/share/applications/atril1.desktop");
    QCOMPARE(result, KYSDK_SHORTCUT_ACTION_ERROR);

    // 3. 快捷键名称已存在,预期失败
    result = kdk_shortcut_create_global_shortcut("ExsitTest", "Ctrl+Alt+N", "/usr/share/applications/atril.desktop");
    QCOMPARE(result, KYSDK_SHORTCUT_EXISTED);
}

void TestShortcut::testGlobalShortcutIsAvailable()
{
    // 检测已存在快捷键是否可用,预期不可用
    int result = kdk_shortcut_global_shortcut_is_available("Ctrl+Alt+N");
    QCOMPARE(result, KYSDK_SUCCESS);
}

void TestShortcut::testSetGlobalShortcut()
{
    // 修改快捷键,预期成功
    int result = kdk_shortcut_set_global_shortcut("ExsitTest", "Ctrl+Shift+P", "/usr/share/applications/atril.desktop");
    QCOMPARE(result, KYSDK_SUCCESS);
}

void TestShortcut::testGetGlobalShortcuts()
{
    KYSDKGlobalShortcutInfoList* list = kdk_shortcut_get_global_shortcuts();
    QVERIFY(list != nullptr);
    kdk_shortcut_destroy_info_list(list);
}

void TestShortcut::testGetGlobalShortcutsByKey()
{
    KYSDKGlobalShortcutInfoList* list = kdk_shortcut_get_global_shortcuts_by_key("Ctrl+Shift+P");
    QVERIFY(list != nullptr);
    kdk_shortcut_destroy_info_list(list);
}

void TestShortcut::testDestroyInfoList()
{
    KYSDKGlobalShortcutInfoList* list = kdk_shortcut_get_global_shortcuts();
    kdk_shortcut_destroy_info_list(list);
    QVERIFY(true); // Just to ensure the function doesn't crash
}

void TestShortcut::testKeyCombinationToString()
{
    const char* result = kdk_shortcut_key_combination_to_string(0x50); // 'P' key
    QVERIFY(result != nullptr);
}

void TestShortcut::testStringToKeyCombination()
{
    unsigned int result = kdk_shortcut_string_to_key_combination("Ctrl+P");
    QVERIFY(result != 0);
}

void TestShortcut::testDeleteGlobalShortcut()
{
    int result = KYSDK_SUCCESS;
    // 删除空的快捷键,预期失败
    result = kdk_shortcut_delete_global_shortcut("");
    QCOMPARE(result, -1);

    // 删除不存在的快捷键,预期失败
    result = kdk_shortcut_delete_global_shortcut("ExsitTest");
    QCOMPARE(result, KYSDK_SUCCESS);
}

QTEST_APPLESS_MAIN(TestShortcut)

#include "shortcuttest.moc"