#include "services/modeldownloadservice.h"
#include "services/apiservice.h"

#include <QCryptographicHash>
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "utils/kylininputdialog.h"
#include <QLineEdit>
#include <QNetworkRequest>
#include <QProcessEnvironment>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>

#include <sys/stat.h>

namespace {

QString fixedModelRootPath()
{
    return QDir(QDir::homePath()).filePath(QStringLiteral(".local/share/kylin-agent/models"));
}

QString kylinAgentDataPath()
{
    return QDir(QDir::homePath()).filePath(QStringLiteral(".local/share/kylin-agent"));
}

QString hfdScriptPath()
{
    return QDir(kylinAgentDataPath()).filePath(QStringLiteral("hfd.sh"));
}

QString fixedModelProviderName(const QString &modelId)
{
    QString slug = modelId.trimmed().toLower();
    slug.replace(QRegularExpression(QStringLiteral("[^a-z0-9]+")), QStringLiteral("-"));
    while (slug.startsWith(QLatin1Char('-'))) {
        slug.remove(0, 1);
    }
    while (slug.endsWith(QLatin1Char('-'))) {
        slug.chop(1);
    }
    return QStringLiteral("kylin-local-%1").arg(slug.isEmpty() ? QStringLiteral("model") : slug);
}

QString runtimeConfigPath()
{
    const QByteArray agentRuntimeHome = qgetenv("AGENT_RUNTIME_HOME");
    if (!agentRuntimeHome.isEmpty()) {
        return QDir(QString::fromLocal8Bit(agentRuntimeHome)).filePath(QStringLiteral("config.yaml"));
    }

    const QByteArray runtimeHome = qgetenv("HERMES_HOME");
    if (!runtimeHome.isEmpty()) {
        return QDir(QString::fromLocal8Bit(runtimeHome)).filePath(QStringLiteral("config.yaml"));
    }

    return QDir(QDir::homePath()).filePath(QStringLiteral(".kylin-agent-runtime/config.yaml"));
}

bool backendConfigLooksReady(const QString &modelId)
{
    QFile file(runtimeConfigPath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    const QString content = QString::fromUtf8(file.readAll());
    const QString provider = fixedModelProviderName(modelId);
    return content.contains(QStringLiteral("provider: \"%1\"").arg(provider)) &&
           content.contains(QStringLiteral("default: \"%1\"").arg(modelId)) &&
           content.contains(QStringLiteral("name: \"%1\"").arg(provider)) &&
           content.contains(QStringLiteral("model: \"%1\"").arg(modelId)) &&
           content.contains(QStringLiteral("api_mode: \"chat_completions\""));
}

QString md5ForFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QCryptographicHash hash(QCryptographicHash::Md5);
    while (!file.atEnd()) {
        hash.addData(file.read(1024 * 1024));
    }
    return QString::fromLatin1(hash.result().toHex());
}

QString markerPath(const QString &rootPath, const QString &marker)
{
    return QDir(rootPath).filePath(marker);
}

bool markerExists(const QString &rootPath, const QString &marker)
{
    return QFileInfo::exists(markerPath(rootPath, marker));
}

bool writeMarker(const QString &rootPath, const QString &marker, const QString &content = QString())
{
    QDir().mkpath(rootPath);
    QFile file(markerPath(rootPath, marker));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }
    file.write(content.toUtf8());
    if (!content.endsWith(QLatin1Char('\n'))) {
        file.write("\n");
    }
    return true;
}

QString currentSystemUser()
{
    QString user = QString::fromLocal8Bit(qgetenv("USER")).trimmed();
    if (user.isEmpty()) {
        user = QString::fromLocal8Bit(qgetenv("LOGNAME")).trimmed();
    }
    return user;
}

qint64 downloadedFileBytes(const QString &filePath)
{
    const QFileInfo info(filePath);
    if (!info.exists() || !info.isFile()) {
        return 0;
    }

    const qint64 logicalSize = info.size();
    const QByteArray encodedPath = QFile::encodeName(filePath);
    struct stat st {};
    if (::stat(encodedPath.constData(), &st) == 0 && S_ISREG(st.st_mode)) {
        const qint64 allocatedSize = static_cast<qint64>(st.st_blocks) * 512;
        if (allocatedSize > 0) {
            return qMin(logicalSize, allocatedSize);
        }
    }

    return logicalSize;
}

} // namespace

ModelDownloadService &ModelDownloadService::instance()
{
    static ModelDownloadService service;
    return service;
}

ModelDownloadService::ModelDownloadService(QObject *parent)
    : QObject(parent)
{
}

QVector<ModelDownloadService::FileSpec> ModelDownloadService::filesForModel(const QString &modelId) const
{
    if (modelId == QStringLiteral("qwen3.6-27b")) {
        return {
            {
                QString(),
                QStringLiteral("Qwen3.6-27B-Q4_K_M.gguf"),
                QStringLiteral("model.gguf"),
                90,
                16547398784LL,
                QStringLiteral("https://www.modelscope.cn/models/lmstudio-community/Qwen3.6-27B-GGUF/resolve/master/Qwen3.6-27B-Q4_K_M.gguf")
            },
            {
                QString(),
                QStringLiteral("mmproj-Qwen3.6-27B-BF16.gguf"),
                QStringLiteral("mmproj.gguf"),
                10,
                931145856LL,
                QStringLiteral("https://www.modelscope.cn/models/lmstudio-community/Qwen3.6-27B-GGUF/resolve/master/mmproj-Qwen3.6-27B-BF16.gguf")
            }
        };
    }
    return {};
}

QString ModelDownloadService::modelRootPath(const QString &modelId) const
{
    return QDir(fixedModelRootPath()).filePath(modelId);
}

bool ModelDownloadService::startDownload(const QString &modelId, QString *errorMessage)
{
    Task &task = m_tasks[modelId];
    if (task.state == State::Downloading) {
        return true;
    }

    const QVector<FileSpec> files = filesForModel(modelId);
    if (files.isEmpty()) {
        if (errorMessage) {
            *errorMessage = tr("该模型暂未配置下载地址。");
        }
        return false;
    }
    const QString rootPath = modelRootPath(modelId);
    if (!QDir().mkpath(rootPath)) {
        if (errorMessage) {
            *errorMessage = tr("无法创建模型目录：%1").arg(rootPath);
        }
        return false;
    }

    task.modelId = modelId;
    task.files = files;
    task.fileIndex = 0;
    task.state = State::Downloading;
    task.error.clear();
    task.progress = 0;
    task.completedBytes = 0;
    task.totalBytes = 0;
    task.progressReady = false;
    closeTaskFile(task);
    closeTaskProcess(task);
    task.reply = nullptr;
    task.process = nullptr;
    removeSetupMarkers(modelId);
    m_downloadMetrics.remove(modelId);

    emit downloadChanged(modelId);
    startNextFile(task);
    return true;
}

void ModelDownloadService::cancelDownload(const QString &modelId)
{
    auto it = m_tasks.find(modelId);
    if (it == m_tasks.end()) {
        return;
    }

    Task &task = it.value();
    if (task.reply) {
        task.reply->disconnect(this);
        task.reply->abort();
        task.reply->deleteLater();
        task.reply = nullptr;
    }
    if (task.process) {
        task.process->disconnect(this);
        task.process->kill();
        task.process->deleteLater();
        task.process = nullptr;
    }
    closeTaskFile(task);
    removeDownloadFiles(modelId, task.files);
    task.state = State::Idle;
    task.error.clear();
    task.progress = 0;
    m_downloadMetrics.remove(modelId);
    emit downloadChanged(modelId);
}

ModelDownloadService::State ModelDownloadService::state(const QString &modelId) const
{
    return m_tasks.value(modelId).state;
}

int ModelDownloadService::progressPercent(const QString &modelId) const
{
    const Task task = m_tasks.value(modelId);
    if (task.state != State::Downloading) {
        return task.progress;
    }

    const qint64 expectedBytes = expectedDownloadBytes(task);
    if (expectedBytes <= 0) {
        return task.progress;
    }

    const qint64 downloaded = currentDownloadedBytes(modelId, task);
    return qBound(0, static_cast<int>((downloaded * 100) / expectedBytes), 99);
}

qint64 ModelDownloadService::downloadedBytes(const QString &modelId) const
{
    const Task task = m_tasks.value(modelId);
    if (task.state != State::Downloading) {
        return 0;
    }

    return currentDownloadedBytes(modelId, task);
}

qint64 ModelDownloadService::downloadSpeedBytesPerSecond(const QString &modelId) const
{
    constexpr qint64 speedWindowMs = 30000;
    const Task task = m_tasks.value(modelId);
    if (task.state != State::Downloading) {
        return 0;
    }

    const qint64 bytes = currentDownloadedBytes(modelId, task);
    DownloadMetrics &metrics = m_downloadMetrics[modelId];
    if (!metrics.timer.isValid()) {
        metrics.bytesPerSecond = 0;
        metrics.timer.start();
        metrics.samples = {{0, bytes}};
        return 0;
    }

    const qint64 nowMs = metrics.timer.elapsed();
    if (!metrics.samples.isEmpty() &&
        metrics.samples.last().elapsedMs == nowMs &&
        metrics.samples.last().bytes == bytes) {
        return metrics.bytesPerSecond;
    }

    metrics.samples.append({nowMs, bytes});
    while (metrics.samples.size() > 1 &&
           metrics.samples.at(1).elapsedMs <= nowMs - speedWindowMs) {
        metrics.samples.removeFirst();
    }

    if (metrics.samples.size() < 2) {
        metrics.bytesPerSecond = 0;
        return 0;
    }

    const qint64 deltaBytes = qMax<qint64>(0, bytes - metrics.samples.first().bytes);
    const qint64 windowMs = qMax<qint64>(1, nowMs - metrics.samples.first().elapsedMs);
    metrics.bytesPerSecond = (deltaBytes * 1000) / windowMs;
    return metrics.bytesPerSecond;
}

bool ModelDownloadService::isDownloading(const QString &modelId) const
{
    return state(modelId) == State::Downloading;
}

bool ModelDownloadService::hasActiveDownloads() const
{
    for (const Task &task : m_tasks) {
        if (task.state == State::Downloading) {
            return true;
        }
    }
    return false;
}

QStringList ModelDownloadService::activeModelNames() const
{
    QStringList names;
    for (const Task &task : m_tasks) {
        if (task.state == State::Downloading) {
            names.append(task.modelId);
        }
    }
    return names;
}

QString ModelDownloadService::lastError(const QString &modelId) const
{
    return m_tasks.value(modelId).error;
}

bool ModelDownloadService::reconcileInstalledModels(QStringList *errors)
{
    bool ok = true;
    const QStringList ids = {
        QStringLiteral("qwen3.6-27b")
    };
    for (const QString &modelId : ids) {
        QString error;
        if (!ensureModelReady(modelId, &error)) {
            const QString rootPath = modelRootPath(modelId);
            if (markerExists(rootPath, QStringLiteral("md5sum")) &&
                (!markerExists(rootPath, QStringLiteral("install")) ||
                 !markerExists(rootPath, QStringLiteral("config")) ||
                 !backendConfigLooksReady(modelId))) {
                ok = false;
                if (errors) {
                    errors->append(error.isEmpty()
                        ? tr("%1 配置或注册失败。").arg(modelId)
                        : tr("%1：%2").arg(modelId, error));
                }
            }
        }
    }
    return ok;
}

bool ModelDownloadService::ensureModelReady(const QString &modelId, QString *errorMessage)
{
    const QString rootPath = modelRootPath(modelId);
    if (!markerExists(rootPath, QStringLiteral("md5sum"))) {
        return false;
    }

    if (!markerExists(rootPath, QStringLiteral("install"))) {
        if (!registerModelRuntime(modelId, errorMessage)) {
            return false;
        }
    }
    if (!markerExists(rootPath, QStringLiteral("config")) || !backendConfigLooksReady(modelId)) {
        if (!addModelToBackendConfig(modelId, errorMessage)) {
            return false;
        }
    }

    emit downloadChanged(modelId);
    emit modelInstalled(modelId);
    return true;
}

bool ModelDownloadService::hasPendingModelSetup() const
{
    const QStringList ids = {
        QStringLiteral("qwen3.6-27b")
    };
    for (const QString &modelId : ids) {
        const QString rootPath = modelRootPath(modelId);
        if (markerExists(rootPath, QStringLiteral("md5sum")) &&
            (!markerExists(rootPath, QStringLiteral("install")) ||
             !markerExists(rootPath, QStringLiteral("config")) ||
             !backendConfigLooksReady(modelId))) {
            return true;
        }
    }
    return false;
}

void ModelDownloadService::removeSetupMarkers(const QString &modelId) const
{
    const QString rootPath = modelRootPath(modelId);
    QFile::remove(QDir(rootPath).filePath(QStringLiteral("md5sum")));
    QFile::remove(QDir(rootPath).filePath(QStringLiteral("install")));
    QFile::remove(QDir(rootPath).filePath(QStringLiteral("config")));
}

void ModelDownloadService::removeDownloadFiles(const QString &modelId, const QVector<FileSpec> &files) const
{
    removeSetupMarkers(modelId);
    const QString rootPath = modelRootPath(modelId);
    for (const FileSpec &spec : files) {
        QFile::remove(QDir(rootPath).filePath(spec.fileName));
        QFile::remove(QDir(rootPath).filePath(spec.includePattern));
    }
}

void ModelDownloadService::startNextFile(Task &task)
{
    closeTaskFile(task);
    if (task.fileIndex >= task.files.size()) {
        QString error;
        if (!writeMd5File(task.modelId, &error)) {
            failTask(task, error);
            return;
        }
        if (!ensureModelReady(task.modelId, &error)) {
            failTask(task, error);
            return;
        }
        task.state = State::Finished;
        task.progress = 100;
        emit downloadChanged(task.modelId);
        return;
    }

    qint64 previousBytes = 0;
    for (int i = 0; i < task.fileIndex; ++i) {
        previousBytes += QFileInfo(QDir(modelRootPath(task.modelId)).filePath(task.files.at(i).fileName)).size();
    }
    task.completedBytes = previousBytes;
    startCurrentFile(task);
}

void ModelDownloadService::startCurrentFile(Task &task)
{
    const FileSpec spec = task.files.at(task.fileIndex);
    if (spec.directUrl.isEmpty() && !ensureHfdScript(&task.error)) {
        failTask(task, task.error);
        return;
    }

    const QString rootPath = modelRootPath(task.modelId);
    const QString targetPath = QDir(modelRootPath(task.modelId)).filePath(spec.fileName);
    if (spec.expectedBytes > 0 && QFileInfo(targetPath).size() >= spec.expectedBytes) {
        ++task.fileIndex;
        startNextFile(task);
        return;
    }

    task.totalBytes = task.completedBytes;
    for (int i = task.fileIndex + 1; i < task.files.size(); ++i) {
        task.totalBytes += QFileInfo(QDir(modelRootPath(task.modelId)).filePath(task.files.at(i).fileName)).size();
    }

    task.progressReady = true;
    task.process = new QProcess(this);
    if (spec.directUrl.isEmpty()) {
        task.process->setProgram(hfdScriptPath());
        task.process->setArguments({
            spec.repo,
            QStringLiteral("--include"),
            spec.includePattern,
            QStringLiteral("--local-dir"),
            rootPath
        });
    } else {
        const QString curlPath = QStandardPaths::findExecutable(QStringLiteral("curl"));
        if (curlPath.isEmpty()) {
            failTask(task, tr("无法找到 curl，不能从国内模型源下载。"));
            return;
        }
        task.process->setProgram(curlPath);
        task.process->setArguments({
            QStringLiteral("--location"),
            QStringLiteral("--fail"),
            QStringLiteral("--retry"), QStringLiteral("3"),
            QStringLiteral("--retry-delay"), QStringLiteral("2"),
            QStringLiteral("--continue-at"), QStringLiteral("-"),
            QStringLiteral("--output"), QDir(rootPath).filePath(spec.includePattern),
            spec.directUrl
        });
    }
    task.process->setWorkingDirectory(kylinAgentDataPath());

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("HF_ENDPOINT"), QStringLiteral("https://hf-mirror.com"));
    task.process->setProcessEnvironment(env);

    connect(task.process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, modelId = task.modelId](int exitCode, QProcess::ExitStatus exitStatus) {
                Task &current = m_tasks[modelId];
                QProcess *process = current.process;
                if (!process) {
                    return;
                }

                const QString processOutput =
                    QString::fromUtf8(process->readAllStandardError() + process->readAllStandardOutput()).trimmed();
                process->deleteLater();
                current.process = nullptr;

                if (exitStatus != QProcess::NormalExit || exitCode != 0) {
                    failTask(current, processOutput.isEmpty()
                        ? tr("模型下载失败，下载进程退出码：%1").arg(exitCode)
                        : processOutput);
                    return;
                }

                finishCurrentFile(current);
            });

    connect(task.process, &QProcess::errorOccurred, this,
            [this, modelId = task.modelId](QProcess::ProcessError error) {
                Task &current = m_tasks[modelId];
                if (!current.process) {
                    return;
                }
                failTask(current, tr("无法启动模型下载工具：%1").arg(static_cast<int>(error)));
            });

    task.process->start();
    if (!task.process->waitForStarted(3000)) {
        failTask(task, tr("无法启动模型下载工具：%1").arg(task.process->errorString()));
        return;
    }
    emit downloadChanged(task.modelId);
}

void ModelDownloadService::finishCurrentFile(Task &task)
{
    const FileSpec spec = task.files.at(task.fileIndex);
    const QString rootPath = modelRootPath(task.modelId);
    const QString downloadedPath = QDir(rootPath).filePath(spec.includePattern);
    const QString targetPath = QDir(rootPath).filePath(spec.fileName);
    if (!QFileInfo::exists(downloadedPath)) {
        failTask(task, tr("模型下载完成但未找到文件：%1").arg(downloadedPath));
        return;
    }
    if (downloadedPath != targetPath) {
        QFile::remove(targetPath);
        if (!QFile::rename(downloadedPath, targetPath)) {
            failTask(task, tr("无法重命名模型文件：%1 -> %2").arg(downloadedPath, targetPath));
            return;
        }
    }

    ++task.fileIndex;
    if (!task.files.isEmpty()) {
        int totalWeight = 0;
        int completedWeight = 0;
        for (int i = 0; i < task.files.size(); ++i) {
            const int weight = qMax(1, task.files.at(i).weight);
            totalWeight += weight;
            if (i < task.fileIndex) {
                completedWeight += weight;
            }
        }
        task.progress = qBound(0, (completedWeight * 100) / qMax(1, totalWeight), 99);
        emit downloadChanged(task.modelId);
    }
    startNextFile(task);
}

void ModelDownloadService::failTask(Task &task, const QString &message)
{
    if (task.reply) {
        task.reply->deleteLater();
        task.reply = nullptr;
    }
    if (task.process) {
        task.process->kill();
        task.process->deleteLater();
        task.process = nullptr;
    }
    closeTaskFile(task);
    task.state = State::Failed;
    task.error = message;
    m_downloadMetrics.remove(task.modelId);
    emit downloadChanged(task.modelId);
}

void ModelDownloadService::closeTaskFile(Task &task)
{
    if (!task.file) {
        return;
    }
    task.file->flush();
    task.file->close();
    delete task.file;
    task.file = nullptr;
}

void ModelDownloadService::closeTaskProcess(Task &task)
{
    if (!task.process) {
        return;
    }
    task.process->disconnect(this);
    if (task.process->state() != QProcess::NotRunning) {
        task.process->kill();
        task.process->waitForFinished(1000);
    }
    task.process->deleteLater();
    task.process = nullptr;
}

bool ModelDownloadService::ensureHfdScript(QString *errorMessage) const
{
    const QString scriptPath = hfdScriptPath();
    QFileInfo scriptInfo(scriptPath);
    if (scriptInfo.exists() && scriptInfo.isFile() && scriptInfo.isExecutable()) {
        return true;
    }

    if (!QDir().mkpath(kylinAgentDataPath())) {
        if (errorMessage) {
            *errorMessage = tr("无法创建下载工具目录：%1").arg(kylinAgentDataPath());
        }
        return false;
    }

    const QString tmpPath = scriptPath + QStringLiteral(".tmp");
    QFile::remove(tmpPath);

    auto runDownloader = [&](const QString &program, const QStringList &arguments) {
        QProcess process;
        process.setProgram(program);
        process.setArguments(arguments);
        process.setWorkingDirectory(kylinAgentDataPath());
        process.start();
        if (!process.waitForStarted(3000)) {
            return false;
        }
        if (!process.waitForFinished(60000)) {
            process.kill();
            process.waitForFinished(1000);
            return false;
        }
        return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
    };

    bool downloaded = runDownloader(QStringLiteral("wget"), {
        QStringLiteral("https://hf-mirror.com/hfd/hfd.sh"),
        QStringLiteral("-O"),
        tmpPath
    });
    if (!downloaded) {
        downloaded = runDownloader(QStringLiteral("curl"), {
            QStringLiteral("-fsSL"),
            QStringLiteral("https://hf-mirror.com/hfd/hfd.sh"),
            QStringLiteral("-o"),
            tmpPath
        });
    }

    if (!downloaded || !QFileInfo::exists(tmpPath)) {
        QFile::remove(tmpPath);
        if (errorMessage) {
            *errorMessage = tr("无法从 hf-mirror.com 下载 hfd.sh，请检查网络或手动安装：%1").arg(scriptPath);
        }
        return false;
    }

    QFile::remove(scriptPath);
    if (!QFile::rename(tmpPath, scriptPath)) {
        QFile::remove(tmpPath);
        if (errorMessage) {
            *errorMessage = tr("无法保存下载工具：%1").arg(scriptPath);
        }
        return false;
    }

    if (!QFile::setPermissions(scriptPath,
                               QFileDevice::ReadOwner |
                               QFileDevice::WriteOwner |
                               QFileDevice::ExeOwner |
                               QFileDevice::ReadGroup |
                               QFileDevice::ExeGroup |
                               QFileDevice::ReadOther |
                               QFileDevice::ExeOther)) {
        if (errorMessage) {
            *errorMessage = tr("无法设置 hfd.sh 可执行权限：%1").arg(scriptPath);
        }
        return false;
    }

    return true;
}

bool ModelDownloadService::writeMd5File(const QString &modelId, QString *errorMessage) const
{
    const QVector<FileSpec> files = filesForModel(modelId);
    const QString rootPath = modelRootPath(modelId);
    QStringList lines;
    for (const FileSpec &spec : files) {
        const QString filePath = QDir(rootPath).filePath(spec.fileName);
        const QString md5 = md5ForFile(filePath);
        if (md5.isEmpty()) {
            if (errorMessage) {
                *errorMessage = tr("无法计算模型文件校验值：%1").arg(filePath);
            }
            return false;
        }
        lines.append(QStringLiteral("%1  %2").arg(md5, spec.fileName));
    }

    QFile md5File(QDir(rootPath).filePath(QStringLiteral("md5sum")));
    if (!md5File.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = tr("无法写入 md5sum 文件：%1").arg(md5File.fileName());
        }
        return false;
    }
    md5File.write(lines.join(QLatin1Char('\n')).toUtf8());
    md5File.write("\n");
    return true;
}

bool ModelDownloadService::registerModelRuntime(const QString &modelId, QString *errorMessage)
{
    const QString rootPath = modelRootPath(modelId);
    if (markerExists(rootPath, QStringLiteral("install"))) {
        return true;
    }

    QJsonObject source;
    source.insert(QStringLiteral("type"), QStringLiteral("local_path"));
    source.insert(QStringLiteral("uri"), QDir(rootPath).filePath(QStringLiteral("model.gguf")));

    QJsonObject payload;
    payload.insert(QStringLiteral("model"), modelId);
    payload.insert(QStringLiteral("task"), QStringLiteral("chat"));
    payload.insert(QStringLiteral("runtime"), QStringLiteral("llama_cpp"));
    payload.insert(QStringLiteral("source"), source);
    const QString mmprojPath = QDir(rootPath).filePath(QStringLiteral("mmproj.gguf"));
    if (QFileInfo::exists(mmprojPath)) {
        payload.insert(QStringLiteral("mmproj"), mmprojPath);
    }
    payload.insert(QStringLiteral("force"), true);

    auto postInstall = [&](QString *error) {
        QNetworkRequest request(QUrl(QStringLiteral("http://127.0.0.1:18080/v1/models:install")));
        request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        request.setTransferTimeout(15000);
        QNetworkReply *reply = m_network.post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));

        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        const bool ok = reply->error() == QNetworkReply::NoError;
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QString networkError = reply->errorString();
        const QByteArray body = reply->readAll();
        reply->deleteLater();

        if (!ok || status < 200 || status >= 300) {
            if (error) {
                const QString statusText = status > 0 ? tr("HTTP %1，").arg(status) : QString();
                *error = tr("模型推理服务注册失败：%1%2%3")
                    .arg(statusText,
                         networkError,
                         body.isEmpty() ? QString() : QStringLiteral(" - ") + QString::fromUtf8(body));
            }
            return false;
        }
        return true;
    };

    QString registerError;
    if (!postInstall(&registerError)) {
        QString grantError;
        if (!grantModelGatewayAccess(&grantError)) {
            if (errorMessage) {
                *errorMessage = registerError + QStringLiteral("\n") + grantError;
            }
            return false;
        }
        if (!postInstall(&registerError)) {
            if (errorMessage) {
                *errorMessage = registerError;
            }
            return false;
        }
    }

    if (!writeMarker(rootPath, QStringLiteral("install"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate))) {
        if (errorMessage) {
            *errorMessage = tr("模型已注册，但无法写入 install 标记文件：%1")
                .arg(markerPath(rootPath, QStringLiteral("install")));
        }
        return false;
    }
    return true;
}

bool ModelDownloadService::grantModelGatewayAccess(QString *errorMessage)
{
    const QString user = currentSystemUser();
    if (user.isEmpty()) {
        if (errorMessage) {
            *errorMessage = tr("无法获取当前系统用户名，不能授权模型目录访问。");
        }
        return false;
    }

    bool ok = false;
    const QString password = KylinInputDialog::getText(
        QApplication::activeWindow(),
        tr("模型目录授权"),
        tr("注册模型需要模型目录的访问权限。\n请输入当前系统用户“%1”的密码以继续。").arg(user),
        QLineEdit::Password,
        QString(),
        &ok);
    if (!ok) {
        if (errorMessage) {
            *errorMessage = tr("已取消模型目录授权。");
        }
        return false;
    }

    QProcess process;
    process.setProgram(QStringLiteral("sudo"));
    process.setArguments({
        QStringLiteral("-S"),
        QStringLiteral("-p"),
        QString(),
        QStringLiteral("model-gateway-grant-access"),
        QStringLiteral("--path"),
        fixedModelRootPath(),
        QStringLiteral("--user"),
        user
    });
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start();
    if (!process.waitForStarted(5000)) {
        if (errorMessage) {
            *errorMessage = tr("无法启动授权命令：sudo model-gateway-grant-access。");
        }
        return false;
    }

    process.write(password.toUtf8());
    process.write("\n");
    process.closeWriteChannel();

    if (!process.waitForFinished(120000)) {
        process.kill();
        process.waitForFinished(3000);
        if (errorMessage) {
            *errorMessage = tr("模型目录授权命令超时。");
        }
        return false;
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        if (errorMessage) {
            const QString output = QString::fromUtf8(process.readAll()).trimmed();
            *errorMessage = output.isEmpty()
                ? tr("模型目录授权失败。")
                : tr("模型目录授权失败：%1").arg(output);
        }
        return false;
    }

    return true;
}

bool ModelDownloadService::addModelToBackendConfig(const QString &modelId, QString *errorMessage)
{
    const QString rootPath = modelRootPath(modelId);
    if (markerExists(rootPath, QStringLiteral("config")) && backendConfigLooksReady(modelId)) {
        return true;
    }

    QJsonArray array;
    bool replaced = false;
    const QString fixedId = QStringLiteral("fixed-local/%1").arg(modelId);
    const QString provider = fixedModelProviderName(modelId);
    QJsonObject fixedModel;
    fixedModel.insert(QStringLiteral("id"), fixedId);
    fixedModel.insert(QStringLiteral("name"), modelId);
    fixedModel.insert(QStringLiteral("provider"), provider);
    fixedModel.insert(QStringLiteral("model"), modelId);
    fixedModel.insert(QStringLiteral("baseUrl"), QStringLiteral("http://127.0.0.1:18080/v1"));
    fixedModel.insert(QStringLiteral("readOnly"), false);
    array.append(fixedModel);

    const QVector<QMap<QString, QVariant>> models = ApiService::instance().getConfigModelsSync();
    for (const QMap<QString, QVariant> &model : models) {
        QJsonObject obj;
        for (auto it = model.begin(); it != model.end(); ++it) {
            obj.insert(it.key(), QJsonValue::fromVariant(it.value()));
        }
        if (obj.value(QStringLiteral("id")).toString() == fixedId ||
            (obj.value(QStringLiteral("baseUrl")).toString() == QStringLiteral("http://127.0.0.1:18080/v1") &&
             obj.value(QStringLiteral("model")).toString() == modelId)) {
            replaced = true;
            continue;
        }
        array.append(obj);
    }

    Q_UNUSED(replaced);

    if (!ApiService::instance().replaceConfigModelsSync(array, modelId)) {
        if (errorMessage) {
            *errorMessage = tr("模型已下载，但写入后端模型配置失败。");
        }
        return false;
    }
    ApiService::instance().loadModels();
    qDebug() << "Model config was written; keeping existing kylin-agent-runtime service process running";
    if (!writeMarker(rootPath, QStringLiteral("config"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate))) {
        if (errorMessage) {
            *errorMessage = tr("模型配置已写入，但无法写入 config 标记文件：%1")
                .arg(markerPath(rootPath, QStringLiteral("config")));
        }
        return false;
    }
    return true;
}

void ModelDownloadService::updateProgress(Task &task, qint64 received, qint64 total)
{
    if (!task.progressReady) {
        return;
    }
    if (total <= 0 || task.files.isEmpty()) {
        return;
    }

    int totalWeight = 0;
    int completedWeight = 0;
    for (int i = 0; i < task.files.size(); ++i) {
        const int weight = qMax(1, task.files.at(i).weight);
        totalWeight += weight;
        if (i < task.fileIndex) {
            completedWeight += weight;
        }
    }

    if (totalWeight <= 0) {
        return;
    }

    const int currentWeight = qMax(1, task.files.at(task.fileIndex).weight);
    const qint64 currentWeightedProgress = (received * currentWeight) / total;
    const int nextProgress = qBound(
        0,
        static_cast<int>(((completedWeight + currentWeightedProgress) * 100) / totalWeight),
        99);
    if (nextProgress > task.progress) {
        task.progress = nextProgress;
        emit downloadChanged(task.modelId);
    }
}

qint64 ModelDownloadService::currentDownloadedBytes(const QString &modelId, const Task &task) const
{
    if (task.files.isEmpty()) {
        return 0;
    }

    qint64 bytes = 0;
    const QDir root(modelRootPath(modelId));
    for (const FileSpec &spec : task.files) {
        const QString targetPath = root.filePath(spec.fileName);
        const QString sourcePath = root.filePath(spec.includePattern);
        bytes += qMax(downloadedFileBytes(targetPath), downloadedFileBytes(sourcePath));
    }
    return bytes;
}

qint64 ModelDownloadService::expectedDownloadBytes(const Task &task) const
{
    qint64 bytes = 0;
    for (const FileSpec &spec : task.files) {
        if (spec.expectedBytes <= 0) {
            return 0;
        }
        bytes += spec.expectedBytes;
    }
    return bytes;
}
