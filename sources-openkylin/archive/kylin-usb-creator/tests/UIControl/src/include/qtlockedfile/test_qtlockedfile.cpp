#include "test_qtlockedfile.h"
#include <QApplication>
#include <QFile>
#include <QDebug>

void TestQtLockedFile::initTestCase()
{
    m_lockedFile = nullptr;
    m_tempFile = nullptr;
}

void TestQtLockedFile::cleanupTestCase()
{
    if (m_lockedFile) {
        delete m_lockedFile;
        m_lockedFile = nullptr;
    }
    
    if (m_tempFile) {
        m_tempFile->remove();
        delete m_tempFile;
        m_tempFile = nullptr;
    }
}

void TestQtLockedFile::testDefaultConstructor()
{
    // 测试默认构造函数
    m_lockedFile = new QtLockedFile();
    QVERIFY(m_lockedFile != nullptr);
    
    // 验证初始状态
    QVERIFY(!m_lockedFile->isLocked());
    QCOMPARE(m_lockedFile->lockMode(), QtLockedFile::NoLock);
    
    delete m_lockedFile;
    m_lockedFile = nullptr;
}

void TestQtLockedFile::testNamedConstructor()
{
    // 创建临时文件用于测试
    m_tempFile = new QTemporaryFile();
    QVERIFY(m_tempFile->open());
    QString fileName = m_tempFile->fileName();
    m_tempFile->close();
    
    // 测试带文件名的构造函数
    m_lockedFile = new QtLockedFile(fileName);
    QVERIFY(m_lockedFile != nullptr);
    
    // 验证初始状态
    QVERIFY(!m_lockedFile->isLocked());
    QCOMPARE(m_lockedFile->lockMode(), QtLockedFile::NoLock);
    
    delete m_lockedFile;
    m_lockedFile = nullptr;
}

void TestQtLockedFile::testOpenWithTruncate()
{
    // 创建临时文件用于测试
    m_tempFile = new QTemporaryFile();
    QVERIFY(m_tempFile->open());
    QString fileName = m_tempFile->fileName();
    m_tempFile->close();
    
    // 测试带截断模式的打开（应该失败）
    m_lockedFile = new QtLockedFile(fileName);
    QVERIFY(m_lockedFile != nullptr);
    
    // 尝试以截断模式打开应该返回false
    bool result = m_lockedFile->open(QIODevice::WriteOnly | QIODevice::Truncate);
    QVERIFY(!result);
    
    delete m_lockedFile;
    m_lockedFile = nullptr;
}

void TestQtLockedFile::testOpenWithoutTruncate()
{
    // 创建临时文件用于测试
    m_tempFile = new QTemporaryFile();
    QVERIFY(m_tempFile->open());
    QString fileName = m_tempFile->fileName();
    m_tempFile->close();
    
    // 测试不带截断模式的打开（应该成功）
    m_lockedFile = new QtLockedFile(fileName);
    QVERIFY(m_lockedFile != nullptr);
    
    // 尝试以正常模式打开应该返回true
    bool result = m_lockedFile->open(QIODevice::ReadWrite);
    QVERIFY(result);
    QVERIFY(m_lockedFile->isOpen());
    
    delete m_lockedFile;
    m_lockedFile = nullptr;
}

void TestQtLockedFile::testLockUnlock()
{
    // 创建临时文件用于测试
    m_tempFile = new QTemporaryFile();
    QVERIFY(m_tempFile->open());
    QString fileName = m_tempFile->fileName();
    m_tempFile->close();
    
    // 创建QtLockedFile实例
    m_lockedFile = new QtLockedFile(fileName);
    QVERIFY(m_lockedFile != nullptr);
    
    // 先以正常模式打开文件
    QVERIFY(m_lockedFile->open(QIODevice::ReadWrite));
    
    // 测试加读锁
    bool lockResult = m_lockedFile->lock(QtLockedFile::ReadLock);
    QVERIFY(lockResult);
    QVERIFY(m_lockedFile->isLocked());
    QCOMPARE(m_lockedFile->lockMode(), QtLockedFile::ReadLock);
    
    // 测试解锁
    bool unlockResult = m_lockedFile->unlock();
    QVERIFY(unlockResult);
    QVERIFY(!m_lockedFile->isLocked());
    QCOMPARE(m_lockedFile->lockMode(), QtLockedFile::NoLock);
    
    // 测试加写锁
    lockResult = m_lockedFile->lock(QtLockedFile::WriteLock);
    QVERIFY(lockResult);
    QVERIFY(m_lockedFile->isLocked());
    QCOMPARE(m_lockedFile->lockMode(), QtLockedFile::WriteLock);
    
    // 再次解锁
    unlockResult = m_lockedFile->unlock();
    QVERIFY(unlockResult);
    QVERIFY(!m_lockedFile->isLocked());
    QCOMPARE(m_lockedFile->lockMode(), QtLockedFile::NoLock);
    
    delete m_lockedFile;
    m_lockedFile = nullptr;
}

void TestQtLockedFile::testIsLocked()
{
    // 创建临时文件用于测试
    m_tempFile = new QTemporaryFile();
    QVERIFY(m_tempFile->open());
    QString fileName = m_tempFile->fileName();
    m_tempFile->close();
    
    // 创建QtLockedFile实例
    m_lockedFile = new QtLockedFile(fileName);
    QVERIFY(m_lockedFile != nullptr);
    
    // 初始状态应该是未锁定
    QVERIFY(!m_lockedFile->isLocked());
    
    // 打开文件
    QVERIFY(m_lockedFile->open(QIODevice::ReadWrite));
    
    // 加锁后应该显示为已锁定
    QVERIFY(m_lockedFile->lock(QtLockedFile::ReadLock));
    QVERIFY(m_lockedFile->isLocked());
    
    // 解锁后应该显示为未锁定
    QVERIFY(m_lockedFile->unlock());
    QVERIFY(!m_lockedFile->isLocked());
    
    delete m_lockedFile;
    m_lockedFile = nullptr;
}

void TestQtLockedFile::testLockMode()
{
    // 创建临时文件用于测试
    m_tempFile = new QTemporaryFile();
    QVERIFY(m_tempFile->open());
    QString fileName = m_tempFile->fileName();
    m_tempFile->close();
    
    // 创建QtLockedFile实例
    m_lockedFile = new QtLockedFile(fileName);
    QVERIFY(m_lockedFile != nullptr);
    
    // 初始锁模式应该是NoLock
    QCOMPARE(m_lockedFile->lockMode(), QtLockedFile::NoLock);
    
    // 打开文件
    QVERIFY(m_lockedFile->open(QIODevice::ReadWrite));
    
    // 加读锁后锁模式应该是ReadLock
    QVERIFY(m_lockedFile->lock(QtLockedFile::ReadLock));
    QCOMPARE(m_lockedFile->lockMode(), QtLockedFile::ReadLock);
    
    // 解锁后锁模式应该是NoLock
    QVERIFY(m_lockedFile->unlock());
    QCOMPARE(m_lockedFile->lockMode(), QtLockedFile::NoLock);
    
    // 加写锁后锁模式应该是WriteLock
    QVERIFY(m_lockedFile->lock(QtLockedFile::WriteLock));
    QCOMPARE(m_lockedFile->lockMode(), QtLockedFile::WriteLock);
    
    // 解锁后锁模式应该是NoLock
    QVERIFY(m_lockedFile->unlock());
    QCOMPARE(m_lockedFile->lockMode(), QtLockedFile::NoLock);
    
    delete m_lockedFile;
    m_lockedFile = nullptr;
}

void TestQtLockedFile::testDestructor()
{
    // 创建临时文件用于测试
    m_tempFile = new QTemporaryFile();
    QVERIFY(m_tempFile->open());
    QString fileName = m_tempFile->fileName();
    m_tempFile->close();
    
    // 创建QtLockedFile实例
    m_lockedFile = new QtLockedFile(fileName);
    QVERIFY(m_lockedFile != nullptr);
    
    // 打开文件并加锁
    QVERIFY(m_lockedFile->open(QIODevice::ReadWrite));
    QVERIFY(m_lockedFile->lock(QtLockedFile::WriteLock));
    QVERIFY(m_lockedFile->isLocked());
    
    // 析构函数会自动解锁并释放资源
    delete m_lockedFile;
    m_lockedFile = nullptr;
    
    // 验证文件仍然可以被其他实例访问
    QFile testFile(fileName);
    QVERIFY(testFile.open(QIODevice::ReadWrite));
    testFile.close();
}