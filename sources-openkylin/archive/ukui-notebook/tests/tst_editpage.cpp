// tst_editpage.cpp
#include "tst_editpage.h"
#include <QSignalSpy>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QImage>

void TestEditPage::init()
{
    if (!qApp) {
        QSKIP("This test requires a GUI environment");
    }
    mainWidget = new Widget;
    editPage = new EditPage(mainWidget, TEST_NOTE_ID);
}

void TestEditPage::cleanup()
{
    delete editPage;
    delete mainWidget;
}

void TestEditPage::testInitialization()
{
    QVERIFY(editPage->m_noteHeadMenu != nullptr);
    QVERIFY(editPage->m_noteHead != nullptr);
    QVERIFY(editPage->ui->textEdit != nullptr);
    QCOMPARE(editPage->m_noteId, TEST_NOTE_ID);
}

void TestEditPage::testTextEdit()
{
    // 测试文本编辑功能
    QString testText = "Test Content";
    QSignalSpy textChangedSpy(editPage, SIGNAL(texthasChanged(int, int)));
    
    editPage->ui->textEdit->setText(testText);
    QCOMPARE(editPage->ui->textEdit->toPlainText(), testText);
    QCOMPARE(textChangedSpy.count(), 1);
}

void TestEditPage::testBold()
{
    // 测试加粗功能
    editPage->ui->textEdit->setText("Test");
    editPage->ui->textEdit->selectAll();
    
    editPage->setBoldSlot();
    QTextCursor cursor = editPage->ui->textEdit->textCursor();
    QCOMPARE(cursor.charFormat().fontWeight(), QFont::Bold);
    
    // 再次点击取消加粗
    editPage->setBoldSlot();
    QCOMPARE(cursor.charFormat().fontWeight(), QFont::Normal);
}

void TestEditPage::testItalic()
{
    editPage->ui->textEdit->setText("Test");
    editPage->ui->textEdit->selectAll();
    
    editPage->setItalicSlot();
    QTextCursor cursor = editPage->ui->textEdit->textCursor();
    QVERIFY(cursor.charFormat().fontItalic());
    
    editPage->setItalicSlot();
    QVERIFY(!cursor.charFormat().fontItalic());
}

void TestEditPage::testUnderline()
{
    editPage->ui->textEdit->setText("Test");
    editPage->ui->textEdit->selectAll();
    
    editPage->setUnderlineSlot();
    QTextCursor cursor = editPage->ui->textEdit->textCursor();
    QVERIFY(cursor.charFormat().fontUnderline());
    
    editPage->setUnderlineSlot();
    QVERIFY(!cursor.charFormat().fontUnderline());
}

void TestEditPage::testOrderedList()
{
    editPage->ui->textEdit->setText("Item 1\nItem 2\nItem 3");
    editPage->ui->textEdit->selectAll();
    
    editPage->setOrderedListSlot(true);
    QTextCursor cursor = editPage->ui->textEdit->textCursor();
    QVERIFY(cursor.currentList() != nullptr);
    QCOMPARE(cursor.currentList()->format().style(), QTextListFormat::ListDecimal);
}

void TestEditPage::testUnorderedList()
{
    editPage->ui->textEdit->setText("Item 1\nItem 2\nItem 3");
    editPage->ui->textEdit->selectAll();
    
    editPage->setUnorderedListSlot(true);
    QTextCursor cursor = editPage->ui->textEdit->textCursor();
    QVERIFY(cursor.currentList() != nullptr);
    QCOMPARE(cursor.currentList()->format().style(), QTextListFormat::ListDisc);
}

void TestEditPage::testFontColor()
{
    editPage->ui->textEdit->setText("Test");
    editPage->ui->textEdit->selectAll();
    
    QColor testColor = Qt::red;
    editPage->setFontColorSlot(testColor);
    
    QTextCursor cursor = editPage->ui->textEdit->textCursor();
    QCOMPARE(cursor.charFormat().foreground().color(), testColor);
}

void TestEditPage::testFullScreen()
{
    QVERIFY(!editPage->m_isFullscreen);
    editPage->showFullScreenSlot();
    QVERIFY(editPage->m_isFullscreen);
    QVERIFY(editPage->isFullScreen());
    
    editPage->showFullScreenSlot();
    QVERIFY(!editPage->m_isFullscreen);
    QVERIFY(!editPage->isFullScreen());
}

void TestEditPage::testDropImage()
{
    QImage testImage(100, 100, QImage::Format_RGB32);
    testImage.fill(Qt::red);
    
    editPage->dropImage(testImage, "png");
    
    // 验证图片是否被插入
    QTextDocument* doc = editPage->ui->textEdit->document();
    QVERIFY(doc->isEmpty() == false);
}

void TestEditPage::testFontOperations()
{
    // 测试字体相关操作
    editPage->ui->textEdit->setText("Test Font");
    editPage->ui->textEdit->selectAll();

    // 测试字号
    editPage->setFontSizeSlot();

    // 获取当前字体
    QFont currentFont = editPage->ui->textEdit->currentFont();
    QVERIFY(currentFont.pointSize() > 0);
}

void TestEditPage::testListOperations()
{
    QString testText = "Item 1\nItem 2\nItem 3";
    editPage->ui->textEdit->setText(testText);
    editPage->ui->textEdit->selectAll();

    // 测试有序列表
    editPage->setOrderedListSlot(true);
    QTextCursor cursor = editPage->ui->textEdit->textCursor();
    QVERIFY(cursor.currentList() != nullptr);
    QCOMPARE(cursor.currentList()->format().style(), QTextListFormat::ListDecimal);

    // 测试无序列表
    editPage->setUnorderedListSlot(true);
    QCOMPARE(cursor.currentList()->format().style(), QTextListFormat::ListDisc);
}

void TestEditPage::testColorOperations()
{
    editPage->ui->textEdit->setText("Test Color");
    editPage->ui->textEdit->selectAll();

    // 测试文字颜色
    QColor testColor = Qt::red;
    editPage->setFontColorSlot(testColor);

    QTextCursor cursor = editPage->ui->textEdit->textCursor();
    QCOMPARE(cursor.charFormat().foreground().color(), testColor);

    // 测试默认颜色
    editPage->defaultTextColorSlot();
}

void TestEditPage::testStrikeOut()
{
    editPage->ui->textEdit->setText("Test Strike Out");
    editPage->ui->textEdit->selectAll();

    editPage->setStrikeOutSlot();
    QTextCursor cursor = editPage->ui->textEdit->textCursor();
    QVERIFY(cursor.charFormat().fontStrikeOut());

    // 测试取消删除线
    editPage->setStrikeOutSlot();
    QVERIFY(!cursor.charFormat().fontStrikeOut());
}

void TestEditPage::testBackgroundColor()
{
    editPage->ui->textEdit->setText("Test Background");
    editPage->ui->textEdit->selectAll();

    // 测试背景色
    QColor testColor = editPage->m_editColor;
    editPage->handleBtnSlot(testColor);

    // 验证颜色变化信号
    QSignalSpy colorChangedSpy(editPage, SIGNAL(colorhasChanged(QColor,int)));
    QCOMPARE(colorChangedSpy.count(), 1);
}

void TestEditPage::testKeepAbove()
{
//    // 测试窗口置顶
//    editPage->setWindowKeepAbove();
//    QTest::qWait(100);  // 等待窗口状态更新

//    // 测试取消置顶
//    editPage->setWindowStatusClear();
//    QTest::qWait(100);
}

void TestEditPage::testInsertImage()
{
    // 创建测试图片
    QImage testImage(100, 100, QImage::Format_RGB32);
    testImage.fill(Qt::blue);

    // 测试图片插入
    editPage->dropImage(testImage, "png");

    // 验证文档非空
    QVERIFY(!editPage->ui->textEdit->document()->isEmpty());
}

#include "moc_tst_editpage.cpp"
