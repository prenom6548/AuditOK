#ifndef SESSIONSERVICE_H
#define SESSIONSERVICE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include "../models/session.h"

class SessionService : public QObject
{
    Q_OBJECT

public:
    static SessionService& instance();
    
    // Session management
    QString createSession(const QString &source = "desktop", const QString &model = "");
    bool updateSessionTitle(const QString &sessionId, const QString &title);
    bool deleteSession(const QString &sessionId);
    SessionSummary getSession(const QString &sessionId);
    QVector<SessionSummary> listSessions(int limit = 30, int offset = 0);
    QVector<SearchResult> searchSessions(const QString &query, int limit = 20);
    
    // Message management
    bool addMessage(const QString &sessionId, const QString &role, const QString &content);
    QVector<SessionMessage> getSessionMessages(const QString &sessionId);
    
    // Current session
    QString currentSessionId() const { return m_currentSessionId; }
    void setCurrentSession(const QString &sessionId) { m_currentSessionId = sessionId; }

signals:
    void sessionCreated(const QString &sessionId);
    void sessionUpdated(const QString &sessionId);
    void sessionDeleted(const QString &sessionId);
    void messageAdded(const QString &sessionId, const SessionMessage &message);

private:
    explicit SessionService(QObject *parent = nullptr);
    ~SessionService();
    SessionService(const SessionService&) = delete;
    SessionService& operator=(const SessionService&) = delete;
    
    QString generateSessionId();
    QString extractTitle(const QString &firstMessage);
    
    QString m_currentSessionId;
};

#endif // SESSIONSERVICE_H
