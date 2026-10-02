#include "action-label-test.h"

#include <QSignalSpy>
#include <QTest>

#include "action-label.h"

using namespace UkuiSearch;

void ActionLabelTest::testTextAndCursorInitialized()
{
    ActionLabel label(QStringLiteral("Open"), QStringLiteral("key"));

    QCOMPARE(label.text(), QStringLiteral("Open"));
    QCOMPARE(label.cursor().shape(), Qt::PointingHandCursor);
}

void ActionLabelTest::testMouseReleaseEmitsActionTriggered()
{
    qRegisterMetaType<QString>("QString&");
    ActionLabel label(QStringLiteral("Install"), QStringLiteral("key"));
    QSignalSpy spy(&label, SIGNAL(actionTriggered(QString&)));
    QVERIFY(spy.isValid());

    QMouseEvent releaseEvent(QEvent::MouseButtonRelease,
                             QPointF(1, 1),
                             Qt::LeftButton,
                             Qt::LeftButton,
                             Qt::NoModifier);
    QApplication::sendEvent(&label, &releaseEvent);

    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(ActionLabelTest)
