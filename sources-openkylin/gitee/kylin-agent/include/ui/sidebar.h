#ifndef SIDEBAR_H// SIDEBAR_H
#define SIDEBAR_H
#include <QWidget>
#include <QJsonObject>
#include <QVector>
#include <QString>
#include "models/session.h"

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QLineEdit;
class SessionListWidget;

class Sidebar : public QWidget
{
    Q_OBJECT

public:
    explicit Sidebar(QWidget *parent = nullptr);
    ~Sidebar();
    
    QString getFirstSessionId() const;
    QString createNewSession();
    void selectSession(const QString &sessionId);
    void updateSessionTitle(const QString &id, const QString &title);
    void moveSessionToTop(const QString &sessionId);
    void setSessionStreaming(const QString &sessionId, bool streaming);
    void markSessionUnread(const QString &sessionId);
    void refreshSessions();
    void removeSession(const QString &sessionId);
    void removeSessions(const QStringList &sessionIds);
    void setSessionCount(int count);
    void setSessions(const QVector<SessionSummary> &sessions);
    
signals:
    void newSessionRequested();
    void searchChanged(const QString &text);
    void sessionSelected(const QString &sessionId);
    void sessionRenameRequested(const QString &sessionId);
    void sessionDeleteRequested(const QString &sessionId);
    void sessionDeleted(const QString &sessionId);
    void sessionsDeleted(const QStringList &sessionIds);
    void settingsClicked();

private:
    void setupUi();
    void setupConnections();
    void loadRemoteSessions();

    QVBoxLayout *m_mainLayout;
    QVBoxLayout *m_headerLayout;
    QLabel *m_sessionCountLabel;
    QLabel *m_titleLabel;
    QPushButton *m_newSessionButton;
    QPushButton *m_newChatBtn;
    QLineEdit *m_searchEdit;
    SessionListWidget *m_sessionList;
    QVBoxLayout *m_bottomLayout;
    QPushButton *m_settingsBtn;
    QLabel *m_statusLabel;
};

#endif // SIDEBAR_H
