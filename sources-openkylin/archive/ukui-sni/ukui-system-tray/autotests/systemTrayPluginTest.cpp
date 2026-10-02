#include <QtTest>
#include <qqml.h>
#include <QQmlEngine>

#include "plugin/system-tray-plugin.h"

class systemTrayPlugin_test : public QObject
{
    Q_OBJECT

private slots:
    void test_registerTypesAndCreateQmlObjects();
};

void systemTrayPlugin_test::test_registerTypesAndCreateQmlObjects()
{
    SystemTrayPlugin plugin;
    plugin.registerTypes("org.ukui.systemTray");

    const int groupTypeId = qmlTypeId("org.ukui.systemTray", 1, 0, "GroupModel");
    const int itemModelTypeId = qmlTypeId("org.ukui.systemTray", 1, 0, "ItemModel");
    const int functionControlTypeId = qmlTypeId("org.ukui.systemTray", 1, 0, "FunctionControl");
    const int controlPanelTypeId = qmlTypeId("org.ukui.systemTray", 1, 0, "ControlPanelModel");

    QVERIFY(groupTypeId > 0);
    QVERIFY(itemModelTypeId > 0);
    QVERIFY(functionControlTypeId > 0);
    QVERIFY(controlPanelTypeId > 0);

    auto *engine = new QQmlEngine;
    QVERIFY(engine->singletonInstance<QObject *>(functionControlTypeId) != nullptr);
}

QTEST_MAIN(systemTrayPlugin_test)

#include "systemTrayPluginTest.moc"
