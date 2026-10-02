#include <QtTest>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QSignalSpy>
#include <QTimer>

#include "../src/chat.h"

namespace {

class FakeNetworkReply : public QNetworkReply
{
    Q_OBJECT

public:
    FakeNetworkReply(const QNetworkRequest &request,
                     const QByteArray &payload,
                     QNetworkReply::NetworkError errorCode = QNetworkReply::NoError,
                     const QString &errorMessage = {},
                     QObject *parent = nullptr)
        : QNetworkReply(parent), m_payload(payload)
    {
        setRequest(request);
        setUrl(request.url());
        open(QIODevice::ReadOnly | QIODevice::Unbuffered);

        if (errorCode != QNetworkReply::NoError) {
            setError(errorCode, errorMessage);
        }

        setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

        QTimer::singleShot(0, this, [this] {
            Q_EMIT readyRead();
            Q_EMIT finished();
        });
    }

    void abort() override
    {
    }

    qint64 bytesAvailable() const override
    {
        return m_payload.size() - m_offset + QIODevice::bytesAvailable();
    }

protected:
    qint64 readData(char *data, qint64 maxSize) override
    {
        if (m_offset >= m_payload.size()) {
            return -1;
        }

        const qint64 size = qMin(maxSize, m_payload.size() - m_offset);
        memcpy(data, m_payload.constData() + m_offset, static_cast<size_t>(size));
        m_offset += size;
        return size;
    }

private:
    QByteArray m_payload;
    qint64 m_offset = 0;
};

class FakeNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT

public:
    explicit FakeNetworkAccessManager(QObject *parent = nullptr) : QNetworkAccessManager(parent)
    {
    }

    struct ResponseSpec
    {
        ResponseSpec() = default;
        ResponseSpec(QByteArray payloadValue,
                     QNetworkReply::NetworkError errorCodeValue = QNetworkReply::NoError,
                     QString errorMessageValue = {})
            : payload(std::move(payloadValue)),
              errorCode(errorCodeValue),
              errorMessage(std::move(errorMessageValue))
        {
        }

        QByteArray payload;
        QNetworkReply::NetworkError errorCode = QNetworkReply::NoError;
        QString errorMessage;
    };

    QList<QNetworkRequest> requests;
    QList<QByteArray> bodies;
    QList<ResponseSpec> responseSpecs;

protected:
    QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData) override
    {
        Q_UNUSED(op);

        requests.append(request);
        bodies.append(outgoingData ? outgoingData->readAll() : QByteArray());

        ResponseSpec spec;
        if (!responseSpecs.isEmpty()) {
            spec = responseSpecs.takeFirst();
        }

        auto *reply = new FakeNetworkReply(request, spec.payload, spec.errorCode, spec.errorMessage, this);
        return reply;
    }
};

QByteArray tokenPayload(const QString &token = "token", int expiresIn = 3600)
{
    return QJsonDocument(QJsonObject{
        {"access_token", token},
        {"expires_in", expiresIn}
    }).toJson(QJsonDocument::Compact);
}

QByteArray tokenErrorPayload(const QString &error = "invalid_client", const QString &description = "bad key")
{
    return QJsonDocument(QJsonObject{
        {"error", error},
        {"error_description", description}
    }).toJson(QJsonDocument::Compact);
}

QByteArray responsePayload(const QString &result = "answer")
{
    return QJsonDocument(QJsonObject{
        {"result", result}
    }).toJson(QJsonDocument::Compact);
}

QByteArray responseErrorPayload(const QString &message = "request failed")
{
    return QJsonDocument(QJsonObject{
        {"error_code", "500"},
        {"error_msg", message}
    }).toJson(QJsonDocument::Compact);
}

QJsonArray requestMessages(const QByteArray &body)
{
    return QJsonDocument::fromJson(body).object().value("messages").toArray();
}

}

class chat_test : public QObject
{
    Q_OBJECT

private slots:
    void test_NewChatSuccess();
    void test_NewChatErrorPaths();
    void test_NewChatWhileWaiting();
    void test_TalkStateAndExpired();
    void test_TalkResponsePaths();
    void test_Clear();
    void test_SetNetworkAccessManagerForTest();
};

void chat_test::test_NewChatSuccess()
{
    Chat chat;
    auto *networkManager = new FakeNetworkAccessManager(&chat);
    networkManager->responseSpecs.append(FakeNetworkAccessManager::ResponseSpec(tokenPayload("access-token", 120)));
    chat.setNetworkAccessManagerForTest(networkManager);

    QSignalSpy readySpy(&chat, &Chat::readyToChat);
    QSignalSpy errorSpy(&chat, &Chat::error);

    chat.newChat("client-id", "client-secret");

    QTRY_COMPARE(readySpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(networkManager->requests.size(), 1);
    QVERIFY(networkManager->requests.first().url().toString().contains("client_id=client-id"));
    QVERIFY(networkManager->requests.first().url().toString().contains("client_secret=client-secret"));
    QVERIFY(!chat.waitingForTokenForTest());
    QVERIFY(chat.isValid());
}

void chat_test::test_NewChatErrorPaths()
{
    {
        Chat chat;
        auto *networkManager = new FakeNetworkAccessManager(&chat);
        networkManager->responseSpecs.append(
            FakeNetworkAccessManager::ResponseSpec(QByteArray(), QNetworkReply::ConnectionRefusedError, "network down"));
        chat.setNetworkAccessManagerForTest(networkManager);

        QSignalSpy errorSpy(&chat, &Chat::error);
        chat.newChat("id", "secret");

        QTRY_COMPARE(errorSpy.count(), 1);
        QCOMPARE(errorSpy.takeFirst().at(0).toString(), QString("network down"));
        QVERIFY(!chat.waitingForTokenForTest());
    }

    {
        Chat chat;
        auto *networkManager = new FakeNetworkAccessManager(&chat);
        networkManager->responseSpecs.append(
            FakeNetworkAccessManager::ResponseSpec(QByteArray("{"), QNetworkReply::NoError, QString()));
        chat.setNetworkAccessManagerForTest(networkManager);

        QSignalSpy errorSpy(&chat, &Chat::error);
        chat.newChat("id", "secret");

        QTRY_COMPARE(errorSpy.count(), 1);
        QVERIFY(!errorSpy.takeFirst().at(0).toString().isEmpty());
        QVERIFY(!chat.waitingForTokenForTest());
    }

    {
        Chat chat;
        auto *networkManager = new FakeNetworkAccessManager(&chat);
        networkManager->responseSpecs.append(
            FakeNetworkAccessManager::ResponseSpec(tokenErrorPayload("invalid_client", "bad key")));
        chat.setNetworkAccessManagerForTest(networkManager);

        QSignalSpy errorSpy(&chat, &Chat::error);
        chat.newChat("id", "secret");

        QTRY_COMPARE(errorSpy.count(), 1);
        QCOMPARE(errorSpy.takeFirst().at(0).toString(), QString("bad key"));
        QVERIFY(!chat.waitingForTokenForTest());
        QVERIFY(chat.waitingForTokenForTest() == false);
    }
}

void chat_test::test_NewChatWhileWaiting()
{
    Chat chat;
    auto *networkManager = new FakeNetworkAccessManager(&chat);
    networkManager->responseSpecs.append(FakeNetworkAccessManager::ResponseSpec(tokenPayload("token", 60)));
    chat.setNetworkAccessManagerForTest(networkManager);

    chat.newChat("first", "secret");
    QVERIFY(chat.waitingForTokenForTest());

    chat.newChat("second", "secret");
    QCOMPARE(networkManager->requests.size(), 1);

    QTRY_VERIFY(!chat.waitingForTokenForTest());
}

void chat_test::test_TalkStateAndExpired()
{
    {
        Chat chat;
        auto *networkManager = new FakeNetworkAccessManager(&chat);
        chat.setNetworkAccessManagerForTest(networkManager);
        chat.setTokenStateForTest("token", QDateTime::currentDateTime().addSecs(-120), 10);

        QSignalSpy expiredSpy(&chat, &Chat::expired);
        chat.talk("hello");

        QCOMPARE(expiredSpy.count(), 1);
        QCOMPARE(networkManager->requests.size(), 0);
        QCOMPARE(chat.cacheSizeForTest(), 0);
        QVERIFY(!chat.waitingForResponseForTest());
    }

    {
        Chat chat;
        auto *networkManager = new FakeNetworkAccessManager(&chat);
        networkManager->responseSpecs.append(FakeNetworkAccessManager::ResponseSpec(responsePayload("answer")));
        chat.setNetworkAccessManagerForTest(networkManager);
        chat.setTokenStateForTest("token", QDateTime::currentDateTime(), 3600);

        chat.talk("first");
        QVERIFY(chat.waitingForResponseForTest());
        QCOMPARE(networkManager->requests.size(), 1);

        chat.talk("second");
        QCOMPARE(networkManager->requests.size(), 1);

        QTRY_VERIFY(!chat.waitingForResponseForTest());
    }
}

void chat_test::test_TalkResponsePaths()
{
    {
        Chat chat;
        auto *networkManager = new FakeNetworkAccessManager(&chat);
        networkManager->responseSpecs.append(
            FakeNetworkAccessManager::ResponseSpec(responsePayload("server answer")));
        chat.setNetworkAccessManagerForTest(networkManager);
        chat.setTokenStateForTest("token", QDateTime::currentDateTime(), 3600);

        QSignalSpy responseSpy(&chat, &Chat::response);
        QSignalSpy errorSpy(&chat, &Chat::error);

        chat.talk("question");

        QTRY_COMPARE(responseSpy.count(), 1);
        QCOMPARE(errorSpy.count(), 0);
        QCOMPARE(responseSpy.takeFirst().at(0).toString(), QString("server answer"));
        QCOMPARE(networkManager->requests.size(), 1);

        const QJsonArray messages = requestMessages(networkManager->bodies.first());
        QCOMPARE(messages.size(), 1);
        QCOMPARE(messages.first().toObject().value("content").toString(), QString("question"));
        QCOMPARE(messages.first().toObject().value("role").toString(), QString("user"));
        QCOMPARE(chat.cacheSizeForTest(), 2);
        QVERIFY(!chat.waitingForResponseForTest());
    }

    {
        Chat chat;
        auto *networkManager = new FakeNetworkAccessManager(&chat);
        networkManager->responseSpecs.append(
            FakeNetworkAccessManager::ResponseSpec(QByteArray(), QNetworkReply::TimeoutError, "timeout"));
        chat.setNetworkAccessManagerForTest(networkManager);
        chat.setTokenStateForTest("token", QDateTime::currentDateTime(), 3600);

        QSignalSpy errorSpy(&chat, &Chat::error);
        chat.talk("question");

        QTRY_COMPARE(errorSpy.count(), 1);
        QCOMPARE(errorSpy.takeFirst().at(0).toString(), QString("timeout"));
        QCOMPARE(chat.cacheSizeForTest(), 0);
        QVERIFY(!chat.waitingForResponseForTest());
    }

    {
        Chat chat;
        auto *networkManager = new FakeNetworkAccessManager(&chat);
        networkManager->responseSpecs.append(
            FakeNetworkAccessManager::ResponseSpec(responseErrorPayload("bad request")));
        chat.setNetworkAccessManagerForTest(networkManager);
        chat.setTokenStateForTest("token", QDateTime::currentDateTime(), 3600);

        QSignalSpy errorSpy(&chat, &Chat::error);
        chat.talk("question");

        QTRY_COMPARE(errorSpy.count(), 1);
        QCOMPARE(errorSpy.takeFirst().at(0).toString(), QString("bad request"));
        QCOMPARE(chat.cacheSizeForTest(), 0);
        QVERIFY(!chat.waitingForResponseForTest());
    }

    {
        Chat chat;
        auto *networkManager = new FakeNetworkAccessManager(&chat);
        networkManager->responseSpecs.append(FakeNetworkAccessManager::ResponseSpec(QByteArray("{")));
        chat.setNetworkAccessManagerForTest(networkManager);
        chat.setTokenStateForTest("token", QDateTime::currentDateTime(), 3600);

        QSignalSpy errorSpy(&chat, &Chat::error);
        chat.talk("question");

        QTRY_COMPARE(errorSpy.count(), 1);
        QVERIFY(!errorSpy.takeFirst().at(0).toString().isEmpty());
        QCOMPARE(chat.cacheSizeForTest(), 0);
        QVERIFY(!chat.waitingForResponseForTest());
    }
}

void chat_test::test_Clear()
{
    Chat chat;
    auto *networkManager = new FakeNetworkAccessManager(&chat);
    networkManager->responseSpecs.append(FakeNetworkAccessManager::ResponseSpec(responsePayload("answer")));
    chat.setNetworkAccessManagerForTest(networkManager);
    chat.setTokenStateForTest("token", QDateTime::currentDateTime(), 3600);

    chat.talk("question");
    QTRY_VERIFY(!chat.waitingForResponseForTest());
    QCOMPARE(chat.cacheSizeForTest(), 2);

    chat.clear();
    QCOMPARE(chat.cacheSizeForTest(), 0);
}

void chat_test::test_SetNetworkAccessManagerForTest()
{
    Chat chat;
    auto *networkManager = new FakeNetworkAccessManager(&chat);
    networkManager->responseSpecs.append(FakeNetworkAccessManager::ResponseSpec(responsePayload("parented")));

    chat.setNetworkAccessManagerForTest(nullptr);
    chat.setNetworkAccessManagerForTest(networkManager);
    chat.setNetworkAccessManagerForTest(networkManager);
    chat.setTokenStateForTest("token", QDateTime::currentDateTime(), 3600);

    QSignalSpy responseSpy(&chat, &Chat::response);
    chat.talk("question");

    QTRY_COMPARE(responseSpy.count(), 1);
    QCOMPARE(responseSpy.takeFirst().at(0).toString(), QString("parented"));
    QCOMPARE(networkManager->requests.size(), 1);

    auto *unparentedNetworkManager = new FakeNetworkAccessManager();
    unparentedNetworkManager->responseSpecs.append(FakeNetworkAccessManager::ResponseSpec(responsePayload("unparented")));
    chat.setNetworkAccessManagerForTest(unparentedNetworkManager);

    QSignalSpy secondResponseSpy(&chat, &Chat::response);
    chat.talk("another question");

    QTRY_COMPARE(secondResponseSpy.count(), 1);
    QCOMPARE(secondResponseSpy.takeFirst().at(0).toString(), QString("unparented"));
    QCOMPARE(unparentedNetworkManager->parent(), &chat);
    QCOMPARE(unparentedNetworkManager->requests.size(), 1);
}

QTEST_MAIN(chat_test)

#include "chat-test.moc"
