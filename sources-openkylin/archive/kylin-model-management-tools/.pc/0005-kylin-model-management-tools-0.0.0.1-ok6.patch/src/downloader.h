#pragma once

#include <memory>

#include <QMap>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

class Downloader : public QObject {
    Q_OBJECT
public:
    explicit Downloader(QObject *parent = nullptr);

    const QStringList getModelTypes() const;
    const QStringList getModelNames(const QString &modelType) const;
    const QStringList getModelFiles(const QString &modelName) const;

    void setModelCacheDirectory(const QString &modelCacheDirectory);
    void setDownloadParams(const QString &modelType, const QString &modelName,
                           const QString &modelFile,
                           const QString &modelRevision,
                           const QString &modelscopeCacheDir);

    bool isDownloading() const;
    void download();

    void checkModelFiles();

signals:
    void downloadOutput(QByteArray);
    void downloadSucceed();
    void downloadFailed();

private:
    void initModelMaps();

private slots:
    void processOutput(QByteArray output);
    void downloadProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QMap<QString, QStringList> modelTypeToNameMap_;
    QMap<QString, QStringList> modelNameToFileMap_;

    QString modelTypeToDownload_;
    QString modelNameToDownload_;
    QString modelFileToDownload_;
    QString modelCacheDirectory_;

    std::unique_ptr<QProcess> downloadProcess_;
};
