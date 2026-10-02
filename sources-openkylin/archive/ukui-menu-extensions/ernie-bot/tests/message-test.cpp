#include <QtTest>

#include "../src/message.h"
#include "../src/message-action.h"

class message_test : public QObject
{
    Q_OBJECT

private slots:
    void test_Message();
};

void message_test::test_Message()
{
    Message message(Message::SentMessage, "hello");
    QCOMPARE(message.type(), Message::SentMessage);
    QCOMPARE(message.content(), QString("hello"));
    QCOMPARE(message.header(), QString());

    message.setType(Message::ReceivedMessage);
    message.setContent("answer");
    message.setHeader("header");
    QCOMPARE(message.type(), Message::ReceivedMessage);
    QCOMPARE(message.content(), QString("answer"));
    QCOMPARE(message.header(), QString("header"));

    auto action = new MessageAction("text", "name", &message);
    message.setAction({action});
    QCOMPARE(message.actions().size(), 1);
    QCOMPARE(message.actions().first(), action);

    const QHash<int, QByteArray> roles = Message::roleNames();
    QCOMPARE(roles.value(Message::MessageType), QByteArray("type"));
    QCOMPARE(roles.value(Message::MessageHeader), QByteArray("header"));
    QCOMPARE(roles.value(Message::MessageContent), QByteArray("content"));
    QCOMPARE(roles.value(Message::MessageActions), QByteArray("actions"));
}

QTEST_MAIN(message_test)

#include "message-test.moc"
