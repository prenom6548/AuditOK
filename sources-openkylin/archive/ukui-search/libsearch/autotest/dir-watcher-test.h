#ifndef DIR_WATCHER_TEST_H
#define DIR_WATCHER_TEST_H

#include <QObject>

class DirWatcherTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testSingletonAndForwardedSignals();
    void testServiceBackedMethodsReturnValuesAndInvokeDbusCalls();
    void testFallbackMethodsWithoutService();
};

#endif // DIR_WATCHER_TEST_H
