#ifndef APP_DATA_UTILS_TEST_H
#define APP_DATA_UTILS_TEST_H

#include <QObject>

class AppDataUtilsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDesktopPathsFromXdgDataDirsAndHomePath();
    void testDuplicateXdgDataDirsAreRemoved();
    void testDesktopPathsWithoutXdgDataDirs();
};

#endif // APP_DATA_UTILS_TEST_H
