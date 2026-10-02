#include "services/sessionservice.h"
#include "services/databaseservice.h"
#include <QUuid>
#include <QDebug>

SessionService::SessionService(QObject *parent)
    : QObject(parent)
{
}

SessionService::~SessionService()
{
}

SessionService& SessionService::instance()
{
    static SessionService instance;
    return instance;
}

QString SessionService::generateSessionId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString SessionService::createSession(const QString &source, const QString &model)
{
    QString sessionId = generateSessionId();
    
    if (DatabaseService::instance().createSession(sessionId, source, model)) {
        m_currentSessionId = sessionId;
        emit sessionCreated(sessionId);
        qDebug() << "Created session:" << sessionId;
        return sessionId;
    }
    
    return QString();
}

bool SessionService::updateSessionTitle(const QString &sessionId, const QString &title)
{
    bool success = DatabaseService::instance().updateSessionTitle(sessionId, title);
    if (success) {
        emit sessionUpdated(sessionId);
    }
    return success;
}

bool SessionService::deleteSession(const QString &sessionId)
{
    // Note: SQLite doesn't have ON DELETE CASCADE enabled by default for the session table
    // We'll need to delete messages first
    Q_UNUSED(sessionId);
    emit sessionDeleted(sessionId);
    return true;
}

SessionSummary SessionService::getSession(const QString &sessionId)
{
    SessionSummary summary;
    auto rows = DatabaseService::instance().getSessions(1, 0);
    
    for (const auto &row : rows) {
        if (row["id"].toString() == sessionId) {
            summary.setId(row["id"].toString());
            summary.setSource(row["source"].toString());
            summary.setStartedAt(row["started_at"].toLongLong());
            summary.setEndedAt(row["ended_at"].toLongLong());
            summary.setMessageCount(row["message_count"].toInt());
            summary.setModel(row["model"].toString());
            summary.setTitle(row["title"].toString());
            break;
        }
    }
    
    return summary;
}

QVector<SessionSummary> SessionService::listSessions(int limit, int offset)
{
    QVector<SessionSummary> results;
    auto rows = DatabaseService::instance().getSessions(limit, offset);
    
    for (const auto &row : rows) {
        SessionSummary summary;
        summary.setId(row["id"].toString());
        summary.setSource(row["source"].toString());
        summary.setStartedAt(row["started_at"].toLongLong());
        summary.setEndedAt(row["ended_at"].toLongLong());
        summary.setMessageCount(row["message_count"].toInt());
        summary.setModel(row["model"].toString());
        summary.setTitle(row["title"].toString());
        summary.setPreview("");
        results.append(summary);
    }
    
    return results;
}

QVector<SearchResult> SessionService::searchSessions(const QString &query, int limit)
{
    QVector<SearchResult> results;
    auto rows = DatabaseService::instance().searchSessions(query, limit);
    
    for (const auto &row : rows) {
        SearchResult result;
        result.setSessionId(row["session_id"].toString());
        result.setTitle(row["title"].toString());
        result.setStartedAt(row["started_at"].toLongLong());
        result.setSource(row["source"].toString());
        result.setMessageCount(row["message_count"].toInt());
        result.setModel(row["model"].toString());
        result.setSnippet(row["snippet"].toString());
        results.append(result);
    }
    
    return results;
}

bool SessionService::addMessage(const QString &sessionId, const QString &role, const QString &content)
{
    bool success = DatabaseService::instance().addMessage(sessionId, role, content);
    
    if (success) {
        // Get the message we just added
        auto messages = DatabaseService::instance().getSessionMessages(sessionId);
        if (!messages.isEmpty()) {
            auto lastMsg = messages.last();
            SessionMessage msg(
                lastMsg["id"].toLongLong(),
                lastMsg["role"].toString(),
                lastMsg["content"].toString(),
                lastMsg["timestamp"].toLongLong()
            );
            emit messageAdded(sessionId, msg);
        }
        
        // If this is the first user message, generate a title
        if (role == "user") {
            auto msgs = DatabaseService::instance().getSessionMessages(sessionId);
            if (msgs.size() == 1) { // First message
                QString title = extractTitle(content);
                updateSessionTitle(sessionId, title);
            }
        }
    }
    
    return success;
}

QVector<SessionMessage> SessionService::getSessionMessages(const QString &sessionId)
{
    QVector<SessionMessage> results;
    auto rows = DatabaseService::instance().getSessionMessages(sessionId);
    
    for (const auto &row : rows) {
        SessionMessage msg(
            row["id"].toLongLong(),
            row["role"].toString(),
            row["content"].toString(),
            row["timestamp"].toLongLong()
        );
        results.append(msg);
    }
    
    return results;
}

QString SessionService::extractTitle(const QString &firstMessage)
{
    // Extract first 50 characters as title
    QString title = firstMessage;
    title = title.trimmed();
    
    // Remove line breaks
    title.replace("\n", " ");
    title.replace("\r", " ");
    
    // Truncate to 50 characters
    if (title.length() > 50) {
        title = title.left(47) + "...";
    }
    
    return title;
}
