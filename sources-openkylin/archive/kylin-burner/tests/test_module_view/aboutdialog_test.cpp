#include <QtTest>
#include "src/view/aboutdialog.h"

class AboutDialogTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    void testAboutDialogCreation();
    void testAboutDialogShow();
};

void AboutDialogTest::initTestCase()
{
    // 在所有测试开始前执行
}

void AboutDialogTest::cleanupTestCase()
{
    // 在所有测试结束后执行
}

void AboutDialogTest::init()
{
    // 在每个测试函数执行前执行
}

void AboutDialogTest::cleanup()
{
    // 在每个测试函数执行后执行
}

void AboutDialogTest::testAboutDialogCreation()
{
    AboutDialog *dialog = new AboutDialog();
    QVERIFY(dialog != nullptr);
    delete dialog;
}

void AboutDialogTest::testAboutDialogShow()
{
    AboutDialog *dialog = new AboutDialog();
    QVERIFY(dialog != nullptr);
    
    // 测试显示功能
    dialog->show();
    QVERIFY(dialog->isVisible() == true);
    
    // 测试隐藏功能
    dialog->hide();
    QVERIFY(dialog->isVisible() == false);
    
    delete dialog;
}

QTEST_MAIN(AboutDialogTest)

#include "aboutdialog_test.moc"
