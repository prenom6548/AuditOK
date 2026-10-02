#include <QtTest>
#include <QSignalSpy>
#include <QTest>
#include <ctime>

#include "../src/message.h"

#define private public
#include "../src/session.h"
#undef private

Q_DECLARE_METATYPE(Message::Type)

void skipNextChatStubTalkResponseForTest();
void emitExpiredOnNextChatStubTalkForTest();

namespace {

QVariant dataAt(QAbstractItemModel *model, int row, int role)
{
    return model->data(model->index(row, 0), role);
}

}

class session_test : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void test_Session();
};

void session_test::initTestCase()
{
    qRegisterMetaType<Message::Type>("Message::Type");
}

void session_test::test_Session()
{
    const QByteArray originalTimeZone = qgetenv("TZ");
    qputenv("TZ", "UTC");
    tzset();
    Session morningSession(100, "key", "secret");
    QVERIFY(!dataAt(&morningSession, 0, Message::MessageHeader).toString().isEmpty());
    if (originalTimeZone.isNull()) {
        qunsetenv("TZ");
    } else {
        qputenv("TZ", originalTimeZone);
    }
    tzset();

    Session session(7, "key", "secret");
    QCOMPARE(session.sessionId(), 7);
    QCOMPARE(session.sessionIsReady(), false);
    QCOMPARE(session.responseIsReady(), true);
    QCOMPARE(session.sessionName(), QString("Session: 7"));
    QCOMPARE(session.rowCount(QModelIndex()), 1);
    QCOMPARE(dataAt(&session, 0, Message::MessageType).value<Message::Type>(), Message::TipMessage);
    QVERIFY(!dataAt(&session, 0, Message::MessageHeader).toString().isEmpty());
    QVERIFY(!dataAt(&session, 0, Message::MessageContent).toString().isEmpty());
    QCOMPARE(dataAt(&session, 0, Message::MessageActions).toList().size(), 3);
    QCOMPARE(session.roleNames().value(Message::MessageContent), QByteArray("content"));
    QVERIFY(!session.data(QModelIndex(), Message::MessageContent).isValid());
    QVERIFY(!session.data(session.index(0, 0), -1).isValid());

    const QVariantList tipActions = dataAt(&session, 0, Message::MessageActions).toList();
    QCOMPARE(tipActions.size(), 3);
    auto *tipAction1 = tipActions.at(0).value<MessageAction *>();
    auto *tipAction2 = tipActions.at(1).value<MessageAction *>();
    auto *tipAction3 = tipActions.at(2).value<MessageAction *>();
    QVERIFY(tipAction1 != nullptr);
    QVERIFY(tipAction2 != nullptr);
    QVERIFY(tipAction3 != nullptr);
    tipAction1->exec();
    tipAction2->exec();
    tipAction3->exec();
    QCOMPARE(session.rowCount(QModelIndex()), 1);

    QSignalSpy readySpy(&session, &Session::sessionIsReadyChanged);
    session.openChatSession();
    QCOMPARE(session.sessionIsReady(), true);
    QCOMPARE(readySpy.count(), 1);

    QSignalSpy insertSpy(&session, &QAbstractItemModel::rowsInserted);
    QSignalSpy changedSpy(&session, &QAbstractItemModel::dataChanged);
    QSignalSpy responseSpy(&session, &Session::responseIsReadyChanged);
    session.talk("<hello>");
    QCOMPARE(session.responseIsReady(), true);
    QCOMPARE(responseSpy.count(), 2);
    QCOMPARE(insertSpy.count(), 3);
    QCOMPARE(session.rowCount(QModelIndex()), 4);
    QVERIFY(session.lastTalkTime().isValid());
    QCOMPARE(dataAt(&session, 2, Message::MessageType).value<Message::Type>(), Message::SentMessage);
    QCOMPARE(dataAt(&session, 2, Message::MessageContent).toString(), QString("&lt;hello&gt;"));
    QCOMPARE(dataAt(&session, 3, Message::MessageType).value<Message::Type>(), Message::ReceivedMessage);
    QCOMPARE(dataAt(&session, 3, Message::MessageContent).toString(), QString("response: &lt;hello&gt;"));
    QCOMPARE(changedSpy.count(), 1);

    const int rowCountBeforeTipAction = session.rowCount(QModelIndex());
    tipAction2->exec();
    QCOMPARE(session.rowCount(QModelIndex()), rowCountBeforeTipAction + 2);
    QCOMPARE(dataAt(&session, session.rowCount(QModelIndex()) - 2, Message::MessageContent).toString(),
             tipAction2->text());
    tipAction3->exec();
    QCOMPARE(session.rowCount(QModelIndex()), rowCountBeforeTipAction + 4);

    const int rowCountBeforeSecondTalk = session.rowCount(QModelIndex());
    session.talk("again");
    QCOMPARE(session.rowCount(QModelIndex()), rowCountBeforeSecondTalk + 2);
    QCOMPARE(dataAt(&session, session.rowCount(QModelIndex()) - 2, Message::MessageType).value<Message::Type>(),
             Message::SentMessage);
    QCOMPARE(dataAt(&session, session.rowCount(QModelIndex()) - 1, Message::MessageType).value<Message::Type>(),
             Message::ReceivedMessage);

    session.onResponse("standalone response");
    QCOMPARE(dataAt(&session, session.rowCount(QModelIndex()) - 1, Message::MessageType).value<Message::Type>(),
             Message::ReceivedMessage);
    QCOMPARE(dataAt(&session, session.rowCount(QModelIndex()) - 1, Message::MessageContent).toString(),
             QString("standalone response"));

    session.exportSession();

    session.setSessionIsReady(true);
    session.setResponseIsReady(true);
    skipNextChatStubTalkResponseForTest();
    const int rowCountBeforeLoadTipError = session.rowCount(QModelIndex());
    session.talk("pending");
    QCOMPARE(dataAt(&session, session.rowCount(QModelIndex()) - 1, Message::MessageType).value<Message::Type>(),
             Message::LoadTipMessage);
    session.onError("pending error");
    QCOMPARE(session.rowCount(QModelIndex()), rowCountBeforeLoadTipError + 2);
    QCOMPARE(dataAt(&session, session.rowCount(QModelIndex()) - 1, Message::MessageType).value<Message::Type>(),
             Message::ErrorMessage);
    QVERIFY(dataAt(&session, session.rowCount(QModelIndex()) - 1, Message::MessageActions).toList().isEmpty());

    const int rowCountBeforeInvalidTalk = session.rowCount(QModelIndex());
    session.talk(QString());
    session.exitSession();
    session.talk("ignored");
    QCOMPARE(session.rowCount(QModelIndex()), rowCountBeforeInvalidTalk);
    QCOMPARE(session.sessionIsReady(), false);
    QCOMPARE(session.responseIsReady(), false);

    session.onError("network");
    QCOMPARE(session.responseIsReady(), true);
    QCOMPARE(dataAt(&session, session.rowCount(QModelIndex()) - 1, Message::MessageType).value<Message::Type>(),
             Message::ErrorMessage);
    QVERIFY(dataAt(&session, session.rowCount(QModelIndex()) - 1, Message::MessageContent).toString().contains("network"));
    QCOMPARE(dataAt(&session, session.rowCount(QModelIndex()) - 1, Message::MessageActions).toList().size(), 1);
    const QVariantList retryActions = dataAt(&session, session.rowCount(QModelIndex()) - 1, Message::MessageActions).toList();
    auto *retryAction = retryActions.first().value<MessageAction *>();
    QVERIFY(retryAction != nullptr);
    retryAction->exec();
    QCOMPARE(session.sessionIsReady(), true);
    readySpy.clear();
    session.openChatSession();
    QCOMPARE(readySpy.count(), 0);

    session.setSessionIsReady(true);
    session.setResponseIsReady(false);
    const int rowCountBeforeWaitingTalk = session.rowCount(QModelIndex());
    session.talk("waiting");
    QCOMPARE(session.rowCount(QModelIndex()), rowCountBeforeWaitingTalk);

    session.setSessionIsReady(true);
    session.setResponseIsReady(false);
    const int rowCountBeforeBlockedClear = session.rowCount(QModelIndex());
    session.clearSession();
    QCOMPARE(session.rowCount(QModelIndex()), rowCountBeforeBlockedClear);

    session.setSessionIsReady(true);
    session.setResponseIsReady(true);
    session.clearSession();
    QCOMPARE(session.rowCount(QModelIndex()), 1);
    QVERIFY(!session.lastTalkTime().isValid());

    session.onReadyToChat();
    session.onError("ready error");
    QCOMPARE(dataAt(&session, session.rowCount(QModelIndex()) - 1, Message::MessageType).value<Message::Type>(),
             Message::ErrorMessage);
    QVERIFY(dataAt(&session, session.rowCount(QModelIndex()) - 1, Message::MessageActions).toList().isEmpty());

    session.setSessionIsReady(true);
    session.setResponseIsReady(true);
    emitExpiredOnNextChatStubTalkForTest();
    session.talk("expired");
    QCOMPARE(session.sessionIsReady(), true);

    session.setResponseIsReady(true);
    session.m_lastTalkTime = QDateTime::currentDateTime().addSecs(-10 * 60);
    const int rowCountBeforeOldTalk = session.rowCount(QModelIndex());
    session.talk("after old time");
    QCOMPARE(session.rowCount(QModelIndex()), rowCountBeforeOldTalk + 3);

    Session emptySession(8, "key", "secret");
    emptySession.m_messages.clear();
    emptySession.onResponse("response without pending message");
    QCOMPARE(emptySession.rowCount(QModelIndex()), 1);
    QCOMPARE(dataAt(&emptySession, 0, Message::MessageType).value<Message::Type>(), Message::ReceivedMessage);
    emptySession.m_messages.clear();
    emptySession.onError("error without pending message");
    QCOMPARE(emptySession.rowCount(QModelIndex()), 1);
    QCOMPARE(dataAt(&emptySession, 0, Message::MessageType).value<Message::Type>(), Message::ErrorMessage);
}

QTEST_MAIN(session_test)

#include "session-test.moc"
