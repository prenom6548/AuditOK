#include "services/tokenizerservice.h"
#include "ui/chatwidget.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>

#include <algorithm>
#include <climits>

namespace {

QString defaultTokenizerPath()
{
    const QString envPath = QString::fromLocal8Bit(qgetenv("KYLIN_AGENT_TOKENIZER_PATH")).trimmed();
    if (!envPath.isEmpty()) {
        return envPath;
    }

    const QString relative = QStringLiteral("Qwen3.6-27B-tokenizer/tokenizer.json");
    const QStringList roots = {
        QDir::currentPath(),
        QCoreApplication::applicationDirPath(),
        QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(QStringLiteral("..")),
        QStringLiteral("/usr/share/kylin-agent"),
#ifdef KYLIN_AGENT_SOURCE_DIR
        QStringLiteral(KYLIN_AGENT_SOURCE_DIR),
#endif
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
    };
    for (const QString &root : roots) {
        if (root.trimmed().isEmpty()) {
            continue;
        }
        const QString candidate = QDir(root).absoluteFilePath(relative);
        if (QFile::exists(candidate)) {
            return candidate;
        }
    }
    return QDir::current().absoluteFilePath(relative);
}

QHash<uint, QChar> buildByteEncoder()
{
    QList<uint> bytes;
    for (uint b = '!'; b <= '~'; ++b) bytes.append(b);
    for (uint b = 0xA1; b <= 0xAC; ++b) bytes.append(b);
    for (uint b = 0xAE; b <= 0xFF; ++b) bytes.append(b);

    QSet<uint> visible(bytes.begin(), bytes.end());
    QHash<uint, QChar> encoder;
    uint extra = 0;
    for (uint b = 0; b < 256; ++b) {
        if (visible.contains(b)) {
            encoder.insert(b, QChar(static_cast<ushort>(b)));
        } else {
            encoder.insert(b, QChar(static_cast<ushort>(256 + extra)));
            ++extra;
        }
    }
    return encoder;
}

QString byteLevelToken(const QString &piece, const QHash<uint, QChar> &encoder)
{
    QString encoded;
    const QByteArray bytes = piece.toUtf8();
    encoded.reserve(bytes.size());
    for (unsigned char byte : bytes) {
        encoded.append(encoder.value(byte));
    }
    return encoded;
}

QString pairKey(const QString &left, const QString &right)
{
    return left + QChar(0x0001) + right;
}

} // namespace

TokenizerService &TokenizerService::instance()
{
    static TokenizerService service;
    return service;
}

TokenizerService::TokenizerService()
    : m_tokenizerPath(defaultTokenizerPath())
    , m_byteEncoder(buildByteEncoder())
{
}

bool TokenizerService::isAvailable() const
{
    return m_loaded || QFile::exists(m_tokenizerPath);
}

QString TokenizerService::tokenizerPath() const
{
    return m_tokenizerPath;
}

bool TokenizerService::ensureLoaded()
{
    if (m_loaded) {
        return true;
    }
    if (m_loadAttempted) {
        return false;
    }
    m_loadAttempted = true;
    m_loaded = loadTokenizer(m_tokenizerPath);
    return m_loaded;
}

bool TokenizerService::loadTokenizer(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return false;
    }
    const QJsonObject root = doc.object();
    const QJsonObject model = root.value(QStringLiteral("model")).toObject();
    const QJsonObject vocab = model.value(QStringLiteral("vocab")).toObject();
    const QJsonArray merges = model.value(QStringLiteral("merges")).toArray();
    if (vocab.isEmpty() || merges.isEmpty()) {
        return false;
    }

    m_vocab.clear();
    for (auto it = vocab.constBegin(); it != vocab.constEnd(); ++it) {
        m_vocab.insert(it.key(), it.value().toInt());
    }

    m_mergeRanks.clear();
    int rank = 0;
    for (const QJsonValue &value : merges) {
        const QString merge = value.toString();
        const int split = merge.indexOf(QLatin1Char(' '));
        if (split <= 0 || split >= merge.size() - 1) {
            continue;
        }
        m_mergeRanks.insert(pairKey(merge.left(split), merge.mid(split + 1)), rank++);
    }

    m_specialTokens.clear();
    const QJsonArray addedTokens = root.value(QStringLiteral("added_tokens")).toArray();
    for (const QJsonValue &value : addedTokens) {
        const QJsonObject token = value.toObject();
        if (token.value(QStringLiteral("special")).toBool(false)) {
            const QString content = token.value(QStringLiteral("content")).toString();
            if (!content.isEmpty()) {
                m_specialTokens.append(content);
            }
        }
    }
    std::sort(m_specialTokens.begin(), m_specialTokens.end(),
              [](const QString &a, const QString &b) { return a.size() > b.size(); });
    return true;
}

QStringList TokenizerService::splitSpecialTokens(const QString &text) const
{
    QStringList parts;
    int index = 0;
    while (index < text.size()) {
        int bestPos = -1;
        QString bestToken;
        for (const QString &token : m_specialTokens) {
            const int pos = text.indexOf(token, index);
            if (pos >= 0 && (bestPos < 0 || pos < bestPos ||
                             (pos == bestPos && token.size() > bestToken.size()))) {
                bestPos = pos;
                bestToken = token;
            }
        }
        if (bestPos < 0) {
            parts.append(text.mid(index));
            break;
        }
        if (bestPos > index) {
            parts.append(text.mid(index, bestPos - index));
        }
        parts.append(bestToken);
        index = bestPos + bestToken.size();
    }
    return parts;
}

long long TokenizerService::countText(const QString &text)
{
    if (text.isEmpty()) {
        return 0;
    }
    if (!ensureLoaded()) {
        return std::max(1, text.size() / 4);
    }

    long long total = 0;
    const QStringList parts = splitSpecialTokens(text);
    for (const QString &part : parts) {
        if (part.isEmpty()) {
            continue;
        }
        if (m_specialTokens.contains(part) && m_vocab.contains(part)) {
            total += 1;
        } else {
            total += countOrdinaryText(part);
        }
    }
    return total;
}

long long TokenizerService::countOrdinaryText(const QString &text)
{
    static const QRegularExpression pattern(QStringLiteral(
        "(?i:'s|'t|'re|'ve|'m|'ll|'d)|[^\\r\\n\\p{L}\\p{N}]?[\\p{L}\\p{M}]+|\\p{N}| ?[^\\s\\p{L}\\p{M}\\p{N}]+[\\r\\n]*|\\s*[\\r\\n]+|\\s+(?!\\S)|\\s+"));

    long long total = 0;
    QRegularExpressionMatchIterator it = pattern.globalMatch(text);
    while (it.hasNext()) {
        const QString piece = it.next().captured(0);
        if (piece.isEmpty()) {
            continue;
        }
        total += countBpeToken(byteLevelToken(piece, m_byteEncoder));
    }
    return total;
}

int TokenizerService::countBpeToken(const QString &token)
{
    if (token.isEmpty()) {
        return 0;
    }
    const auto cacheIt = m_bpeCountCache.constFind(token);
    if (cacheIt != m_bpeCountCache.constEnd()) {
        return cacheIt.value();
    }
    if (m_vocab.contains(token)) {
        m_bpeCountCache.insert(token, 1);
        return 1;
    }

    QStringList symbols;
    symbols.reserve(token.size());
    for (const QChar ch : token) {
        symbols.append(QString(ch));
    }

    while (symbols.size() > 1) {
        int bestRank = INT_MAX;
        int bestIndex = -1;
        for (int i = 0; i < symbols.size() - 1; ++i) {
            const int rank = m_mergeRanks.value(pairKey(symbols.at(i), symbols.at(i + 1)), INT_MAX);
            if (rank < bestRank) {
                bestRank = rank;
                bestIndex = i;
            }
        }
        if (bestIndex < 0 || bestRank == INT_MAX) {
            break;
        }
        symbols[bestIndex] += symbols.at(bestIndex + 1);
        symbols.removeAt(bestIndex + 1);
    }

    const int count = symbols.size();
    m_bpeCountCache.insert(token, count);
    return count;
}

long long TokenizerService::countQwenChat(const QVector<ChatMessage> &messages,
                                          const QString &pendingUserMessage)
{
    QString prompt;
    for (const ChatMessage &message : messages) {
        if (message.role.trimmed().isEmpty() || message.content.isEmpty()) {
            continue;
        }
        prompt += QStringLiteral("<|im_start|>") + message.role + QLatin1Char('\n') +
                  message.content + QStringLiteral("<|im_end|>\n");
    }
    if (!pendingUserMessage.trimmed().isEmpty()) {
        prompt += QStringLiteral("<|im_start|>user\n") + pendingUserMessage +
                  QStringLiteral("<|im_end|>\n");
    }
    prompt += QStringLiteral("<|im_start|>assistant\n");
    return countText(prompt);
}

long long TokenizerService::countQwenChatMessages(const QJsonArray &messages)
{
    QString prompt;
    for (const QJsonValue &value : messages) {
        const QJsonObject message = value.toObject();
        const QString role = message.value(QStringLiteral("role")).toString().trimmed();
        if (role.isEmpty()) {
            continue;
        }

        QString content;
        const QJsonValue contentValue = message.value(QStringLiteral("content"));
        if (contentValue.isString()) {
            content = contentValue.toString();
        } else if (contentValue.isArray()) {
            QStringList parts;
            for (const QJsonValue &partValue : contentValue.toArray()) {
                const QJsonObject part = partValue.toObject();
                const QString text = part.value(QStringLiteral("text")).toString();
                if (!text.isEmpty()) {
                    parts.append(text);
                }
            }
            content = parts.join(QLatin1Char('\n'));
        } else if (contentValue.isObject()) {
            content = contentValue.toObject().value(QStringLiteral("text")).toString();
        }

        prompt += QStringLiteral("<|im_start|>") + role + QLatin1Char('\n') +
                  content + QStringLiteral("<|im_end|>\n");
    }
    prompt += QStringLiteral("<|im_start|>assistant\n");
    return countText(prompt);
}
