#include "downloader.h"

#include <QDebug>
#include <QFile>

static const QString DOWNLOAD_LOG = "/tmp/download-log";
static const QString MODEL_CACHE_DIR = "/usr/share/kylin-ai/ondevice/models/";

Downloader::Downloader(QObject *parent) { initModelMaps(); }

const QStringList Downloader::getModelTypes() const {
    return modelTypeToNameMap_.keys();
}

const QStringList Downloader::getModelNames(const QString &modelType) const {
    return modelTypeToNameMap_[modelType];
}

const QStringList Downloader::getModelFiles(const QString &modelName) const {
    return modelNameToFileMap_[modelName];
}

void Downloader::setModelCacheDirectory(const QString &modelCacheDirectory) {
    modelCacheDirectory_ = modelCacheDirectory;
}

void Downloader::setDownloadParams(const QString &modelType,
                                   const QString &modelName,
                                   const QString &modelFile,
                                   const QString &modelRevision,
                                   const QString &modelscopeCacheDir) {
    (void)modelRevision;
    (void)modelscopeCacheDir;

    modelTypeToDownload_ = modelType;
    modelNameToDownload_ = modelName;
    modelFileToDownload_ = modelFile;
}

bool Downloader::isDownloading() const {
    return downloadProcess_ && (downloadProcess_->state() == QProcess::Running);
}

void Downloader::downloadOne(const QString &modelType, const QString &modelName,
                             const QString &modelFile) {
    setDownloadParams(modelType, modelName, modelFile, "", "");
    download();

    loop_.reset(new QEventLoop());

    QObject::connect(this, &Downloader::linkFinished, loop_.get(),
                     &QEventLoop::quit);

    loop_->exec();
}

void Downloader::downloadAllModels() {
    if (isDownloading()) {
        return;
    }

    for (auto modelType : getModelTypes()) {
        for (auto modelName : getModelNames(modelType)) {
            auto modelFiles = getModelFiles(modelName);
            if (modelFiles.isEmpty()) {
                downloadOne(modelType, modelName, "");
            } else {
                for (auto modelFile : modelFiles) {
                    downloadOne(modelType, modelName, modelFile);
                }
            }
        }
    }
    emit allModelsDownloaded();
}

void Downloader::download() {
    emit downloadStart();
    downloadProcess_.reset(new QProcess());
    downloadProcess_->setProcessChannelMode(QProcess::MergedChannels);

    QStringList arguments;
    arguments << "download-in-venv" << "/usr/bin/download.py"
              << modelNameToDownload_ << "--cache-dir=" + MODEL_CACHE_DIR;

    if (!modelFileToDownload_.isEmpty()) {
        arguments << "--file-path=" + modelFileToDownload_;
    }

    downloadProcess_->start("pkexec", arguments);
    connect(downloadProcess_.get(),
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            &Downloader::downloadProcessFinished);

    connect(
        downloadProcess_.get(), &QProcess::readyReadStandardOutput, this,
        [this]() { processOutput(downloadProcess_->readAllStandardOutput()); });
    connect(
        downloadProcess_.get(), &QProcess::readyReadStandardError, this,
        [this]() { processOutput(downloadProcess_->readAllStandardError()); });
}

void Downloader::checkModelFiles() {
    // 检查 md5s 文件中的模型文件 md5
    QProcess checkProcess;
    checkProcess.setProcessChannelMode(QProcess::MergedChannels);

    QStringList arguments;
    arguments << "-c" << "/usr/share/kylin-ai/ondevice/modelsmd5";

    checkProcess.start("md5sum", arguments);

    connect(&checkProcess, &QProcess::readyReadStandardOutput, this,
            [this, &checkProcess]() {
                processOutput(checkProcess.readAllStandardOutput());
            });
    connect(&checkProcess, &QProcess::readyReadStandardError, this,
            [this, &checkProcess]() {
                processOutput(checkProcess.readAllStandardError());
            });

    checkProcess.waitForFinished();
}

void Downloader::quitProcess() {
    if (!downloadProcess_) {
        return;
    }
    downloadProcess_->terminate();
    if (!downloadProcess_->waitForFinished(3000)) {
        downloadProcess_->kill();
    }
}

void Downloader::initModelMaps() {
    // TODO init map with local file
    modelTypeToNameMap_ = {
        {"nlp", {"qwen/Qwen1.5-7B-Chat-GGUF"}},
        {"speech",
         {"ondevice/keyword-spotter", "ondevice/online-recognizer",
          "ondevice/offline-recognizer", "ondevice/offline-tts",
          "ondevice/speaker-identification"}},
        {"搜索", {"ondevice/bge-m3-onnx", "ondevice/cn-clip-onnx"}}};

    modelNameToFileMap_ = {
        {"qwen/Qwen1.5-7B-Chat-GGUF", {"qwen1_5-7b-chat-q5_k_m.gguf"}},
        {"ondevice/keyword-spotter", {}},
        {"ondevice/online-recognizer", {}},
        {"ondevice/offline-recognizer", {}},
        {"ondevice/offline-tts", {}},
        {"ondevice/speaker-identification", {}},
        {"ondevice/bge-m3-onnx", {}},
        {"ondevice/cn-clip-onnx", {}}};
}

void Downloader::processOutput(QByteArray output) {
    emit downloadOutput(output);

    QString str = QString::fromUtf8(output);
    QStringList lines = str.split("\n");
    for (const QString &line : lines) {
        if (line.contains("Model directory:")) {
            const QString modelCacheDirectory = line.split(":")[1].trimmed();
            setModelCacheDirectory(modelCacheDirectory);
            break;
        }
    }
}

void Downloader::downloadProcessFinished(int exitCode,
                                         QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        emit downloadSucceed();
    } else {
        emit downloadFailed();
        return;
    }

    QProcess p;
    QStringList args;
    p.setProcessChannelMode(QProcess::MergedChannels);

    connect(&p, &QProcess::readyReadStandardOutput, this,
            [this, &p]() { emit downloadOutput(p.readAllStandardOutput()); });
    connect(&p, &QProcess::readyReadStandardError, this,
            [this, &p]() { emit downloadOutput(p.readAllStandardError()); });

    args << "download-in-venv" << modelCacheDirectory_;
    if (QFile::exists(modelCacheDirectory_)) {
        if (modelTypeToDownload_ == "nlp") {
            args << "/usr/share/kylin-ai/ondevice/nlp/";
        } else if (modelTypeToDownload_ == "speech") {
            args << "/usr/share/kylin-ai/ondevice/speech/";
        } else if (modelTypeToDownload_ == "搜索") {
            args << "/usr/share/kylin-datamanagement-models/";
        }
    }
    p.start("pkexec", args);
    p.waitForFinished();

    emit linkFinished();
}