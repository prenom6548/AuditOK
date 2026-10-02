#include <QtTest>
#include <QSignalSpy>

#include "../src/message-model.h"
#include "../src/session.h"

class messageModel_test : public QObject
{
    Q_OBJECT

private slots:
    void test_MessageModel();
};

void messageModel_test::test_MessageModel()
{
    MessageModel model;
    QCOMPARE(model.currentSessionId(), -1);

    Session session(3, "key", "secret");
    QSignalSpy resetSpy(&model, &QAbstractItemModel::modelReset);
    QSignalSpy currentSessionSpy(&model, &MessageModel::currentSessionIdChanged);
    model.setSession(&session);
    QCOMPARE(resetSpy.count(), 2);
    QCOMPARE(currentSessionSpy.count(), 1);
    QCOMPARE(model.currentSessionId(), 3);
    QCOMPARE(model.sourceModel(), &session);

    model.setSession(&session);
    QCOMPARE(resetSpy.count(), 2);
    QCOMPARE(currentSessionSpy.count(), 1);

    session.openChatSession();
    model.talk("hello");
    QCOMPARE(session.rowCount(QModelIndex()), 4);

    model.removeSession(2);
    QCOMPARE(model.currentSessionId(), 3);
    model.removeSession(3);
    QCOMPARE(model.currentSessionId(), -1);
    QCOMPARE(model.sourceModel(), nullptr);
    QCOMPARE(resetSpy.count(), 4);
    QCOMPARE(currentSessionSpy.count(), 2);

    MessageModel emptyModel;
    emptyModel.talk("ignored");
    emptyModel.removeSession(99);
    QCOMPARE(emptyModel.currentSessionId(), -1);
}

QTEST_MAIN(messageModel_test)

#include "message-model-test.moc"
