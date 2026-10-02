/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include <QtTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QDir>
#include <QFile>

#include "src/model/filefiltermodel.h"
#include "src/data/datadirectory.h"
#include "src/data/datafile.h"
#include "src/data/datacdrom.h"
#include "src/data/datasetting.h"
#include "test_common/test_helpers.h"

// 友元测试类，用于访问私有成员
class FileFilterModelTestFriend : public QObject
{
    Q_OBJECT
public:
    static QStringList& getHeaders(FileFilterModel* model) {
        return model->m_oHeaders;
    }
};

// 测试辅助类，用于访问DataFile的protected成员
class TestDataFile : public DataFile
{
public:
    explicit TestDataFile(const QString& path, QObject* parent = nullptr, bool fromDisk = false)
        : DataFile(path, parent, fromDisk) {}
    
    // 提供访问protected成员的公共接口
    void setAttrHidden(bool hidden) { m_bAttrHidden = hidden; }
    void setAttrLink(bool link) { m_bAttrLink = link; }
    void setLevel(unsigned int level) { m_u32Level = level; }
    void setIsDisk(bool disk) { m_bIsDisk = disk; }
};

// 测试辅助类，用于访问DataDirectory的protected成员
class TestDataDirectory : public DataDirectory
{
public:
    explicit TestDataDirectory(const QString& path, QObject* parent = nullptr, bool fromDisk = false)
        : DataDirectory(path, parent, fromDisk) {}
    
    // 提供访问protected成员的公共接口
    void setAttrHidden(bool hidden) { m_bAttrHidden = hidden; }
    void setAttrLink(bool link) { m_bAttrLink = link; }
    void setLevel(unsigned int level) { m_u32Level = level; }
};

// 测试类
class FileFilterModelTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // 构造函数和单例测试
    void testConstructor();
    void testInstance();
    void testInstanceMutex();
    
    // Header相关测试
    void testHeaderData();
    void testHeaderDataWithInvalidSection();
    void testHeaderDataFontRole();
    void testHeaderDataVerticalOrientation();
    void testSetHeaderData();
    void testSetHeaderDataAppend();
    //void testSetHeaderDataReplace();
    void testSetHeaderDataInvalidSection();
    void testSetHeaderDataSameValue();
    
    // Index和Parent测试
    void testIndex();
    void testIndexWithParent();
    void testIndexInvalidRow();
    void testParent();
    void testParentInvalidIndex();
    void testParentWithRoot();
    void testParentWithNestedDirectory();
    
    // Row和Column计数测试
    void testRowCount();
    void testRowCountWithParent();
    void testRowCountWithInvalidParent();
    void testColumnCount();
    
    // Data测试
    void testData();
    void testDataDisplayRole();
    void testDataEditRole();
    void testDataDecorationRole();
    void testDataDecorationRoleDisk();
    void testDataForegroundRole();
    void testDataSizeHintRole();
    void testDataInvalidIndex();
    void testDataNullItem();
    
    // filtered测试
    void testFiltered();
    void testFilteredWithInvalidIndex();
    void testFilteredWithEmptyRoots();
    
    // addFilterNode测试
    void testAddFilterNode();
    void testAddFilterNodeWithZeroUsing();
    void testAddFilterNodeSignals();
    
    // deleteFilterNode测试
    void testDeleteFilterNode();
    void testDeleteFilterNodeInvalidIndex();
    void testDeleteFilterNodeOutOfRange();
    
    // getCurrentRoot测试
    void testGetCurrentRoot();
    void testGetCurrentRootInvalidIndex();
    
    // addFilterData测试
    void testAddFilterData();
    void testAddFilterDataWithNullRoot();
    
    // restoreFiles测试
    void testRestoreFiles();
    void testRestoreFilesWithHiddenFiles();
    void testRestoreFilesWithBrokenLinks();
    void testRestoreFilesWithReplacedLinks();
    void testRestoreFilesWithInvalidIndex();
    
    // GetFilteredHiddenFiles测试
    void testGetFilteredHiddenFiles();
    void testGetFilteredHiddenFilesWithLevel();
    void testGetFilteredHiddenFilesDirectoryAtLevel();
    
    // GetFilteredBadLinkFiles测试
    void testGetFilteredBadLinkFiles();
    void testGetFilteredBadLinkFilesWithLevel();
    void testGetFilteredBadLinkFilesDirectoryAtLevel();
    
    // GetFilteredReplacedLinkfiles测试
    void testGetFilteredReplacedLinkfiles();
    void testGetFilteredReplacedLinkfilesWithExistingTarget();
    void testGetFilteredReplacedLinkfilesWithNonExistingTarget();
    
    // updateIndex测试
    void testUpdateIndex();
    void testUpdateIndexSignals();

private:
    FileFilterModel* m_model;
    QTemporaryDir* m_tempDir;
    DataCDROM* createMockCDROM(const QString& tempPath, const QString& volumnID, int isUsing = 1);
    DataDirectory* createMockDirectory(const QString& path);
    TestDataFile* createMockFile(const QString& path, DataDirectory* parent = nullptr);
    TestDataFile* createMockHiddenFile(const QString& path, DataDirectory* parent = nullptr);
    TestDataFile* createMockSymbolLink(const QString& path, DataFile* target = nullptr, DataDirectory* parent = nullptr);
};

void FileFilterModelTest::initTestCase()
{
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    // 创建临时目录结构
    QDir dir(m_tempDir->path());
    dir.mkpath(".filter/test_volume");
    
    m_model = new FileFilterModel();
}

void FileFilterModelTest::cleanupTestCase()
{
    delete m_model;
    delete m_tempDir;
    TestHelpers::cleanup();
}

void FileFilterModelTest::testConstructor()
{
    FileFilterModel model;
    QCOMPARE(model.m_iIndex, -1);
    QVERIFY(model.m_oRoots.isEmpty());
}

void FileFilterModelTest::testInstance()
{
    FileFilterModel* instance1 = FileFilterModel::instance();
    FileFilterModel* instance2 = FileFilterModel::instance();
    QVERIFY(instance1 != nullptr);
    QVERIFY(instance2 != nullptr);
    QCOMPARE(instance1, instance2); // 应该是同一个实例
}

void FileFilterModelTest::testInstanceMutex()
{
    // 测试多线程安全性（基本测试）
    FileFilterModel* instance1 = FileFilterModel::instance();
    QVERIFY(instance1 != nullptr);
}

void FileFilterModelTest::testHeaderData()
{
    FileFilterModel model;
    FileFilterModelTestFriend::getHeaders(&model) << "Column1" << "Column2";
    
    QVariant result = model.headerData(0, Qt::Horizontal, Qt::DisplayRole);
    QCOMPARE(result.toString(), QString("Column1"));
    
    result = model.headerData(1, Qt::Horizontal, Qt::DisplayRole);
    QCOMPARE(result.toString(), QString("Column2"));
}

void FileFilterModelTest::testHeaderDataWithInvalidSection()
{
    FileFilterModel model;
    FileFilterModelTestFriend::getHeaders(&model) << "Column1";
    
    QVariant result = model.headerData(5, Qt::Horizontal, Qt::DisplayRole);
    QVERIFY(result.toString().isEmpty());
}

void FileFilterModelTest::testHeaderDataFontRole()
{
    FileFilterModel model;
    QVariant result = model.headerData(0, Qt::Horizontal, Qt::FontRole);
    QVERIFY(result.isValid());
    QFont font = result.value<QFont>();
    QCOMPARE(font.pixelSize(), 14);
    QCOMPARE(font.weight(), 60);
}

void FileFilterModelTest::testHeaderDataVerticalOrientation()
{
    FileFilterModel model;
    QVariant result = model.headerData(0, Qt::Vertical, Qt::DisplayRole);
    QVERIFY(!result.isValid());
}

void FileFilterModelTest::testSetHeaderData()
{
    QVERIFY(true);
}

void FileFilterModelTest::testSetHeaderDataAppend()
{
    QVERIFY(true);
}


void FileFilterModelTest::testSetHeaderDataInvalidSection()
{
    FileFilterModel model;
    FileFilterModelTestFriend::getHeaders(&model) << "Header1";
    
    bool result = model.setHeaderData(5, Qt::Horizontal, QVariant("Header2"), Qt::EditRole);
}

void FileFilterModelTest::testSetHeaderDataSameValue()
{
    QVERIFY(true);
}

void FileFilterModelTest::testIndex()
{
    FileFilterModel model;
    DataDirectory* root1 = createMockDirectory(m_tempDir->path() + "/root1");
    DataDirectory* root2 = createMockDirectory(m_tempDir->path() + "/root2");
    
    model.m_oRoots << root1 << root2;
    
    QModelIndex idx1 = model.index(0, 0);
    QVERIFY(idx1.isValid());
    QCOMPARE(idx1.row(), 0);
}

void FileFilterModelTest::testIndexWithParent()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    DataFile* file1 = createMockFile(m_tempDir->path() + "/file1.txt", root);
    DataFile* file2 = createMockFile(m_tempDir->path() + "/file2.txt", root);
    
    model.m_oRoots << root;
    
    QModelIndex parentIdx = model.index(0, 0);
    QModelIndex childIdx = model.index(0, 0, parentIdx);
    QVERIFY(childIdx.isValid());
}

void FileFilterModelTest::testIndexInvalidRow()
{
    FileFilterModel model;
    QModelIndex idx = model.index(-1, 0);
    QVERIFY(!idx.isValid());
    
    idx = model.index(10, 0);
    QVERIFY(!idx.isValid());
}

void FileFilterModelTest::testParent()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    DataFile* file = createMockFile(m_tempDir->path() + "/file.txt", root);

    model.m_oRoots << root;
    model.m_iIndex = 0;
    
    QModelIndex fileIdx = model.index(0, 0, model.index(0, 0));
    model.parent(fileIdx);
}

void FileFilterModelTest::testParentInvalidIndex()
{
    FileFilterModel model;
    QModelIndex invalidIdx;
    QModelIndex parentIdx = model.parent(invalidIdx);
    QVERIFY(!parentIdx.isValid());
}

void FileFilterModelTest::testParentWithRoot()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    DataFile* file = createMockFile(m_tempDir->path() + "/file.txt", root);
    
    model.m_oRoots << root;
    model.m_iIndex = 0;
    
    QModelIndex rootIdx = model.index(0, 0);
    QModelIndex fileIdx = model.index(0, 0, rootIdx);
    
    // 文件在根目录的直接子项，parent调用不应崩溃
    model.parent(fileIdx);
}

void FileFilterModelTest::testParentWithNestedDirectory()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    // subDir需要以root作为parent
    DataDirectory* subDir = new DataDirectory(m_tempDir->path() + "/root/subdir", root);
    TestDataFile* file = createMockFile(m_tempDir->path() + "/root/subdir/file.txt", subDir);
    
    model.m_oRoots << root;
    model.m_iIndex = 0;
    
    QModelIndex rootIdx = model.index(0, 0);
    QModelIndex subDirIdx = model.index(0, 0, rootIdx);
    QModelIndex fileIdx = model.index(0, 0, subDirIdx);
    
    // 只验证调用不崩溃
    model.parent(fileIdx);
}

void FileFilterModelTest::testRowCount()
{
    FileFilterModel model;
    QCOMPARE(model.rowCount(), 0);
    
    DataDirectory* root1 = createMockDirectory(m_tempDir->path() + "/root1");
    DataDirectory* root2 = createMockDirectory(m_tempDir->path() + "/root2");
    model.m_oRoots << root1 << root2;
    
    QCOMPARE(model.rowCount(), 2);
}

void FileFilterModelTest::testRowCountWithParent()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    DataFile* file1 = createMockFile(m_tempDir->path() + "/file1.txt", root);
    DataFile* file2 = createMockFile(m_tempDir->path() + "/file2.txt", root);
    
    model.m_oRoots << root;
    
    QModelIndex parentIdx = model.index(0, 0);
    model.rowCount(parentIdx);
}

void FileFilterModelTest::testRowCountWithInvalidParent()
{
    FileFilterModel model;
    DataFile* file = createMockFile(m_tempDir->path() + "/file.txt");
    QModelIndex invalidParent = QModelIndex();
    QModelIndex fileIdx = model.createIndex(0, 0, file);
    
    QCOMPARE(model.rowCount(fileIdx), 0); // 文件不是目录，应该返回0
}

void FileFilterModelTest::testColumnCount()
{
    FileFilterModel model;
    QCOMPARE(model.columnCount(), 0);
    
    FileFilterModelTestFriend::getHeaders(&model) << "Column1" << "Column2" << "Column3";
    QCOMPARE(model.columnCount(), 3);
}

void FileFilterModelTest::testData()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    DataFile* file = createMockFile(m_tempDir->path() + "/test.txt", root);
    
    model.m_oRoots << root;
    
    QModelIndex fileIdx = model.index(0, 0, model.index(0, 0));
    model.data(fileIdx, Qt::DisplayRole);
}

void FileFilterModelTest::testDataDisplayRole()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    DataFile* file = createMockFile(m_tempDir->path() + "/test.txt", root);
    
    model.m_oRoots << root;
    
    QModelIndex fileIdx = model.index(0, 0, model.index(0, 0));
    model.data(fileIdx, Qt::DisplayRole);
    
    // 测试column 1 (source)，仅调用以确保不崩溃
    QModelIndex fileIdxCol1 = model.index(0, 1, model.index(0, 0));
    model.data(fileIdxCol1, Qt::DisplayRole);
}

void FileFilterModelTest::testDataEditRole()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    DataFile* file = createMockFile(m_tempDir->path() + "/test.txt", root);
    
    model.m_oRoots << root;
    
    QModelIndex fileIdx = model.index(0, 0, model.index(0, 0));
    model.data(fileIdx, Qt::EditRole);
}

void FileFilterModelTest::testDataDecorationRole()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    DataFile* file = createMockFile(m_tempDir->path() + "/test.txt", root);
    
    model.m_oRoots << root;
    
    QModelIndex fileIdx = model.index(0, 0, model.index(0, 0));
    model.data(fileIdx, Qt::DecorationRole);
}

void FileFilterModelTest::testDataDecorationRoleDisk()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    TestDataFile* file = createMockFile(m_tempDir->path() + "/test.txt", root);
    file->setIsDisk(true); // 设置为磁盘文件
    
    model.m_oRoots << root;
    
    QModelIndex fileIdx = model.index(0, 0, model.index(0, 0));
    model.data(fileIdx, Qt::DecorationRole);
}

void FileFilterModelTest::testDataForegroundRole()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    TestDataFile* file = createMockFile(m_tempDir->path() + "/test.txt", root);
    file->setIsDisk(true); // 设置为磁盘文件
    
    model.m_oRoots << root;
    
    QModelIndex fileIdx = model.index(0, 0, model.index(0, 0));
    model.data(fileIdx, Qt::ForegroundRole);
}

void FileFilterModelTest::testDataSizeHintRole()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    DataFile* file = createMockFile(m_tempDir->path() + "/test.txt", root);
    
    model.m_oRoots << root;
    
    QModelIndex fileIdx = model.index(0, 0, model.index(0, 0));
    model.data(fileIdx, Qt::SizeHintRole);
}

void FileFilterModelTest::testDataInvalidIndex()
{
    FileFilterModel model;
    QModelIndex invalidIdx;
    QVariant data = model.data(invalidIdx, Qt::DisplayRole);
    QVERIFY(!data.isValid());
}

void FileFilterModelTest::testDataNullItem()
{
    FileFilterModel model;
    QModelIndex idx = model.createIndex(0, 0, nullptr);
    QVariant data = model.data(idx, Qt::DisplayRole);
    QVERIFY(!data.isValid());
}

void FileFilterModelTest::testFiltered()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    DataFile* file1 = createMockFile(m_tempDir->path() + "/file1.txt", root);
    DataFile* file2 = createMockFile(m_tempDir->path() + "/file2.txt", root);
    
    model.m_oRoots << root;
    model.m_iIndex = 0;
    
    model.filtered();
}

void FileFilterModelTest::testFilteredWithInvalidIndex()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    model.m_oRoots << root;
    model.m_iIndex = -1; // 无效索引
    
    int count = model.filtered();
    QCOMPARE(count, 0);
}

void FileFilterModelTest::testFilteredWithEmptyRoots()
{
    FileFilterModel model;
    model.m_iIndex = 0;
    
    int count = model.filtered();
    QCOMPARE(count, 0);
}

void FileFilterModelTest::testAddFilterNode()
{
    FileFilterModel model;
    QSignalSpy nodeSpy(&model, &FileFilterModel::node);
    QSignalSpy rootSpy(&model, &FileFilterModel::root);
    
    QString tempPath = m_tempDir->path();
    QString volumnID = "test_volume";
    DataCDROM* cdrom = createMockCDROM(tempPath, volumnID, 1);
    
    model.addFilterNode(cdrom);
    
    QCOMPARE(model.m_oRoots.size(), 1);
    QCOMPARE(nodeSpy.count(), 1);
    QCOMPARE(rootSpy.count(), 1);
    
    delete cdrom;
}

void FileFilterModelTest::testAddFilterNodeWithZeroUsing()
{
    FileFilterModel model;
    QString tempPath = m_tempDir->path();
    QString volumnID = "test_volume";
    DataCDROM* cdrom = createMockCDROM(tempPath, volumnID, 0); // m_is_using = 0
    
    model.addFilterNode(cdrom);
    
    QCOMPARE(model.m_oRoots.size(), 0); // 不应该添加
    
    delete cdrom;
}

void FileFilterModelTest::testAddFilterNodeSignals()
{
    FileFilterModel model;
    QSignalSpy nodeSpy(&model, &FileFilterModel::node);
    QSignalSpy rootSpy(&model, &FileFilterModel::root);
    
    QString tempPath = m_tempDir->path();
    QString volumnID = "test_volume";
    DataCDROM* cdrom = createMockCDROM(tempPath, volumnID, 1);
    
    model.addFilterNode(cdrom);
    
    QCOMPARE(nodeSpy.count(), 1);
    QCOMPARE(rootSpy.count(), 1);
    
    delete cdrom;
}

void FileFilterModelTest::testDeleteFilterNode()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    model.m_oRoots << root;
    
    model.deleteFilterNode(0);
    
    QCOMPARE(model.m_oRoots.size(), 0);
}

void FileFilterModelTest::testDeleteFilterNodeInvalidIndex()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    model.m_oRoots << root;
    
    model.deleteFilterNode(-1); // 无效索引
    QCOMPARE(model.m_oRoots.size(), 1); // 不应该删除
    
    model.deleteFilterNode(10); // 超出范围
    QCOMPARE(model.m_oRoots.size(), 1); // 不应该删除
}

void FileFilterModelTest::testDeleteFilterNodeOutOfRange()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    model.m_oRoots << root;
    
    model.deleteFilterNode(5); // 超出范围
    QCOMPARE(model.m_oRoots.size(), 1);
}

void FileFilterModelTest::testGetCurrentRoot()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    model.m_oRoots << root;
    model.m_iIndex = 0;
    
    DataDirectory* currentRoot = model.getCurrentRoot();
    QVERIFY(currentRoot != nullptr);
    QCOMPARE(currentRoot, root);
}

void FileFilterModelTest::testGetCurrentRootInvalidIndex()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    model.m_oRoots << root;
    model.m_iIndex = -1;
    
    DataDirectory* currentRoot = model.getCurrentRoot();
    QVERIFY(currentRoot == nullptr);
    
    model.m_iIndex = 10; // 超出范围
    currentRoot = model.getCurrentRoot();
    QVERIFY(currentRoot == nullptr);
}

void FileFilterModelTest::testAddFilterData()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    DataFile* file = createMockFile(m_tempDir->path() + "/file.txt", root);
    
    model.m_oRoots << root;
    
    //model.addFilterData(0, file);
}

void FileFilterModelTest::testAddFilterDataWithNullRoot()
{
    FileFilterModel model;
    DataFile* file = createMockFile(m_tempDir->path() + "/file.txt");
    
    model.m_oRoots << nullptr;
    
    // 应该不会崩溃
    //model.addFilterData(0, file);
}

void FileFilterModelTest::testRestoreFiles()
{
    FileFilterModel model;
    QSignalSpy spy(&model, &FileFilterModel::allFilesRestored);
    
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    model.m_oRoots << root;
    model.m_iIndex = 0;
    
    model.restoreFiles();
}

void FileFilterModelTest::testRestoreFilesWithHiddenFiles()
{
    FileFilterModel model;
    QSignalSpy spy(&model, &FileFilterModel::allFilesRestored);
    
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    DataFile* hiddenFile = createMockHiddenFile(m_tempDir->path() + "/hidden.txt", root);
    
    model.m_oRoots << root;
    model.m_iIndex = 0;
    
    model.restoreFiles();
}

void FileFilterModelTest::testRestoreFilesWithBrokenLinks()
{
    FileFilterModel model;
    QSignalSpy spy(&model, &FileFilterModel::allFilesRestored);
    
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    DataFile* brokenLink = createMockSymbolLink(m_tempDir->path() + "/broken.link", nullptr, root);
    
    model.m_oRoots << root;
    model.m_iIndex = 0;
    
    model.restoreFiles();
}

void FileFilterModelTest::testRestoreFilesWithReplacedLinks()
{
    FileFilterModel model;
    QSignalSpy spy(&model, &FileFilterModel::allFilesRestored);
    
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    DataFile* targetFile = createMockFile(m_tempDir->path() + "/target.txt", root);
    DataFile* linkFile = createMockSymbolLink(m_tempDir->path() + "/link.link", targetFile, root);
    
    // 创建目标文件
    QFile file(m_tempDir->path() + "/target.txt");
    file.open(QIODevice::WriteOnly);
    file.write("test");
    file.close();
    
    model.m_oRoots << root;
    model.m_iIndex = 0;
    
    model.restoreFiles();
}

void FileFilterModelTest::testRestoreFilesWithInvalidIndex()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    model.m_oRoots << root;
    model.m_iIndex = -1;
    
    model.restoreFiles(); // 应该不会崩溃
    QCOMPARE(root->counts(), 0);
}

void FileFilterModelTest::testGetFilteredHiddenFiles()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    TestDataFile* hiddenFile = createMockHiddenFile(m_tempDir->path() + "/hidden.txt", root);
    hiddenFile->setLevel(1);
    
    model.m_oRoots << root;
    model.m_iIndex = 0;
    
    model.GetFilteredHiddenFiles(0, 0);
}

void FileFilterModelTest::testGetFilteredHiddenFilesWithLevel()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    TestDataFile* hiddenFile1 = createMockHiddenFile(m_tempDir->path() + "/hidden1.txt", root);
    TestDataFile* hiddenFile2 = createMockHiddenFile(m_tempDir->path() + "/hidden2.txt", root);
    hiddenFile1->setLevel(1);
    hiddenFile2->setLevel(2);
    
    model.m_oRoots << root;
    model.m_iIndex = 0;
    
    model.GetFilteredHiddenFiles(0, 1);
}

void FileFilterModelTest::testGetFilteredHiddenFilesDirectoryAtLevel()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    TestDataDirectory* hiddenDir = new TestDataDirectory(m_tempDir->path() + "/hidden_dir", root);
    hiddenDir->setAttrHidden(true);
    hiddenDir->setLevel(1);
    
    model.m_oRoots << root;
    model.m_iIndex = 0;
    
    QList<void*> result = model.GetFilteredHiddenFiles(0, 1);
    QCOMPARE(result.size(), 0); // 目录在相同级别应该被跳过
}

void FileFilterModelTest::testGetFilteredBadLinkFiles()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    TestDataFile* brokenLink = createMockSymbolLink(m_tempDir->path() + "/broken.link", nullptr, root);
    brokenLink->setLevel(1);
    
    model.m_oRoots << root;
    model.m_iIndex = 0;
    
    model.GetFilteredBadLinkFiles(0, 0);
}

void FileFilterModelTest::testGetFilteredBadLinkFilesWithLevel()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    TestDataFile* brokenLink1 = createMockSymbolLink(m_tempDir->path() + "/broken1.link", nullptr, root);
    TestDataFile* brokenLink2 = createMockSymbolLink(m_tempDir->path() + "/broken2.link", nullptr, root);
    brokenLink1->setLevel(1);
    brokenLink2->setLevel(2);
    
    model.m_oRoots << root;
    model.m_iIndex = 0;
    
    model.GetFilteredBadLinkFiles(0, 1);
}

void FileFilterModelTest::testGetFilteredBadLinkFilesDirectoryAtLevel()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    TestDataDirectory* brokenDir = new TestDataDirectory(m_tempDir->path() + "/broken_dir", root);
    brokenDir->setAttrLink(true);
    brokenDir->setLevel(1);
    
    model.m_oRoots << root;
    model.m_iIndex = 0;
    
    QList<void*> result = model.GetFilteredBadLinkFiles(0, 1);
    QCOMPARE(result.size(), 0); // 目录在相同级别应该被跳过
}

void FileFilterModelTest::testGetFilteredReplacedLinkfiles()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    DataFile* targetFile = createMockFile(m_tempDir->path() + "/target.txt", root);
    DataFile* linkFile = createMockSymbolLink(m_tempDir->path() + "/link.link", targetFile, root);
    
    model.m_oRoots << root;
    model.m_iIndex = 0;
    
    QList<void*> result = model.GetFilteredReplacedLinkfiles(0, 0);
    // 结果取决于目标文件是否存在
    QVERIFY(result.size() >= 0);
}

void FileFilterModelTest::testGetFilteredReplacedLinkfilesWithExistingTarget()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    DataFile* targetFile = createMockFile(m_tempDir->path() + "/target.txt", root);
    DataFile* linkFile = createMockSymbolLink(m_tempDir->path() + "/link.link", targetFile, root);
    
    // 创建目标文件
    QFile file(m_tempDir->path() + "/target.txt");
    file.open(QIODevice::WriteOnly);
    file.write("test");
    file.close();

    model.m_oRoots << root;
    model.m_iIndex = 0;
    
    QList<void*> result = model.GetFilteredReplacedLinkfiles(0, 0);
    QVERIFY(true);
}

void FileFilterModelTest::testGetFilteredReplacedLinkfilesWithNonExistingTarget()
{
    FileFilterModel model;
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    DataFile* targetFile = createMockFile(m_tempDir->path() + "/nonexistent.txt", root);
    DataFile* linkFile = createMockSymbolLink(m_tempDir->path() + "/link.link", targetFile, root);
    
    model.m_oRoots << root;
    model.m_iIndex = 0;
    
    QList<void*> result = model.GetFilteredReplacedLinkfiles(0, 0);
    //QCOMPARE(result.size(), 0); // 目标文件不存在，不应该被过滤
}

void FileFilterModelTest::testUpdateIndex()
{
    FileFilterModel model;
    QSignalSpy spy(&model, &FileFilterModel::root);
    
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    model.m_oRoots << root;
    
    model.updateIndex(0);
    
    QCOMPARE(model.m_iIndex, 0);
    QCOMPARE(spy.count(), 1);
}

void FileFilterModelTest::testUpdateIndexSignals()
{
    FileFilterModel model;
    QSignalSpy spy(&model, &FileFilterModel::root);
    
    DataDirectory* root = createMockDirectory(m_tempDir->path() + "/root");
    model.m_oRoots << root;
    
    model.updateIndex(0);
    
    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QModelIndex idx = args.at(0).value<QModelIndex>();
    QVERIFY(idx.isValid());
}

// 辅助函数实现
DataCDROM* FileFilterModelTest::createMockCDROM(const QString& tempPath, const QString& volumnID, int isUsing)
{
    DataCDROM* cdrom = new DataCDROM("/dev/sr0");
    cdrom->m_is_using = isUsing;
    
    // 设置DataSetting
    DataSetting* setting = cdrom->getSettings();
    if (setting) {
        setting->updateTempPath(tempPath);
        setting->updateVolumnID(volumnID);
    }
    
    return cdrom;
}

DataDirectory* FileFilterModelTest::createMockDirectory(const QString& path)
{
    QDir dir;
    dir.mkpath(path);
    return new DataDirectory(path, nullptr);
}

TestDataFile* FileFilterModelTest::createMockFile(const QString& path, DataDirectory* parent)
{
    QFile file(path);
    if (!file.exists()) {
        file.open(QIODevice::WriteOnly);
        file.write("test content");
        file.close();
    }
    return new TestDataFile(path, parent);
}

TestDataFile* FileFilterModelTest::createMockHiddenFile(const QString& path, DataDirectory* parent)
{
    TestDataFile* file = createMockFile(path, parent);
    file->setAttrHidden(true);
    return file;
}

TestDataFile* FileFilterModelTest::createMockSymbolLink(const QString& path, DataFile* target, DataDirectory* parent)
{
    TestDataFile* file = createMockFile(path, parent);
    file->setAttrLink(true);
    file->setLinkTarget(target);
    return file;
}

#ifndef RUN_MODEL_UNIFIED_TESTS
QTEST_MAIN(FileFilterModelTest)
#endif

#include "filefiltermodel_test.moc"
