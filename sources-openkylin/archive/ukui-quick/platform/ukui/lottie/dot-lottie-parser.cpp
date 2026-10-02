/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: amingamingaming <wangyiming01@kylinos.cn>
 *
 */

#include "dot-lottie-parser.h"
#include "dot-lottie-parser-json_p.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>
#include <QTemporaryDir>

#include <utility>

namespace UkuiQuick {

namespace {

// 调用 unzip 命令的最长等待时间。
constexpr int kUnzipTimeoutMs = 30 * 1000;
// 等待 unzip 启动或退出时的检查间隔。间隔较短，取消请求可以及时生效。
constexpr int kUnzipPollMs = 50;
// 终止 unzip 后，等待操作系统回收子进程的最长时间，避免留下僵尸进程。
constexpr int kUnzipReapTimeoutMs = 5 * 1000;

// 标记解压目录已完整发布的完成文件名。
const QString kCompleteMarker = QStringLiteral(".complete");

// 调用方取消加载时使用的统一错误描述。
const QString kCancelledError = QStringLiteral("asset work was cancelled");

using CancellationCheck = UkuiQuick::DotLottieParser::CancellationCheck;

// 返回调用方是否已经取消当前加载。
bool cancellationRequested(const CancellationCheck &isCancelled)
{
    return isCancelled && isCancelled();
}

// 在调用方提供错误输出参数时写入错误描述。
void setErrorString(QString *errorString, const QString &message)
{
    if (errorString) {
        *errorString = message;
    }
}

// 检查动画名和主题名，拒绝“../”、路径分隔符及平台保留字符。
bool isSafeIdentifier(const QString &id)
{
    return !id.isEmpty() &&
           id.size() <= 128 &&
           id != QStringLiteral(".") &&
           id != QStringLiteral("..") &&
           !id.contains(QLatin1Char('/')) &&
           !id.contains(QLatin1Char('\\')) &&
           !id.contains(QLatin1Char(':')) &&
           !id.contains(QChar::Null);
}

// 清理路径中的“.”和“..”后，确认它仍位于指定根目录内。
bool pathIsWithin(const QString &rootPath, const QString &candidatePath)
{
    if (rootPath.isEmpty() || candidatePath.isEmpty()) {
        return false;
    }
    const QString prefix = rootPath.endsWith(QLatin1Char('/'))
        ? rootPath
        : rootPath + QLatin1Char('/');
    return candidatePath == rootPath || candidatePath.startsWith(prefix);
}

// 返回存在、非符号链接目录的规范化路径。
QString canonicalDirectoryPath(const QString &path)
{
    const QFileInfo info(path);
    if (!info.exists() || !info.isDir() || info.isSymLink()) {
        return {};
    }
    return info.canonicalFilePath();
}

// 返回位于可信根目录中的现有常规文件，拒绝符号链接与越界路径。
QString safeExistingFile(const QString &rootPath, const QString &candidatePath)
{
    const QString canonicalRoot = canonicalDirectoryPath(rootPath);
    const QFileInfo candidateInfo(candidatePath);
    if (canonicalRoot.isEmpty() ||
        !candidateInfo.exists() ||
        !candidateInfo.isFile() ||
        candidateInfo.isSymLink()) {
        return {};
    }

    const QString canonicalCandidate = candidateInfo.canonicalFilePath();
    return pathIsWithin(canonicalRoot, canonicalCandidate) ? canonicalCandidate : QString();
}

// 动画验证时顺带提取的帧元数据。
struct AnimationMetadata
{
    int frameCount = 0;
    qreal frameRate = 0.0;
    QByteArray animationData;
    QStringList imageResourcePaths;
};

// 把已经验证的动画元数据和解压目录一次写入资产，避免调用方分别传递同一组字段。
void configureExtractedAsset(LottieAsset &asset,
                             const QString &sourcePath,
                             const QString &canonicalExtractDir,
                             const QString &animationJsonPath,
                             const AnimationMetadata &metadata)
{
    const QDir extractDir(canonicalExtractDir);
    const QFileInfo animationInfo(animationJsonPath);
    QStringList resourceSearchDirs{
        animationInfo.absolutePath(),
        canonicalExtractDir,
        extractDir.filePath(QStringLiteral("images")),
        extractDir.filePath(QStringLiteral("i")),
        extractDir.filePath(QStringLiteral("a")),
        extractDir.filePath(QStringLiteral("animations"))
    };
    const QFileInfo archiveInfo(sourcePath);
    if (!sourcePath.startsWith(QLatin1Char(':')) &&
        archiveInfo.exists() &&
        archiveInfo.isFile() &&
        !archiveInfo.isSymLink()) {
        resourceSearchDirs << archiveInfo.absolutePath()
                           << archiveInfo.absoluteDir().filePath(QStringLiteral("images"))
                           << archiveInfo.absoluteDir().filePath(QStringLiteral("i"));
    }

    asset.setExtractDir(canonicalExtractDir);
    asset.setAnimationJsonPath(animationInfo.canonicalFilePath());
    asset.setAnimationJsonData(metadata.animationData);
    asset.setFrameCount(metadata.frameCount);
    asset.setFrameRate(metadata.frameRate);
    asset.setResourceDir(animationInfo.absolutePath());
    asset.setResourceSearchDirs(resourceSearchDirs);
    asset.setImageResourcePaths(metadata.imageResourcePaths);
}

// 检查解析后的 JSON 是否包含合法 Lottie 字段，并读取帧数、帧率等元数据。
bool validateLottieAnimation(const QJsonDocument &document, AnimationMetadata *metadata = nullptr)
{
    if (!document.isObject()) {
        return false;
    }
    const QJsonObject object = document.object();
    if (!(object.value(QStringLiteral("fr")).isDouble() &&
          object.value(QStringLiteral("ip")).isDouble() &&
          object.value(QStringLiteral("op")).isDouble() &&
          object.value(QStringLiteral("w")).isDouble() &&
          object.value(QStringLiteral("h")).isDouble() &&
          object.value(QStringLiteral("layers")).isArray())) {
        return false;
    }
    if (metadata) {
        metadata->frameRate = object.value(QStringLiteral("fr")).toDouble();
        const int inPoint = qRound(object.value(QStringLiteral("ip")).toDouble());
        const int outPoint = qRound(object.value(QStringLiteral("op")).toDouble());
        metadata->frameCount = qMax(0, outPoint - inPoint);
    }
    int imageDeclarationCount = 0;
    for (const QJsonValue &assetValue : object.value(QStringLiteral("assets")).toArray()) {
        const QJsonObject imageAsset = assetValue.toObject();
        const QString imageSource = imageAsset.value(QStringLiteral("p")).toString();
        if (imageSource.isEmpty()) {
            continue;
        }
        if (++imageDeclarationCount > DotLottieParser::maxImageResources()) {
            return false;
        }
        if (metadata) {
            const QString logicalPath = imageSource.startsWith(
                                            QStringLiteral("data:"),
                                            Qt::CaseInsensitive)
                ? imageSource
                : imageAsset.value(QStringLiteral("u")).toString() + imageSource;
            metadata->imageResourcePaths.append(logicalPath);
        }
    }
    return true;
}

// 读取并验证候选文件是否为可用的 Lottie 动画。
bool isValidLottieAnimationFile(const QString &path,
                                AnimationMetadata *metadata = nullptr,
                                const CancellationCheck &isCancelled = {})
{
    const LottieParserInternal::JsonParseResult parsed =
        LottieParserInternal::parseJsonFile(path, isCancelled);
    if (parsed.wasCancelled() || !parsed.hasInput) {
        return false;
    }
    if (parsed.parseError.error != QJsonParseError::NoError ||
        !validateLottieAnimation(parsed.document, metadata)) {
        return false;
    }
    if (metadata) {
        metadata->animationData = parsed.inputData;
    }
    return true;
}

// 只有带完成标记的解压目录才可使用，避免读到解压一半的内容。
bool isPublishedExtractDirectory(const QString &path)
{
    if (canonicalDirectoryPath(path).isEmpty()) {
        return false;
    }
    const QFileInfo markerInfo(QDir(path).filePath(kCompleteMarker));
    return markerInfo.exists() && markerInfo.isFile() && !markerInfo.isSymLink();
}

// 验证归档条目路径并输出安全的相对路径。
bool safeArchivePath(const QByteArray &pathBytes, QString *cleanPath, QString *errorString)
{
    if (pathBytes.isEmpty()) {
        setErrorString(errorString, QStringLiteral("archive entry path is empty"));
        return false;
    }

    const QString path = QString::fromUtf8(pathBytes);
    if (path.toUtf8() != pathBytes ||
        path.contains(QChar::Null) ||
        path.contains(QLatin1Char('\r')) ||
        path.contains(QLatin1Char('\n')) ||
        path.contains(QLatin1Char('\\')) ||
        path.startsWith(QLatin1Char('/')) ||
        QDir::isAbsolutePath(path) ||
        (path.size() >= 2 && path.at(1) == QLatin1Char(':'))) {
        setErrorString(errorString, QStringLiteral("archive entry path is not a safe relative UTF-8 path"));
        return false;
    }

    QString normalized = path;
    while (normalized.endsWith(QLatin1Char('/'))) {
        normalized.chop(1);
    }
    const QStringList components = normalized.split(QLatin1Char('/'), Qt::KeepEmptyParts);
    if (components.isEmpty()) {
        setErrorString(errorString, QStringLiteral("archive entry path is empty"));
        return false;
    }
    for (const QString &component : components) {
        if (component.isEmpty() ||
            component == QStringLiteral(".") ||
            component == QStringLiteral("..") ||
            component == kCompleteMarker) {
            setErrorString(errorString, QStringLiteral("archive entry path contains a forbidden component"));
            return false;
        }
    }

    *cleanPath = components.join(QLatin1Char('/'));
    return true;
}

// 反复检查已终止的子进程是否被系统回收；超过上限后不再无限等待。
bool reapProcess(QProcess &process)
{
    QElapsedTimer timer;
    timer.start();
    while (!process.waitForFinished(kUnzipPollMs)) {
        if (timer.elapsed() >= kUnzipReapTimeoutMs) {
            break;
        }
    }
    return process.state() == QProcess::NotRunning;
}

// 使用系统 unzip 读取归档目录或执行解压。调用方取消时会终止子进程，
// 等待系统回收它，并通过 wasCancelled 告知上层这不是普通解压错误。
bool runUnzip(const QString &executable,
              const QStringList &arguments,
              QByteArray *standardOutput,
              QString *errorString,
              const CancellationCheck &isCancelled = {},
              bool *wasCancelled = nullptr)
{
    if (wasCancelled) {
        *wasCancelled = false;
    }
    if (executable.isEmpty()) {
        setErrorString(errorString, QStringLiteral("unzip command is unavailable"));
        return false;
    }

    QProcess process;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.remove(QStringLiteral("UNZIP"));
    environment.remove(QStringLiteral("UNZIPOPT"));
    environment.remove(QStringLiteral("ZIPINFO"));
    environment.remove(QStringLiteral("ZIPINFOOPT"));
    environment.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
    environment.insert(QStringLiteral("LANG"), QStringLiteral("C"));
    process.setProcessEnvironment(environment);
    process.setProgram(executable);
    process.setArguments(arguments);
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start();
    {
        // 启动过程中也定期检查取消，不必等到 unzip 完全启动后才能停止。
        QElapsedTimer startTimer;
        startTimer.start();
        while (!process.waitForStarted(kUnzipPollMs)) {
            if (process.error() == QProcess::FailedToStart) {
                setErrorString(errorString, QStringLiteral("failed to start unzip: ") + process.errorString());
                return false;
            }
            if (cancellationRequested(isCancelled)) {
                process.kill();
                const bool reaped = reapProcess(process);
                if (wasCancelled) {
                    *wasCancelled = true;
                }
                setErrorString(errorString, reaped
                                   ? kCancelledError
                                   : QStringLiteral("failed to reap unzip after cancellation"));
                return false;
            }
            if (startTimer.elapsed() >= kUnzipTimeoutMs) {
                process.kill();
                const bool reaped = reapProcess(process);
                setErrorString(errorString, reaped
                                   ? QStringLiteral("failed to start unzip: ") + process.errorString()
                                   : QStringLiteral("failed to reap unzip after start timeout"));
                return false;
            }
        }
    }
    process.closeWriteChannel();

    QByteArray output;
    QByteArray standardError;
    QElapsedTimer timer;
    timer.start();
    while (process.state() != QProcess::NotRunning) {
        if (cancellationRequested(isCancelled)) {
            process.kill();
            const bool reaped = reapProcess(process);
            if (wasCancelled) {
                *wasCancelled = true;
            }
            setErrorString(errorString, reaped
                               ? kCancelledError
                               : QStringLiteral("failed to reap unzip after cancellation"));
            return false;
        }
        const int remainingMs = kUnzipTimeoutMs - static_cast<int>(timer.elapsed());
        if (remainingMs <= 0) {
            process.kill();
            const bool reaped = reapProcess(process);
            setErrorString(errorString, reaped
                               ? QStringLiteral("unzip command timed out")
                               : QStringLiteral("failed to reap unzip after timeout"));
            return false;
        }
        process.waitForReadyRead(qMin(remainingMs, kUnzipPollMs));
        output.append(process.readAllStandardOutput());
        standardError.append(process.readAllStandardError());
    }
    output.append(process.readAllStandardOutput());
    standardError.append(process.readAllStandardError());
    if (standardOutput) {
        *standardOutput = output;
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        const QString detail = QString::fromLocal8Bit(standardError).trimmed();
        setErrorString(errorString,
                       detail.isEmpty()
                           ? QStringLiteral("unzip command failed")
                           : QStringLiteral("unzip command failed: ") + detail);
        return false;
    }
    return true;
}

// 先根据 unzip -Z 列出的文件检查路径、数量、大小和压缩比。全部通过后才
// 真正解压，避免不可信归档先写入磁盘。
bool validateUnzipArchive(const QString &unzipExecutable,
                          const QString &archivePath,
                          QString *errorString,
                          const CancellationCheck &isCancelled = {},
                          bool *wasCancelled = nullptr)
{
    if (wasCancelled) {
        *wasCancelled = false;
    }

    QByteArray nameListing;
    bool cancelled = false;
    if (!runUnzip(unzipExecutable,
                  {QStringLiteral("-Z"), QStringLiteral("-1"), archivePath},
                  &nameListing,
                  errorString,
                  isCancelled,
                  &cancelled)) {
        if (wasCancelled) {
            *wasCancelled = cancelled;
        }
        return false;
    }

    QList<QByteArray> rawPaths = nameListing.split('\n');
    while (!rawPaths.isEmpty() && rawPaths.constLast().isEmpty()) {
        rawPaths.removeLast();
    }
    if (rawPaths.isEmpty()) {
        setErrorString(errorString, QStringLiteral("archive contains no entries"));
        return false;
    }
    if (rawPaths.size() > DotLottieParser::maxArchiveEntries()) {
        setErrorString(errorString, QStringLiteral("archive entry count exceeds the budget"));
        return false;
    }
    QStringList listedPaths;
    QSet<QString> extractedPaths;
    for (const QByteArray &rawPath : std::as_const(rawPaths)) {
        QString relativePath;
        if (!safeArchivePath(rawPath, &relativePath, errorString)) {
            return false;
        }
        if (extractedPaths.contains(relativePath)) {
            setErrorString(errorString, QStringLiteral("archive contains duplicate normalized paths"));
            return false;
        }
        extractedPaths.insert(relativePath);
        listedPaths.append(QString::fromUtf8(rawPath));
    }

    if (cancellationRequested(isCancelled)) {
        if (wasCancelled) {
            *wasCancelled = true;
        }
        setErrorString(errorString, kCancelledError);
        return false;
    }

    QByteArray detailListing;
    if (!runUnzip(unzipExecutable,
                  {QStringLiteral("-Z"), QStringLiteral("-l"), archivePath},
                  &detailListing,
                  errorString,
                  isCancelled,
                  &cancelled)) {
        if (wasCancelled) {
            *wasCancelled = cancelled;
        }
        return false;
    }

    // zipinfo 每行包含权限、声明的解压大小、压缩大小和文件名等字段；
    // 这里使用第 2 个正则捕获组取得归档声明的解压大小。
    static const QRegularExpression entryPattern(
        QStringLiteral(R"(^(\S+)\s+\S+\s+\S+\s+(\d+)\s+\S+\s+\d+\s+\S+\s+\S+\s+\S+\s+(.+)$)"));
    int entryIndex = 0;
    qint64 totalExpandedBytes = 0;
    const QStringList detailLines = QString::fromUtf8(detailListing).split(QLatin1Char('\n'));
    for (QString line : detailLines) {
        if (line.endsWith(QLatin1Char('\r'))) {
            line.chop(1);
        }
        const QRegularExpressionMatch match = entryPattern.match(line);
        if (!match.hasMatch()) {
            continue;
        }
        if (entryIndex >= listedPaths.size() || match.captured(3) != listedPaths.at(entryIndex)) {
            setErrorString(errorString, QStringLiteral("unzip entry listing is inconsistent"));
            return false;
        }

        const QChar entryType = match.captured(1).at(0);
        if (entryType != QLatin1Char('-') && entryType != QLatin1Char('d')) {
            setErrorString(errorString, QStringLiteral("archive links and special files are not allowed"));
            return false;
        }

        bool sizeOk = false;
        const qint64 expandedBytes = match.captured(2).toLongLong(&sizeOk);
        if (!sizeOk || expandedBytes < 0) {
            setErrorString(errorString, QStringLiteral("unzip entry size is invalid"));
            return false;
        }
        if (expandedBytes > DotLottieParser::maxExpandedEntryBytes()) {
            setErrorString(errorString, QStringLiteral("archive entry exceeds the expanded size budget"));
            return false;
        }
        totalExpandedBytes += expandedBytes;
        if (totalExpandedBytes > DotLottieParser::maxTotalExpandedBytes()) {
            setErrorString(errorString, QStringLiteral("archive expanded total exceeds the budget"));
            return false;
        }

        ++entryIndex;
    }

    if (entryIndex != listedPaths.size()) {
        setErrorString(errorString, QStringLiteral("unzip entry metadata is incomplete"));
        return false;
    }

    // 用归档文件的实际字节数计算压缩比，拒绝解压后异常膨胀的压缩炸弹。
    const qint64 compressedBytes = QFileInfo(archivePath).size();
    if (compressedBytes <= 0) {
        setErrorString(errorString, QStringLiteral("archive input is empty"));
        return false;
    }
    if (totalExpandedBytes > DotLottieParser::maxCompressionRatio() * compressedBytes) {
        setErrorString(errorString, QStringLiteral("archive compression ratio exceeds the budget"));
        return false;
    }
    return true;
}

// 解压完成后再次遍历结果：拒绝符号链接和特殊文件，限制文件权限，并按实际
// 文件大小重新检查单文件和总大小。不能只相信归档自己声明的数据。
bool secureExtractedDirectory(const QString &rootPath,
                              QString *errorString,
                              const CancellationCheck &isCancelled = {})
{
    const QString canonicalRoot = canonicalDirectoryPath(rootPath);
    if (canonicalRoot.isEmpty()) {
        setErrorString(errorString, QStringLiteral("extract directory is invalid or a symbolic link"));
        return false;
    }

    qint64 totalExpandedBytes = 0;
    QDirIterator iterator(canonicalRoot,
                          QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot,
                          QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        if (cancellationRequested(isCancelled)) {
            setErrorString(errorString, kCancelledError);
            return false;
        }
        const QString entryPath = iterator.next();
        const QFileInfo entryInfo = iterator.fileInfo();
        if (entryInfo.isSymLink() || (!entryInfo.isDir() && !entryInfo.isFile())) {
            setErrorString(errorString, QStringLiteral("unzip produced a link or special file"));
            return false;
        }
        if (entryInfo.isFile()) {
            const qint64 entryBytes = entryInfo.size();
            if (entryBytes > DotLottieParser::maxExpandedEntryBytes()) {
                setErrorString(errorString, QStringLiteral("archive entry exceeds the expanded size budget"));
                return false;
            }
            totalExpandedBytes += entryBytes;
            if (totalExpandedBytes > DotLottieParser::maxTotalExpandedBytes()) {
                setErrorString(errorString, QStringLiteral("archive expanded total exceeds the budget"));
                return false;
            }
        }

        const QString canonicalEntry = entryInfo.isDir()
            ? canonicalDirectoryPath(entryPath)
            : safeExistingFile(canonicalRoot, entryPath);
        if (!pathIsWithin(canonicalRoot, canonicalEntry)) {
            setErrorString(errorString, QStringLiteral("unzip output escaped the extract root"));
            return false;
        }

        const QFileDevice::Permissions permissions = entryInfo.isDir()
            ? QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner
            : QFileDevice::ReadOwner | QFileDevice::WriteOwner;
        if (!QFile(entryPath).setPermissions(permissions)) {
            setErrorString(errorString, QStringLiteral("failed to secure unzip output permissions"));
            return false;
        }
    }
    return true;
}

// 读取 JSON 对象中的字符串字段。
QString stringValue(const QJsonObject &object, const QString &key)
{
    return object.value(key).toString();
}

// 读取指定数量的数值数组元素，缺失元素以默认值填充。
QJsonArray numericArrayPrefix(const QJsonValue &value, int count, double fallback = 0.0)
{
    QJsonArray result;
    const QJsonArray source = value.toArray();
    for (int i = 0; i < count; ++i) {
        result.append(i < source.size() && source.at(i).isDouble()
                          ? source.at(i).toDouble()
                          : fallback);
    }
    return result;
}

// 将颜色值规范化为 RGB 或 RGBA 数组。
QJsonArray colorArrayValue(const QJsonValue &value)
{
    const QJsonArray source = value.toArray();
    return numericArrayPrefix(value, source.size() >= 4 ? 4 : 3, 1.0);
}

// 读取可选 JSON 字段，并统一保留未定义状态。
QJsonValue optionalValue(const QJsonObject &object, const QString &key)
{
    const QJsonValue value = object.value(key);
    return value.isUndefined() ? QJsonValue() : value;
}

// 将源对象中存在且非空的字段复制为目标字段。
void insertIfPresent(QJsonObject &object,
                     const QString &targetKey,
                     const QJsonObject &source,
                     const QString &sourceKey)
{
    const QJsonValue value = optionalValue(source, sourceKey);
    if (!value.isUndefined() && !value.isNull()) {
        object.insert(targetKey, value);
    }
}

// 把 dotLottie 的贝塞尔控制点转换成 ThorVG 主题规则需要的 JSON 对象。
QJsonObject bezierObject(const QJsonObject &source)
{
    QJsonObject result;
    insertIfPresent(result, QStringLiteral("x"), source, QStringLiteral("x"));
    insertIfPresent(result, QStringLiteral("y"), source, QStringLiteral("y"));
    return result;
}

// 转换纯色关键帧时，保留它的切线和“保持当前值”标记。
void insertColorKeyframeFields(QJsonObject &target, const QJsonObject &source)
{
    const QJsonValue inTangent = optionalValue(source, QStringLiteral("inTangent"));
    if (inTangent.isObject()) {
        target.insert(QStringLiteral("i"), bezierObject(inTangent.toObject()));
    }

    const QJsonValue outTangent = optionalValue(source, QStringLiteral("outTangent"));
    if (outTangent.isObject()) {
        target.insert(QStringLiteral("o"), bezierObject(outTangent.toObject()));
    }

    insertIfPresent(target, QStringLiteral("ti"), source, QStringLiteral("valueInTangent"));
    insertIfPresent(target, QStringLiteral("to"), source, QStringLiteral("valueOutTangent"));

    if (source.contains(QStringLiteral("hold"))) {
        target.insert(QStringLiteral("h"), source.value(QStringLiteral("hold")).toBool() ? 1 : 0);
    }
}

// 把静态值或一组关键帧转换成 Lottie 可识别的动画属性对象。
QJsonObject propertyObject(const QJsonValue &staticValue,
                           const QJsonArray &keyframes,
                           const QString &expression = QString())
{
    QJsonObject property;
    if (!keyframes.isEmpty()) {
        property.insert(QStringLiteral("a"), 1);
        property.insert(QStringLiteral("k"), keyframes);
    } else {
        property.insert(QStringLiteral("a"), 0);
        property.insert(QStringLiteral("k"), staticValue);
    }
    if (!expression.isEmpty()) {
        property.insert(QStringLiteral("x"), expression);
    }
    return property;
}

} // namespace

DotLottieParser::DotLottieParser(QString unzipExecutable)
    : m_unzipExecutable(unzipExecutable.isEmpty()
                            ? QStandardPaths::findExecutable(
                                  QStringLiteral("unzip"),
                                  {QStringLiteral("/usr/bin"), QStringLiteral("/bin")})
                            : std::move(unzipExecutable))
{
}

LottieAsset DotLottieParser::createJsonAsset(const QString &sourcePath,
                                             const CancellationCheck &isCancelled) const
{
    if (cancellationRequested(isCancelled)) {
        return {};
    }
    const QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists() || !sourceInfo.isFile() || sourceInfo.isSymLink()) {
        return {};
    }

    const LottieParserInternal::JsonParseResult parsed =
        LottieParserInternal::parseJsonFile(sourcePath, isCancelled);
    AnimationMetadata metadata;
    if (parsed.wasCancelled() || !parsed.hasInput ||
        parsed.parseError.error != QJsonParseError::NoError ||
        !validateLottieAnimation(parsed.document, &metadata)) {
        return {};
    }

    LottieAsset asset;
    asset.setAnimationJsonPath(sourcePath);
    asset.setAnimationJsonData(parsed.inputData);
    asset.setFrameCount(metadata.frameCount);
    asset.setFrameRate(metadata.frameRate);
    asset.setImageResourcePaths(metadata.imageResourcePaths);
    asset.setResourceDir(sourceInfo.absolutePath());
    asset.setResourceSearchDirs(QStringList{
        sourceInfo.absolutePath(),
        sourceInfo.absoluteDir().filePath(QStringLiteral("images")),
        sourceInfo.absoluteDir().filePath(QStringLiteral("i"))
    });
    return asset;
}

LottieAsset DotLottieParser::createJsonAsset(const QString &sourcePath,
                                             const QByteArray &animationData,
                                             const CancellationCheck &isCancelled) const
{
    if (animationData.isEmpty() ||
        animationData.size() > maxJsonInputBytes() ||
        cancellationRequested(isCancelled)) {
        return {};
    }

    const LottieParserInternal::JsonParseResult parsed =
        LottieParserInternal::parseJsonData(animationData, isCancelled);
    AnimationMetadata metadata;
    if (parsed.wasCancelled() || !parsed.hasInput ||
        parsed.parseError.error != QJsonParseError::NoError ||
        !validateLottieAnimation(parsed.document, &metadata)) {
        return {};
    }

    const QFileInfo sourceInfo(sourcePath);
    LottieAsset asset;
    asset.setAnimationJsonData(animationData);
    asset.setFrameCount(metadata.frameCount);
    asset.setFrameRate(metadata.frameRate);
    asset.setImageResourcePaths(metadata.imageResourcePaths);
    asset.setResourceDir(sourceInfo.absolutePath());
    asset.setResourceSearchDirs(QStringList{
        sourceInfo.absolutePath(),
        sourceInfo.absoluteDir().filePath(QStringLiteral("images")),
        sourceInfo.absoluteDir().filePath(QStringLiteral("i"))
    });
    return asset;
}

LottieAsset DotLottieParser::createManifestAnimationAsset(const QString &sourcePath,
                                                          const QString &canonicalExtractDir,
                                                          const QString &animationDirectory,
                                                          const QStringList &themes,
                                                          const QString &manifestInitialThemeId,
                                                          const QJsonObject &animation,
                                                          const QString &requestedTheme,
                                                          const CancellationCheck &isCancelled) const
{
    const QString animationId = withoutJsonSuffix(animation.value(QStringLiteral("id")).toString());
    if (!isSafeIdentifier(animationId)) {
        return {};
    }

    const QDir extractDir(canonicalExtractDir);
    const QString candidate = safeExistingFile(
        canonicalExtractDir,
        extractDir.filePath(animationDirectory + animationId + QStringLiteral(".json")));
    AnimationMetadata metadata;
    if (candidate.isEmpty() || !isValidLottieAnimationFile(candidate, &metadata, isCancelled)) {
        return {};
    }

    LottieAsset asset;
    configureExtractedAsset(asset,
                            sourcePath,
                            canonicalExtractDir,
                            candidate,
                            metadata);
    asset.setAvailableThemes(themes);
    asset.setManifestInitialThemeId(manifestInitialThemeId);
    asset.setAnimationInitialThemeId(animation.value(QStringLiteral("initialTheme")).toString());
    asset.setActiveAnimationId(animationId);
    if (animation.contains(QStringLiteral("themes")) && animation.value(QStringLiteral("themes")).isArray()) {
        asset.setActiveAnimationRestrictsThemes(true);
        asset.setActiveAnimationThemes(stringArray(animation.value(QStringLiteral("themes")).toArray()));
    }
    asset.setTheme(asset.preferredTheme(requestedTheme));
    return asset;
}

LottieAsset DotLottieParser::parseManifestAsset(const QString &sourcePath,
                                                  const QString &canonicalExtractDir,
                                                  const QString &manifestPath,
                                                  const QStringList &directoryThemes,
                                                  const QString &requestedTheme,
                                                  QString *errorString,
                                                  const CancellationCheck &isCancelled) const
{
    if (cancellationRequested(isCancelled)) {
        setErrorString(errorString, kCancelledError);
        return {};
    }
    const LottieParserInternal::JsonParseResult parsed =
        LottieParserInternal::parseJsonFile(manifestPath, isCancelled);
    if (parsed.wasCancelled()) {
        setErrorString(errorString, kCancelledError);
        return {};
    }
    if (!parsed.hasInput) {
        setErrorString(errorString,
                       parsed.readError.isEmpty() ? QStringLiteral("manifest is empty")
                                                  : parsed.readError);
        return {};
    }

    if (parsed.parseError.error != QJsonParseError::NoError || !parsed.document.isObject()) {
        setErrorString(errorString,
                       QStringLiteral("failed to parse manifest: ") + parsed.parseError.errorString());
        return {};
    }

    const QJsonObject manifestObject = parsed.document.object();
    const QString manifestVersion = manifestObject.value(QStringLiteral("version")).toString();
    const int dotLottieVersion = manifestVersion == QStringLiteral("2") ? 2 : 1;
    const QJsonArray animations = manifestObject.value(QStringLiteral("animations")).toArray();
    QStringList themes = themesFromManifest(manifestObject.value(QStringLiteral("themes")).toArray());
    appendMissingThemes(themes, directoryThemes);

    const QJsonObject initialObject = manifestObject.value(QStringLiteral("initial")).toObject();
    const QString initialAnimationId = initialObject.value(QStringLiteral("animation")).toString();
    const QString manifestInitialThemeId = initialObject.value(QStringLiteral("theme")).toString();
    QString activeAnimationId = isSafeIdentifier(withoutJsonSuffix(initialAnimationId))
        ? initialAnimationId
        : QString();
    if (activeAnimationId.isEmpty() && !animations.isEmpty()) {
        activeAnimationId = animations.first().toObject().value(QStringLiteral("id")).toString();
    }

    const QString animationDirectory = dotLottieVersion == 2
        ? QStringLiteral("a/")
        : QStringLiteral("animations/");
    const auto createAsset = [&](const QJsonObject &animation) {
        return createManifestAnimationAsset(sourcePath,
                                            canonicalExtractDir,
                                            animationDirectory,
                                            themes,
                                            manifestInitialThemeId,
                                            animation,
                                            requestedTheme,
                                            isCancelled);
    };

    if (!activeAnimationId.isEmpty()) {
        for (const QJsonValue &animationValue : animations) {
            const QJsonObject animation = animationValue.toObject();
            if (withoutJsonSuffix(animation.value(QStringLiteral("id")).toString()) ==
                withoutJsonSuffix(activeAnimationId)) {
                LottieAsset asset = createAsset(animation);
                if (asset.isValid()) {
                    return asset;
                }
            }
        }

        QJsonObject syntheticAnimation;
        syntheticAnimation.insert(QStringLiteral("id"), withoutJsonSuffix(activeAnimationId));
        LottieAsset asset = createAsset(syntheticAnimation);
        if (asset.isValid()) {
            return asset;
        }
    }

    for (const QJsonValue &animationValue : animations) {
        LottieAsset asset = createAsset(animationValue.toObject());
        if (asset.isValid()) {
            return asset;
        }
    }

    setErrorString(errorString, QStringLiteral("manifest does not reference a valid Lottie animation"));
    return {};
}

LottieAsset DotLottieParser::findFallbackAnimationAsset(const QString &sourcePath,
                                                        const QString &canonicalExtractDir,
                                                        const QStringList &directoryThemes,
                                                        const QString &requestedTheme,
                                                        QString *errorString,
                                                        const CancellationCheck &isCancelled) const
{
    const QDir extractDir(canonicalExtractDir);
    const QStringList animationDirectories = {
        extractDir.filePath(QStringLiteral("a")),
        extractDir.filePath(QStringLiteral("animations")),
        canonicalExtractDir
    };
    for (const QString &directoryPath : animationDirectories) {
        const QDir animationDirectory(directoryPath);
        const QFileInfoList candidates = animationDirectory.entryInfoList(
            QStringList{QStringLiteral("*.json")},
            QDir::Files | QDir::NoSymLinks,
            QDir::Name);
        for (const QFileInfo &candidateInfo : candidates) {
            if (cancellationRequested(isCancelled)) {
                setErrorString(errorString, kCancelledError);
                return {};
            }
            if (candidateInfo.fileName().compare(QStringLiteral("manifest.json"), Qt::CaseInsensitive) == 0) {
                continue;
            }
            const QString candidate = safeExistingFile(canonicalExtractDir, candidateInfo.absoluteFilePath());
            AnimationMetadata metadata;
            if (candidate.isEmpty() || !isValidLottieAnimationFile(candidate, &metadata, isCancelled)) {
                continue;
            }

            LottieAsset asset;
            configureExtractedAsset(asset,
                                    sourcePath,
                                    canonicalExtractDir,
                                    candidate,
                                    metadata);
            asset.setAvailableThemes(directoryThemes);
            asset.setTheme(asset.preferredTheme(requestedTheme));
            return asset;
        }
    }

    setErrorString(errorString, QStringLiteral("no animation json found in extract directory"));
    return {};
}

LottieAsset DotLottieParser::parseExtractedDirectory(const QString &sourcePath,
                                                     const QString &extractDirPath,
                                                     const QString &requestedTheme,
                                                     QString *errorString,
                                                     const CancellationCheck &isCancelled) const
{
    if (cancellationRequested(isCancelled)) {
        setErrorString(errorString, kCancelledError);
        return {};
    }
    const QString canonicalExtractDir = canonicalDirectoryPath(extractDirPath);
    if (canonicalExtractDir.isEmpty()) {
        setErrorString(errorString, QStringLiteral("extract directory is missing, invalid, or a symbolic link"));
        return {};
    }

    const QDir extractDir(canonicalExtractDir);
    const QStringList directoryThemes = themesFromDirectory(canonicalExtractDir);
    const QString manifestPath = extractDir.filePath(QStringLiteral("manifest.json"));
    const QFileInfo manifestInfo(manifestPath);
    const bool hasManifest = manifestInfo.exists();
    const QString safeManifestPath = hasManifest
        ? safeExistingFile(canonicalExtractDir, manifestPath)
        : QString();
    if (hasManifest && safeManifestPath.isEmpty()) {
        setErrorString(errorString, QStringLiteral("manifest is not a regular file inside the extract directory"));
        return {};
    }

    LottieAsset asset = hasManifest
        ? parseManifestAsset(sourcePath,
                             canonicalExtractDir,
                             safeManifestPath,
                             directoryThemes,
                             requestedTheme,
                             errorString,
                             isCancelled)
        : findFallbackAnimationAsset(sourcePath,
                                     canonicalExtractDir,
                                     directoryThemes,
                                     requestedTheme,
                                     errorString,
                                     isCancelled);
    if (asset.isValid()) {
        if (asset.availableThemes().size() > maxSelectableThemes()) {
            setErrorString(errorString, QStringLiteral("source declares too many selectable themes"));
            return {};
        }
        cacheThemeSlots(asset, isCancelled);
        if (cancellationRequested(isCancelled)) {
            setErrorString(errorString, kCancelledError);
            return {};
        }
    }
    return asset;
}

LottieAsset DotLottieParser::extractArchiveAndParse(const QString &sourcePath,
                                                    const QString &archivePath,
                                                    const QString &extractDirPath,
                                                    const QString &requestedTheme,
                                                    QString *errorString,
                                                    const CancellationCheck &isCancelled) const
{
    if (sourcePath.isEmpty()) {
        setErrorString(errorString, QStringLiteral("source path is empty"));
        return {};
    }
    if (cancellationRequested(isCancelled)) {
        setErrorString(errorString, kCancelledError);
        return {};
    }

    if (isPublishedExtractDirectory(extractDirPath)) {
        return parseExtractedDirectory(sourcePath, extractDirPath, requestedTheme, errorString, isCancelled);
    }

    const QFileInfo existingExtractInfo(extractDirPath);
    if (existingExtractInfo.isSymLink() || existingExtractInfo.exists()) {
        setErrorString(errorString, QStringLiteral("extract generation path already exists but is not complete"));
        return {};
    }

    const QFileInfo extractInfo(extractDirPath);
    const QString parentPath = extractInfo.absolutePath();
    if (!QDir().mkpath(parentPath)) {
        setErrorString(errorString, QStringLiteral("failed to create extract cache parent"));
        return {};
    }

    QTemporaryDir temporaryDir(QDir(parentPath).filePath(QStringLiteral(".extract-XXXXXX")));
    if (!temporaryDir.isValid()) {
        setErrorString(errorString, QStringLiteral("failed to create temporary extract directory"));
        return {};
    }

    if (!unzipToDirectory(archivePath, temporaryDir.path(), errorString, isCancelled)) {
        return {};
    }
    if (cancellationRequested(isCancelled)) {
        setErrorString(errorString, kCancelledError);
        return {};
    }

    QFile marker(temporaryDir.filePath(kCompleteMarker));
    if (!marker.open(QIODevice::WriteOnly | QIODevice::NewOnly) || marker.write("ok\n") != 3) {
        setErrorString(errorString, QStringLiteral("failed to create extract completion marker"));
        return {};
    }
    marker.close();

    if (!QDir().rename(temporaryDir.path(), extractDirPath)) {
        if (!isPublishedExtractDirectory(extractDirPath)) {
            setErrorString(errorString, QStringLiteral("failed to atomically publish extract generation"));
            return {};
        }
    } else {
        temporaryDir.setAutoRemove(false);
    }

    return parseExtractedDirectory(sourcePath, extractDirPath, requestedTheme, errorString, isCancelled);
}

QString DotLottieParser::withoutJsonSuffix(QString id)
{
    if (id.endsWith(QStringLiteral(".json"), Qt::CaseInsensitive) ||
        id.endsWith(QStringLiteral(".lot"), Qt::CaseInsensitive)) {
        const int dot = id.lastIndexOf(QLatin1Char('.'));
        if (dot >= 0) {
            id.truncate(dot);
        }
    }
    return id;
}

QStringList DotLottieParser::stringArray(const QJsonArray &array)
{
    QStringList values;
    for (const QJsonValue &value : array) {
        const QString text = value.toString();
        if (!text.isEmpty()) {
            values.append(text);
        }
    }
    values.removeDuplicates();
    return values;
}

QStringList DotLottieParser::themesFromDirectory(const QString &extractDirPath)
{
    QStringList entries;
    const QDir themeDir(QDir(extractDirPath).filePath(QStringLiteral("t")));
    const QFileInfoList files = themeDir.entryInfoList(QStringList{QStringLiteral("*.json")},
                                                       QDir::Files | QDir::NoSymLinks,
                                                       QDir::Name);
    for (const QFileInfo &fileInfo : files) {
        const QString id = fileInfo.completeBaseName();
        if (isSafeIdentifier(id) && !safeExistingFile(extractDirPath, fileInfo.absoluteFilePath()).isEmpty()) {
            entries.append(id);
        }
    }
    return entries;
}

QStringList DotLottieParser::themesFromManifest(const QJsonArray &themes)
{
    QStringList entries;
    QSet<QString> seenIds;
    for (const QJsonValue &themeValue : themes) {
        QString id;
        if (themeValue.isString()) {
            id = themeValue.toString();
        } else {
            const QJsonObject theme = themeValue.toObject();
            id = stringValue(theme, QStringLiteral("id"));
        }
        if (id.endsWith(QStringLiteral(".json"), Qt::CaseInsensitive)) {
            id.chop(5);
        }
        if (isSafeIdentifier(id) && !seenIds.contains(id)) {
            seenIds.insert(id);
            entries.append(id);
        }
    }
    return entries;
}

void DotLottieParser::appendMissingThemes(QStringList &themes, const QStringList &fallbackThemes)
{
    QSet<QString> existingIds;
    for (const QString &theme : std::as_const(themes)) {
        existingIds.insert(theme);
    }
    for (const QString &theme : fallbackThemes) {
        if (!existingIds.contains(theme)) {
            themes.append(theme);
            existingIds.insert(theme);
        }
    }
}

QByteArray DotLottieParser::themeToSlotJson(const QJsonObject &themeObject,
                                            const QString &activeAnimationId)
{
    QJsonObject slotMap;
    const QJsonArray rules = themeObject.value(QStringLiteral("rules")).toArray();
    for (const QJsonValue &ruleValue : rules) {
        const QJsonObject rule = ruleValue.toObject();
        if (!shouldProcessThemeRule(rule, activeAnimationId)) {
            continue;
        }

        const QString slotId = rule.value(QStringLiteral("id")).toString();
        if (slotId.isEmpty()) {
            continue;
        }

        const QJsonValue slotValue = themeRuleToSlotValue(rule);
        if (slotValue.isUndefined() || slotValue.isNull()) {
            continue;
        }

        QJsonObject wrapper;
        wrapper.insert(QStringLiteral("p"), slotValue);
        slotMap.insert(slotId, wrapper);
    }

    return QJsonDocument(slotMap).toJson(QJsonDocument::Compact);
}

QJsonValue DotLottieParser::themeRuleToSlotValue(const QJsonObject &rule)
{
    const QString type = rule.value(QStringLiteral("type")).toString();
    const QString expression = rule.value(QStringLiteral("expression")).toString();
    const QJsonArray keyframes = rule.value(QStringLiteral("keyframes")).toArray();

    if (type == QStringLiteral("Color")) {
        QJsonArray lottieKeyframes;
        for (const QJsonValue &keyframeValue : keyframes) {
            const QJsonObject keyframe = keyframeValue.toObject();
            QJsonObject lottieKeyframe;
            lottieKeyframe.insert(QStringLiteral("t"), keyframe.value(QStringLiteral("frame")).toInt());
            lottieKeyframe.insert(QStringLiteral("s"), colorArrayValue(keyframe.value(QStringLiteral("value"))));
            insertColorKeyframeFields(lottieKeyframe, keyframe);
            lottieKeyframes.append(lottieKeyframe);
        }
        return propertyObject(colorArrayValue(rule.value(QStringLiteral("value"))), lottieKeyframes, expression);
    }

    return {};
}

bool DotLottieParser::shouldProcessThemeRule(const QJsonObject &rule, const QString &activeAnimationId)
{
    const QJsonValue animationsValue = rule.value(QStringLiteral("animations"));
    if (!animationsValue.isArray()) {
        return true;
    }

    const QStringList animations = stringArray(animationsValue.toArray());
    for (const QString &animation : animations) {
        if (withoutJsonSuffix(animation) == activeAnimationId) {
            return true;
        }
    }
    return false;
}

bool DotLottieParser::unzipToDirectory(const QString &sourcePath,
                                       const QString &extractDirPath,
                                       QString *errorString,
                                       const CancellationCheck &isCancelled) const
{
    const QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists() || !sourceInfo.isFile() || sourceInfo.isSymLink()) {
        setErrorString(errorString, QStringLiteral(".lottie file does not exist"));
        return false;
    }
    if (sourceInfo.size() > maxArchiveInputBytes()) {
        setErrorString(errorString, QStringLiteral("archive compressed input exceeds the budget"));
        return false;
    }
    const QString canonicalExtractDir = canonicalDirectoryPath(extractDirPath);
    if (canonicalExtractDir.isEmpty()) {
        setErrorString(errorString, QStringLiteral("extract directory is invalid or a symbolic link"));
        return false;
    }

    if (!validateUnzipArchive(m_unzipExecutable, sourcePath, errorString, isCancelled)) {
        return false;
    }
    if (!runUnzip(m_unzipExecutable,
                  {QStringLiteral("-qq"),
                   QStringLiteral("-DD"),
                   QStringLiteral("-n"),
                   sourcePath,
                   QStringLiteral("-d"),
                   canonicalExtractDir},
                  nullptr,
                  errorString,
                  isCancelled)) {
        return false;
    }
    return secureExtractedDirectory(canonicalExtractDir, errorString, isCancelled);
}

void DotLottieParser::cacheThemeSlots(LottieAsset &asset, const CancellationCheck &isCancelled) const
{
    if (asset.extractDir().isEmpty()) {
        return;
    }

    const QDir themeDir(QDir(asset.extractDir()).filePath(QStringLiteral("t")));
    for (const QString &theme : asset.availableThemes()) {
        if (cancellationRequested(isCancelled)) {
            return;
        }
        if (!isSafeIdentifier(theme)) {
            continue;
        }
        QString fileName = theme;
        if (!fileName.endsWith(QStringLiteral(".json"), Qt::CaseInsensitive)) {
            fileName += QStringLiteral(".json");
        }

        const QString path = safeExistingFile(asset.extractDir(), themeDir.filePath(fileName));
        if (path.isEmpty()) {
            continue;
        }
        const LottieParserInternal::JsonParseResult parsed =
            LottieParserInternal::parseJsonFile(path, isCancelled);
        if (parsed.wasCancelled()) {
            return;
        }
        if (!parsed.hasInput) {
            continue;
        }

        if (parsed.parseError.error != QJsonParseError::NoError || !parsed.document.isObject()) {
            qWarning() << "DotLottieParser: Failed to parse theme:" << path
                       << "error:" << parsed.parseError.errorString();
            continue;
        }

        const QByteArray slotJson = themeToSlotJson(parsed.document.object(), asset.activeAnimationId());
        asset.setThemeSlotJson(theme, slotJson);
    }
}

} // namespace UkuiQuick
