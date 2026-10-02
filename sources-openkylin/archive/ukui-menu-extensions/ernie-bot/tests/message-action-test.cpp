#include <QtTest>
#include <QSignalSpy>

#include "../src/message-action.h"

class messageAction_test : public QObject
{
    Q_OBJECT

private slots:
    void test_MessageAction();
};

void messageAction_test::test_MessageAction()
{
    MessageAction emptyAction(nullptr);
    QVERIFY(emptyAction.isValid());
    QCOMPARE(emptyAction.text(), QString());
    QCOMPARE(emptyAction.name(), QString());

    QSignalSpy execSpy(&emptyAction, &MessageAction::actionExecuted);
    emptyAction.exec();
    QCOMPARE(execSpy.count(), 1);

    MessageAction action("Question", "Retry");
    QVERIFY(!action.isValid());
    QCOMPARE(action.text(), QString("Question"));
    QCOMPARE(action.name(), QString("Retry"));

    action.setText("New Question");
    action.setName("New Name");
    QCOMPARE(action.text(), QString("New Question"));
    QCOMPARE(action.name(), QString("New Name"));
}

QTEST_MAIN(messageAction_test)

#include "message-action-test.moc"
