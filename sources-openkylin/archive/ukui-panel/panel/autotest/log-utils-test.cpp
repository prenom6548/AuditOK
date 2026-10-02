#include "log-utils-test.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QTest>
#include <functional>
#include <unistd.h>
#include <QSignalSpy>
#include <sys/stat.h>

#define private public
#include "log-utils.h"
#undef private

namespace {

QString logDirectoryPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.log/ukui-panel/";
}

bool ensureLogDirectoryAvailable()
{
    const QString logDir = logDirectoryPath();
    const QString logRoot = QFileInfo(QDir::cleanPath(logDir)).path();

    if (QFileInfo::exists(logDir) && !QFileInfo(logDir).isDir()) {
        if (!QFile::remove(logDir)) {
            return false;
        }
    }

    if (QFileInfo::exists(logRoot) && !QFileInfo(logRoot).isDir()) {
        if (!QFile::remove(logRoot)) {
            return false;
        }
    }

    return QDir().mkpath(logDir);
}

QString uniqueLogBaseName()
{
    return QStringLiteral("autotest-log-utils-%1-%2")
            .arg(QCoreApplication::applicationPid())
            .arg(QDateTime::currentMSecsSinceEpoch());
}

QString logFilePath(const QString &baseName, int index)
{
    return logDirectoryPath() + baseName + QStringLiteral("-%1.log").arg(index);
}

void writeSizedFile(const QString &filePath, qint64 size)
{
    QVERIFY(ensureLogDirectoryAvailable());
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QVERIFY(file.resize(size));
    file.close();
}

QString readLogFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return QString::fromLocal8Bit(file.readAll());
}

FILE *streamForFd(int fd)
{
    return fd == STDERR_FILENO ? stderr : stdout;
}

QString captureDescriptorOutput(int fd, const std::function<void()> &action)
{
    QTemporaryFile outputFile;
    outputFile.setAutoRemove(true);
    if (!outputFile.open()) {
        return {};
    }

    fflush(streamForFd(fd));
    const int savedFd = dup(fd);
    if (savedFd < 0) {
        return {};
    }
    if (dup2(outputFile.handle(), fd) < 0) {
        close(savedFd);
        return {};
    }

    action();

    fflush(streamForFd(fd));
    if (dup2(savedFd, fd) < 0) {
        close(savedFd);
        return {};
    }
    close(savedFd);

    outputFile.seek(0);
    return QString::fromLocal8Bit(outputFile.readAll());
}

// 辅助函数：修改目录权限使其无法创建
bool makeDirUnwritable(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        if (!dir.mkpath(path)) return false;
    }
    return chmod(path.toLocal8Bit().constData(), 0555) == 0;
}

// 辅助函数：恢复目录权限
bool restoreDirWritable(const QString &path)
{
    return chmod(path.toLocal8Bit().constData(), 0755) == 0;
}

} // namespace

static QString g_logUtilsTestBaseName;

void LogUtilsTest::initTestCase()
{
    QVERIFY(ensureLogDirectoryAvailable());
    g_logUtilsTestBaseName = uniqueLogBaseName();
}

void LogUtilsTest::cleanupTestCase()
{
    for (int i = 0; i < 2; ++i) {
        QFile::remove(logFilePath(g_logUtilsTestBaseName, i));
    }
    restoreDirWritable(logDirectoryPath());
    QVERIFY(ensureLogDirectoryAvailable());
}

void LogUtilsTest::testInitLogFilePrefersReusableFile()
{
    const auto baseName = g_logUtilsTestBaseName + QStringLiteral("-reuse");
    const auto file0 = logFilePath(baseName, 0);
    const auto file1 = logFilePath(baseName, 1);

    writeSizedFile(file0, 128);
    QFile::remove(file1);

    LogUtils::m_logFileId = -1;
    LogUtils::m_logFileName.clear();
    LogUtils::initLogFile(baseName);

    QCOMPARE(LogUtils::m_logFileId.loadRelaxed(), 0);
    QCOMPARE(LogUtils::m_logFileName, logDirectoryPath() + baseName + QStringLiteral("-%0.log"));
    QVERIFY(QFile::exists(file0));
    QVERIFY(!QFile::exists(file1));

    QFile::remove(file0);
    QFile::remove(file1);
}

void LogUtilsTest::testInitLogFileClearsOversizedFiles()
{
    const auto baseName = g_logUtilsTestBaseName + QStringLiteral("-oversized");
    const auto file0 = logFilePath(baseName, 0);
    const auto file1 = logFilePath(baseName, 1);

    writeSizedFile(file0, 4194305);
    writeSizedFile(file1, 4194305);

    LogUtils::m_logFileId = -1;
    LogUtils::initLogFile(baseName);

    QCOMPARE(LogUtils::m_logFileId.loadRelaxed(), 0);
    QCOMPARE(QFile(file0).size(), 0);
    QVERIFY(QFile(file1).size() > 0);

    QFile::remove(file0);
    QFile::remove(file1);
}

void LogUtilsTest::testCheckFileSizeAndClearFile()
{
    const auto filePath = logFilePath(g_logUtilsTestBaseName + QStringLiteral("-size"), 0);

    writeSizedFile(filePath, 64);
    QVERIFY(LogUtils::checkFileSize(filePath));

    // 测试刚好等于阈值的情况（边界值）
    writeSizedFile(filePath, 4194304);
    QVERIFY(!LogUtils::checkFileSize(filePath));

    writeSizedFile(filePath, 4194305);
    QVERIFY(!LogUtils::checkFileSize(filePath));

    LogUtils::clearFile(filePath);
    QCOMPARE(QFile(filePath).size(), 0);

    QFile::remove(filePath);
}

void LogUtilsTest::testCheckLogFileKeepsCurrentFile()
{
    const auto baseName = g_logUtilsTestBaseName + QStringLiteral("-keep");
    const auto file0 = logFilePath(baseName, 0);
    const auto file1 = logFilePath(baseName, 1);

    writeSizedFile(file0, 128);
    writeSizedFile(file1, 128);

    LogUtils::m_logFileName = logDirectoryPath() + baseName + QStringLiteral("-%0.log");
    LogUtils::m_logFileId = 0;
    LogUtils::m_startUpTime = QDateTime::currentMSecsSinceEpoch();

    LogUtils::checkLogFile();

    QCOMPARE(LogUtils::m_logFileId.loadRelaxed(), 0);

    QFile::remove(file0);
    QFile::remove(file1);
}

void LogUtilsTest::testCheckLogFileRotatesAndClearsTarget()
{
    const auto baseName = g_logUtilsTestBaseName + QStringLiteral("-rotate");
    const auto file0 = logFilePath(baseName, 0);
    const auto file1 = logFilePath(baseName, 1);

    writeSizedFile(file0, 4194305);
    writeSizedFile(file1, 4194305);

    LogUtils::m_logFileName = logDirectoryPath() + baseName + QStringLiteral("-%0.log");
    LogUtils::m_logFileId = 0;
    LogUtils::m_startUpTime = QDateTime::currentMSecsSinceEpoch();

    LogUtils::checkLogFile();

    QCOMPARE(LogUtils::m_logFileId.loadRelaxed(), 1);
    QCOMPARE(QFile(file1).size(), 0);

    QFile::remove(file0);
    QFile::remove(file1);
}

void LogUtilsTest::testMessageOutputWritesDifferentLevels()
{
    const auto baseName = g_logUtilsTestBaseName + QStringLiteral("-output");
    const auto file0 = logFilePath(baseName, 0);
    const auto file1 = logFilePath(baseName, 1);

    QFile::remove(file0);
    QFile::remove(file1);

    LogUtils::m_logFileId = -1;
    LogUtils::initLogFile(baseName);

    const QMessageLogContext context(__FILE__, 123, Q_FUNC_INFO, "autotest.category");

    LogUtils::messageOutput(QtDebugMsg, context, QStringLiteral("debug message"));
    LogUtils::messageOutput(QtInfoMsg, context, QStringLiteral("info message"));
    LogUtils::messageOutput(QtWarningMsg, context, QStringLiteral("warning message"));
    LogUtils::messageOutput(QtCriticalMsg, context, QStringLiteral("critical message"));

    const auto currentFile = LogUtils::m_logFileName.arg(LogUtils::m_logFileId.loadRelaxed());
    const auto content = readLogFile(currentFile);

    QVERIFY(content.contains(QStringLiteral("Debug:")));
    QVERIFY(content.contains(QStringLiteral("debug message")));
    QVERIFY(content.contains(QStringLiteral("Info:")));
    QVERIFY(content.contains(QStringLiteral("info message")));
    QVERIFY(content.contains(QStringLiteral("Warning:")));
    QVERIFY(content.contains(QStringLiteral("warning message")));
    QVERIFY(content.contains(QStringLiteral("Critical:")));
    QVERIFY(content.contains(QStringLiteral("critical message")));

    QFile::remove(file0);
    QFile::remove(file1);
}

void LogUtilsTest::testMessageOutputWritesFatalLevel()
{
    const auto baseName = g_logUtilsTestBaseName + QStringLiteral("-fatal");
    const auto file0 = logFilePath(baseName, 0);
    const auto file1 = logFilePath(baseName, 1);

    QFile::remove(file0);
    QFile::remove(file1);

    LogUtils::m_logFileId = -1;
    LogUtils::initLogFile(baseName);

    const QMessageLogContext context(__FILE__, 456, Q_FUNC_INFO, "autotest.category");
    LogUtils::messageOutput(QtFatalMsg, context, QStringLiteral("fatal message"));

    const auto currentFile = LogUtils::m_logFileName.arg(LogUtils::m_logFileId.loadRelaxed());
    const auto content = readLogFile(currentFile);

    QVERIFY(content.contains(QStringLiteral("Fatal:")));
    QVERIFY(content.contains(QStringLiteral("fatal message")));

    QFile::remove(file0);
    QFile::remove(file1);
}

void LogUtilsTest::testMessageOutputFallsBackWithoutLogFile()
{
    LogUtils::m_startUpTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    LogUtils::m_logFileId = 0;
    LogUtils::m_logFileName = QStringLiteral("/proc/ukui-panel-autotest-%0.log");

    const QMessageLogContext nullContext(nullptr, 9, nullptr, "autotest.category");

    const auto debugOutput = captureDescriptorOutput(STDERR_FILENO, [&] {
        LogUtils::messageOutput(QtDebugMsg, nullContext, QStringLiteral("debug skipped"));
    });
    QVERIFY(debugOutput.isEmpty());

    const auto infoOutput = captureDescriptorOutput(STDOUT_FILENO, [&] {
        LogUtils::messageOutput(QtInfoMsg, nullContext, QStringLiteral("info fallback"));
    });
    QVERIFY(infoOutput.contains(QStringLiteral("Info:")));
    QVERIFY(infoOutput.contains(QStringLiteral("info fallback")));
    QVERIFY(infoOutput.contains(QStringLiteral("(:9, )")));

    const auto stderrOutput = captureDescriptorOutput(STDERR_FILENO, [&] {
        LogUtils::messageOutput(QtWarningMsg, nullContext, QStringLiteral("warning fallback"));
        LogUtils::messageOutput(QtCriticalMsg, nullContext, QStringLiteral("critical fallback"));
        LogUtils::messageOutput(QtFatalMsg, nullContext, QStringLiteral("fatal fallback"));
    });
    QVERIFY(stderrOutput.contains(QStringLiteral("Warning:")));
    QVERIFY(stderrOutput.contains(QStringLiteral("warning fallback")));
    QVERIFY(stderrOutput.contains(QStringLiteral("Critical:")));
    QVERIFY(stderrOutput.contains(QStringLiteral("critical fallback")));
    QVERIFY(stderrOutput.contains(QStringLiteral("Fatal:")));
    QVERIFY(stderrOutput.contains(QStringLiteral("fatal fallback")));
    QVERIFY(stderrOutput.contains(QStringLiteral("(:9, )")));
}

// 新增：测试initLogFile创建目录失败的分支
void LogUtilsTest::testInitLogFileCreateDirFailed()
{
    const auto baseName = g_logUtilsTestBaseName + QStringLiteral("-dir-fail");
    const QString logDir = logDirectoryPath();
    const QString blockedPath = QFileInfo(QDir::cleanPath(logDir)).path();
    QDir rootDir;

    restoreDirWritable(logDir);
    if (QFileInfo::exists(logDir)) {
        if (QFileInfo(logDir).isDir()) {
            QVERIFY(QDir(logDir).removeRecursively());
        } else {
            QVERIFY(QFile::remove(logDir));
        }
    }
    if (QFileInfo::exists(blockedPath)) {
        if (QFileInfo(blockedPath).isDir()) {
            QVERIFY(QDir(blockedPath).removeRecursively());
        } else {
            QVERIFY(QFile::remove(blockedPath));
        }
    }

    // 用父级路径同名文件阻塞目录创建，稳定命中 mkpath 失败分支
    QFile blocker(blockedPath);
    QVERIFY(blocker.open(QIODevice::WriteOnly | QIODevice::Truncate));
    blocker.close();

    // 重置状态
    LogUtils::m_logFileId = -1;
    LogUtils::m_logFileName.clear();

    captureDescriptorOutput(STDERR_FILENO, [&] {
        LogUtils::initLogFile(baseName);
    });

    QVERIFY(QFile::remove(blockedPath));
    QVERIFY(rootDir.mkpath(logDir));

    // 验证分支执行
    QVERIFY(LogUtils::m_logFileName.isEmpty());
    QCOMPARE(LogUtils::m_logFileId.loadRelaxed(), -1);
}

// 新增：测试checkLogFile时间间隔超过阈值的分支
void LogUtilsTest::testCheckLogFileTimeIntervalExceeded()
{
    const auto baseName = g_logUtilsTestBaseName + QStringLiteral("-time-exceed");
    const auto file0 = logFilePath(baseName, 0);
    const auto file1 = logFilePath(baseName, 1);

    writeSizedFile(file0, 128); // 文件大小正常
    writeSizedFile(file1, 128);

    LogUtils::m_logFileName = logDirectoryPath() + baseName + QStringLiteral("-%0.log");
    LogUtils::m_logFileId = 0;
    // 设置启动时间为12小时前（超过MAX_LOG_CHECK_INTERVAL=12h）
    LogUtils::m_startUpTime = QDateTime::currentMSecsSinceEpoch() - 43200001;

    LogUtils::checkLogFile();

    // 验证文件轮转
    QCOMPARE(LogUtils::m_logFileId.loadRelaxed(), 1);
    // 目标文件大小正常，无需清空
    QCOMPARE(QFile(file1).size(), 128);

    QFile::remove(file0);
    QFile::remove(file1);
}

// 新增：测试messageOutput中日志文件打开失败的分支
void LogUtilsTest::testMessageOutputLogFileOpenFailed()
{
    const auto baseName = g_logUtilsTestBaseName + QStringLiteral("-open-fail");
    const auto file0 = logFilePath(baseName, 0);
    const auto file1 = logFilePath(baseName, 1);

    QVERIFY(ensureLogDirectoryAvailable());
    QFile::remove(file0);
    QFile::remove(file1);
    QVERIFY(QDir().mkpath(file0));

    // 初始化日志文件
    LogUtils::m_logFileId = -1;
    LogUtils::initLogFile(baseName);
    QCOMPARE(LogUtils::m_logFileId.loadRelaxed(), 0);

    const QMessageLogContext context(__FILE__, 789, Q_FUNC_INFO, "autotest.category");

    // 捕获Debug级别输出（文件打开失败时Debug不输出）
    const auto debugOutput = captureDescriptorOutput(STDERR_FILENO, [&] {
        LogUtils::messageOutput(QtDebugMsg, context, QStringLiteral("debug open fail"));
    });
    QVERIFY(debugOutput.isEmpty());

    // 捕获Info级别输出（回退到stdout）
    const auto infoOutput = captureDescriptorOutput(STDOUT_FILENO, [&] {
        LogUtils::messageOutput(QtInfoMsg, context, QStringLiteral("info open fail"));
    });
    QVERIFY(infoOutput.contains(QStringLiteral("Info:")));
    QVERIFY(infoOutput.contains(QStringLiteral("info open fail")));

    QVERIFY(QDir(file0).removeRecursively());
    QFile::remove(file1);
}

// 新增：测试initLogFile所有文件都超大时的分支
void LogUtilsTest::testInitLogFileAllFilesOversized()
{
    const auto baseName = g_logUtilsTestBaseName + QStringLiteral("-all-oversized");
    const auto file0 = logFilePath(baseName, 0);
    const auto file1 = logFilePath(baseName, 1);

    // 两个文件都设置为超大
    writeSizedFile(file0, 4194305);
    writeSizedFile(file1, 4194305);

    LogUtils::m_logFileId = -1;
    LogUtils::m_logFileName.clear();
    LogUtils::initLogFile(baseName);

    // 验证选中0号文件并清空
    QCOMPARE(LogUtils::m_logFileId.loadRelaxed(), 0);
    QCOMPARE(QFile(file0).size(), 0);
    QVERIFY(QFile(file1).size() > 0);

    QFile::remove(file0);
    QFile::remove(file1);
}

QTEST_MAIN(LogUtilsTest)
