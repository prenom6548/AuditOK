#ifndef APISERVICE_H
#define APISERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHash>
#include <QQueue>
#include <QMutex>
#include <QUuid>

class ChatMessage;
class ChatSession;
class QTimer;

class ApiService : public QObject
{
    Q_OBJECT

public:
    static ApiService& instance();
    
    // Connection state
    bool isConnected() const { return m_connected; }
    QString apiUrl() const { return m_apiUrl; }
    QString networkApiUrl() const { return apiUrl(); }
    bool usesLocalStorageMode() const;
    void setApiUrl(const QString& url);
    void checkConnection();
    
    // Session operations
    void loadSessions();
    void loadSessionMessages(const QString &sessionId);
    void loadTaskRecovery(const QString &sessionId);
    void interruptSessionTask(const QString &sessionId, bool recoverable = true);
    void createSession(const QString &title = QString());
    void deleteSession(const QString &sessionId);
    
    // Chat operations
    void sendMessage(const QString &sessionId, const QString &content,
                     const QString &model = QString(), int priority = 0,
                     const QString &recoveryTaskId = QString());
    void cancelCurrentStream();
    void cancelCurrentStream(const QString &sessionId);
    void sendChatCompletion(const QString &sessionId,
                            const QJsonArray &messages,
                            const QString &model);
    void cancelChatCompletion(const QString &sessionId);
    void setCurrentModel(const QString &model);
    
    // Model operations
    void loadModels();
    QVector<QMap<QString, QVariant>> getSessionsSync(int limit = 50, int offset = 0);
    QVector<QMap<QString, QVariant>> getSessionMessagesSync(const QString &sessionId);
    QJsonObject getTaskRecoverySync(const QString &sessionId);
    QVector<QJsonObject> importChangedCronRemindersSync(bool *hasActiveJobs = nullptr);
    QString getSessionDetailsSync(const QString &sessionId);
    QMap<QString, QVariant> getSessionUsageSync(const QString &sessionId);
    QVector<QMap<QString, QVariant>> getConfigModelsSync();
    int detectLocalModelContextLengthSync(const QString &baseUrl, const QString &modelId);
    bool replaceConfigModelsSync(const QJsonArray &models,
                                 const QString &preferredDefaultModel = QString(),
                                 const QString &preferredDefaultBaseUrl = QString());
    QMap<QString, QVariant> getUsageSync();
    QString getPersonaSync();
    bool putPersonaSync(const QString &content);
    QMap<QString, QString> getMemorySync();
    bool putMemorySync(const QString &memory, const QString &user);
    QJsonObject getUserProfileSync();
    bool putUserProfileSync(const QJsonObject &entries, const QString &expectedUpdatedAt);
    QVector<QMap<QString, QVariant>> getLogsSync();
    QString getLogContentSync(const QString &name);
    QVector<QMap<QString, QVariant>> getSkillsSync();
    bool deleteSkillSync(const QString &skillId);
    bool importSkillArchiveSync(const QString &filename, const QByteArray &content);
    bool importSkillFilesSync(const QString &filename, const QJsonArray &files);
    bool createSessionSync(const QString &sessionId, const QString &title = QString(), const QString &model = QString());
    bool updateSessionTitleSync(const QString &sessionId, const QString &title);
    bool addMessageSync(const QString &sessionId, const QString &role, const QString &content,
                        qint64 timestamp = 0);
    bool deleteSessionSync(const QString &sessionId);
    
signals:
    void backendChanged();
    void connectedChanged(bool connected);
    void sessionsLoaded(const QVector<QJsonObject> &sessions);
    void sessionMessagesLoaded(const QString &sessionId,
                               const QVector<QMap<QString, QVariant>> &messages);
    void sessionMessagesLoadFailed(const QString &sessionId, const QString &message);
    void taskRecoveryLoaded(const QString &sessionId, const QJsonObject &recovery);
    void sessionTaskInterrupted(const QString &sessionId, bool recoverable);
    void sessionCreated(const QJsonObject &session);
    void sessionDeleted(const QString &sessionId);
    void sessionDeleteFailed(const QString &sessionId, const QString &message);
    void messageReceived(const QString &sessionId, const QJsonObject &message);
    void streamChunk(const QString &sessionId, const QString &content, bool isComplete);
    void streamError(const QString &sessionId, const QString &message);
    void streamDetailEvent(const QString &sessionId, const QJsonObject &event);
    void segmentBoundary(const QString &sessionId);
    void chatCompletionDelta(const QString &sessionId, const QString &delta);
    void chatCompletionFinished(const QString &sessionId, const QString &content, bool ok);
    void modelsLoaded(const QVector<QJsonObject> &models);
    void error(const QString &error);

private:
    ApiService(QObject *parent = nullptr);
    ~ApiService();
    ApiService(const ApiService&) = delete;
    ApiService& operator=(const ApiService&) = delete;
    
    void processQueue();
    void sendRequest(const QString &endpoint, const QJsonObject &body, int priority = 0);
    QJsonObject parseResponse(QNetworkReply *reply);
    QJsonDocument requestSync(const QString &method, const QString &endpoint,
                              const QJsonObject &body = QJsonObject(), bool *ok = nullptr);
    void cacheSessionUsage(const QMap<QString, QVariant> &session);
    void markSessionMemoryDirty(const QString &sessionId);
    void scheduleSessionMemoryActivation(const QString &sessionId);
    void activateSessionMemory(const QString &sessionId);
    void clearSessionMemoryTracking(const QString &sessionId);

    struct StreamState {
        QNetworkReply *reply = nullptr;
        QString partialData;
        bool inCustomEvent = false;
        bool completionEmitted = false;
        bool receivedContent = false;
        bool errorEmitted = false;
    };

    struct SessionMemoryState {
        QTimer *inactivityTimer = nullptr;
        bool sessionMemoryActivated = false;
        quint64 activityGeneration = 0;
    };

    void cancelStreamForSession(const QString &sessionId, bool emitCompletion);
    
    QNetworkAccessManager *m_networkManager;
    QString m_apiUrl;
    bool m_connected;
    QQueue<QPair<QString, QJsonObject>> m_requestQueue;
    QMutex m_queueMutex;
    QHash<QString, StreamState> m_streams;
    QHash<QString, SessionMemoryState> m_sessionMemoryStates;
    QHash<QString, QMap<QString, QVariant>> m_sessionUsageCache;
    QString m_currentModel;
};

#endif // APISERVICE_H
