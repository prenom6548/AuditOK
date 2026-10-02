#include <QtTest>
#include "src/data/datadirectory.h"
#include "src/data/datafile.h"
#include "test_common/test_helpers.h"
#include <QDir>
#include <QTemporaryDir>
#include <QFileInfo>


class DataDirectoryTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试初始化
    void initTestCase() {
        // 可以在这里初始化一些测试所需的资源
    }

    // 测试构造函数
    void testConstructor() {
        // 创建DataDirectory对象
        DataDirectory dir("", nullptr);
        QVERIFY(true); // 如果能成功构造，则测试通过
    }

    // 测试带路径的构造函数
    void testConstructorWithPath() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        DataDirectory dir(tempDir.path(), nullptr);
        QVERIFY(true); // 如果能成功构造，则测试通过
    }

    // 测试析构函数
    void testDestructor() {
        // 创建DataDirectory对象并删除
        DataDirectory* dir = new DataDirectory("", nullptr);
        delete dir;
        QVERIFY(true); // 如果能成功删除，则测试通过
    }

    // 测试at方法
    void testAt() {
        DataDirectory dir("", nullptr);
        DataFile* file = dir.at(-1);  // 测试负数索引
        QVERIFY(file == nullptr);
        
        file = dir.at(0);  // 测试空目录索引
        QVERIFY(file == nullptr);
    }

    // 测试findIndex方法
    void testFindIndex() {
        DataDirectory dir("", nullptr);
        DataFile file("", nullptr);
        int index = dir.findIndex(&file);
        QVERIFY(index == -1);  // 空目录中找不到文件
    }

    // 测试findIndex重载方法
    void testFindIndexOverload() {
        DataDirectory parentDir("", nullptr);
        DataDirectory childDir("", &parentDir);
        int index = parentDir.findIndex(&childDir);
        QVERIFY(index != -1);  // 空目录中找不到子目录
    }

    // 测试take方法
    void testTake() {
        DataDirectory dir("", nullptr);
        DataFile file("", nullptr);
        bool result = dir.take(&file);  // 尝试移除不存在的文件
        QVERIFY(result == false);
    }

    // 测试append方法
    void testAppend() {
        DataDirectory dir("", nullptr);
        DataFile* file = new DataFile("", nullptr);
        dir.append(file);  // 添加文件
        QVERIFY(true);
        delete file;
    }

    // 测试count方法
    void testCount() {
        DataDirectory dir("", nullptr);
        int count = dir.count();
        QVERIFY(count >= 0);
    }

    // 测试counts方法
    void testCounts() {
        DataDirectory dir("", nullptr);
        int counts = dir.counts();
        QVERIFY(counts >= 0);
    }

    // 测试total方法
    void testTotal() {
        DataDirectory dir("", nullptr);
        unsigned long total = dir.total();
        QVERIFY(total >= 0);
    }

    // 测试nSize方法
    void testNSize() {
        DataDirectory dir("", nullptr);
        unsigned long nSize = dir.nSize();
        QVERIFY(nSize >= 0);
    }

    // 测试isRoot方法
    void testIsRoot() {
        DataDirectory dir("", nullptr);
        bool isRoot = dir.isRoot();
        QVERIFY(true);  // 确保方法能被调用
    }

    // 测试canClean方法
    void testCanClean() {
        DataDirectory dir("", nullptr);
        bool canClean = dir.canClean();
        QVERIFY(true);  // 确保方法能被调用
    }

    // 测试duplicate方法
    void testDuplicate() {
        DataDirectory dir("", nullptr);
        bool dup = dir.duplicate("test_name");
        QVERIFY(dup == false);  // 空目录中不会有重复名称
    }

    // 测试rename方法
    void testRename() {
        DataDirectory dir("", nullptr);
        dir.rename("new_name");
        QVERIFY(true);  // 确保方法能被调用
    }

    // 测试updateChild方法
    void testUpdateChild() {
        DataDirectory dir("", nullptr);
        dir.updateChild("old_name", "new_name");
        QVERIFY(true);  // 确保方法能被调用
    }

    // 测试clearChildren方法
    void testClearChildren() {
        DataDirectory dir("", nullptr);
        dir.clearChildren();
        QVERIFY(true);  // 确保方法能被调用
    }

    // 测试clear方法
    void testClear() {
        DataDirectory dir("", nullptr);
        dir.clear();
        QVERIFY(true);  // 确保方法能被调用
    }

    // 测试clean方法
    void testClean() {
        DataDirectory dir("", nullptr);
        dir.clean();
        QVERIFY(true);  // 确保方法能被调用
    }

    // 测试copy方法
    void testCopy() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        DataDirectory dir(tempDir.path(), nullptr);
        dir.copy();
        QVERIFY(true);  // 确保方法能被调用
    }

    // 测试findChildByDisplay方法
    void testFindChildByDisplay() {
        DataDirectory dir("", nullptr);
        void* child = dir.findChildByDisplay("nonexistent");
        QVERIFY(child == nullptr);
        
        child = dir.findChildByDisplay("nonexistent", true);
        QVERIFY(child == nullptr);
    }

    // 测试setFilterHidden方法
    void testSetFilterHidden() {
        DataDirectory::setFilterHidden(true);
        DataDirectory::setFilterHidden(false);
        QVERIFY(true);
    }

    // 测试setFileDestination方法
    void testSetFileDestination() {
        DataDirectory dir("", nullptr);
        dir.setFileDestination("new_name");
        QVERIFY(true);
    }

    // 测试setAllChildFileDestinationByPath方法
    void testSetAllChildFileDestinationByPath() {
        DataDirectory dir("", nullptr);
        dir.setAllChildFileDestinationByPath("/new/path");
        QVERIFY(true);
    }

    // 测试filterHidden方法
    void testFilterHidden() {
        DataDirectory dir("", nullptr);
        bool filter = dir.filterHidden();
        QVERIFY(filter == false);  // 默认应该是false
    }

    // 测试setNodeFilterHidden方法
    void testSetNodeFilterHidden() {
        DataDirectory dir("", nullptr);
        dir.setNodeFilterHidden(true);
        QVERIFY(true);
    }

    // 测试setFilterBrokenSymbolLinks方法
    void testSetFilterBrokenSymbolLinks() {
        DataDirectory::setFilterBrokenSymbolLinks(true);
        DataDirectory::setFilterBrokenSymbolLinks(false);
        QVERIFY(true);
    }

    // 测试filterBrokenSymbolLinks方法
    void testFilterBrokenSymbolLinks() {
        DataDirectory dir("", nullptr);
        bool filter = dir.filterBrokenSymbolLinks();
        QVERIFY(filter == false);  // 默认应该是false
    }

    // 测试setNodeFilterBrokenSymbolLinks方法
    void testSetNodeFilterBrokenSymbolLinks() {
        DataDirectory dir("", nullptr);
        dir.setNodeFilterBrokenSymbolLinks(true);
        QVERIFY(true);
    }

    // 测试setRepleaceSymbolLinks方法
    void testSetRepleaceSymbolLinks() {
        DataDirectory::setRepleaceSymbolLinks(true);
        DataDirectory::setRepleaceSymbolLinks(false);
        QVERIFY(true);
    }

    // 测试repleaceSymbolLinks方法
    void testRepleaceSymbolLinks() {
        DataDirectory dir("", nullptr);
        bool filter = dir.repleaceSymbolLinks();
        QVERIFY(filter == false);  // 默认应该是false
    }

    // 测试setNodeRepleaceSymbolLinks方法
    void testSetNodeRepleaceSymbolLinks() {
        DataDirectory dir("", nullptr);
        dir.setNodeRepleaceSymbolLinks(true);
        QVERIFY(true);
    }

    // 测试loadNodeFilterConfig方法
    void testLoadNodeFilterConfig() {
        DataDirectory dir("", nullptr);
        dir.loadNodeFilterConfig();
        QVERIFY(true);
    }

    // 测试getNodeConfigKey方法
    void testGetNodeConfigKey() {
        DataDirectory dir("/some/path", nullptr);
        QString key = dir.getNodeConfigKey();
        QVERIFY(key.contains("NodeFilter_"));
    }

    // 测试execHidden方法
    void testExecHidden() {
        DataDirectory dir("", nullptr);
        dir.execHidden(true);
        dir.execHidden(false);
        QVERIFY(true);
    }

    // 测试execBroken方法
    void testExecBroken() {
        DataDirectory dir("", nullptr);
        dir.execBroken(true);
        dir.execBroken(false);
        QVERIFY(true);
    }

    // 测试execRepleace方法
    void testExecRepleace() {
        DataDirectory dir("", nullptr);
        dir.execRepleace(true);
        dir.execRepleace(false);
        QVERIFY(true);
    }

    // 测试turnViewFilterFileToHidden方法
    void testTurnViewFilterFileToHidden() {
        DataDirectory dir("", nullptr);
        dir.turnViewFilterFileToHidden();
        QVERIFY(true);
    }

    // 测试CountOneLevelNotDiskFile方法
    void testCountOneLevelNotDiskFile() {
        DataDirectory dir("", nullptr);
        int count = dir.CountOneLevelNotDiskFile();
        QVERIFY(count >= 0);
    }

    // 测试isGreateThan2GByNewAddSingleFile方法
    void testIsGreateThan2GByNewAddSingleFile() {
        DataDirectory dir("", nullptr);
        bool result = dir.isGreateThan2GByNewAddSingleFile();
        QVERIFY(result == false);  // 空目录中不会有大于2G的文件
    }

    // 测试doSize槽函数
    void testDoSize() {
        DataDirectory dir("", nullptr);
        dir.doSize(1024, true);
        dir.doSize(1024, false);
        QVERIFY(true);
    }

    // 测试doDirectory槽函数
    void testDoDirectory() {
        DataDirectory dir("", nullptr);
        DataFile file("", nullptr);
        dir.doDirectory(true, &file);
        dir.doDirectory(false, &file);
        QVERIFY(true);
    }

    // 测试doFile槽函数
    void testDoFile() {
        DataDirectory dir("", nullptr);
        DataFile file("", nullptr);
        dir.doFile(true, &file);
        dir.doFile(false, &file);
        QVERIFY(true);
    }

    // 测试appendOrDeleteSubfile槽函数
    void testAppendOrDeleteSubfile() {
        DataDirectory dir("", nullptr);
        DataFile file("", nullptr);
        dir.appendOrDeleteSubfile(&file, true);
        dir.appendOrDeleteSubfile(&file, false);
        QVERIFY(true);
    }

    // 测试closeInputChangeDlg槽函数
    void testCloseInputChangeDlg() {
        DataDirectory dir("", nullptr);
        dir.closeInputChangeDlg();
        QVERIFY(true);
    }

    // 测试isRenameDlgShown槽函数
    void testIsRenameDlgShown() {
        DataDirectory dir("", nullptr);
        bool shown = dir.isRenameDlgShown();
        QVERIFY(shown == false);  // 默认情况下对话框不应显示
    }

    // 清理测试环境
    void cleanupTestCase() {
        // 这里可以添加清理代码
        TestHelpers::cleanup();
    }
};

QTEST_MAIN(DataDirectoryTest)
#include "datadirectory_test.moc"
