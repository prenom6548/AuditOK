#ifndef SESSION_H
#define SESSION_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QUuid>
#include <QVariant>

class SessionMessage
{
    Q_GADGET
    Q_PROPERTY(qint64 id READ id)
    Q_PROPERTY(QString role READ role)
    Q_PROPERTY(QString content READ content)
    Q_PROPERTY(qint64 timestamp READ timestamp)

public:
    SessionMessage() = default;
    SessionMessage(qint64 id, const QString &role, const QString &content, qint64 timestamp)
        : m_id(id), m_role(role), m_content(content), m_timestamp(timestamp) {}

    qint64 id() const { return m_id; }
    QString role() const { return m_role; }
    QString content() const { return m_content; }
    qint64 timestamp() const { return m_timestamp; }

    void setId(qint64 id) { m_id = id; }
    void setRole(const QString &role) { m_role = role; }
    void setContent(const QString &content) { m_content = content; }
    void setTimestamp(qint64 timestamp) { m_timestamp = timestamp; }

private:
    qint64 m_id = 0;
    QString m_role;
    QString m_content;
    qint64 m_timestamp = 0;
};

class SessionSummary
{
    Q_GADGET
    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(QString source READ source)
    Q_PROPERTY(qint64 startedAt READ startedAt)
    Q_PROPERTY(qint64 endedAt READ endedAt)
    Q_PROPERTY(int messageCount READ messageCount)
    Q_PROPERTY(QString model READ model)
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QString preview READ preview)

public:
    SessionSummary() = default;
    SessionSummary(const QString &id, const QString &source, qint64 startedAt,
                   qint64 endedAt, int messageCount, const QString &model,
                   const QString &title, const QString &preview)
        : m_id(id), m_source(source), m_startedAt(startedAt), m_endedAt(endedAt),
          m_messageCount(messageCount), m_model(model), m_title(title), m_preview(preview) {}

    QString id() const { return m_id; }
    QString source() const { return m_source; }
    qint64 startedAt() const { return m_startedAt; }
    qint64 endedAt() const { return m_endedAt; }
    int messageCount() const { return m_messageCount; }
    QString model() const { return m_model; }
    QString title() const { return m_title; }
    QString preview() const { return m_preview; }

    void setId(const QString &id) { m_id = id; }
    void setSource(const QString &source) { m_source = source; }
    void setStartedAt(qint64 startedAt) { m_startedAt = startedAt; }
    void setEndedAt(qint64 endedAt) { m_endedAt = endedAt; }
    void setMessageCount(int count) { m_messageCount = count; }
    void setModel(const QString &model) { m_model = model; }
    void setTitle(const QString &title) { m_title = title; }
    void setPreview(const QString &preview) { m_preview = preview; }

private:
    QString m_id;
    QString m_source;
    qint64 m_startedAt = 0;
    qint64 m_endedAt = 0;
    int m_messageCount = 0;
    QString m_model;
    QString m_title;
    QString m_preview;
};

class SearchResult
{
    Q_GADGET
    Q_PROPERTY(QString sessionId READ sessionId)
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(qint64 startedAt READ startedAt)
    Q_PROPERTY(QString source READ source)
    Q_PROPERTY(int messageCount READ messageCount)
    Q_PROPERTY(QString model READ model)
    Q_PROPERTY(QString snippet READ snippet)

public:
    SearchResult() = default;
    
    QString sessionId() const { return m_sessionId; }
    QString title() const { return m_title; }
    qint64 startedAt() const { return m_startedAt; }
    QString source() const { return m_source; }
    int messageCount() const { return m_messageCount; }
    QString model() const { return m_model; }
    QString snippet() const { return m_snippet; }

    void setSessionId(const QString &id) { m_sessionId = id; }
    void setTitle(const QString &title) { m_title = title; }
    void setStartedAt(qint64 startedAt) { m_startedAt = startedAt; }
    void setSource(const QString &source) { m_source = source; }
    void setMessageCount(int count) { m_messageCount = count; }
    void setModel(const QString &model) { m_model = model; }
    void setSnippet(const QString &snippet) { m_snippet = snippet; }

private:
    QString m_sessionId;
    QString m_title;
    qint64 m_startedAt = 0;
    QString m_source;
    int m_messageCount = 0;
    QString m_model;
    QString m_snippet;
};

Q_DECLARE_METATYPE(SessionMessage)
Q_DECLARE_METATYPE(SessionSummary)
Q_DECLARE_METATYPE(SearchResult)

#endif // SESSION_H
