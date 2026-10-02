#include "action-transmiter-test.h"

#include <QSignalSpy>
#include <QTest>

#include "action-transmiter.h"

using namespace UkuiSearch;

void ActionTransmiterTest::testSingletonIdentity()
{
    QCOMPARE(ActionTransmiter::getInstance(), ActionTransmiter::getInstance());
}

void ActionTransmiterTest::testInvokeHideUiAction()
{
    ActionTransmiter *transmiter = ActionTransmiter::getInstance();
    QSignalSpy spy(transmiter, SIGNAL(hideUIAction()));
    QVERIFY(spy.isValid());

    transmiter->invokeActions(SearchPluginIface::InvokableAction::HideUI);
    QCOMPARE(spy.count(), 1);
}

void ActionTransmiterTest::testInvokeNoneActionDoesNotEmit()
{
    ActionTransmiter *transmiter = ActionTransmiter::getInstance();
    QSignalSpy spy(transmiter, SIGNAL(hideUIAction()));
    QVERIFY(spy.isValid());

    transmiter->invokeActions(SearchPluginIface::InvokableAction::None);
    QCOMPARE(spy.count(), 0);
}

QTEST_MAIN(ActionTransmiterTest)
