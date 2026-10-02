#include "show-more-label-test.h"

#include <QEvent>
#include <QLabel>
#include <QSignalSpy>
#include <QTest>

#define private public
#define protected public
#include "show-more-label.h"
#undef protected
#undef private

using namespace UkuiSearch;

void ShowMoreLabelTest::testSetAndResetExpandedState()
{
    ShowMoreLabel label;

    QVERIFY(!label.getExpanded());
    label.setLabel();
    QVERIFY(label.getExpanded());
    label.resetLabel();
    QVERIFY(!label.getExpanded());
}

void ShowMoreLabelTest::testMouseClickTogglesAndEmitsSignals()
{
    ShowMoreLabel label;
    auto *icon = label.findChild<QLabel *>();
    QVERIFY(icon != nullptr);

    QSignalSpy showMoreSpy(&label, SIGNAL(showMoreClicked()));
    QSignalSpy retractSpy(&label, SIGNAL(retractClicked()));
    QVERIFY(showMoreSpy.isValid());
    QVERIFY(retractSpy.isValid());

    QTest::mouseClick(icon, Qt::LeftButton);
    QVERIFY(label.getExpanded());
    QCOMPARE(showMoreSpy.count(), 1);
    QCOMPARE(retractSpy.count(), 0);

    QTest::mouseClick(icon, Qt::LeftButton);
    QVERIFY(!label.getExpanded());
    QCOMPARE(showMoreSpy.count(), 1);
    QCOMPARE(retractSpy.count(), 1);
}

void ShowMoreLabelTest::testEventFilterIgnoresNonIconAndTimerActiveBranches()
{
    ShowMoreLabel label;
    auto *icon = label.findChild<QLabel *>();
    QVERIFY(icon != nullptr);

    QSignalSpy showMoreSpy(&label, SIGNAL(showMoreClicked()));
    QSignalSpy retractSpy(&label, SIGNAL(retractClicked()));
    QVERIFY(showMoreSpy.isValid());
    QVERIFY(retractSpy.isValid());

    QEvent nonMouse(QEvent::Enter);
    QVERIFY(!label.eventFilter(icon, &nonMouse));
    QVERIFY(!label.getExpanded());
    QCOMPARE(showMoreSpy.count(), 0);
    QCOMPARE(retractSpy.count(), 0);

    QObject unrelated;
    QEvent press(QEvent::MouseButtonPress);
    QVERIFY(!label.eventFilter(&unrelated, &press));
    QVERIFY(!label.getExpanded());
    QCOMPARE(showMoreSpy.count(), 0);
    QCOMPARE(retractSpy.count(), 0);

    label.m_timer->start(1000);
    QVERIFY(label.m_timer->isActive());
    QVERIFY(!label.eventFilter(icon, &press));
    QVERIFY(!label.getExpanded());
    QCOMPARE(showMoreSpy.count(), 0);
    QCOMPARE(retractSpy.count(), 0);
    label.m_timer->stop();
}

QTEST_MAIN(ShowMoreLabelTest)
