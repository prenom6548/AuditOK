#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <QString>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>

// 测试辅助工具类
class TestHelpers
{
public:
    // 创建临时测试目录
    static QTemporaryDir* createTempDir();
    
    // 创建临时测试文件
    static QFile* createTempFile(const QString& content = "");
    
    // 比较两个文件是否内容相同
    static bool compareFiles(const QString& file1, const QString& file2);
    
    // 清理测试资源
    static void cleanup();
};

#endif // TEST_HELPERS_H