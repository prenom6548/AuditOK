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

#include <QtTest>

#include "function-control.h"

using namespace UkuiQuick;

class function_control_test : public QObject
{
    Q_OBJECT

private slots:
    void singletonCanBeCreatedAndDestroyed();
    void watchesCanBeAddedAndRemovedWithoutConfig();
    void watchesForDifferentNamesDoNotInterfere();
};

void function_control_test::singletonCanBeCreatedAndDestroyed()
{
    auto *control = FunctionControl::instance();

    QVERIFY(control != nullptr);
    QCOMPARE(FunctionControl::instance(), control);

    delete control;
}

void function_control_test::watchesCanBeAddedAndRemovedWithoutConfig()
{
    auto *control = FunctionControl::instance();
    const QString featureName = QStringLiteral("codex.feature");

    control->removeAllWatches(featureName);
    QVERIFY(control->functionControlObject(featureName).isEmpty());

    control->addWatch(featureName);
    control->addWatch(featureName);
    QVERIFY(control->functionControlObject(featureName).isEmpty());

    control->removeWatch(featureName);
    QVERIFY(control->functionControlObject(featureName).isEmpty());

    control->removeWatch(featureName);
    QVERIFY(control->functionControlObject(featureName).isEmpty());

    control->removeWatch(featureName);
    control->removeAllWatches(featureName);
    QVERIFY(control->functionControlObject(featureName).isEmpty());

    delete control;
}

void function_control_test::watchesForDifferentNamesDoNotInterfere()
{
    auto *control = FunctionControl::instance();
    const QString first = QStringLiteral("codex.first");
    const QString second = QStringLiteral("codex.second");

    control->removeAllWatches(first);
    control->removeAllWatches(second);

    control->addWatch(first);
    control->addWatch(second);
    control->removeWatch(first);

    QVERIFY(control->functionControlObject(first).isEmpty());
    QVERIFY(control->functionControlObject(second).isEmpty());

    control->removeWatch(second);
    QVERIFY(control->functionControlObject(second).isEmpty());

    delete control;
}

QTEST_GUILESS_MAIN(function_control_test)

#include "test-function-control.moc"
