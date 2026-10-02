#include "index-status-recorder-test.h"

#include <QTest>

#include "index-status-recorder.h"

using namespace UkuiSearch;

void IndexStatusRecorderTest::testSingletonAndStatusPersistence()
{
    IndexStatusRecorder *recorder = IndexStatusRecorder::getInstance();
    QVERIFY(recorder != nullptr);
    QCOMPARE(IndexStatusRecorder::getInstance(), recorder);

    recorder->setStatus(INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::Updating);
    QCOMPARE(recorder->getStatus(INDEX_DATABASE_STATE_KEY).toInt(),
             static_cast<int>(IndexStatusRecorder::Updating));

    recorder->setStatus(CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::Error);
    QCOMPARE(recorder->getStatus(CONTENT_INDEX_DATABASE_STATE_KEY).toInt(),
             static_cast<int>(IndexStatusRecorder::Error));
}

void IndexStatusRecorderTest::testVersionChecksAndEnableFlags()
{
    IndexStatusRecorder *recorder = IndexStatusRecorder::getInstance();

    recorder->setVersion(INDEX_DATABASE_VERSION_KEY, QStringLiteral("1.2.3"));
    QVERIFY(recorder->versionCheck(INDEX_DATABASE_VERSION_KEY, QStringLiteral("1.2.3")));
    QVERIFY(!recorder->versionCheck(INDEX_DATABASE_VERSION_KEY, QStringLiteral("9.9.9")));

    recorder->setStatus(INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::Ready);
    QVERIFY(recorder->indexDatabaseEnable());
    recorder->setStatus(INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::Off);
    QVERIFY(!recorder->indexDatabaseEnable());

    recorder->setStatus(CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::Ready);
    QVERIFY(recorder->contentIndexDatabaseEnable());
    recorder->setStatus(CONTENT_INDEX_DATABASE_STATE_KEY, IndexStatusRecorder::Initializing);
    QVERIFY(!recorder->contentIndexDatabaseEnable());
}

QTEST_MAIN(IndexStatusRecorderTest)
