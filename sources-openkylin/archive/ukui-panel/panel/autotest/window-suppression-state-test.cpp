#include "window-suppression-state-test.h"

#include <QTest>

#include "common.h"

using UkuiPanel::WindowSuppressionState;

void WindowSuppressionStateTest::testDefaultStateNotSuppressed()
{
    WindowSuppressionState state;

    QVERIFY(!state.suppressed());
    QVERIFY(!state.screenLockHidden());
    QVERIFY(!state.shellHidden());
    QVERIFY(!state.controlHidden());
}

void WindowSuppressionStateTest::testScreenLockDoesNotClearControlHidden()
{
    WindowSuppressionState state;
    state.setControlHidden(true);
    state.setScreenLockHidden(true);

    QVERIFY(state.suppressed());

    state.setScreenLockHidden(false);

    QVERIFY(state.suppressed());
    QVERIFY(state.controlHidden());
    QVERIFY(!state.screenLockHidden());
}

void WindowSuppressionStateTest::testScreenLockDoesNotClearShellHidden()
{
    WindowSuppressionState state;
    state.setShellHidden(true);
    state.setScreenLockHidden(true);

    QVERIFY(state.suppressed());

    state.setScreenLockHidden(false);

    QVERIFY(state.suppressed());
    QVERIFY(state.shellHidden());
    QVERIFY(!state.screenLockHidden());
}

void WindowSuppressionStateTest::testClearingLastSuppressorRestoresVisibility()
{
    WindowSuppressionState state;
    state.setShellHidden(true);
    QVERIFY(state.suppressed());

    state.setShellHidden(false);

    QVERIFY(!state.suppressed());
}

QTEST_MAIN(WindowSuppressionStateTest)
