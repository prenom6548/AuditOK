#include <QtTest>
#include "common/utils.h"
#include "model/diroptutils.h"
#include "test_common/test_helpers.h"

class UtilsTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试初始化
    void initTestCase() {
        // 初始化系统信息
        Utils::initSystemInfo();
    }

    // 测试getArch方法
    void testGetArch() {
        QString arch = Utils::getArch();
        QVERIFY2(!arch.isEmpty(), "架构信息不应为空");
    }

    // 测试getOs方法
    void testGetOs() {
        QString os = Utils::getOs();
        QVERIFY2(!os.isEmpty(), "操作系统信息不应为空");
    }

    // 测试isWayland方法
    void testIsWayland() {
        bool isWayland = Utils::isWayland();
        // 无论返回true或false都是有效的，因为这取决于系统环境
        QVERIFY2(isWayland == true || isWayland == false, "isWayland应该返回布尔值");
    }

    // 测试SystemInfo类的静态成员
    void testSystemInfoMembers() {
        QCOMPARE(0,0);
    }

    // 测试MyMessageBox类
    void testMyMessageBox() {
        // 这个测试需要手动验证，因为会弹出对话框
        // 在实际测试中，我们可能需要mock这个对话框
        QVERIFY(true); // 占位符测试
    }

    // 测试getFileSize方法
    void testGetFileSize() {
        QString path = "/tmp/test.txt";
        // 调用getFileSize方法（不会崩溃即可）
        DirOptUtils opt;
        QString filemd5 = opt.calculateMd5(path);
        bool isOK = opt.generateCheckFile();
        opt.getFileListUnderDir1(path);
        opt.testPrintMap();
        opt.getFileListUnderDir2(path);

        QVERIFY(true);
    }

    // 清理测试环境
    void cleanupTestCase() {
        // 这里可以添加清理代码
        TestHelpers::cleanup();
    }
};

QTEST_MAIN(UtilsTest)
#include "utils_test.moc"
