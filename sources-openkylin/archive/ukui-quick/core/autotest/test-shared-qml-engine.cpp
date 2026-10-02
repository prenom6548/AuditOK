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

#include <QQmlComponent>
#include <QQmlEngine>
#include <QtTest>

#include "shared-qml-engine.h"

using namespace UkuiQuick;

class shared_qml_engine_test : public QObject
{
    Q_OBJECT

private slots:
    void sharedEngineReturnsSameInstance();
    void sharedEngineCanCreateSimpleQmlObject();
};

void shared_qml_engine_test::sharedEngineReturnsSameInstance()
{
    QQmlEngine *first = SharedQmlEngine::sharedQmlEngine();
    QQmlEngine *second = SharedQmlEngine::sharedQmlEngine();

    QVERIFY(first != nullptr);
    QCOMPARE(first, second);
}

void shared_qml_engine_test::sharedEngineCanCreateSimpleQmlObject()
{
    QQmlEngine *engine = SharedQmlEngine::sharedQmlEngine();
    QVERIFY(engine != nullptr);

    QQmlComponent component(engine);
    component.setData(R"(
        import QtQml 2.15
        QtObject {
            property int answer: 42
        }
    )", QUrl());

    std::unique_ptr<QObject> object(component.create());
    QVERIFY2(object != nullptr, qPrintable(component.errorString()));
    QCOMPARE(object->property("answer").toInt(), 42);
    QCOMPARE(object->parent(), nullptr);
}

QTEST_GUILESS_MAIN(shared_qml_engine_test)

#include "test-shared-qml-engine.moc"
