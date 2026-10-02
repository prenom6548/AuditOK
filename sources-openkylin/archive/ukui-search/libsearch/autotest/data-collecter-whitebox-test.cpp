#include <QTest>

#include "data-collecter.h"

namespace {
int g_initSource = 0;
int g_initEvent = 0;
int g_appendCount = 0;
QString g_propertyKey;
QString g_propertyValue;
QString g_eventName;
QString g_pluginName;
bool g_freed = false;
}

extern "C" KTrackData *kdk_dia_data_init(int source, int event)
{
    g_initSource = source;
    g_initEvent = event;
    g_freed = false;
    auto *data = new KTrackData;
    data->source = source;
    data->event = event;
    return data;
}

extern "C" void kdk_dia_append_custom_property(KTrackData *node, KCustomProperty *property, int count)
{
    QVERIFY(node != nullptr);
    g_appendCount = count;
    if (property) {
        g_propertyKey = QString::fromLocal8Bit(property->key);
        g_propertyValue = QString::fromLocal8Bit(property->value);
    }
}

extern "C" void kdk_dia_upload_default(KTrackData *node, char *eventName, char *pluginName)
{
    QVERIFY(node != nullptr);
    g_eventName = QString::fromLocal8Bit(eventName);
    g_pluginName = QString::fromLocal8Bit(pluginName);
    free(eventName);
    free(pluginName);
}

extern "C" void kdk_dia_data_free(KTrackData *node)
{
    g_freed = true;
    delete node;
}

class DataCollecterWhiteboxTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void collectLaunchEventForwardsSdkPayload();
};

void DataCollecterWhiteboxTest::collectLaunchEventForwardsSdkPayload()
{
    DataCollecter::collectLaunchEvent(QStringLiteral("web"), QStringLiteral("open"));

    QCOMPARE(g_initSource, KEVENTSOURCE_DESKTOP);
    QCOMPARE(g_initEvent, KEVENT_CLICK);
    QCOMPARE(g_appendCount, 1);
    QCOMPARE(g_propertyKey, QStringLiteral("pluginName"));
    QCOMPARE(g_propertyValue, QStringLiteral("open"));
    QCOMPARE(g_eventName, QStringLiteral("search_plugin_launch_event"));
    QCOMPARE(g_pluginName, QStringLiteral("web"));
    QVERIFY(g_freed);
}

QTEST_MAIN(DataCollecterWhiteboxTest)

#include "data-collecter-whitebox-test.moc"
