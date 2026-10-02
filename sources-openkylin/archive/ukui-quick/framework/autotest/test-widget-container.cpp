/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include <QAction>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include "widget-container.h"

using namespace UkuiQuick;

namespace {

WidgetMetadata createMetadata(const QString &basePath, const QString &id, bool isContainer = false)
{
    const QString root = QDir(basePath).filePath(id);
    QDir().mkpath(root);

    QJsonObject object{
        {QStringLiteral("Id"), id},
        {QStringLiteral("Name"), id},
        {QStringLiteral("ShowIn"), QStringLiteral("All")},
        {QStringLiteral("Contents"), QJsonObject{
            {QStringLiteral("Main"), QStringLiteral("main.qml")}
        }}
    };

    if (isContainer) {
        object.insert(QStringLiteral("WidgetType"), QStringLiteral("Container"));
    }

    QFile file(QDir(root).filePath(QStringLiteral("metadata.json")));
    file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text);
    file.write(QJsonDocument(object).toJson());
    file.close();

    return WidgetMetadata(root);
}

}

class widget_container_test : public QObject
{
    Q_OBJECT

private slots:
    void propertiesAndSignalsReflectMutations();
    void addWidgetStoresConfigAndUpdatesActiveState();
    void disableInstancesSyncConfigAndRemoveWidgets();
    void hostFocusRequestsAreForwardedOnlyWhileContained();
};

void widget_container_test::propertiesAndSignalsReflectMutations()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    WidgetContainer container(createMetadata(dir.path(), QStringLiteral("container-root"), true));
    QSignalSpy geometrySpy(&container, &WidgetContainer::geometryChanged);
    QSignalSpy radiusSpy(&container, &WidgetContainer::radiusChanged);
    QSignalSpy positionSpy(&container, &WidgetContainer::positionChanged);
    QSignalSpy hostsSpy(&container, &WidgetContainer::hostsChanged);

    QVERIFY(container.margin() != nullptr);
    QVERIFY(container.padding() != nullptr);
    QCOMPARE(container.type(), Types::Container);
    QCOMPARE(container.geometry(), QRect());
    QCOMPARE(container.radius(), 0);
    QCOMPARE(container.position(), Types::NoPosition);
    QCOMPARE(container.hosts(), WidgetMetadata::All);

    container.setGeometry(QRect(1, 2, 30, 40));
    container.setRadius(12);
    container.setPosition(Types::Bottom);
    container.setHosts(WidgetMetadata::Panel | WidgetMetadata::Desktop);

    QCOMPARE(container.geometry(), QRect(1, 2, 30, 40));
    QCOMPARE(container.radius(), 12);
    QCOMPARE(container.position(), Types::Bottom);
    QVERIFY(container.hosts().testFlag(WidgetMetadata::Panel));
    QVERIFY(container.hosts().testFlag(WidgetMetadata::Desktop));
    QCOMPARE(geometrySpy.count(), 1);
    QCOMPARE(radiusSpy.count(), 1);
    QCOMPARE(positionSpy.count(), 1);
    QCOMPARE(hostsSpy.count(), 1);
}

void widget_container_test::addWidgetStoresConfigAndUpdatesActiveState()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    WidgetContainer container(createMetadata(dir.path(), QStringLiteral("container-root"), true));
    Config config(QVariantMap{
        {QStringLiteral("widgets"), QVariantList()}
    }, static_cast<QObject *>(nullptr));
    container.setConfig(&config);

    auto *widget = new Widget(createMetadata(dir.path(), QStringLiteral("child-widget")));
    widget->setInstanceId(7);
    QSignalSpy addedSpy(&container, &WidgetContainer::widgetAdded);
    QSignalSpy removedSpy(&container, &WidgetContainer::widgetRemoved);
    QSignalSpy activeSpy(&container, &WidgetContainer::activeChanged);

    container.addWidget(widget);

    QCOMPARE(container.widgets().size(), 1);
    QCOMPARE(container.widget(7), widget);
    QCOMPARE(widget->container(), &container);
    QCOMPARE(addedSpy.count(), 1);
    QVERIFY(config.child(QStringLiteral("widgets"), 7) != nullptr);
    QCOMPARE(config.child(QStringLiteral("widgets"), 7)->getValue(QStringLiteral("id")).toString(), QStringLiteral("child-widget"));

    widget->setActive(true);
    QVERIFY(container.active());
    QCOMPARE(activeSpy.count(), 1);

    container.removeWidget(widget);
    QCOMPARE(container.widgets().size(), 0);
    QCOMPARE(removedSpy.count(), 1);
}

void widget_container_test::disableInstancesSyncConfigAndRemoveWidgets()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    WidgetContainer container(createMetadata(dir.path(), QStringLiteral("container-root"), true));
    Config config(QVariantMap{
        {QStringLiteral("widgets"), QVariantList()}
    }, static_cast<QObject *>(nullptr));
    container.setConfig(&config);

    auto *first = new Widget(createMetadata(dir.path(), QStringLiteral("widget-one")));
    first->setInstanceId(1);
    auto *second = new Widget(createMetadata(dir.path(), QStringLiteral("widget-two")));
    second->setInstanceId(2);
    container.addWidget(first);
    container.addWidget(second);

    container.setDisableInstances({2, 2});

    QCOMPARE(container.disableInstances().size(), 1);
    QCOMPARE(container.disableInstances().first(), 2);
    QCOMPARE(container.widgets().size(), 1);
    QCOMPARE(container.widget(1), first);
    QCOMPARE(container.widget(2), nullptr);
    QCOMPARE(config.getValue(QStringLiteral("disabledInstances")).toJsonArray().size(), 1);

    container.removeDisableInstance(2);
    QVERIFY(container.disableInstances().isEmpty());

    container.addDisableInstance(1);
    QCOMPARE(container.widgets().size(), 0);
    QCOMPARE(container.disableInstances(), QList<int>({1}));
}

void widget_container_test::hostFocusRequestsAreForwardedOnlyWhileContained()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    WidgetContainer container(createMetadata(dir.path(), QStringLiteral("container-root"), true));
    auto *widget = new Widget(createMetadata(dir.path(), QStringLiteral("focus-request-widget")));
    widget->setInstanceId(9);
    QSignalSpy requestSpy(&container, &WidgetContainer::widgetHostFocusRequested);
    QSignalSpy inheritedRequestSpy(&container, &Widget::hostFocusRequested);

    container.addWidget(widget);
    widget->requestHostFocus();

    QCOMPARE(requestSpy.count(), 1);
    QCOMPARE(qvariant_cast<Widget *>(requestSpy.first().at(0)), widget);
    QCOMPARE(inheritedRequestSpy.count(), 0);

    container.removeWidget(widget);
    widget->requestHostFocus();

    QCOMPARE(requestSpy.count(), 1);
}

QTEST_GUILESS_MAIN(widget_container_test)

#include "test-widget-container.moc"
