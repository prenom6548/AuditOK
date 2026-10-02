#ifndef MODELDOWNLOADSERVICE_H
#define MODELDOWNLOADSERVICE_H

#include <QHash>
#include <QElapsedTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

class QFile;
class QProcess;

class ModelDownloadService : public QObject
{
    Q_OBJECT

public:
    enum class State {
        Idle,
        Downloading,
        Finished,
        Failed
    };

    static ModelDownloadService &instance();

    bool startDownload(const QString &modelId, QString *errorMessage = nullptr);
    void cancelDownload(const QString &modelId);
    bool reconcileInstalledModels(QStringList *errors = nullptr);
    bool ensureModelReady(const QString &modelId, QString *errorMessage = nullptr);
    bool hasPendingModelSetup() const;

    State state(const QString &modelId) const;
    int progressPercent(const QString &modelId) const;
    qint64 downloadedBytes(const QString &modelId) const;
    qint64 downloadSpeedBytesPerSecond(const QString &modelId) const;
    bool isDownloading(const QString &modelId) const;
    bool hasActiveDownloads() const;
    QStringList activeModelNames() const;
    QString lastError(const QString &modelId) const;

signals:
    void downloadChanged(const QString &modelId);
    void modelInstalled(const QString &modelId);

private:
    struct FileSpec {
        QString repo;
        QString includePattern;
        QString fileName;
        int weight = 1;
        qint64 expectedBytes = 0;
        QString directUrl;
    };

    struct Task {
        QString modelId;
        QVector<FileSpec> files;
        int fileIndex = 0;
        State state = State::Idle;
        QString error;
        int progress = 0;
        qint64 completedBytes = 0;
        qint64 totalBytes = 0;
        bool progressReady = false;
        QNetworkReply *reply = nullptr;
        QFile *file = nullptr;
        QProcess *process = nullptr;
    };

    struct DownloadSample {
        qint64 elapsedMs = 0;
        qint64 bytes = 0;
    };

    struct DownloadMetrics {
        qint64 bytesPerSecond = 0;
        QElapsedTimer timer;
        QVector<DownloadSample> samples;
    };

    explicit ModelDownloadService(QObject *parent = nullptr);
    ModelDownloadService(const ModelDownloadService &) = delete;
    ModelDownloadService &operator=(const ModelDownloadService &) = delete;

    QVector<FileSpec> filesForModel(const QString &modelId) const;
    QString modelRootPath(const QString &modelId) const;
    void removeSetupMarkers(const QString &modelId) const;
    void removeDownloadFiles(const QString &modelId, const QVector<FileSpec> &files) const;
    void startNextFile(Task &task);
    void startCurrentFile(Task &task);
    void finishCurrentFile(Task &task);
    void failTask(Task &task, const QString &message);
    void closeTaskFile(Task &task);
    void closeTaskProcess(Task &task);
    bool ensureHfdScript(QString *errorMessage) const;
    bool writeMd5File(const QString &modelId, QString *errorMessage) const;
    bool registerModelRuntime(const QString &modelId, QString *errorMessage);
    bool grantModelGatewayAccess(QString *errorMessage);
    bool addModelToBackendConfig(const QString &modelId, QString *errorMessage);
    void updateProgress(Task &task, qint64 received, qint64 total);
    qint64 currentDownloadedBytes(const QString &modelId, const Task &task) const;
    qint64 expectedDownloadBytes(const Task &task) const;

    QNetworkAccessManager m_network;
    QHash<QString, Task> m_tasks;
    mutable QHash<QString, DownloadMetrics> m_downloadMetrics;
};

#endif // MODELDOWNLOADSERVICE_H
