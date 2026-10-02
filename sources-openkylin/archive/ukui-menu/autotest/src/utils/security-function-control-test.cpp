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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#define private public
#include "../../../src/utils/security-function-control.h"
#undef private

#include <QtTest>
#include <QSignalSpy>

#include "fakes/function-control.h"

using namespace UkuiMenu;

namespace {

QHash<QString, QJsonObject> g_functionControlObjects;
QStringList g_watchedNames;

}

namespace UkuiQuick {

void FunctionControl::addWatch(const QString &name)
{
    g_watchedNames.append(name);
}

QJsonObject FunctionControl::functionControlObject(const QString &name) const
{
    return g_functionControlObjects.value(name);
}

void FunctionControl::reset()
{
    g_functionControlObjects.clear();
    g_watchedNames.clear();
}

void FunctionControl::setFunctionControlObject(const QString &name, const QJsonObject &object)
{
    g_functionControlObjects.insert(name, object);
}

QStringList FunctionControl::watchedNames()
{
    return g_watchedNames;
}

}

class securityFunctionControl_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void test_instance_registersWatchAndDefaults();
    void test_compareListBySort();
    void test_canAppDisplay_withWhiteAndBlackLists();
    void test_onFunctionControlChanged_ignoresOtherName();
    void test_onFunctionControlChanged_appliesConfigAndSignals();
    void test_onFunctionControlChanged_clearsStateOnExit();
};

void securityFunctionControl_test::init()
{
    UkuiQuick::FunctionControl::reset();
}

void securityFunctionControl_test::test_instance_registersWatchAndDefaults()
{
    SecurityFunctionControl control;

    QVERIFY(UkuiQuick::FunctionControl::watchedNames().contains(QStringLiteral("ukui-menu-v11")));
    QCOMPARE(control.disable(), false);
    QCOMPARE(control.disableMouseLeftButton(), false);
    QCOMPARE(control.disableMouseRightButton(), false);
    QCOMPARE(control.disableMouseHover(), false);
    QCOMPARE(control.disableUninstallAction(), false);
    QCOMPARE(control.windowMaximize(), false);
    QCOMPARE(control.showFoldButton(), true);
    QCOMPARE(control.powerButtonSoleOperation(), QString());
}

void securityFunctionControl_test::test_compareListBySort()
{
    SecurityFunctionControl control;
    QStringList left { QStringLiteral("b"), QStringLiteral("a") };
    QStringList right { QStringLiteral("a"), QStringLiteral("b") };
    QStringList mismatch { QStringLiteral("a") };

    QVERIFY(control.compareListBySort(left, right));
    QVERIFY(!control.compareListBySort(left, mismatch));
}

void securityFunctionControl_test::test_canAppDisplay_withWhiteAndBlackLists()
{
    SecurityFunctionControl control;
    control.m_appWhiteList = QStringList({ QStringLiteral("alpha.desktop"), QStringLiteral("beta.desktop") });
    control.m_appBlackList = QStringList({ QStringLiteral("beta.desktop") });

    QVERIFY(control.canAppDisplay(QStringLiteral("/tmp/alpha.desktop")));
    QVERIFY(!control.canAppDisplay(QStringLiteral("/tmp/gamma.desktop")));
    QVERIFY(!control.canAppDisplay(QStringLiteral("/tmp/beta.desktop")));

    control.m_appWhiteList.clear();
    QVERIFY(control.canAppDisplay(QStringLiteral("/tmp/alpha.desktop")));
}

void securityFunctionControl_test::test_onFunctionControlChanged_ignoresOtherName()
{
    SecurityFunctionControl control;
    control.m_disable = true;

    control.onFunctionControlChanged(QStringLiteral("other-name"), false);

    QCOMPARE(control.disable(), true);
}

void securityFunctionControl_test::test_onFunctionControlChanged_appliesConfigAndSignals()
{
    SecurityFunctionControl control;
    QSignalSpy disableSpy(&control, &SecurityFunctionControl::disableChanged);
    QSignalSpy leftSpy(&control, &SecurityFunctionControl::disableMouseLeftButtonChanged);
    QSignalSpy rightSpy(&control, &SecurityFunctionControl::disableMouseRightButtonChanged);
    QSignalSpy hoverSpy(&control, &SecurityFunctionControl::disableMouseHoverChanged);
    QSignalSpy whiteSpy(&control, &SecurityFunctionControl::appWhiteListChanged);
    QSignalSpy blackSpy(&control, &SecurityFunctionControl::appBlackListChanged);
    QSignalSpy uninstallSpy(&control, &SecurityFunctionControl::disableUninstallActionChanged);
    QSignalSpy maximizeSpy(&control, &SecurityFunctionControl::windowMaximizeChanged);
    QSignalSpy foldSpy(&control, &SecurityFunctionControl::showFoldButtonChanged);
    QSignalSpy powerSpy(&control, &SecurityFunctionControl::powerButtonSoleOperationChanged);

    QJsonObject value;
    value.insert(QStringLiteral("disable"), true);
    value.insert(QStringLiteral("disableMouseLeftButton"), true);
    value.insert(QStringLiteral("disableMouseRightButton"), true);
    value.insert(QStringLiteral("disableMouseHover"), true);
    value.insert(QStringLiteral("appWhiteList"), QJsonArray({ QStringLiteral("a.desktop"), QStringLiteral("b.desktop") }));
    value.insert(QStringLiteral("appBlackList"), QJsonArray({ QStringLiteral("c.desktop") }));
    value.insert(QStringLiteral("disableUninstallAction"), true);
    value.insert(QStringLiteral("windowMaximize"), true);
    value.insert(QStringLiteral("powerButtonSoleOperation"), QStringLiteral("shutdown"));

    QJsonObject wrapper;
    wrapper.insert(QStringLiteral("ukui-menu-v11"), value);
    UkuiQuick::FunctionControl::setFunctionControlObject(QStringLiteral("ukui-menu-v11"), wrapper);

    control.onFunctionControlChanged(QStringLiteral("ukui-menu-v11"), false);

    QCOMPARE(control.disable(), true);
    QCOMPARE(control.disableMouseLeftButton(), true);
    QCOMPARE(control.disableMouseRightButton(), true);
    QCOMPARE(control.disableMouseHover(), true);
    QCOMPARE(control.appWhiteList(), QStringList({ QStringLiteral("a.desktop"), QStringLiteral("b.desktop") }));
    QCOMPARE(control.appBlackList(), QStringList({ QStringLiteral("c.desktop") }));
    QCOMPARE(control.disableUninstallAction(), true);
    QCOMPARE(control.windowMaximize(), true);
    QCOMPARE(control.showFoldButton(), false);
    QCOMPARE(control.powerButtonSoleOperation(), QStringLiteral("shutdown"));

    QCOMPARE(disableSpy.count(), 1);
    QCOMPARE(leftSpy.count(), 1);
    QCOMPARE(rightSpy.count(), 1);
    QCOMPARE(hoverSpy.count(), 1);
    QCOMPARE(whiteSpy.count(), 1);
    QCOMPARE(blackSpy.count(), 1);
    QCOMPARE(uninstallSpy.count(), 1);
    QCOMPARE(maximizeSpy.count(), 1);
    QCOMPARE(foldSpy.count(), 1);
    QCOMPARE(powerSpy.count(), 1);
}

void securityFunctionControl_test::test_onFunctionControlChanged_clearsStateOnExit()
{
    SecurityFunctionControl control;

    control.m_disable = true;
    control.m_disableMouseLeftButton = true;
    control.m_disableMouseRightButton = true;
    control.m_disableMouseHover = true;
    control.m_appWhiteList = QStringList({ QStringLiteral("a.desktop") });
    control.m_appBlackList = QStringList({ QStringLiteral("b.desktop") });
    control.m_disableUninstallAction = true;
    control.m_windowMaximize = true;
    control.m_showFoldButton = false;
    control.m_powerButtonSoleOperation = QStringLiteral("restart");

    control.onFunctionControlChanged(QStringLiteral("ukui-menu-v11"), true);

    QCOMPARE(control.disable(), false);
    QCOMPARE(control.disableMouseLeftButton(), false);
    QCOMPARE(control.disableMouseRightButton(), false);
    QCOMPARE(control.disableMouseHover(), false);
    QVERIFY(control.appWhiteList().isEmpty());
    QVERIFY(control.appBlackList().isEmpty());
    QCOMPARE(control.disableUninstallAction(), false);
    QCOMPARE(control.windowMaximize(), false);
    QCOMPARE(control.showFoldButton(), true);
    QCOMPARE(control.powerButtonSoleOperation(), QString());
}

QTEST_MAIN(securityFunctionControl_test)

#include "security-function-control-test.moc"
