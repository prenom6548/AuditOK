#include <QCoreApplication>
#include "../libkyshortcut/libkyshortcut.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    int ret = KYSDK_SUCCESS;
    // 1. 快捷键名称空,预期成功
    ret = kdk_shortcut_create_global_shortcut("", "Ctrl+Alt+M", "/usr/share/applications/atril.desktop");
    printf("Creat-1: 快捷键名称空,期望失败测试%s,返回值%d\n", ret ? "失败" : "成功", ret);

    // 删除空的快捷键
    ret = kdk_shortcut_delete_global_shortcut("");
    printf("Delete-1: 删除空的快捷键,期望失败测试%s,返回值%d\n", ret ? "失败" : "成功", ret);

    // 2. 快捷键名称不存在 & 未添加过的快捷键 & 正常终端执行,预期成功
    ret = kdk_shortcut_create_global_shortcut("ExsitTest", "Ctrl+Alt+Q", "/usr/share/applications/atril.desktop");
    printf("Creat-2: 快捷键名称不存在,期望成功测试%s,返回值%d\n", ret ? "失败" : "成功", ret);

    // 3. 快捷键名称已存在,预期失败
    ret = kdk_shortcut_create_global_shortcut("ExsitTest", "Ctrl+Alt+N", "/usr/share/applications/atril.desktop");
    printf("Creat-3: 快捷键名称已存在,期望失败测试%s,返回值%d\n", ret ? "失败" : "成功", ret);

    // 4. 错误的键,预期失败
    ret = kdk_shortcut_create_global_shortcut("ExsitTest", "Ctrdl+Alt+N", "/usr/share/applications/atril.desktop");
    printf("Creat-4: 错误的快捷键,期望失败测试%s,返回值%d\n", ret ? "失败" : "成功", ret);

    // 5. 快捷键空,预期失败
    ret = kdk_shortcut_create_global_shortcut("KeyTest1", "", "/usr/share/applications/atril.desktop");
    printf("Creat-5: 快捷键为空,期望失败测试%s,返回值%d\n", ret ? "失败" : "成功", ret);

    // 6. 快捷键单键-字母,预期失败
    ret = kdk_shortcut_create_global_shortcut("KeyTest2", "K", "/usr/share/applications/atril.desktop");
    printf("Creat-6: 快捷键单键,期望失败测试%s,返回值%d\n", ret ? "失败" : "成功", ret);

    // 7. 快捷键多键-2个键,预期成功
    ret = kdk_shortcut_create_global_shortcut("KeyTest4", "Ctrl+J", "/usr/share/applications/atril.desktop");
    printf("Creat-7: 快捷键2个键,期望成功测试%s,返回值%d\n", ret ? "失败" : "成功", ret);

    // 删除存在的快捷键
    ret = kdk_shortcut_delete_global_shortcut("KeyTest4");
    printf("Delete-2: 删除空的快捷键,期望成功测试%s,返回值%d\n", ret ? "失败" : "成功", ret);

    // 8. 快捷键多键-3个键,预期成功
    ret = kdk_shortcut_create_global_shortcut("KeyTest5", "Ctrl+Shift+G", "/usr/share/applications/atril.desktop");
    printf("Creat-8: 快捷键3个键,期望成功测试%s,返回值%d\n", ret ? "失败" : "成功", ret);

    // 9. 快捷键多键-4个键
    ret = kdk_shortcut_create_global_shortcut("KeyTest6", "Ctrl+Shift+Alt+G", "/usr/share/applications/atril.desktop");
    printf("Creat-9: 快捷键4个键,期望失败测试%s,返回值%d\n", ret ? "失败" : "成功", ret);

    // 10. 快捷键多键-5个键
    ret = kdk_shortcut_create_global_shortcut("KeyTest7", "Ctrl+Shift+Alt+Meta+G", "/usr/share/applications/atril.desktop");
    printf("Creat-10: 快捷键5个键,期望失败测试%s,返回值%d\n", ret ? "失败" : "成功", ret);

    ret = kdk_shortcut_global_shortcut_is_available("Ctrl+Shift+G");
    printf("Check-1: 检测已存在快捷键是否可用,期望不可用测试%s\n", ret ? "不可用" : "可用");

    // 11. 快捷键多键-已添加过的快捷键-删除原有 or 修改当前,预期不能添加
    ret = kdk_shortcut_create_global_shortcut("KeyTest8", "Ctrl+Shift+G", "/usr/share/applications/atril.desktop");
    printf("Creat-11: 已添加过的快捷键,期望失败测试%s,返回值%d\n", ret ? "失败" : "成功", ret);

    // 删除存在快捷键
    ret = kdk_shortcut_delete_global_shortcut("KeyTest5");
    printf("Delete-3: 删除快捷键,期望成功测试%s,返回值%d\n", ret ? "失败" : "成功", ret);

    // 删除空的快捷键
    ret = kdk_shortcut_delete_global_shortcut("KeyTest8");
    printf("Delete-4: 删除快捷键,期望失败测试%s,返回值%d\n", ret ? "失败" : "成功", ret);

    // 12. 快捷键执行-空,预期失败
    ret = kdk_shortcut_create_global_shortcut("KeyDesktop1", "Ctrl+M", "");
    printf("Creat-12: 空执行,期望失败测试%s,返回值%d\n", ret ? "失败" : "成功", ret);

    // 13. 快捷键执行-不存在的desktop,预期失败
    ret = kdk_shortcut_create_global_shortcut("KeyDesktop2", "Ctrl+M", "/usr/share/applications/atril1.desktop");
    printf("Creat-13: 不存在的执行,期望失败测试%s,返回值%d\n", ret ? "失败" : "成功", ret);

    KYSDKGlobalShortcutInfoList *list = kdk_shortcut_get_global_shortcuts();
    KYSDKGlobalShortcutInfoList *tmp = list;
    while (tmp)
    {
        printf("%s\n", tmp->data->name);
        printf("\t%s\n", tmp->data->compenteName);
        printf("\t%s\n", kdk_shortcut_key_combination_to_string(tmp->data->keys[0]));
        tmp = tmp->next;
    }
    kdk_shortcut_destroy_info_list(list);

    list = kdk_shortcut_get_global_shortcuts_by_key("Ctrl+Alt+N");
    tmp = list;
    printf("Search by key: Ctrl+Alt+Q\n");
    while (tmp)
    {
        printf("\t%s\n", tmp->data->name);
        printf("\t%s\n", tmp->data->compenteName);
        printf("\t%s\n", kdk_shortcut_key_combination_to_string(tmp->data->keys[0]));
        tmp = tmp->next;
    }
    kdk_shortcut_destroy_info_list(list);

    // 删除ExsitTest快捷键
    ret = kdk_shortcut_delete_global_shortcut("ExsitTest");
    printf("Delete-5: 删除快捷键,期望成功测试%s,返回值%d\n", ret ? "失败" : "成功", ret);

    unsigned int combination = kdk_shortcut_string_to_key_combination("Ctrl+Alt+Q");
    printf("Ctrl+Alt+Q : %u\n", combination);

    // 值转为字符串， 期望为"Ctrl+Alt+P"
    const char *str =  kdk_shortcut_key_combination_to_string(KDK_CTRL + KDK_ALT +KDK_Key_D);
    printf("%s\n", str);

    return a.exec();
}
