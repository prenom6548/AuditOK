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

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlContext>
#include <QQmlEngine>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

#include "widget-bridge.h"
#include "widget-feature-provider.h"
#include "widget-item-context.h"
#include "widget.h"

using namespace UkuiQuick;

namespace {

WidgetMetadata createMetadata(const QString &basePath, const QString &id)
{
    const QString root = QDir(basePath).filePath(id);
    QDir().mkpath(root);

    QFile file(QDir(root).filePath(QStringLiteral("metadata.json")));
    file.open(QFile::WriteOnly | QFile::Truncate | QFile::Text);
    file.write(QJsonDocument(QJsonObject{
        {QStringLiteral("Id"), id},
        {QStringLiteral("Name"), id},
        {QStringLiteral("ShowIn"), QStringLiteral("All")}
    }).toJson());
    file.close();

    return WidgetMetadata(root);
}

class TestFeatureProvider : public QObject, public WidgetFeatureProvider
{
    Q_OBJECT

public:
    void addFeature(const QString &key, QObject *feature)
    {
        if (!m_featureOrder.contains(key)) {
            m_featureOrder.append(key);
        }
        m_features.insert(key, feature);
    }

    bool hasFeature(const QString &key) const override
    {
        return m_features.contains(key);
    }

    QObject *feature(const QString &key) override
    {
        return m_features.value(key);
    }

    QStringList availableFeatures() const override
    {
        return m_featureOrder;
    }

private:
    QStringList m_featureOrder;
    QHash<QString, QObject *> m_features;
};

void setContextForWidget(QObject *object, QQmlEngine *engine, Widget *widget)
{
    auto *widgetContext = new WidgetItemContext(engine, widget, object);
    auto *childContext = new QQmlContext(widgetContext, object);
    QQmlEngine::setContextForObject(object, childContext);
}

} // namespace

class widget_bridge_test : public QObject
{
    Q_OBJECT

private slots:
    void bridgeResolvesWidgetFromAttachedContext();
    void missingProviderUpdatesLastError();
    void hostProvidersExposeFeaturesInPriorityOrder();
};

void widget_bridge_test::bridgeResolvesWidgetFromAttachedContext()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    Widget widget(createMetadata(dir.path(), QStringLiteral("bridge-widget")));
    QQmlEngine engine;
    QObject owner;
    setContextForWidget(&owner, &engine, &widget);

    WidgetBridge bridge(&owner);

    QCOMPARE(bridge.widget(), &widget);
    QCOMPARE(bridge.pluginObject(), nullptr);
    QVERIFY(bridge.availableFeatures().isEmpty());
}

void widget_bridge_test::missingProviderUpdatesLastError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    Widget widget(createMetadata(dir.path(), QStringLiteral("bridge-no-provider")));
    QQmlEngine engine;
    QObject owner;
    setContextForWidget(&owner, &engine, &widget);
    WidgetBridge bridge(&owner);
    QSignalSpy lastErrorSpy(&bridge, &WidgetBridge::lastErrorChanged);

    QCOMPARE(bridge.feature(QStringLiteral("missing.feature")), nullptr);

    QVERIFY(bridge.lastError().contains(QStringLiteral("No feature provider")));
    QCOMPARE(lastErrorSpy.count(), 1);
    QVERIFY(!bridge.hasFeature(QStringLiteral("missing.feature")));
}

void widget_bridge_test::hostProvidersExposeFeaturesInPriorityOrder()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    Widget widget(createMetadata(dir.path(), QStringLiteral("bridge-host-provider")));
    QQmlEngine engine;
    QObject owner;
    QObject firstFeature;
    QObject secondFeature;
    QObject uniqueFeature;
    TestFeatureProvider firstProvider;
    TestFeatureProvider secondProvider;
    setContextForWidget(&owner, &engine, &widget);
    WidgetBridge bridge(&owner);
    QSignalSpy availableSpy(&bridge, &WidgetBridge::availableFeaturesChanged);

    firstProvider.addFeature(QStringLiteral("shared"), &firstFeature);
    firstProvider.addFeature(QStringLiteral("first"), &uniqueFeature);
    secondProvider.addFeature(QStringLiteral("shared"), &secondFeature);
    secondProvider.addFeature(QStringLiteral("second"), &secondFeature);

    WidgetBridge::setFeatureProvider(&widget, &firstProvider);
    WidgetBridge::setFeatureProvider(&widget, &secondProvider);

    QCOMPARE(availableSpy.count(), 2);
    QCOMPARE(bridge.availableFeatures(), QStringList({QStringLiteral("shared"), QStringLiteral("second"), QStringLiteral("first")}));
    QVERIFY(bridge.hasFeature(QStringLiteral("shared")));
    QCOMPARE(bridge.feature(QStringLiteral("shared")), &secondFeature);
    QCOMPARE(bridge.feature(QStringLiteral("first")), &uniqueFeature);
    QCOMPARE(bridge.lastError(), QString());

    QCOMPARE(bridge.feature(QStringLiteral("unknown")), nullptr);
    QVERIFY(bridge.lastError().contains(QStringLiteral("Feature is unavailable")));

    WidgetBridge::clearFeatureProvider(&widget);
    QCOMPARE(availableSpy.count(), 3);
    QVERIFY(bridge.availableFeatures().isEmpty());
    QCOMPARE(bridge.lastError(), QString());
}

QTEST_GUILESS_MAIN(widget_bridge_test)

#include "test-widget-bridge.moc"
