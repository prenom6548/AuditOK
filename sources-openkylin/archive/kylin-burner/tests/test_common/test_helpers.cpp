#include "test_helpers.h"
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include <QCoreApplication>
#include <QUuid>

// 创建临时测试目录
QTemporaryDir* TestHelpers::createTempDir()
{
    QTemporaryDir* tempDir = new QTemporaryDir();
    tempDir->setAutoRemove(true);
    return tempDir;
}

// 创建临时测试文件
QFile* TestHelpers::createTempFile(const QString& content)
{
    QString uniqueName = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString tempFileName = QDir::tempPath() + "/" + uniqueName + ".tmp";
    QFile* tempFile = new QFile(tempFileName);
    
    if (tempFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(tempFile);
        out << content;
        tempFile->close();
    }
    
    return tempFile;
}

// 比较两个文件是否内容相同
bool TestHelpers::compareFiles(const QString& file1, const QString& file2)
{
    QFile f1(file1);
    QFile f2(file2);
    
    if (!f1.open(QIODevice::ReadOnly) || !f2.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    // 比较文件大小
    if (f1.size() != f2.size()) {
        return false;
    }
    
    // 按块比较文件内容
    const qint64 bufferSize = 4096;
    while (!f1.atEnd() && !f2.atEnd()) {
        QByteArray buffer1 = f1.read(bufferSize);
        QByteArray buffer2 = f2.read(bufferSize);
        
        if (buffer1 != buffer2) {
            return false;
        }
    }
    
    return true;
}

// 清理测试资源
void TestHelpers::cleanup()
{
    // 清理操作将在析构函数中自动完成
}