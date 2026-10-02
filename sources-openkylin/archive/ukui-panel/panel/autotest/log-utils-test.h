#ifndef LOG_UTILS_TEST_H
#define LOG_UTILS_TEST_H

#include <QObject>

class LogUtilsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testInitLogFilePrefersReusableFile();
    void testInitLogFileClearsOversizedFiles();
    void testCheckFileSizeAndClearFile();
    void testCheckLogFileKeepsCurrentFile();
    void testCheckLogFileRotatesAndClearsTarget();
    void testMessageOutputWritesDifferentLevels();
    void testMessageOutputWritesFatalLevel();
    void testMessageOutputFallsBackWithoutLogFile();

    void testInitLogFileCreateDirFailed();
    void testCheckLogFileTimeIntervalExceeded();
    void testMessageOutputLogFileOpenFailed();
    void testInitLogFileAllFilesOversized();
};

#endif // LOG_UTILS_TEST_H
