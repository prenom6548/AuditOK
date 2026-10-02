#ifndef REALTIMEASRSERVICE_H
#define REALTIMEASRSERVICE_H

#include <QObject>
#include <QUrl>

class QAudioInput;
class QIODevice;
class QWebSocket;

class RealtimeAsrService : public QObject
{
    Q_OBJECT

public:
    explicit RealtimeAsrService(QObject *parent = nullptr);
    ~RealtimeAsrService();

    bool isRunning() const { return m_running; }

public slots:
    void start();
    void stop();

signals:
    void transcriptChanged(const QString &text, bool sentenceEnd);
    void runningChanged(bool running);
    void errorOccurred(const QString &message);

private slots:
    void onConnected();
    void onTextMessageReceived(const QString &message);
    void onAudioReadyRead();

private:
    enum class Protocol {
        DashScopeTask,
        Qwen3Realtime
    };

    void cleanup();
    void finishTask();
    QUrl configuredUrl(QString *errorMessage) const;
    QString configuredApiKey() const;
    QString configuredModel() const;
    int configuredMaxSentenceSilence() const;
    Protocol protocolForModel(const QString &model) const;
    void sendDashScopeRunTask();
    void sendQwen3SessionUpdate();
    void handleDashScopeMessage(const QJsonObject &message, const QString &event);
    void handleQwen3Message(const QJsonObject &message, const QString &event);

    QWebSocket *m_socket;
    QAudioInput *m_audioInput;
    QIODevice *m_audioDevice;
    QString m_taskId;
    QString m_model;
    Protocol m_protocol;
    bool m_running;
    bool m_taskStarted;
    bool m_finishing;
};

#endif // REALTIMEASRSERVICE_H
