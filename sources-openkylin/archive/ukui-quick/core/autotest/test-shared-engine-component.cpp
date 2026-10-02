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
#include <QTemporaryDir>
#include <QtTest>

#include "shared-engine-component.h"
#include "shared-qml-engine.h"

using namespace UkuiQuick;

namespace {

QUrl writeQml(QTemporaryDir &dir, const QString &name, const QByteArray &content)
{
    const QString path = dir.filePath(name);
    QFile file(path);
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    file.write(content);
    file.close();
    return QUrl::fromLocalFile(path);
}

}

class shared_engine_component_test : public QObject
{
    Q_OBJECT

private slots:
    void sharedEngineUsesGlobalInstance();
    void constructorCreatesContextAndStoresInitialData();
    void emptySourceIsIgnored();
    void validSourceCreatesRootObjectAndAppliesInitialData();
    void invalidSourceKeepsRootObjectEmptyAndExposesComponentError();
};

void shared_engine_component_test::sharedEngineUsesGlobalInstance()
{
    QCOMPARE(SharedEngineComponent::sharedEngine(), SharedQmlEngine::sharedQmlEngine());
    QCOMPARE(SharedEngineComponent::sharedEngine(), SharedEngineComponent::sharedEngine());
}

void shared_engine_component_test::constructorCreatesContextAndStoresInitialData()
{
    const QVariantMap data{{QStringLiteral("title"), QStringLiteral("demo")},
                           {QStringLiteral("count"), 7}};
    SharedEngineComponent component(data, nullptr);

    QVERIFY(component.rootContext() != nullptr);
    QCOMPARE(component.initializeData(), data);
    QVERIFY(component.source().isEmpty());
    QCOMPARE(component.rootObject(), nullptr);
    QCOMPARE(component.component(), nullptr);
}

void shared_engine_component_test::emptySourceIsIgnored()
{
    SharedEngineComponent component;

    component.setSource(QUrl());

    QVERIFY(component.source().isEmpty());
    QCOMPARE(component.component(), nullptr);
    QCOMPARE(component.rootObject(), nullptr);
}

void shared_engine_component_test::validSourceCreatesRootObjectAndAppliesInitialData()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QUrl source = writeQml(dir, QStringLiteral("component.qml"), R"(
        import QtQml 2.15
        QtObject {
            property string title: "initial"
            property int count: 0
        }
    )");

    const QVariantMap data{{QStringLiteral("title"), QStringLiteral("configured")},
                           {QStringLiteral("count"), 42}};
    SharedEngineComponent component(data, nullptr);

    component.setSource(source);

    QCOMPARE(component.source(), source);
    QVERIFY(component.component() != nullptr);
    QVERIFY(component.rootObject() != nullptr);
    QCOMPARE(component.rootObject()->property("title").toString(), QStringLiteral("configured"));
    QCOMPARE(component.rootObject()->property("count").toInt(), 42);
}

void shared_engine_component_test::invalidSourceKeepsRootObjectEmptyAndExposesComponentError()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QUrl source = writeQml(dir, QStringLiteral("broken.qml"), R"(
        import QtQml 2.15
        QtObject {
            property int broken:
        }
    )");

    SharedEngineComponent component;

    component.setSource(source);

    QCOMPARE(component.source(), source);
    QVERIFY(component.component() != nullptr);
    QCOMPARE(component.rootObject(), nullptr);
    QVERIFY(component.component()->isError());
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    shared_engine_component_test testCase;
    return QTest::qExec(&testCase, argc, argv);
}

#include "test-shared-engine-component.moc"
