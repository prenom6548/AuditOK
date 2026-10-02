// tst_editpage.h
#ifndef TST_EDITPAGE_H
#define TST_EDITPAGE_H

#include <QObject>
#include <QtTest>
#include "editPage.h"
#include "widget.h"

class TestEditPage : public QObject
{
    Q_OBJECT

public:
    explicit TestEditPage(QObject *parent = nullptr) : QObject(parent) {}

private slots:
    void init();
    void cleanup();

    // 基本功能测试
    void testInitialization();
    void testTextEdit();
    void testFontOperations();
    void testListOperations();
    void testColorOperations();
    
    // 文本格式测试
    void testBold();
    void testItalic();
    void testUnderline();
    void testStrikeOut();

    // 列表功能测试
    void testOrderedList();
    void testUnorderedList();

    // 颜色功能测试
    void testFontColor();
    void testBackgroundColor();

    // 窗口状态测试
    void testFullScreen();
    void testKeepAbove();

    // 图片功能测试
    void testDropImage();
    void testInsertImage();

private:
    Widget* mainWidget;
    EditPage* editPage;
    const int TEST_NOTE_ID = 1;
};

#endif // TST_EDITPAGE_H
