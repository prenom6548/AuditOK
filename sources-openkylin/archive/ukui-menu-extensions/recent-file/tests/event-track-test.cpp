#include <QtTest>

#include <kysdk/diagnosetest/libkydatacollect.h>

#include "../event-track.h"

using namespace UkuiMenu;

namespace {

struct DataCollectCallState
{
    int initCalls = 0;
    int freeCalls = 0;
    int defaultUploadCalls = 0;
    int searchUploadCalls = 0;
    KEventSourceType lastSourceType = KEVENTSOURCE_WEB;
    KEventType lastEventType = KEVENT_VISIT;
    KTrackData trackData{};
    KTrackData *lastInitNode = nullptr;
    KTrackData *lastFreedNode = nullptr;
    KTrackData *lastDefaultNode = nullptr;
    KTrackData *lastSearchNode = nullptr;
    QString lastEventCode;
    QString lastPageName;
    QString lastContent;
};

DataCollectCallState g_dataCollectCallState;

void resetDataCollectCallState()
{
    g_dataCollectCallState = DataCollectCallState();
}

}

extern "C" KTrackData * __wrap_kdk_dia_data_init(KEventSourceType sourceType, KEventType evnetType)
{
    ++g_dataCollectCallState.initCalls;
    g_dataCollectCallState.lastSourceType = sourceType;
    g_dataCollectCallState.lastEventType = evnetType;
    g_dataCollectCallState.trackData = KTrackData{};
    g_dataCollectCallState.trackData.eventSourceType = sourceType;
    g_dataCollectCallState.trackData.eventType = evnetType;
    g_dataCollectCallState.lastInitNode = &g_dataCollectCallState.trackData;
    return g_dataCollectCallState.lastInitNode;
}

extern "C" void __wrap_kdk_dia_upload_default(KTrackData *node, char *eventCode, char *pageName)
{
    ++g_dataCollectCallState.defaultUploadCalls;
    g_dataCollectCallState.lastDefaultNode = node;
    g_dataCollectCallState.lastEventCode = QString::fromUtf8(eventCode);
    g_dataCollectCallState.lastPageName = QString::fromUtf8(pageName);
    g_dataCollectCallState.lastContent.clear();
}

extern "C" void __wrap_kdk_dia_upload_search_content(KTrackData *node,
                                                       char *eventCode,
                                                       char *pageName,
                                                       char *content)
{
    ++g_dataCollectCallState.searchUploadCalls;
    g_dataCollectCallState.lastSearchNode = node;
    g_dataCollectCallState.lastEventCode = QString::fromUtf8(eventCode);
    g_dataCollectCallState.lastPageName = QString::fromUtf8(pageName);
    g_dataCollectCallState.lastContent = QString::fromUtf8(content);
}

extern "C" void __wrap_kdk_dia_data_free(KTrackData *node)
{
    ++g_dataCollectCallState.freeCalls;
    g_dataCollectCallState.lastFreedNode = node;
}

class eventTrack_test : public QObject
{
    Q_OBJECT

private slots:
    void test_EventTrack();
};

void eventTrack_test::test_EventTrack()
{
    EventTrack *eventTrack = EventTrack::instance();
    QVERIFY(eventTrack != nullptr);
    QCOMPARE(eventTrack, EventTrack::instance());
    QCOMPARE(eventTrack->parent(), nullptr);

    resetDataCollectCallState();
    eventTrack->sendClickEvent("open_recent_file", "RecentFile");
    QCOMPARE(g_dataCollectCallState.initCalls, 1);
    QCOMPARE(g_dataCollectCallState.defaultUploadCalls, 1);
    QCOMPARE(g_dataCollectCallState.searchUploadCalls, 0);
    QCOMPARE(g_dataCollectCallState.freeCalls, 1);
    QCOMPARE(g_dataCollectCallState.lastSourceType, KEVENTSOURCE_DESKTOP);
    QCOMPARE(g_dataCollectCallState.lastEventType, KEVENT_CLICK);
    QCOMPARE(g_dataCollectCallState.lastEventCode, QString("open_recent_file"));
    QCOMPARE(g_dataCollectCallState.lastPageName, QString("RecentFile"));
    QCOMPARE(g_dataCollectCallState.lastDefaultNode, g_dataCollectCallState.lastInitNode);
    QCOMPARE(g_dataCollectCallState.lastFreedNode, g_dataCollectCallState.lastInitNode);

    resetDataCollectCallState();
    eventTrack->sendDefaultEvent("clear_recent_files", "RightClickMenu");
    QCOMPARE(g_dataCollectCallState.initCalls, 1);
    QCOMPARE(g_dataCollectCallState.defaultUploadCalls, 1);
    QCOMPARE(g_dataCollectCallState.searchUploadCalls, 0);
    QCOMPARE(g_dataCollectCallState.freeCalls, 1);
    QCOMPARE(g_dataCollectCallState.lastSourceType, KEVENTSOURCE_DESKTOP);
    QCOMPARE(g_dataCollectCallState.lastEventType, KEVENT_CUSTOM);
    QCOMPARE(g_dataCollectCallState.lastEventCode, QString("clear_recent_files"));
    QCOMPARE(g_dataCollectCallState.lastPageName, QString("RightClickMenu"));
    QCOMPARE(g_dataCollectCallState.lastDefaultNode, g_dataCollectCallState.lastInitNode);
    QCOMPARE(g_dataCollectCallState.lastFreedNode, g_dataCollectCallState.lastInitNode);

    resetDataCollectCallState();
    eventTrack->sendSearchEvent("search_recent_file", "RecentFile", "report.txt");
    QCOMPARE(g_dataCollectCallState.initCalls, 1);
    QCOMPARE(g_dataCollectCallState.defaultUploadCalls, 0);
    QCOMPARE(g_dataCollectCallState.searchUploadCalls, 1);
    QCOMPARE(g_dataCollectCallState.freeCalls, 1);
    QCOMPARE(g_dataCollectCallState.lastSourceType, KEVENTSOURCE_DESKTOP);
    QCOMPARE(g_dataCollectCallState.lastEventType, KEVENT_SEARCH);
    QCOMPARE(g_dataCollectCallState.lastEventCode, QString("search_recent_file"));
    QCOMPARE(g_dataCollectCallState.lastPageName, QString("RecentFile"));
    QCOMPARE(g_dataCollectCallState.lastContent, QString("report.txt"));
    QCOMPARE(g_dataCollectCallState.lastSearchNode, g_dataCollectCallState.lastInitNode);
    QCOMPARE(g_dataCollectCallState.lastFreedNode, g_dataCollectCallState.lastInitNode);
}

QTEST_MAIN(eventTrack_test)

#include "event-track-test.moc"