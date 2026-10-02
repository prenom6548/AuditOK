#ifndef INDEX_STATUS_RECORDER_TEST_H
#define INDEX_STATUS_RECORDER_TEST_H

#include <QObject>

class IndexStatusRecorderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSingletonAndStatusPersistence();
    void testVersionChecksAndEnableFlags();
};

#endif // INDEX_STATUS_RECORDER_TEST_H
