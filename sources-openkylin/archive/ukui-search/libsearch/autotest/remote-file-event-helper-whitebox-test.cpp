#include <QTest>

#define private public
#include "remote-file-event-helper.h"
#undef private

class RemoteFileEventHelperWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void nodeUrlUsesDisplayOnX11();
    void nodeUrlUsesWaylandDisplayOnWayland();
    void constructAndEmitRequestWithoutRemoteService();
};

void RemoteFileEventHelperWhiteboxTest::nodeUrlUsesDisplayOnX11()
{
    qputenv("USER", "whitebox");
    qputenv("XDG_SESSION_TYPE", "x11");
    qputenv("DISPLAY", ":27");
    qputenv("WAYLAND_DISPLAY", "wayland-9");

    QCOMPARE(RemoteFileEventHelper::nodeUrl(), QUrl(QStringLiteral("local:ukui-idm-whitebox:27")));
}

void RemoteFileEventHelperWhiteboxTest::nodeUrlUsesWaylandDisplayOnWayland()
{
    qputenv("USER", "whitebox");
    qputenv("XDG_SESSION_TYPE", "wayland");
    qputenv("DISPLAY", ":27");
    qputenv("WAYLAND_DISPLAY", "wayland-9");

    QCOMPARE(RemoteFileEventHelper::nodeUrl(), QUrl(QStringLiteral("local:ukui-idm-whiteboxwayland-9")));
}

void RemoteFileEventHelperWhiteboxTest::constructAndEmitRequestWithoutRemoteService()
{
    qputenv("USER", "whitebox");
    qputenv("XDG_SESSION_TYPE", "x11");
    qputenv("DISPLAY", ":27");

    RemoteFileEventHelper helper;
    QVERIFY(helper.m_remoteNode != nullptr);
    QVERIFY(helper.m_fileEventHandlerIface != nullptr);

    Q_EMIT helper.handleFileEventRequest(1, QStringLiteral("/tmp/a"), QStringLiteral("/tmp/b"));
    QCoreApplication::processEvents();
}

QTEST_MAIN(RemoteFileEventHelperWhiteboxTest)

#include "remote-file-event-helper-whitebox-test.moc"
