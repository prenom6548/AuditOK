#ifndef TOKENIZERSERVICE_H
#define TOKENIZERSERVICE_H

#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QJsonArray>

#include <memory>

struct ChatMessage;

class TokenizerService
{
public:
    static TokenizerService &instance();

    bool isAvailable() const;
    QString tokenizerPath() const;
    long long countText(const QString &text);
    long long countQwenChat(const QVector<ChatMessage> &messages,
                            const QString &pendingUserMessage = QString());
    long long countQwenChatMessages(const QJsonArray &messages);

private:
    TokenizerService();
    bool ensureLoaded();
    bool loadTokenizer(const QString &path);
    QStringList splitSpecialTokens(const QString &text) const;
    long long countOrdinaryText(const QString &text);
    int countBpeToken(const QString &token);

    QString m_tokenizerPath;
    bool m_loadAttempted = false;
    bool m_loaded = false;
    QHash<QString, int> m_vocab;
    QHash<QString, int> m_mergeRanks;
    QHash<QString, int> m_bpeCountCache;
    QStringList m_specialTokens;
    QHash<uint, QChar> m_byteEncoder;
};

#endif // TOKENIZERSERVICE_H
