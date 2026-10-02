#ifndef SESSIONLISTWIDGET_H
#define SESSIONLISTWIDGET_H

#include <QListWidget>
#include <QVector>
#include <QJsonObject>
#include <QString>
#include <QContextMenuEvent>
#include <QSet>
#include <QTimer>
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QStringList>
#include "models/session.h"

class SessionListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit SessionListWidget(QWidget *parent = nullptr);
    ~SessionListWidget();
    
    void loadSessions(const QVector<QJsonObject> &sessions);
    void setSessions(const QVector<SessionSummary> &sessions);
    void addSession(const QString &id, const QString &title);
    void appendSession(const QString &id, const QString &title);
    void removeSession(const QString &id);
    void removeSessions(const QStringList &ids);
    void moveSessionToTop(const QString &id);
    void filterSessions(const QString &searchText);
    QString getFirstSessionId() const;
    QString firstSessionId() const;
    QString getLastSessionId() const;
    void updateSessionTitle(const QString &id, const QString &title);
    void setSessionStreaming(const QString &id, bool streaming);
    bool isSessionUnread(const QString &id) const;
    void markSessionRead(const QString &id);
    void markAllSessionsRead();
    void markSessionUnread(const QString &id);
    QStringList getSelectedSessionIds() const;
    QString getTopmostSelectedSessionId() const;

signals:
    void sessionSelected(const QString &sessionId);
    void sessionDeleted(const QString &sessionId);
    void sessionsDeleted(const QStringList &sessionIds);
    void sessionRenameRequested(const QString &sessionId);
    void sessionDeleteRequested(const QString &sessionId);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void onItemClicked(QListWidgetItem *item);
    QVector<QPair<QString, QString>> m_sessions; // id, title
    QSet<QString> m_streamingSessions;
    QSet<QString> m_unreadSessions;
    QTimer *m_streamingTimer;
    int m_streamingFrame = 0;

    // Generated icon cache
    QIcon m_greenDotIcon;
    QIcon m_spinnerFrames[12]; // 12 frames for smooth rotation

    QString sessionTitle(const QString &id) const;
    QString fallbackTitle(const QString &title, const QString &id) const;
    void updateItemDisplay(const QString &id);
    void updateItemDecoration(QListWidgetItem *item, const QString &id) const;
    void updateStreamingAnimation();
    void generateIcons();

    QIcon generateSpinnerFrame(int frameIndex) const;
    QIcon generateGreenDotIcon() const;
};

#endif // SESSIONLISTWIDGET_H
