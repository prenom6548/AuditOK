#include "../src/chat.h"

namespace {

bool g_emitErrorForTest = false;
QString g_errorForTest;
bool g_skipNextTalkResponseForTest = false;
bool g_emitExpiredOnNextTalkForTest = false;

}

void setChatStubErrorForTest(const QString &error)
{
    g_emitErrorForTest = true;
    g_errorForTest = error;
}

void skipNextChatStubTalkResponseForTest()
{
    g_skipNextTalkResponseForTest = true;
}

void emitExpiredOnNextChatStubTalkForTest()
{
    g_emitExpiredOnNextTalkForTest = true;
}

Chat::Chat(QObject *parent) : QObject(parent)
{
}

void Chat::newChat(const QString &clientId, const QString &clientSecret)
{
    Q_UNUSED(clientId)
    Q_UNUSED(clientSecret)

    if (g_emitErrorForTest) {
        g_emitErrorForTest = false;
        Q_EMIT error(g_errorForTest);
        return;
    }

    Q_EMIT readyToChat();
}

void Chat::clear()
{
}

bool Chat::isValid() const
{
    return true;
}

void Chat::talk(const QString &content)
{
    if (g_emitExpiredOnNextTalkForTest) {
        g_emitExpiredOnNextTalkForTest = false;
        Q_EMIT expired();
        return;
    }

    if (g_skipNextTalkResponseForTest) {
        g_skipNextTalkResponseForTest = false;
        return;
    }

    Q_EMIT response(QString("response: %1").arg(content));
}
