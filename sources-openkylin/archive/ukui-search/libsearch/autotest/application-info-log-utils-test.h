#ifndef APPLICATION_INFO_LOG_UTILS_TEST_H
#define APPLICATION_INFO_LOG_UTILS_TEST_H

#include <QObject>

class ApplicationInfoLogUtilsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void logUtilsInitLogFileCreatesAndSelectsExpectedFiles();
    void logUtilsWritesAllMessageTypesAndRotatesFiles();
    void logUtilsHandlesMissingLogFileAndTimeBasedRotation();
};

#endif // APPLICATION_INFO_LOG_UTILS_TEST_H
