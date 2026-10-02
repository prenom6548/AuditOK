#ifndef TST_WIDGET_H
#define TST_WIDGET_H

#include <QtTest>
#include <QObject>
#include <QApplication>

#include "widget.h"
#include "noteModel.h"
#include "editPage.h"
#include "dbManager.h"

class TestWidget : public QObject {
    Q_OBJECT

private:
    Widget *m_widget;
    NoteModel *m_noteModel;
    QSortFilterProxyModel *m_proxyModel;

private slots:
    // 初始化和清理测试环境
    void init();
    void cleanup();

    // 测试构造函数和基本初始化
    void testConstructor();
    void testInitialization();

    // 测试笔记创建相关方法
    void testCreateNewNote();
    void testGenerateNote();
    void testCreateNewNoteIfEmpty();

    // 测试笔记删除相关方法
    void testDeleteSelectedNote();
    void testClearNoteSlot();

    // 测试搜索功能
    void testSearchFunctionality();
    void testOnSearchEditTextChanged();

    // 测试视图模式切换
    void testChangePageSlot();
    void testSortSlot();

    // 测试编辑器相关方法
    void testListDoubleClickSlot();
    void testOpenMemoWithId();

    // 测试其他功能
    void testExitSlot();
    void testKeyPressEvents();
    void testContextMenu();
};

#endif // TST_WIDGET_H
