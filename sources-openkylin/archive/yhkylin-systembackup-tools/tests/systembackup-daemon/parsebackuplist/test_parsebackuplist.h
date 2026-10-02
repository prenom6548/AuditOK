#ifndef TEST_PARSEBACKUPLIST_H
#define TEST_PARSEBACKUPLIST_H

#include <QObject>
#include <QtTest/QtTest>
#include "../../../systembackup-daemon/parsebackuplist.h"

class TestParseBackupList : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // 测试构造函数
    void testConstructor();
    
    // 测试根据comment查找uuid
    void testFindUuidByComment();
    
    // 测试根据Uuid查找备份点信息
    void testFindBackupPointByUuid();
    
    // 测试获取xml文件中备份点uuid和backupname的映射
    void testGetXmlUuidNameMap();
    
    // 测试新增备份节点
    void testAddItem();
    
    // 测试更新备份点信息
    void testUpdateItem();
    
    // 测试删除备份点记录
    void testDeleteItem();
    
    // 测试获取备份列表
    void testGetBackupPointList();
    
    // 测试统计备份点数量
    void testCountBackupPoint();

    // 以下补充未覆盖的接口
    void testUpdateForFactoryRestore();
    void testGetXmlCommitHashMap();
    void testGetDataCommitHashMap();
    void testGetLastSysBackupPoint();
    void testGetCustomizePaths();

private:
    ParseBackupList* m_parseBackupList;
    QString m_testXmlPath;
};

#endif // TEST_PARSEBACKUPLIST_H