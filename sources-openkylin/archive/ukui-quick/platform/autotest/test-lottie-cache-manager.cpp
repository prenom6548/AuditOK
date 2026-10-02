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
 */

#include <QBuffer>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QLibraryInfo>
#include <QLockFile>
#include <QMutexLocker>
#include <QProcess>
#include <QResource>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QUrl>
#include <QtTest>

#include <cerrno>
#include <dlfcn.h>

#define private public
#include "../ukui/lottie/lottie-cache-manager.h"
#undef private
#include "../ukui/lottie/lottie-source-reference.h"

#include <sys/inotify.h>
#include <sys/stat.h>

namespace {
QByteArray forcedLstatPath;
QByteArray forcedRealpathPath;
int temporaryArchiveCommitCount = 0;
qint64 temporaryArchiveBytesWritten = 0;
// 临时归档复制提交后的测试钩子：可在解析开始前替换原路径内容。
std::function<void()> temporaryArchiveCommitHook;
// 每个临时归档写块完成后的测试钩子，用来确定性模拟复制期间源文件增长。
std::function<void()> temporaryArchiveWriteHook;
}

int lottieCacheManagerTestLstat(const char *path, struct stat *buffer);

using RealpathFunction = char *(*)(const char *, char *);

extern "C" char *realpath(const char *path, char *resolvedPath)
{
    if (!forcedRealpathPath.isEmpty() && QByteArray(path) == forcedRealpathPath) {
        errno = ENOENT;
        return nullptr;
    }

    static const auto systemRealpath = reinterpret_cast<RealpathFunction>(dlsym(RTLD_NEXT, "realpath"));
    return systemRealpath ? systemRealpath(path, resolvedPath) : nullptr;
}

class LottieCacheManagerTestSaveFile : public QSaveFile
{
public:
    using QSaveFile::QSaveFile;

    qint64 write(const char *data, qint64 maxSize)
    {
        const qint64 written = QSaveFile::write(data, maxSize);
        if (written > 0) {
            temporaryArchiveBytesWritten += written;
            if (temporaryArchiveWriteHook) {
                temporaryArchiveWriteHook();
            }
        }
        return written;
    }

    bool commit()
    {
        if (!QSaveFile::commit()) {
            return false;
        }
        ++temporaryArchiveCommitCount;
        if (temporaryArchiveCommitHook) {
            temporaryArchiveCommitHook();
        }
        return true;
    }
};

#define lstat lottieCacheManagerTestLstat
#define QSaveFile LottieCacheManagerTestSaveFile
#include "../ukui/lottie/lottie-cache-manager.cpp"
#undef QSaveFile
#undef lstat

#include <atomic>
#include <limits>
#include <thread>

int lottieCacheManagerTestLstat(const char *path, struct stat *buffer)
{
    if (!forcedLstatPath.isEmpty() && QByteArray(path) == forcedLstatPath) {
        *buffer = {};
        buffer->st_blocks = std::numeric_limits<blkcnt_t>::max();
        return 0;
    }
    return ::lstat(path, buffer);
}

namespace {

constexpr qint64 kMiB = 1024LL * 1024LL;

bool writeFile(const QString &path, const QByteArray &contents)
{
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    return file.write(contents) == contents.size();
}

QByteArray readFile(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

UkuiQuick::LottieAsset loadAsset(
    UkuiQuick::LottieCacheManager *manager,
    const UkuiQuick::LottieSourceReference &source,
    const QString &requestedTheme = QString(),
    const UkuiQuick::LottieCacheManager::CancellationCheck &isCancelled = {})
{
    UkuiQuick::LottieAsset asset = manager->requestAsset(source, isCancelled).asset;
    if (asset.isValid()) {
        asset.setTheme(asset.preferredTheme(requestedTheme));
    }
    return asset;
}

UkuiQuick::LottieAsset loadAsset(
    UkuiQuick::LottieCacheManager *manager,
    const QString &source,
    const QString &requestedTheme = QString(),
    const UkuiQuick::LottieCacheManager::CancellationCheck &isCancelled = {})
{
    return loadAsset(manager,
                     UkuiQuick::LottieSourceReference::fromText(source),
                     requestedTheme,
                     isCancelled);
}

bool createFile(const QString &path, qint64 size, const QDateTime &modified)
{
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate) || !file.resize(size)) {
        return false;
    }
    return file.setFileTime(modified, QFileDevice::FileModificationTime);
}

bool setFileModifiedTime(const QString &path, const QDateTime &modified)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) &&
           file.setFileTime(modified, QFileDevice::FileModificationTime);
}

QByteArray animationJson(const QByteArray &name = QByteArrayLiteral("test"))
{
    return QByteArrayLiteral("{\"v\":\"5.7.4\",\"fr\":30,\"ip\":0,\"op\":60,\"w\":24,\"h\":24,") +
        QByteArrayLiteral("\"nm\":\"") + name + QByteArrayLiteral("\",\"layers\":[]}");
}

QByteArray imageAnimationJson()
{
    return QByteArrayLiteral(
        "{\"v\":\"5.7.4\",\"fr\":30,\"ip\":0,\"op\":60,\"w\":24,\"h\":24,"
        "\"nm\":\"image-test\","
        "\"assets\":[{\"id\":\"image_0\",\"w\":2,\"h\":2,"
        "\"u\":\"images/\",\"p\":\"icon.png\",\"e\":0}],\"layers\":[]}");
}

QByteArray repeatedImageAnimationJson(int declarationCount)
{
    QByteArray assets;
    for (int index = 0; index < declarationCount; ++index) {
        if (!assets.isEmpty()) {
            assets.append(',');
        }
        assets += QByteArrayLiteral("{\"id\":\"image_") + QByteArray::number(index) +
            QByteArrayLiteral("\",\"w\":1024,\"h\":1024,") +
            QByteArrayLiteral("\"u\":\"images/\",\"p\":\"icon.png\",\"e\":0}");
    }
    return QByteArrayLiteral(
               "{\"v\":\"5.7.4\",\"fr\":30,\"ip\":0,\"op\":60,\"w\":24,\"h\":24,") +
        QByteArrayLiteral("\"nm\":\"repeated-image-test\",\"assets\":[") + assets +
        QByteArrayLiteral("],\"layers\":[]}");
}

QByteArray pngImage(const QSize &size, QRgb color)
{
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    if (image.isNull()) {
        return {};
    }
    image.fill(color);
    QByteArray bytes;
    QBuffer buffer(&bytes);
    if (!buffer.open(QIODevice::WriteOnly) || !image.save(&buffer, "PNG")) {
        return {};
    }
    return bytes;
}

void appendLittleEndian16(QByteArray &data, quint16 value)
{
    data.append(static_cast<char>(value & 0xff));
    data.append(static_cast<char>((value >> 8) & 0xff));
}

void appendLittleEndian32(QByteArray &data, quint32 value)
{
    data.append(static_cast<char>(value & 0xff));
    data.append(static_cast<char>((value >> 8) & 0xff));
    data.append(static_cast<char>((value >> 16) & 0xff));
    data.append(static_cast<char>((value >> 24) & 0xff));
}

quint32 crc32(const QByteArray &data)
{
    quint32 checksum = 0xffffffffU;
    for (const unsigned char byte : data) {
        checksum ^= byte;
        for (int bit = 0; bit < 8; ++bit) {
            checksum = (checksum >> 1) ^ (0xedb88320U & (-(checksum & 1U)));
        }
    }
    return ~checksum;
}

struct ZipEntry
{
    QByteArray name;
    QByteArray contents;
    quint32 localHeaderOffset = 0;
};

QByteArray storedZip(QList<ZipEntry> entries)
{
    QByteArray archive;
    for (ZipEntry &entry : entries) {
        entry.localHeaderOffset = static_cast<quint32>(archive.size());
        const quint32 checksum = crc32(entry.contents);
        appendLittleEndian32(archive, 0x04034b50U);
        appendLittleEndian16(archive, 20);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian32(archive, checksum);
        appendLittleEndian32(archive, static_cast<quint32>(entry.contents.size()));
        appendLittleEndian32(archive, static_cast<quint32>(entry.contents.size()));
        appendLittleEndian16(archive, static_cast<quint16>(entry.name.size()));
        appendLittleEndian16(archive, 0);
        archive.append(entry.name);
        archive.append(entry.contents);
    }

    const quint32 centralDirectoryOffset = static_cast<quint32>(archive.size());
    for (const ZipEntry &entry : entries) {
        const quint32 checksum = crc32(entry.contents);
        appendLittleEndian32(archive, 0x02014b50U);
        appendLittleEndian16(archive, 20);
        appendLittleEndian16(archive, 20);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian32(archive, checksum);
        appendLittleEndian32(archive, static_cast<quint32>(entry.contents.size()));
        appendLittleEndian32(archive, static_cast<quint32>(entry.contents.size()));
        appendLittleEndian16(archive, static_cast<quint16>(entry.name.size()));
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian16(archive, 0);
        appendLittleEndian32(archive, 0);
        appendLittleEndian32(archive, entry.localHeaderOffset);
        archive.append(entry.name);
    }

    const quint32 centralDirectorySize = static_cast<quint32>(archive.size()) - centralDirectoryOffset;
    appendLittleEndian32(archive, 0x06054b50U);
    appendLittleEndian16(archive, 0);
    appendLittleEndian16(archive, 0);
    appendLittleEndian16(archive, static_cast<quint16>(entries.size()));
    appendLittleEndian16(archive, static_cast<quint16>(entries.size()));
    appendLittleEndian32(archive, centralDirectorySize);
    appendLittleEndian32(archive, centralDirectoryOffset);
    appendLittleEndian16(archive, 0);
    return archive;
}

QByteArray dotLottieArchive(const QByteArray &animationId, const QByteArray &animation)
{
    return storedZip({
        {QByteArrayLiteral("manifest.json"),
         QByteArrayLiteral("{\"version\":\"2\",\"initial\":{\"animation\":\"") + animationId +
             QByteArrayLiteral("\"},\"animations\":[{\"id\":\"") + animationId +
             QByteArrayLiteral("\"}]} ")},
        {QByteArrayLiteral("a/") + animationId + QByteArrayLiteral(".json"), animation}
    });
}

QString writeLottieArchive(const QString &directory,
                           const QString &fileName,
                           const QByteArray &animationId,
                           const QByteArray &animation)
{
    const QString path = QDir(directory).filePath(fileName);
    return writeFile(path, dotLottieArchive(animationId, animation)) ? path : QString();
}

QString rccExecutable()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return QDir(QLibraryInfo::path(QLibraryInfo::BinariesPath)).filePath(QStringLiteral("rcc"));
#else
    return QDir(QLibraryInfo::location(QLibraryInfo::BinariesPath)).filePath(QStringLiteral("rcc"));
#endif
}

bool registerArchiveResource(const QString &directory, QString *rccPath)
{
    const QString archivePath = QDir(directory).filePath(QStringLiteral("archive.lottie"));
    const QString qrcFilePath = QDir(directory).filePath(QStringLiteral("archive.qrc"));
    const QString outputPath = QDir(directory).filePath(QStringLiteral("archive.rcc"));
    if (!writeFile(archivePath, dotLottieArchive(QByteArrayLiteral("resource"), animationJson(QByteArrayLiteral("resource")))) ||
        !writeFile(qrcFilePath, QByteArrayLiteral(
            "<RCC><qresource prefix=\"/lottie-cache-manager-dynamic\">"
            "<file alias=\"archive.lottie\">archive.lottie</file>"
            "</qresource></RCC>"))) {
        return false;
    }

    QProcess rcc;
    rcc.start(rccExecutable(), {QStringLiteral("--binary"), qrcFilePath, QStringLiteral("-o"), outputPath});
    if (!rcc.waitForStarted() || !rcc.waitForFinished(5000) || rcc.exitStatus() != QProcess::NormalExit ||
        rcc.exitCode() != 0 || !QResource::registerResource(outputPath)) {
        return false;
    }
    *rccPath = outputPath;
    return true;
}

bool registerJsonImageResource(const QString &directory,
                               const QByteArray &imageBytes,
                               QString *rccPath)
{
    const QString animationPath = QDir(directory).filePath(QStringLiteral("anim.json"));
    const QString imagePath = QDir(directory).filePath(QStringLiteral("images/icon.png"));
    const QString qrcFilePath = QDir(directory).filePath(QStringLiteral("image.qrc"));
    const QString outputPath = QDir(directory).filePath(QStringLiteral("image.rcc"));
    if (!writeFile(animationPath, imageAnimationJson()) ||
        !writeFile(imagePath, imageBytes) ||
        !writeFile(qrcFilePath, QByteArrayLiteral(
            "<RCC><qresource prefix=\"/lottie-image-budget\">"
            "<file alias=\"anim.json\">anim.json</file>"
            "<file alias=\"images/icon.png\">images/icon.png</file>"
            "</qresource></RCC>"))) {
        return false;
    }

    QProcess rcc;
    rcc.start(rccExecutable(),
              {QStringLiteral("--binary"), qrcFilePath,
               QStringLiteral("-o"), outputPath});
    if (!rcc.waitForStarted() || !rcc.waitForFinished(5000) ||
        rcc.exitStatus() != QProcess::NormalExit || rcc.exitCode() != 0 ||
        !QResource::registerResource(outputPath)) {
        return false;
    }
    *rccPath = outputPath;
    return true;
}

// 注册一个超出条目预算的 qrc 归档，并可选输出其字节以便本地对照。
bool registerBudgetArchiveResource(const QString &directory,
                                   QString *rccPath,
                                   QByteArray *archiveBytes)
{
    QList<ZipEntry> entries;
    entries.append({QByteArrayLiteral("manifest.json"),
                    QByteArrayLiteral("{\"version\":\"2\",\"initial\":{\"animation\":\"budget\"},")
                        + QByteArrayLiteral("\"animations\":[{\"id\":\"budget\"}]}"),
                    0});
    entries.append({QByteArrayLiteral("a/budget.json"), animationJson(QByteArrayLiteral("budget")), 0});
    while (entries.size() <= UkuiQuick::DotLottieParser::maxArchiveEntries()) {
        entries.append({QByteArrayLiteral("images/pad-") + QByteArray::number(entries.size()) +
                            QByteArrayLiteral(".png"),
                        QByteArrayLiteral("pad"),
                        0});
    }

    const QByteArray archive = storedZip(entries);
    const QString archivePath = QDir(directory).filePath(QStringLiteral("budget.lottie"));
    const QString qrcFilePath = QDir(directory).filePath(QStringLiteral("budget.qrc"));
    const QString outputPath = QDir(directory).filePath(QStringLiteral("budget.rcc"));
    if (!writeFile(archivePath, archive) ||
        !writeFile(qrcFilePath, QByteArrayLiteral(
            "<RCC><qresource prefix=\"/lottie-cache-manager-budget\">"
            "<file alias=\"budget.lottie\">budget.lottie</file>"
            "</qresource></RCC>"))) {
        return false;
    }

    QProcess rcc;
    rcc.start(rccExecutable(), {QStringLiteral("--binary"), qrcFilePath, QStringLiteral("-o"), outputPath});
    if (!rcc.waitForStarted() || !rcc.waitForFinished(5000) || rcc.exitStatus() != QProcess::NormalExit ||
        rcc.exitCode() != 0 || !QResource::registerResource(outputPath)) {
        return false;
    }
    *rccPath = outputPath;
    if (archiveBytes) {
        *archiveBytes = archive;
    }
    return true;
}

QString canonicalPath(const QString &path)
{
    return QFileInfo(path).canonicalFilePath();
}

QString sha256Name(const QString &value)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(value.toUtf8(), QCryptographicHash::Sha256).toHex());
}

QString createPublishedGeneration(const QString &cacheRoot,
                                  const QString &parentName,
                                  const QString &generationName,
                                  qint64 payloadSize,
                                  const QDateTime &completeTime,
                                  const QDateTime &accessTime)
{
    const QString parentPath = parentName.isEmpty()
        ? cacheRoot
        : QDir(cacheRoot).filePath(parentName);
    const QString extractDir = QDir(parentPath).filePath(generationName);
    if (!QDir().mkpath(extractDir) ||
        !createFile(QDir(extractDir).filePath(QStringLiteral("payload.bin")), payloadSize, accessTime) ||
        !createFile(QDir(extractDir).filePath(QStringLiteral(".complete")), 3, completeTime) ||
        !createFile(QDir(extractDir).filePath(QStringLiteral(".access")), 0, accessTime)) {
        return {};
    }
    return extractDir;
}

int publishedGenerationCount(const QString &cacheRoot)
{
    int count = 0;
    const QString extractedRoot = QDir(cacheRoot).filePath(QStringLiteral("extracted"));
    QDirIterator iterator(extractedRoot,
                          QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks,
                          QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        iterator.next();
        if (UkuiQuick::extractGenerationIsPublished(iterator.filePath())) {
            ++count;
        }
    }
    return count;
}

class HomeEnvironment
{
public:
    explicit HomeEnvironment(const QString &path)
        : m_hadHome(qEnvironmentVariableIsSet("HOME"))
        , m_originalHome(qgetenv("HOME"))
        , m_hadCacheHome(qEnvironmentVariableIsSet("XDG_CACHE_HOME"))
        , m_originalCacheHome(qgetenv("XDG_CACHE_HOME"))
    {
        qputenv("HOME", path.toUtf8());
        qputenv("XDG_CACHE_HOME", QDir(path).filePath(QStringLiteral(".cache")).toUtf8());
    }

    ~HomeEnvironment()
    {
        if (m_hadHome) {
            qputenv("HOME", m_originalHome);
        } else {
            qunsetenv("HOME");
        }
        if (m_hadCacheHome) {
            qputenv("XDG_CACHE_HOME", m_originalCacheHome);
        } else {
            qunsetenv("XDG_CACHE_HOME");
        }
    }

private:
    bool m_hadHome = false;
    QByteArray m_originalHome;
    bool m_hadCacheHome = false;
    QByteArray m_originalCacheHome;
};

class TemporaryArchiveCommitHookGuard
{
public:
    explicit TemporaryArchiveCommitHookGuard(std::function<void()> hook)
    {
        Q_ASSERT(!temporaryArchiveCommitHook);
        temporaryArchiveCommitHook = std::move(hook);
    }

    ~TemporaryArchiveCommitHookGuard()
    {
        temporaryArchiveCommitHook = {};
    }
};

class TemporaryArchiveWriteHookGuard
{
public:
    explicit TemporaryArchiveWriteHookGuard(std::function<void()> hook)
    {
        Q_ASSERT(!temporaryArchiveWriteHook);
        temporaryArchiveWriteHook = std::move(hook);
    }

    ~TemporaryArchiveWriteHookGuard()
    {
        temporaryArchiveWriteHook = {};
    }
};

class ScopedFileDescriptor
{
public:
    explicit ScopedFileDescriptor(int fileDescriptor)
        : m_fileDescriptor(fileDescriptor)
    {
    }

    ~ScopedFileDescriptor()
    {
        if (m_fileDescriptor >= 0) {
            ::close(m_fileDescriptor);
        }
    }

private:
    int m_fileDescriptor = -1;
};

class ScopedRealpathFailure
{
public:
    explicit ScopedRealpathFailure(const QString &path)
        : m_previousPath(forcedRealpathPath)
    {
        forcedRealpathPath = QFile::encodeName(path);
    }

    ~ScopedRealpathFailure()
    {
        forcedRealpathPath = m_previousPath;
    }

private:
    QByteArray m_previousPath;
};

class ScopedEnvironmentVariable
{
public:
    ScopedEnvironmentVariable(const char *name, const QByteArray &value)
        : m_name(name)
        , m_hadValue(qEnvironmentVariableIsSet(name))
        , m_originalValue(qgetenv(name))
    {
        qputenv(m_name.constData(), value);
    }

    ~ScopedEnvironmentVariable()
    {
        if (m_hadValue) {
            qputenv(m_name.constData(), m_originalValue);
        } else {
            qunsetenv(m_name.constData());
        }
    }

private:
    QByteArray m_name;
    bool m_hadValue = false;
    QByteArray m_originalValue;
};

void resetManagerState(UkuiQuick::LottieCacheManager *manager)
{
    QMutexLocker locker(&manager->d->parsedAssets.mutex);
    manager->d->parsedAssets.assetCache.clear();
    manager->d->parsedAssets.failedLoads.clear();
    manager->d->parsedAssets.diskCacheLeases.clear();
    manager->d->parsedAssets.cacheAccessTick = 0;
}

void storeAsset(UkuiQuick::LottieCacheManager *manager,
                const QString &cacheKey,
                UkuiQuick::LottieAsset &asset)
{
    QMutexLocker locker(&manager->d->parsedAssets.mutex);
    manager->d->parsedAssets.storeAssetLocked(cacheKey, asset);
}

std::atomic_int failedDotLottieLoads = 0;
std::atomic_int budgetExceededWarnings = 0;
std::atomic_bool countFailedDotLottieLoads = false;
QtMessageHandler previousMessageHandler = nullptr;

void countLottieWarnings(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    if (countFailedDotLottieLoads && type == QtWarningMsg) {
        if (message.startsWith(QStringLiteral("LottieCacheManager: Failed to prepare dotLottie asset:"))) {
            ++failedDotLottieLoads;
        }
        if (message.contains(QStringLiteral("exceeds the input budget"))) {
            ++budgetExceededWarnings;
        }
    }
    if (previousMessageHandler) {
        previousMessageHandler(type, context, message);
    }
}

class FailedLoadWarningCounter
{
public:
    FailedLoadWarningCounter()
    {
        failedDotLottieLoads = 0;
        budgetExceededWarnings = 0;
        countFailedDotLottieLoads = true;
        previousMessageHandler = qInstallMessageHandler(countLottieWarnings);
    }

    ~FailedLoadWarningCounter()
    {
        countFailedDotLottieLoads = false;
        qInstallMessageHandler(previousMessageHandler);
        previousMessageHandler = nullptr;
    }

    int count() const
    {
        return failedDotLottieLoads.load();
    }

    int budgetCount() const
    {
        return budgetExceededWarnings.load();
    }
};

}

class lottie_cache_manager_test : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void coversPathAndFileHelpers();
    void validatesCacheRoots();
    void resolvesSourceReferences();
    void loadsCachesAndInvalidatesJsonAssets();
    void jsonAssetsKeepImmutableBytesWithoutDiskCache();
    void loadsRefreshesAndRejectsDotLottieAssets();
    void localDotLottieUsesStatOnlyPersistentExtraction();
    void qrcDotLottieCachesOnlyExtractedContent();
    void rejectsInvalidSourcesAndLockedCaches();
    void coversAdditionalFailureAndScanningBranches();
    void coversCanonicalizationFailurePaths();
    void logsMaintenanceFailuresWithoutDiscardingAssets();
    void maintainsDiskCacheAndHonorsProtection();
    void preservesLeasedDiskEntriesUntilAssetsAreReleased();
    void removesOnlyValidatedCacheEntries();
    void coversPrivateCacheStateTransitions();
    void trimsInternalCacheState();
    void assignsSourceVersionRevisions();
    void sourceChangeDuringCopyRetriesWithStableBytes();
    void continuouslyChangingSourceFailsAfterOneRetry();
    void growingSourceIsRejectedBeforeTemporaryAppend();
    void oldAndNewRevisionsCoexistWhileLeased();
    void concurrentRequestAfterReplacementJoinsStableRevision();
    void sameContentDifferentSourcesKeepOwnImageRoots();
    void replacingExternalImageCreatesNewAssetRevision();
    void oversizedExternalImageIsRejectedBeforeRasterization();
    void repeatedImageDeclarationsRespectDecodeBudget();
    void repeatedCancelAfterTemporaryCopyLeavesNoPersistentArtifacts();
    void jsonInputBudgetIsEnforcedBeforeParsing();
    void cancelledCacheHitDoesNotDeliverAsset();
    void sameBytesJsonDoesNotBypassDotLottieValidation();
    void wrongFormatCooldownDoesNotBlockCorrectFormat();
    void cancelledLoadRecordsNeitherFailureNorCooldown();
    void rejectsOversizedArchiveInputBudget();
    void rejectsOverBudgetArchivesUniformlyForQrcAndLocalSources();
};

void lottie_cache_manager_test::init()
{
    temporaryArchiveCommitHook = {};
    temporaryArchiveWriteHook = {};
    resetManagerState(UkuiQuick::LottieCacheManager::instance());
}

void lottie_cache_manager_test::cleanup()
{
    temporaryArchiveCommitHook = {};
    temporaryArchiveWriteHook = {};
    resetManagerState(UkuiQuick::LottieCacheManager::instance());
}

void lottie_cache_manager_test::coversPathAndFileHelpers()
{
    QCOMPARE(UkuiQuick::saturatedAdd(3, 4), 7LL);
    QCOMPARE(UkuiQuick::saturatedAdd(3, -1), std::numeric_limits<qint64>::max());
    QCOMPARE(UkuiQuick::saturatedAdd(std::numeric_limits<qint64>::max(), 1),
             std::numeric_limits<qint64>::max());

    const QString validHash(64, QLatin1Char('a'));
    QVERIFY(UkuiQuick::isSha256Name(validHash));
    QVERIFY(!UkuiQuick::isSha256Name(validHash.left(63)));
    QVERIFY(!UkuiQuick::isSha256Name(QString(64, QLatin1Char('A'))));
    QVERIFY(UkuiQuick::isSourceCacheDirectoryName(validHash));
    QVERIFY(!UkuiQuick::isSourceCacheDirectoryName(QStringLiteral("not-a-hash")));
    QVERIFY(UkuiQuick::isExtractGenerationName(validHash + QStringLiteral("_FILES")));
    QVERIFY(!UkuiQuick::isExtractGenerationName(QStringLiteral("animation.lottie_FILES")));
    QVERIFY(!UkuiQuick::isExtractGenerationName(QStringLiteral("_FILES")));

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());
    const QDateTime now = QDateTime::currentDateTimeUtc();
    const QString generation = createPublishedGeneration(temporaryDir.path(),
                                                          validHash,
                                                          validHash + QStringLiteral("_FILES"),
                                                          8 * 1024,
                                                          now.addSecs(-2),
                                                          now.addSecs(-1));
    QVERIFY(!generation.isEmpty());
    QVERIFY(UkuiQuick::extractGenerationIsPublished(generation));
    QVERIFY(!UkuiQuick::extractGenerationIsPublished(
        QDir(temporaryDir.path()).filePath(QStringLiteral("missing_FILES"))));
    QVERIFY(UkuiQuick::extractGenerationBytes(generation) >= 4 * 1024);
    const QString overflowDirectory = QDir(temporaryDir.path()).filePath(QStringLiteral("overflow_FILES"));
    const QString overflowFile = QDir(overflowDirectory).filePath(QStringLiteral("payload.bin"));
    QVERIFY(writeFile(overflowFile, QByteArrayLiteral("overflow")));
    forcedLstatPath = QFile::encodeName(overflowFile);
    QCOMPARE(UkuiQuick::extractGenerationBytes(overflowDirectory), std::numeric_limits<qint64>::max());
    forcedLstatPath.clear();

    const QString accessMarker = QDir(generation).filePath(QStringLiteral(".access"));
    const QString completeMarker = QDir(generation).filePath(QStringLiteral(".complete"));
    QCOMPARE(UkuiQuick::extractGenerationLastAccess(generation), QFileInfo(accessMarker).lastModified());
    QVERIFY(QFile::remove(accessMarker));
    QCOMPARE(UkuiQuick::extractGenerationLastAccess(generation), QFileInfo(completeMarker).lastModified());
    UkuiQuick::touchExtractGeneration(generation);
    QVERIFY(QFileInfo(accessMarker).isFile());
    QVERIFY(QFile::remove(accessMarker));
    QVERIFY(QDir().mkpath(accessMarker));
    UkuiQuick::touchExtractGeneration(generation);
    QVERIFY(QFileInfo(accessMarker).isDir());

    const QString source = QDir(temporaryDir.path()).filePath(QStringLiteral("source.json"));
    QVERIFY(writeFile(source, animationJson(QByteArrayLiteral("copy"))));

    UkuiQuick::LottieAsset asset;
    asset.setExtractDir(generation);
    QCOMPARE(UkuiQuick::diskCachePathForAsset(asset), canonicalPath(generation));
    asset.setExtractDir(QDir(temporaryDir.path()).filePath(QStringLiteral("not-created_FILES")));
    QCOMPARE(UkuiQuick::diskCachePathForAsset(asset),
             QDir::cleanPath(QFileInfo(asset.extractDir()).absoluteFilePath()));
    asset.setExtractDir(QString());
    asset.setAnimationJsonPath(source);
    QVERIFY(UkuiQuick::diskCachePathForAsset(asset).isEmpty());
}

void lottie_cache_manager_test::validatesCacheRoots()
{
    QTemporaryDir temporaryHome;
    QTemporaryDir sourceDir;
    QVERIFY(temporaryHome.isValid());
    QVERIFY(sourceDir.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());

    auto *manager = UkuiQuick::LottieCacheManager::instance();
    const QString cacheRoot = manager->cacheRootDir();
    QVERIFY(!cacheRoot.isEmpty());
    QCOMPARE(cacheRoot,
             canonicalPath(QDir(temporaryHome.path()).filePath(
                 QStringLiteral(".cache/ukui-quick/lottie"))));
    QVERIFY(!QFileInfo::exists(
        QDir(temporaryHome.path()).filePath(QStringLiteral(".local/lottie"))));

    QTemporaryDir alternateCacheHome;
    QVERIFY(alternateCacheHome.isValid());
    {
        ScopedEnvironmentVariable cacheHome("XDG_CACHE_HOME", alternateCacheHome.path().toUtf8());
        const QString alternateRoot = manager->cacheRootDir();
        QCOMPARE(alternateRoot,
                 canonicalPath(QDir(alternateCacheHome.path()).filePath(
                     QStringLiteral("ukui-quick/lottie"))));
    }
    {
        ScopedEnvironmentVariable cacheHome("XDG_CACHE_HOME", QByteArrayLiteral("relative-cache"));
        QCOMPARE(manager->cacheRootDir(), cacheRoot);
    }

    const QString firstSource = QDir(sourceDir.path()).filePath(QStringLiteral("one/same.lottie"));
    const QString secondSource = QDir(sourceDir.path()).filePath(QStringLiteral("two/same.lottie"));
    QVERIFY(writeFile(firstSource, QByteArrayLiteral("one")));
    QVERIFY(writeFile(secondSource, QByteArrayLiteral("two")));
    const QString digestA(64, QLatin1Char('a'));
    const QString digestB(64, QLatin1Char('b'));
    const QString firstExtract = manager->cacheExtractDirForRevision(firstSource, digestA);
    const QString secondExtract = manager->cacheExtractDirForRevision(secondSource, digestA);
    QVERIFY(!firstExtract.isEmpty());
    QVERIFY(!secondExtract.isEmpty());
    QVERIFY(firstExtract != secondExtract);
    QVERIFY(firstExtract.endsWith(digestA + QStringLiteral("_FILES")));
    // 同一路径的新旧内容（不同摘要）得到不同解压代际目录。
    QVERIFY(manager->cacheExtractDirForRevision(firstSource, digestB) != firstExtract);
    QVERIFY(manager->cacheExtractDirForRevision(QStringLiteral("/"), digestA).isEmpty());
    QVERIFY(manager->cacheExtractDirForRevision(firstSource, QString()).isEmpty());
    QVERIFY(manager->cacheExtractDirForRevision(firstSource, QStringLiteral("../not-a-digest")).isEmpty());

    const QString blockedSource = QDir(sourceDir.path()).filePath(QStringLiteral("blocked.lottie"));
    QVERIFY(writeFile(blockedSource, QByteArrayLiteral("blocked")));
    const QString blockedHash = sha256Name(QString::fromUtf8(
        QUrl::fromLocalFile(canonicalPath(blockedSource)).toEncoded(QUrl::FullyEncoded)));
    QVERIFY(writeFile(QDir(cacheRoot).filePath(QStringLiteral("extracted/") + blockedHash),
                      QByteArrayLiteral("not-a-directory")));
    QVERIFY(manager->cacheExtractDirForRevision(blockedSource, digestA).isEmpty());

    QTemporaryDir fileHome;
    QVERIFY(fileHome.isValid());
    {
        HomeEnvironment invalidHome(fileHome.path());
        QVERIFY(writeFile(QDir(fileHome.path()).filePath(QStringLiteral(".cache")),
                          QByteArrayLiteral("file")));
        QVERIFY(manager->cacheRootDir().isEmpty());
    }

    QTemporaryDir linkedHome;
    QTemporaryDir linkedTarget;
    QVERIFY(linkedHome.isValid());
    QVERIFY(linkedTarget.isValid());
    {
        HomeEnvironment invalidHome(linkedHome.path());
        QVERIFY(QDir().mkpath(QDir(linkedHome.path()).filePath(
            QStringLiteral(".cache/ukui-quick"))));
        const QString linkPath = QDir(linkedHome.path()).filePath(
            QStringLiteral(".cache/ukui-quick/lottie"));
        QVERIFY(QFile::link(linkedTarget.path(), linkPath));
        QVERIFY(manager->cacheRootDir().isEmpty());
    }
}

void lottie_cache_manager_test::resolvesSourceReferences()
{
    QTemporaryDir motionIconRoot;
    QTemporaryDir qmlSourceRoot;
    QVERIFY(motionIconRoot.isValid());
    QVERIFY(qmlSourceRoot.isValid());

    const QUrl baseUrl = QUrl::fromLocalFile(qmlSourceRoot.path() + QLatin1Char('/'));
    const UkuiQuick::LottieSourceResolver resolver(motionIconRoot.path());
    QCOMPARE(UkuiQuick::LottieSourceResolver::defaultMotionIconRoot(),
             QStringLiteral("/usr/share/icons/ukui-motion-icons"));
    QVERIFY(!resolver.resolve(UkuiQuick::LottieSourceReference::fromText(QString())).isValid());

    const QString localJson = QDir(qmlSourceRoot.path()).filePath(QStringLiteral("local.json"));
    QVERIFY(writeFile(localJson, animationJson()));
    const auto localUrlReference = UkuiQuick::LottieSourceReference::fromText(
        QUrl::fromLocalFile(localJson).toString());
    QCOMPARE(localUrlReference.kind(), UkuiQuick::LottieSourceKind::LocalFile);
    QCOMPARE(resolver.resolve(localUrlReference).path, canonicalPath(localJson));

    const auto resourceReference = UkuiQuick::LottieSourceReference::fromText(
        QStringLiteral("qrc:/lottie-cache-manager/fixture.json"));
    QCOMPARE(resourceReference.kind(), UkuiQuick::LottieSourceKind::Resource);
    QCOMPARE(resolver.resolve(resourceReference).path,
             QStringLiteral(":/lottie-cache-manager/fixture.json"));

    const QString explicitJsonName = QStringLiteral("themed-motion.json");
    const QString explicitJsonPath = QDir(qmlSourceRoot.path()).filePath(explicitJsonName);
    const QString misleadingInstalledPath = QDir(motionIconRoot.path()).filePath(
        QStringLiteral("themed-motion.lottie"));
    QVERIFY(writeFile(explicitJsonPath, animationJson()));
    QVERIFY(writeFile(misleadingInstalledPath, QByteArrayLiteral("not-selected")));
    const auto explicitJsonReference = UkuiQuick::LottieSourceReference::fromText(
        explicitJsonName, baseUrl);
    QCOMPARE(explicitJsonReference.kind(), UkuiQuick::LottieSourceKind::RelativeFile);
    const auto explicitJsonResult = resolver.resolve(explicitJsonReference);
    QCOMPARE(explicitJsonResult.path, canonicalPath(explicitJsonPath));
    QCOMPARE(explicitJsonResult.format, UkuiQuick::LottieSourceFormat::Json);

    const QString fileName = QStringLiteral("alarm-24-symbolic-motion.lottie");
    const QString qmlSource = QDir(qmlSourceRoot.path()).filePath(fileName);
    QVERIFY(writeFile(qmlSource, QByteArrayLiteral("qml-source")));
    const auto reference = UkuiQuick::LottieSourceReference::fromText(
        QStringLiteral("alarm-24-symbolic-motion"),
        baseUrl);
    QCOMPARE(reference.kind(), UkuiQuick::LottieSourceKind::MotionIcon);
    QCOMPARE(reference.canonicalLocator(), fileName);
    const auto fallbackResult = resolver.resolve(reference);
    QCOMPARE(fallbackResult.path, canonicalPath(qmlSource));
    QCOMPARE(fallbackResult.format, UkuiQuick::LottieSourceFormat::DotLottie);

    const QString installedSource = QDir(motionIconRoot.path()).filePath(fileName);
    QVERIFY(writeFile(installedSource, QByteArrayLiteral("installed-source")));
    QCOMPARE(resolver.resolve(reference).path, canonicalPath(installedSource));

    const auto explicitMotionFile = UkuiQuick::LottieSourceReference::fromText(
        QStringLiteral("animations/alarm-24-symbolic-motion.lottie"), baseUrl);
    QCOMPARE(explicitMotionFile.kind(), UkuiQuick::LottieSourceKind::RelativeFile);
}

void lottie_cache_manager_test::loadsCachesAndInvalidatesJsonAssets()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    const QString source = QDir(temporaryHome.path()).filePath(QStringLiteral("animation.json"));
    const QByteArray firstAnimation = animationJson(QByteArrayLiteral("first"));
    QVERIFY(writeFile(source, firstAnimation));
    temporaryArchiveCommitCount = 0;
    UkuiQuick::LottieAsset firstAsset = loadAsset(manager, source);
    QVERIFY(firstAsset.isValid());
    QCOMPARE(temporaryArchiveCommitCount, 0);
    QVERIFY(firstAsset.animationJsonPath().isEmpty());
    QCOMPARE(firstAsset.animationJsonData(), firstAnimation);

    const UkuiQuick::LottieAsset cachedAsset = loadAsset(manager, source);
    QVERIFY(cachedAsset.isValid());
    QCOMPARE(cachedAsset.animationJsonData(), firstAsset.animationJsonData());
    QCOMPARE(cachedAsset.revision(), firstAsset.revision());
    const UkuiQuick::LottieAsset urlAsset = loadAsset(manager, QUrl::fromLocalFile(source).toString());
    QCOMPARE(urlAsset.animationJsonData(), firstAsset.animationJsonData());
    const UkuiQuick::LottieAsset relativeAsset = loadAsset(
        manager,
        UkuiQuick::LottieSourceReference::fromText(
            QStringLiteral("animation.json"),
            QUrl::fromLocalFile(temporaryHome.path() + QLatin1Char('/'))));
    QVERIFY(relativeAsset.isValid());
    QCOMPARE(relativeAsset.animationJsonData(), firstAsset.animationJsonData());
    {
        QMutexLocker locker(&manager->d->parsedAssets.mutex);
        QCOMPARE(manager->d->parsedAssets.assetCache.size(), 1);
        QVERIFY(manager->d->parsedAssets.cacheAccessTick >= 3);
    }

    const QByteArray secondAnimation = animationJson(QByteArrayLiteral("second-version-with-a-different-size"));
    QVERIFY(writeFile(source, secondAnimation));
    QVERIFY(setFileModifiedTime(source, QDateTime::currentDateTimeUtc().addSecs(2)));
    const UkuiQuick::LottieAsset refreshedAsset = loadAsset(manager, source);
    QVERIFY(refreshedAsset.isValid());
    QVERIFY(refreshedAsset.revision() != firstAsset.revision());
    QCOMPARE(refreshedAsset.animationJsonData(), secondAnimation);
    // 两个已交付版本各自持有内存数据，新读取不会改写旧资产。
    QCOMPARE(firstAsset.animationJsonData(), firstAnimation);

    const QString invalid = QDir(temporaryHome.path()).filePath(QStringLiteral("invalid.json"));
    QVERIFY(writeFile(invalid, QByteArrayLiteral("{}")));
    QVERIFY(!loadAsset(manager, invalid).isValid());

    const QString resource = QStringLiteral(":/lottie-cache-manager/fixture.json");
    const UkuiQuick::LottieAsset resourceAsset = loadAsset(
        manager,
        QStringLiteral("qrc:/lottie-cache-manager/fixture.json"));
    QVERIFY(resourceAsset.isValid());
    QVERIFY(resourceAsset.animationJsonPath().isEmpty());
    QCOMPARE(resourceAsset.animationJsonData(), readFile(resource));
    QVERIFY(loadAsset(manager, resource).isValid());
}

void lottie_cache_manager_test::jsonAssetsKeepImmutableBytesWithoutDiskCache()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    const QString source = QDir(temporaryHome.path()).filePath(QStringLiteral("inline.json"));
    const QByteArray original = animationJson(QByteArrayLiteral("inline-original"));
    QVERIFY(writeFile(source, original));

    const UkuiQuick::LottieAsset asset = loadAsset(manager, source);
    QVERIFY(asset.isValid());
    QVERIFY(asset.animationJsonPath().isEmpty());
    QCOMPARE(asset.animationJsonData(), original);

    // 已交付资产拥有自己的不可变数据；后续替换原路径不能改变它。
    QVERIFY(writeFile(source, animationJson(QByteArrayLiteral("replacement"))));
    QCOMPARE(asset.animationJsonData(), original);

    const QString cacheRoot = manager->cacheRootDir();
    QVERIFY(!cacheRoot.isEmpty());
    QVERIFY(!QFileInfo(QDir(cacheRoot).filePath(QStringLiteral("source-index"))).exists());
    QVERIFY(!QFileInfo(QDir(cacheRoot).filePath(QStringLiteral("snapshots"))).exists());
    QVERIFY(QDir(cacheRoot)
                .entryList(QStringList{QStringLiteral(".source-*.json")},
                           QDir::Files | QDir::Hidden)
                .isEmpty());
}

void lottie_cache_manager_test::loadsRefreshesAndRejectsDotLottieAssets()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    const QByteArray firstAnimation = animationJson(QByteArrayLiteral("first-dot-lottie"));
    const QString source = writeLottieArchive(temporaryHome.path(),
                                              QStringLiteral("animation.lottie"),
                                              QByteArrayLiteral("main"),
                                              firstAnimation);
    QVERIFY(!source.isEmpty());
    UkuiQuick::LottieAsset firstAsset = loadAsset(manager, source);
    QVERIFY(firstAsset.isValid());
    QCOMPARE(firstAsset.activeAnimationId(), QStringLiteral("main"));
    const QString extractDir = firstAsset.extractDir();
    QVERIFY(QFileInfo(QDir(extractDir).filePath(QStringLiteral(".complete"))).isFile());
    QCOMPARE(readFile(firstAsset.animationJsonPath()), firstAnimation);
    {
        const UkuiQuick::LottieAsset cachedAsset = loadAsset(manager, source);
        QVERIFY(cachedAsset.isValid());
        QCOMPARE(cachedAsset.extractDir(), extractDir);
    }

    firstAsset = UkuiQuick::LottieAsset();
    resetManagerState(manager);
    UkuiQuick::LottieAsset currentAsset = loadAsset(manager, source);
    QVERIFY(currentAsset.isValid());
    QCOMPARE(currentAsset.extractDir(), extractDir);
    currentAsset = UkuiQuick::LottieAsset();
    resetManagerState(manager);

    // 文件 stat 代际变化后使用新的解压目录，不复用旧目录。
    const QByteArray secondAnimation = animationJson(QByteArrayLiteral("second-dot-lottie-with-a-different-size"));
    QVERIFY(writeFile(source, dotLottieArchive(QByteArrayLiteral("main"), secondAnimation)));
    const UkuiQuick::LottieAsset refreshedAsset = loadAsset(manager, source);
    QVERIFY(refreshedAsset.isValid());
    QVERIFY(refreshedAsset.extractDir() != extractDir);
    QCOMPARE(readFile(refreshedAsset.animationJsonPath()), secondAnimation);

    const QString upperCaseSource = writeLottieArchive(temporaryHome.path(),
                                                       QStringLiteral("upper.LoTTiE"),
                                                       QByteArrayLiteral("upper"),
                                                       animationJson(QByteArrayLiteral("upper")));
    QVERIFY(!upperCaseSource.isEmpty());
    const UkuiQuick::LottieAsset upperCaseAsset = loadAsset(manager, upperCaseSource);
    QVERIFY(upperCaseAsset.isValid());

    // qrc 的 .lottie 别名指向 JSON 内容：按归档解析失败，且不污染持久缓存。
    const QString resourceArchive = QStringLiteral(":/lottie-cache-manager/fixture.lottie");
    QVERIFY(!loadAsset(manager, resourceArchive).isValid());

    QTemporaryDir resourceDirectory;
    QVERIFY(resourceDirectory.isValid());
    QString registeredRcc;
    QVERIFY(registerArchiveResource(resourceDirectory.path(), &registeredRcc));
    UkuiQuick::LottieAsset resourceDotLottieAsset = loadAsset(
        manager,
        QStringLiteral("qrc:/lottie-cache-manager-dynamic/archive.lottie"));
    QVERIFY(resourceDotLottieAsset.isValid());
    QCOMPARE(resourceDotLottieAsset.activeAnimationId(), QStringLiteral("resource"));
    resourceDotLottieAsset = UkuiQuick::LottieAsset();
    resetManagerState(manager);
    QVERIFY(QResource::unregisterResource(registeredRcc));

    // 解压代际目录被符号链接占用时加载失败。
    const QString staleSource = writeLottieArchive(temporaryHome.path(),
                                                    QStringLiteral("stale.lottie"),
                                                    QByteArrayLiteral("stale"),
                                                    animationJson(QByteArrayLiteral("stale")));
    QString staleExtractDir;
    {
        const UkuiQuick::LottieAsset staleAsset = loadAsset(manager, staleSource);
        QVERIFY(staleAsset.isValid());
        staleExtractDir = staleAsset.extractDir();
    }
    resetManagerState(manager);
    QVERIFY(!staleExtractDir.isEmpty());
    QVERIFY(QDir(staleExtractDir).removeRecursively());
    const QString symlinkTarget = QDir(temporaryHome.path()).filePath(QStringLiteral("symlink-target"));
    QVERIFY(QDir().mkpath(symlinkTarget));
    QVERIFY(QFile::link(symlinkTarget, staleExtractDir));
    QVERIFY(!loadAsset(manager, staleSource).isValid());
    QVERIFY(QFile::remove(staleExtractDir));

    // 已发布目录中的 manifest 损坏时加载失败。
    const QString incompleteSource = writeLottieArchive(temporaryHome.path(),
                                                          QStringLiteral("current-invalid.lottie"),
                                                          QByteArrayLiteral("invalid"),
                                                          animationJson(QByteArrayLiteral("invalid")));
    QFile incompleteFile(incompleteSource);
    QVERIFY(incompleteFile.open(QIODevice::ReadOnly));
    UkuiQuick::SourceVersionStamp incompleteVersion;
    QVERIFY(UkuiQuick::sourceVersionStamp(incompleteFile, &incompleteVersion));
    const QString incompleteDigestHex = QString::fromLatin1(
        UkuiQuick::localArchiveVersionDigest(canonicalPath(incompleteSource), incompleteVersion).toHex());
    const QString incompleteExtractDir = manager->cacheExtractDirForRevision(incompleteSource, incompleteDigestHex);
    QVERIFY(QDir().mkpath(incompleteExtractDir));
    QVERIFY(writeFile(QDir(incompleteExtractDir).filePath(QStringLiteral(".complete")), QByteArrayLiteral("ok\n")));
    QVERIFY(writeFile(QDir(incompleteExtractDir).filePath(QStringLiteral("manifest.json")), QByteArrayLiteral("{}")));
    QVERIFY(!loadAsset(manager, incompleteSource).isValid());

    QTemporaryDir invalidHome;
    QVERIFY(invalidHome.isValid());
    {
        HomeEnvironment invalidEnvironment(invalidHome.path());
        QVERIFY(writeFile(QDir(invalidHome.path()).filePath(QStringLiteral(".cache")),
                          QByteArrayLiteral("file")));
        QVERIFY(!loadAsset(manager, source).isValid());
    }
}

void lottie_cache_manager_test::localDotLottieUsesStatOnlyPersistentExtraction()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    const QString source = writeLottieArchive(temporaryHome.path(),
                                              QStringLiteral("indexed.lottie"),
                                              QByteArrayLiteral("main"),
                                              animationJson(QByteArrayLiteral("indexed")));
    QVERIFY(!source.isEmpty());

    temporaryArchiveCommitCount = 0;
    const UkuiQuick::LottieAsset firstAsset = loadAsset(manager, source);
    QVERIFY(firstAsset.isValid());
    QCOMPARE(temporaryArchiveCommitCount, 1);
    const QString firstExtractDir = firstAsset.extractDir();
    QVERIFY(!firstExtractDir.isEmpty());

    const QString cacheRoot = manager->cacheRootDir();
    const QString extractedDir = QDir(cacheRoot).filePath(QStringLiteral("extracted"));
    const QString canonicalUri = QString::fromUtf8(
        QUrl::fromLocalFile(canonicalPath(source)).toEncoded(QUrl::FullyEncoded));
    const QString sourceRoot = QDir(extractedDir).filePath(sha256Name(canonicalUri));
    const QString expectedGeneration = QDir(sourceRoot).filePath(
        QString::fromLatin1(firstAsset.revision().m_identityDigest.toHex()) +
        QStringLiteral("_FILES"));
    QCOMPARE(QDir::cleanPath(firstExtractDir), QDir::cleanPath(expectedGeneration));
    QVERIFY(QFileInfo(QDir(firstExtractDir).filePath(QStringLiteral(".lease"))).isFile());
    QVERIFY(!QFileInfo(QDir(cacheRoot).filePath(QStringLiteral("source-index"))).exists());
    QVERIFY(!QFileInfo(QDir(cacheRoot).filePath(QStringLiteral("snapshots"))).exists());

    // 模拟新进程丢失内存缓存。文件的 stat 版本没有变化时，直接复用已发布的
    // 解压目录，不再读取、复制、计算归档内容摘要或重新解压。
    resetManagerState(manager);
    temporaryArchiveCommitCount = 0;
    const int notificationDescriptor = ::inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    QVERIFY(notificationDescriptor >= 0);
    ScopedFileDescriptor notificationGuard(notificationDescriptor);
    QVERIFY(::inotify_add_watch(notificationDescriptor,
                                QFile::encodeName(source).constData(),
                                IN_OPEN | IN_ACCESS) >= 0);
    UkuiQuick::LottieAsset secondAsset = loadAsset(manager, source);
    std::array<char, 4096> sourceEvents = {};
    errno = 0;
    const qint64 sourceEventBytes = ::read(notificationDescriptor,
                                           sourceEvents.data(),
                                           sourceEvents.size());
    QCOMPARE(sourceEventBytes, -1LL);
    QCOMPARE(errno, EAGAIN);
    QVERIFY(secondAsset.isValid());
    QCOMPARE(temporaryArchiveCommitCount, 0);
    QCOMPARE(secondAsset.revision(), firstAsset.revision());
    QCOMPARE(secondAsset.extractDir(), firstExtractDir);

    // 仅比较 path + size + mtime 会漏掉“同大小、保留修改时间的替换文件”。
    // inode/ctime 也属于版本身份，所以这种替换必须得到新的 source 版本。
    const QDateTime originalModified = QFileInfo(source).lastModified();
    const QString replacement = source + QStringLiteral(".replacement");
    const QByteArray changedArchive = dotLottieArchive(
        QByteArrayLiteral("main"),
        animationJson(QByteArrayLiteral("changed")));
    QCOMPARE(changedArchive.size(), readFile(source).size());
    QVERIFY(writeFile(replacement, changedArchive));
    QVERIFY(setFileModifiedTime(replacement, originalModified));
    QVERIFY(QFile::remove(source));
    QVERIFY(QFile(replacement).rename(source));
    const UkuiQuick::LottieAsset changedAsset = loadAsset(manager, source);
    QVERIFY(changedAsset.isValid());
    QVERIFY(changedAsset.revision() != firstAsset.revision());
    QVERIFY(changedAsset.extractDir() != firstExtractDir);
}

void lottie_cache_manager_test::qrcDotLottieCachesOnlyExtractedContent()
{
    QTemporaryDir temporaryHome;
    QTemporaryDir resourceDirectory;
    QVERIFY(temporaryHome.isValid());
    QVERIFY(resourceDirectory.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    QString registeredRcc;
    QVERIFY(registerArchiveResource(resourceDirectory.path(), &registeredRcc));
    const QString source = QStringLiteral(
        "qrc:/lottie-cache-manager-dynamic/archive.lottie");

    UkuiQuick::LottieAsset firstAsset = loadAsset(manager, source);
    QVERIFY(firstAsset.isValid());
    const QString firstExtractDir = firstAsset.extractDir();
    QVERIFY(!firstExtractDir.isEmpty());

    const QString cacheRoot = manager->cacheRootDir();
    const QString persistentRoot = canonicalPath(cacheRoot);
    const QString privateExtract = canonicalPath(firstExtractDir);
    QVERIFY(!privateExtract.isEmpty());
    QVERIFY(!privateExtract.startsWith(persistentRoot + QLatin1Char('/')));
    QVERIFY(QFileInfo(QDir(firstExtractDir).filePath(QStringLiteral(".lease"))).isFile());
    QVERIFY(!QFileInfo(QDir(cacheRoot).filePath(QStringLiteral("source-index"))).exists());
    QVERIFY(!QFileInfo(QDir(cacheRoot).filePath(QStringLiteral("snapshots"))).exists());
    QVERIFY(!QFileInfo(QDir(cacheRoot).filePath(QStringLiteral("extracted"))).exists());

    resetManagerState(manager);
    UkuiQuick::LottieAsset secondAsset = loadAsset(manager, source);
    QVERIFY(secondAsset.isValid());
    QCOMPARE(secondAsset.revision(), firstAsset.revision());
    QCOMPARE(secondAsset.extractDir(), firstExtractDir);

    QVERIFY(QResource::unregisterResource(registeredRcc));

    // 进程私有根的所有权随资产 lease 传递；即使 manager 先释放自己的引用，
    // 已交付资产仍可读取。最后一个资产/内存缓存释放后目录自动删除。
    const QString privateRoot = QFileInfo(firstExtractDir).absolutePath() +
        QStringLiteral("/../..");
    const QString cleanPrivateRoot = QDir::cleanPath(privateRoot);
    manager->d->resourceCache.clear();
    QVERIFY(QFileInfo(cleanPrivateRoot).isDir());
    resetManagerState(manager);
    firstAsset = UkuiQuick::LottieAsset();
    secondAsset = UkuiQuick::LottieAsset();
    QVERIFY(!QFileInfo::exists(cleanPrivateRoot));
}

void lottie_cache_manager_test::rejectsInvalidSourcesAndLockedCaches()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    const QString validSource = QDir(temporaryHome.path()).filePath(QStringLiteral("valid.json"));
    const QString directorySource = QDir(temporaryHome.path()).filePath(QStringLiteral("directory"));
    const QString linkSource = QDir(temporaryHome.path()).filePath(QStringLiteral("link.json"));
    QVERIFY(writeFile(validSource, animationJson()));
    QVERIFY(QDir().mkpath(directorySource));
    QVERIFY(QFile::link(validSource, linkSource));
    QVERIFY(!loadAsset(manager, QString()).isValid());
    QVERIFY(!loadAsset(manager, QDir(temporaryHome.path()).filePath(QStringLiteral("missing.json"))).isValid());
    QVERIFY(!loadAsset(manager, directorySource).isValid());
    QVERIFY(!loadAsset(manager, linkSource).isValid());
    QVERIFY(!loadAsset(manager, QStringLiteral("qrc:/lottie-cache-manager/missing.json")).isValid());

    const QString cacheRoot = manager->cacheRootDir();
    QVERIFY(!cacheRoot.isEmpty());
    const QString lockPath = QDir(cacheRoot).filePath(QStringLiteral(".cache.lock"));
    QVERIFY(QDir().mkpath(lockPath));
    QElapsedTimer lockTimer;
    lockTimer.start();
    int lockCancellationChecks = 0;
    QVERIFY(!loadAsset(manager, validSource,
                                   QString(),
                                   [&lockCancellationChecks] {
                                       return ++lockCancellationChecks > 1;
                                   }).isValid());
    QVERIFY(lockTimer.elapsed() < 1000);
    const QString archiveSource = writeLottieArchive(temporaryHome.path(),
                                                      QStringLiteral("locked.lottie"),
                                                      QByteArrayLiteral("locked"),
                                                      animationJson(QByteArrayLiteral("locked")));
    int archiveLockCancellationChecks = 0;
    QVERIFY(!loadAsset(manager, archiveSource,
                                   QString(),
                                   [&archiveLockCancellationChecks] {
                                       return ++archiveLockCancellationChecks > 1;
                                   }).isValid());
    QVERIFY(QDir(lockPath).removeRecursively());

    const QString brokenArchive = QDir(temporaryHome.path()).filePath(QStringLiteral("broken.lottie"));
    const QByteArray brokenBytes = QByteArrayLiteral("not-a-zip-file");
    QVERIFY(writeFile(brokenArchive, brokenBytes));
    FailedLoadWarningCounter warningCounter;
    QVERIFY(!loadAsset(manager, brokenArchive).isValid());
    QCOMPARE(warningCounter.count(), 1);
    // 失败冷却按本地 source 版本记录：重复请求命中冷却，文件版本变化得到
    // 新身份。测试只观察记录行为，不依赖私有键的编码格式。
    QDateTime retryAfter;
    {
        QMutexLocker locker(&manager->d->parsedAssets.mutex);
        QCOMPARE(manager->d->parsedAssets.failedLoads.size(), 1);
        retryAfter = manager->d->parsedAssets.failedLoads.constBegin()->retryAfter;
    }
    QVERIFY(!loadAsset(manager, brokenArchive).isValid());
    QCOMPARE(warningCounter.count(), 1);
    const QByteArray changedBytes = QByteArrayLiteral("still-not-a-zip-file-but-changed");
    QVERIFY(writeFile(brokenArchive, changedBytes));
    QVERIFY(!loadAsset(manager, brokenArchive).isValid());
    QCOMPARE(warningCounter.count(), 2);
    {
        QMutexLocker locker(&manager->d->parsedAssets.mutex);
        QCOMPARE(manager->d->parsedAssets.failedLoads.size(), 2);
        QDateTime newestRetry;
        for (auto failed = manager->d->parsedAssets.failedLoads.begin();
             failed != manager->d->parsedAssets.failedLoads.end(); ++failed) {
            newestRetry = qMax(newestRetry, failed->retryAfter);
            failed->retryAfter = QDateTime::currentDateTimeUtc().addMSecs(-1);
        }
        QVERIFY(newestRetry >= retryAfter);
    }
    QVERIFY(!loadAsset(manager, brokenArchive).isValid());
    QCOMPARE(warningCounter.count(), 3);
}

void lottie_cache_manager_test::coversAdditionalFailureAndScanningBranches()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();
    const QString jsonSource = QDir(temporaryHome.path()).filePath(QStringLiteral("source.json"));
    const QString archiveSource = writeLottieArchive(temporaryHome.path(),
                                                      QStringLiteral("source.lottie"),
                                                      QByteArrayLiteral("main"),
                                                      animationJson());
    QVERIFY(writeFile(jsonSource, animationJson()));
    QVERIFY(!archiveSource.isEmpty());

    QTemporaryDir invalidHome;
    QVERIFY(invalidHome.isValid());
    {
        HomeEnvironment invalidEnvironment(invalidHome.path());
        QVERIFY(writeFile(QDir(invalidHome.path()).filePath(QStringLiteral(".cache")),
                          QByteArrayLiteral("file")));
        QVERIFY(manager->cacheExtractDirForRevision(archiveSource,
                                                  QString(64, QLatin1Char('a'))).isEmpty());
        // 普通 JSON 不使用磁盘缓存，即使缓存根不可用也能加载。
        QVERIFY(loadAsset(manager, jsonSource).isValid());
        QVERIFY(!loadAsset(manager, archiveSource).isValid());
    }

    QVERIFY(!loadAsset(manager, QStringLiteral("qrc:/lottie-cache-manager/missing.lottie")).isValid());
    QVERIFY(!loadAsset(manager, QStringLiteral("qrc:/")).isValid());
    QVERIFY(!loadAsset(manager, QStringLiteral("/")).isValid());

    const QString cacheRoot = manager->cacheRootDir();
    QVERIFY(!cacheRoot.isEmpty());
    const QString symlinkSource = QDir(temporaryHome.path()).filePath(QStringLiteral("symlink-source.lottie"));
    QVERIFY(writeFile(symlinkSource, QByteArrayLiteral("source")));
    const QString sourceHash = sha256Name(QString::fromUtf8(
        QUrl::fromLocalFile(canonicalPath(symlinkSource)).toEncoded(QUrl::FullyEncoded)));
    const QString extractedRoot = QDir(cacheRoot).filePath(QStringLiteral("extracted"));
    QVERIFY(QDir().mkpath(extractedRoot));
    const QString sourceRoot = QDir(extractedRoot).filePath(sourceHash);
    const QString sourceRootTarget = QDir(temporaryHome.path()).filePath(QStringLiteral("source-root-target"));
    QVERIFY(QDir().mkpath(sourceRootTarget));
    QVERIFY(QFile::link(sourceRootTarget, sourceRoot));
    QVERIFY(manager->cacheExtractDirForRevision(symlinkSource, QString(64, QLatin1Char('a'))).isEmpty());

    const QString scanRoot = QDir(extractedRoot).filePath(QString(64, QLatin1Char('9')));
    QVERIFY(QDir().mkpath(QDir(scanRoot).filePath(
        QString(64, QLatin1Char('8')) + QStringLiteral("_FILES"))));
    QVERIFY(QDir().mkpath(QDir(scanRoot).filePath(QStringLiteral("not-a-generation"))));
    QVERIFY(QDir().mkpath(QDir(extractedRoot).filePath(QStringLiteral("not-a-source-root"))));
    QVERIFY(manager->maintainDiskCache(cacheRoot, QString()));
}

void lottie_cache_manager_test::coversCanonicalizationFailurePaths()
{
    auto *manager = UkuiQuick::LottieCacheManager::instance();
    QTemporaryDir cacheRoot;
    QVERIFY(cacheRoot.isValid());

    const QString sourceHash(64, QLatin1Char('c'));
    const QString sourceRootName = QStringLiteral("extracted/") + sourceHash;
    const QString sourceRoot = QDir(cacheRoot.path()).filePath(sourceRootName);
    const QString generation = createPublishedGeneration(cacheRoot.path(),
                                                          sourceRootName,
                                                          QString(64, QLatin1Char('b')) + QStringLiteral("_FILES"),
                                                          4 * 1024,
                                                          QDateTime::currentDateTimeUtc(),
                                                          QDateTime::currentDateTimeUtc());
    QVERIFY(!generation.isEmpty());
    {
        ScopedRealpathFailure failure(generation);
        QVERIFY(manager->maintainDiskCache(cacheRoot.path(), QString()));
    }
    {
        ScopedRealpathFailure failure(sourceRoot);
        QVERIFY(manager->maintainDiskCache(cacheRoot.path(), QString()));
    }

    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    const QString managerCacheRoot = manager->cacheRootDir();
    QVERIFY(!managerCacheRoot.isEmpty());
    const QString archiveSource = QDir(temporaryHome.path()).filePath(QStringLiteral("canonical.lottie"));
    QVERIFY(writeFile(archiveSource, QByteArrayLiteral("archive")));
    const QString archiveHash = sha256Name(QString::fromUtf8(
        QUrl::fromLocalFile(canonicalPath(archiveSource)).toEncoded(QUrl::FullyEncoded)));
    const QString archiveRoot = QDir(managerCacheRoot).filePath(
        QStringLiteral("extracted/") + archiveHash);
    QVERIFY(QDir().mkpath(archiveRoot));
    const QString digestHex(64, QLatin1Char('d'));
    {
        ScopedRealpathFailure failure(archiveRoot);
        QVERIFY(manager->cacheExtractDirForRevision(archiveSource, digestHex).isEmpty());
    }
}

void lottie_cache_manager_test::logsMaintenanceFailuresWithoutDiscardingAssets()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    const QString cacheRoot = manager->cacheRootDir();
    QVERIFY(!cacheRoot.isEmpty());
    const QString retainedGeneration = createPublishedGeneration(
        cacheRoot,
        QStringLiteral("extracted/") + QString(64, QLatin1Char('a')),
        QString(64, QLatin1Char('b')) + QStringLiteral("_FILES"),
        257 * kMiB,
        QDateTime::currentDateTimeUtc().addSecs(-1),
        QDateTime::currentDateTimeUtc().addSecs(-1));
    QVERIFY(!retainedGeneration.isEmpty());

    // Keep the oversized cache entry leased so maintenance cannot evict it.
    UkuiQuick::LottieAsset retainedAsset;
    retainedAsset.setExtractDir(retainedGeneration);
    manager->attachDiskCacheLease(retainedAsset);

    const QString jsonSource = QDir(temporaryHome.path()).filePath(QStringLiteral("maintenance.json"));
    QVERIFY(writeFile(jsonSource, animationJson(QByteArrayLiteral("maintenance-json"))));
    const UkuiQuick::LottieAsset jsonAsset = loadAsset(manager, jsonSource);
    QVERIFY(jsonAsset.isValid());
    QVERIFY(QFileInfo(retainedGeneration).isDir());

    const QString archiveSource = writeLottieArchive(temporaryHome.path(),
                                                      QStringLiteral("maintenance.lottie"),
                                                      QByteArrayLiteral("maintenance"),
                                                      animationJson(QByteArrayLiteral("maintenance-dot")));
    QVERIFY(!archiveSource.isEmpty());
    const UkuiQuick::LottieAsset dotLottieAsset = loadAsset(manager, archiveSource);
    QVERIFY(dotLottieAsset.isValid());
    QVERIFY(QFileInfo(retainedGeneration).isDir());
}

void lottie_cache_manager_test::maintainsDiskCacheAndHonorsProtection()
{
    QTemporaryDir cacheRoot;
    QVERIFY(cacheRoot.isValid());
    const QDateTime now = QDateTime::currentDateTimeUtc();
    const QString extractedPrefix = QStringLiteral("extracted/");
    const QString oldGeneration = createPublishedGeneration(cacheRoot.path(),
                                                            extractedPrefix + QString(64, QLatin1Char('a')),
                                                            QString(64, QLatin1Char('4')) + QStringLiteral("_FILES"),
                                                            140 * kMiB,
                                                            now,
                                                            now.addDays(-2));
    const QString recentGeneration = createPublishedGeneration(cacheRoot.path(),
                                                               extractedPrefix + QString(64, QLatin1Char('b')),
                                                               QString(64, QLatin1Char('5')) + QStringLiteral("_FILES"),
                                                               140 * kMiB,
                                                               now,
                                                               now);
    QVERIFY(!oldGeneration.isEmpty());
    QVERIFY(!recentGeneration.isEmpty());
    auto *manager = UkuiQuick::LottieCacheManager::instance();
    QVERIFY(manager->maintainDiskCache(cacheRoot.path(), recentGeneration));
    QVERIFY(!QFileInfo::exists(oldGeneration));
    QVERIFY(QFileInfo(recentGeneration).isDir());

    const QString olderGeneration = createPublishedGeneration(cacheRoot.path(),
                                                              extractedPrefix + QString(64, QLatin1Char('d')),
                                                              QString(64, QLatin1Char('6')) + QStringLiteral("_FILES"),
                                                              140 * kMiB,
                                                              now,
                                                              now.addDays(-3));
    const QString nestedTemporaryDir = QDir(cacheRoot.path()).filePath(
        extractedPrefix + QString(64, QLatin1Char('b')) + QStringLiteral("/.extract-nested"));
    QVERIFY(!olderGeneration.isEmpty());
    QVERIFY(QDir().mkpath(nestedTemporaryDir));
    QVERIFY(manager->maintainDiskCache(cacheRoot.path(), recentGeneration));
    QVERIFY(!QFileInfo::exists(olderGeneration));
    QVERIFY(!QFileInfo::exists(nestedTemporaryDir));

    const QString protectedGeneration = createPublishedGeneration(cacheRoot.path(),
                                                                  extractedPrefix + QString(64, QLatin1Char('c')),
                                                                  QString(64, QLatin1Char('7')) + QStringLiteral("_FILES"),
                                                                  300 * kMiB,
                                                                  now,
                                                                  now);
    QVERIFY(!protectedGeneration.isEmpty());
    QVERIFY(!manager->maintainDiskCache(cacheRoot.path(), protectedGeneration));
    QVERIFY(QFileInfo(protectedGeneration).isDir());

    QVERIFY(!manager->maintainDiskCache(QDir(cacheRoot.path()).filePath(QStringLiteral("missing")), QString()));
    const QString symlinkRoot = QDir(cacheRoot.path()).filePath(QStringLiteral("symlink-root"));
    QVERIFY(QFile::link(cacheRoot.path(), symlinkRoot));
    QVERIFY(!manager->maintainDiskCache(symlinkRoot, QString()));
}

void lottie_cache_manager_test::preservesLeasedDiskEntriesUntilAssetsAreReleased()
{
    QTemporaryDir cacheRoot;
    QVERIFY(cacheRoot.isValid());
    const QDateTime now = QDateTime::currentDateTimeUtc();
    const QString extractedPrefix = QStringLiteral("extracted/");
    const QString retainedGeneration = createPublishedGeneration(cacheRoot.path(),
                                                                 extractedPrefix + QString(64, QLatin1Char('d')),
                                                                 QString(64, QLatin1Char('1')) + QStringLiteral("_FILES"),
                                                                 140 * kMiB,
                                                                 now,
                                                                 now.addDays(-2));
    const QString protectedGeneration = createPublishedGeneration(cacheRoot.path(),
                                                                  extractedPrefix + QString(64, QLatin1Char('e')),
                                                                  QString(64, QLatin1Char('2')) + QStringLiteral("_FILES"),
                                                                  140 * kMiB,
                                                                  now,
                                                                  now);
    QVERIFY(!retainedGeneration.isEmpty());
    QVERIFY(!protectedGeneration.isEmpty());

    auto *manager = UkuiQuick::LottieCacheManager::instance();
    UkuiQuick::LottieAsset referencedGeneration;
    {
        UkuiQuick::LottieAsset cachedAsset;
        cachedAsset.setExtractDir(retainedGeneration);
        cachedAsset.setAnimationJsonPath(QDir(retainedGeneration).filePath(QStringLiteral("payload.bin")));
        storeAsset(manager, QStringLiteral("retained-generation"), cachedAsset);
        referencedGeneration = cachedAsset;
    }
    QVERIFY(!manager->maintainDiskCache(cacheRoot.path(), protectedGeneration));
    QVERIFY(QFileInfo(retainedGeneration).isDir());
    {
        QMutexLocker locker(&manager->d->parsedAssets.mutex);
        QVERIFY(!manager->d->parsedAssets.assetCache.contains(QStringLiteral("retained-generation")));
    }
    referencedGeneration = UkuiQuick::LottieAsset();
    QVERIFY(manager->maintainDiskCache(cacheRoot.path(), protectedGeneration));
    QVERIFY(!QFileInfo::exists(retainedGeneration));

}

void lottie_cache_manager_test::removesOnlyValidatedCacheEntries()
{
    QTemporaryDir cacheRoot;
    QVERIFY(cacheRoot.isValid());
    const QDateTime now = QDateTime::currentDateTimeUtc();
    const QString parentName = QStringLiteral("extracted/") + QString(64, QLatin1Char('f'));
    const QString parent = QDir(cacheRoot.path()).filePath(parentName);
    const QString generation = createPublishedGeneration(cacheRoot.path(),
                                                          parentName,
                                                          QString(64, QLatin1Char('3')) + QStringLiteral("_FILES"),
                                                          4 * 1024,
                                                          now,
                                                          now);
    QVERIFY(!generation.isEmpty());
    auto *manager = UkuiQuick::LottieCacheManager::instance();
    QVERIFY(manager->removeExtractGeneration(parent, generation));
    QVERIFY(!QFileInfo::exists(generation));

    const QString invalidGeneration = QDir(parent).filePath(QStringLiteral("not-a-generation"));
    QVERIFY(QDir().mkpath(invalidGeneration));
    QVERIFY(!manager->removeExtractGeneration(parent, invalidGeneration));
    QVERIFY(!manager->removeExtractGeneration(cacheRoot.path(), invalidGeneration));
    QVERIFY(!manager->removeExtractGeneration(parent, QDir(parent).filePath(QStringLiteral("missing_FILES"))));

    QVERIFY(manager->retireCachedAssetsUnderDirectory(parent));
}

void lottie_cache_manager_test::coversPrivateCacheStateTransitions()
{
    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());
    const QString source = QDir(temporaryDir.path()).filePath(QStringLiteral("source.json"));
    const QString cachedGeneration = QDir(temporaryDir.path()).filePath(
        QString(64, QLatin1Char('a')) + QStringLiteral("_FILES"));
    const QString animationFile = QDir(cachedGeneration).filePath(QStringLiteral("animation.json"));
    QVERIFY(writeFile(source, animationJson()));
    QVERIFY(QDir().mkpath(cachedGeneration));
    QVERIFY(writeFile(QDir(cachedGeneration).filePath(QStringLiteral(".complete")),
                      QByteArrayLiteral("ok\n")));
    QVERIFY(writeFile(animationFile, animationJson(QByteArrayLiteral("cached"))));
    QFileInfo sourceInfo(source);
    auto *manager = UkuiQuick::LottieCacheManager::instance();
    auto *state = &manager->d->parsedAssets;

    UkuiQuick::LottieAsset validAsset;
    validAsset.setExtractDir(cachedGeneration);
    validAsset.setAnimationJsonPath(animationFile);
    validAsset.setRevision(UkuiQuick::LottieAssetRevision::fromIdentityDigest(
        QCryptographicHash::hash(QByteArrayLiteral("cached"), QCryptographicHash::Sha256)));
    storeAsset(manager, QStringLiteral("cached"), validAsset);
    UkuiQuick::LottieAsset cachedAsset;
    QCOMPARE(state->acquireAssetLoad(QStringLiteral("cached"), sourceInfo, false, &cachedAsset),
             UkuiQuick::ParsedAssetCache::AssetLoadStatus::Cached);
    QCOMPARE(cachedAsset.animationJsonPath(), animationFile);

    UkuiQuick::LottieAsset cancelledAsset;
    const auto cancelled = [] { return true; };
    QCOMPARE(state->acquireAssetLoad(QStringLiteral("cached"),
                                     sourceInfo,
                                     false,
                                     &cancelledAsset,
                                     cancelled),
             UkuiQuick::ParsedAssetCache::AssetLoadStatus::Cancelled);
    QVERIFY(!cancelledAsset.isValid());

    QVERIFY(QFile::remove(animationFile));
    cachedAsset = UkuiQuick::LottieAsset();
    QCOMPARE(state->acquireAssetLoad(QStringLiteral("cached"), sourceInfo, false, &cachedAsset),
             UkuiQuick::ParsedAssetCache::AssetLoadStatus::NeedsLoading);
    UkuiQuick::LottieAsset invalidAsset;
    state->finishAssetLoad(QStringLiteral("cached"), invalidAsset);

    // 按内容摘要记录、只按时间冷却的失败条目。
    UkuiQuick::ParsedAssetCache::FailedLoadData futureFailure;
    futureFailure.versionKeyed = true;
    futureFailure.retryAfter = QDateTime::currentDateTimeUtc().addSecs(60);
    {
        QMutexLocker locker(&state->mutex);
        state->failedLoads.insert(QStringLiteral("failed"), futureFailure);
    }
    QCOMPARE(state->acquireAssetLoad(QStringLiteral("failed"), sourceInfo, false, &cachedAsset),
             UkuiQuick::ParsedAssetCache::AssetLoadStatus::Failed);
    futureFailure.retryAfter = QDateTime::currentDateTimeUtc().addMSecs(-1);
    {
        QMutexLocker locker(&state->mutex);
        state->failedLoads.insert(QStringLiteral("expired"), futureFailure);
    }
    QCOMPARE(state->acquireAssetLoad(QStringLiteral("expired"), sourceInfo, false, &cachedAsset),
             UkuiQuick::ParsedAssetCache::AssetLoadStatus::NeedsLoading);
    state->finishAssetLoad(QStringLiteral("expired"), invalidAsset);

    const QByteArray successfulJson = animationJson(QByteArrayLiteral("success"));
    const QByteArray successfulDigest = QCryptographicHash::hash(
        successfulJson, QCryptographicHash::Sha256);
    const QString externalAnimationFile = QDir(temporaryDir.path()).filePath(
        QStringLiteral("external-animation.json"));
    QVERIFY(writeFile(externalAnimationFile, successfulJson));
    UkuiQuick::LottieAsset loadedAsset;
    loadedAsset.setAnimationJsonData(successfulJson);
    loadedAsset.setRevision(
        UkuiQuick::LottieAssetRevision::fromIdentityDigest(successfulDigest));
    {
        QMutexLocker locker(&state->mutex);
        state->failedLoads.insert(QStringLiteral("success"), futureFailure);
    }
    state->finishAssetLoad(QStringLiteral("success"), loadedAsset);
    {
        QMutexLocker locker(&state->mutex);
        QVERIFY(state->assetCache.contains(QStringLiteral("success")));
        QVERIFY(!state->failedLoads.contains(QStringLiteral("success")));
    }

    // 内联 JSON 需要有效 revision；归档缓存还必须指向完整的解压目录。
    UkuiQuick::ParsedAssetCache::CachedAssetData data;
    data.asset.setAnimationJsonData(successfulJson);
    data.asset.setRevision(
        UkuiQuick::LottieAssetRevision::fromIdentityDigest(successfulDigest));
    QVERIFY(state->assetIsUsable(data));
    data.asset = UkuiQuick::LottieAsset();
    QVERIFY(!state->assetIsUsable(data));

    const QString generation = QDir(temporaryDir.path()).filePath(QStringLiteral("generation_FILES"));
    const QString generationAnimation = QDir(generation).filePath(QStringLiteral("animation.json"));
    QVERIFY(writeFile(generationAnimation, animationJson()));
    QVERIFY(writeFile(QDir(generation).filePath(QStringLiteral(".complete")), QByteArrayLiteral("ok")));
    data.asset.setAnimationJsonPath(generationAnimation);
    data.asset.setExtractDir(generation);
    data.asset.setRevision(
        UkuiQuick::LottieAssetRevision::fromIdentityDigest(successfulDigest));
    QVERIFY(state->assetIsUsable(data));
    QVERIFY(QFile::remove(QDir(generation).filePath(QStringLiteral(".complete"))));
    QVERIFY(!state->assetIsUsable(data));
    QVERIFY(writeFile(QDir(generation).filePath(QStringLiteral(".complete")), QByteArrayLiteral("ok")));
    data.asset.setAnimationJsonPath(externalAnimationFile);
    QVERIFY(!state->assetIsUsable(data));

    // 按源路径记录的失败条目仍需源版本匹配才冷却。
    UkuiQuick::ParsedAssetCache::FailedLoadData cooling;
    cooling.sourceFileSize = sourceInfo.size();
    cooling.sourceLastModified = sourceInfo.lastModified();
    cooling.sourceLastMetadataChange = sourceInfo.metadataChangeTime();
    cooling.retryAfter = QDateTime::currentDateTimeUtc().addSecs(60);
    QVERIFY(state->failedLoadIsCoolingDown(cooling, sourceInfo, false));
    QVERIFY(state->failedLoadIsCoolingDown(cooling, QFileInfo(), true));
    ++cooling.sourceFileSize;
    QVERIFY(!state->failedLoadIsCoolingDown(cooling, sourceInfo, false));
    --cooling.sourceFileSize;
    cooling.retryAfter = QDateTime::currentDateTimeUtc().addMSecs(-1);
    QVERIFY(!state->failedLoadIsCoolingDown(cooling, sourceInfo, false));
    // 内容寻址条目只按时间冷却。
    cooling = UkuiQuick::ParsedAssetCache::FailedLoadData();
    cooling.versionKeyed = true;
    cooling.retryAfter = QDateTime::currentDateTimeUtc().addSecs(60);
    QVERIFY(state->failedLoadIsCoolingDown(cooling, sourceInfo, false));
    cooling.retryAfter = QDateTime::currentDateTimeUtc().addMSecs(-1);
    QVERIFY(!state->failedLoadIsCoolingDown(cooling, sourceInfo, false));
}

void lottie_cache_manager_test::trimsInternalCacheState()
{
    auto *manager = UkuiQuick::LottieCacheManager::instance();
    auto *state = &manager->d->parsedAssets;
    QMutexLocker locker(&state->mutex);
    state->cacheAccessTick = 0;
    QCOMPARE(state->nextCacheAccessTickLocked(), 1ULL);
    UkuiQuick::ParsedAssetCache::CachedAssetData touched;
    state->touchAssetLocked(touched);
    QCOMPARE(touched.lastAccessTick, 2ULL);

    state->assetCache.clear();
    UkuiQuick::ParsedAssetCache::CachedAssetData emptyKeyEntry;
    emptyKeyEntry.lastAccessTick = 0;
    state->assetCache.insert(QString(), emptyKeyEntry);
    for (int index = 0; index <= 127; ++index) {
        UkuiQuick::ParsedAssetCache::CachedAssetData entry;
        entry.lastAccessTick = static_cast<quint64>(index + 1);
        state->assetCache.insert(QStringLiteral("empty-victim-%1").arg(index), entry);
    }
    state->trimAssetCacheLocked();
    QCOMPARE(state->assetCache.size(), 128);
    QVERIFY(!state->assetCache.contains(QString()));

    state->assetCache.clear();
    for (int index = 0; index <= 128; ++index) {
        UkuiQuick::ParsedAssetCache::CachedAssetData entry;
        entry.lastAccessTick = static_cast<quint64>(index);
        state->assetCache.insert(QStringLiteral("asset-%1").arg(index), entry);
    }
    state->trimAssetCacheLocked();
    QCOMPARE(state->assetCache.size(), 128);
    QVERIFY(!state->assetCache.contains(QStringLiteral("asset-0")));
    QVERIFY(state->assetCache.contains(QStringLiteral("asset-128")));

    // 内联 JSON 还受 64 MiB 字节预算限制，不会因为条目数少就无界持有大文件。
    state->assetCache.clear();
    const QByteArray largeInlineJson(16 * 1024 * 1024, ' ');
    for (int index = 0; index < 5; ++index) {
        UkuiQuick::ParsedAssetCache::CachedAssetData entry;
        entry.asset.setAnimationJsonData(largeInlineJson);
        entry.lastAccessTick = static_cast<quint64>(index + 1);
        state->assetCache.insert(QStringLiteral("inline-%1").arg(index), entry);
    }
    state->trimAssetCacheLocked();
    QCOMPARE(state->assetCache.size(), 4);
    QVERIFY(!state->assetCache.contains(QStringLiteral("inline-0")));
    QVERIFY(state->assetCache.contains(QStringLiteral("inline-4")));

    state->failedLoads.clear();
    UkuiQuick::ParsedAssetCache::FailedLoadData expired;
    expired.retryAfter = QDateTime::currentDateTimeUtc().addMSecs(-1);
    state->failedLoads.insert(QStringLiteral("expired"), expired);
    const QDateTime now = QDateTime::currentDateTimeUtc();
    for (int index = 0; index <= 128; ++index) {
        UkuiQuick::ParsedAssetCache::FailedLoadData entry;
        entry.retryAfter = now.addSecs(index + 1);
        state->failedLoads.insert(QStringLiteral("failed-%1").arg(index), entry);
    }
    state->trimFailedLoadsLocked();
    QCOMPARE(state->failedLoads.size(), 128);
    QVERIFY(!state->failedLoads.contains(QStringLiteral("expired")));
    QVERIFY(!state->failedLoads.contains(QStringLiteral("failed-0")));
    QVERIFY(state->failedLoads.contains(QStringLiteral("failed-128")));
}

void lottie_cache_manager_test::assignsSourceVersionRevisions()
{
    QTemporaryDir temporaryHome;
    QTemporaryDir sourceDir;
    QVERIFY(temporaryHome.isValid());
    QVERIFY(sourceDir.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    const QString firstPath = writeLottieArchive(sourceDir.path(),
                                                  QStringLiteral("one/same.lottie"),
                                                  QByteArrayLiteral("main"),
                                                  animationJson(QByteArrayLiteral("main")));
    QVERIFY(!firstPath.isEmpty());
    const QByteArray originalBytes = readFile(firstPath);
    QVERIFY(!originalBytes.isEmpty());

    UkuiQuick::LottieAssetRevision firstRevision;
    {
        const UkuiQuick::LottieAsset firstAsset = loadAsset(manager, firstPath);
        QVERIFY(firstAsset.isValid());
        firstRevision = firstAsset.revision();
        QVERIFY(firstRevision.isValid());

        // 缓存命中与重复请求保持同一 revision。
        QCOMPARE(loadAsset(manager, firstPath).revision(), firstRevision);
    }

    // 本地归档的 revision 描述“这个路径上的这个文件代际”。相同内容位于不同
    // 路径时仍是两个 source；同一路径的普通路径与 file URL 则是同一个 source。
    const QString secondPath = QDir(sourceDir.path()).filePath(QStringLiteral("two/same.lottie"));
    QVERIFY(writeFile(secondPath, originalBytes));
    QVERIFY(loadAsset(manager, secondPath).revision() != firstRevision);
    QCOMPARE(loadAsset(manager, QUrl::fromLocalFile(firstPath).toString()).revision(), firstRevision);

    // 同一路径内容变化得到新 revision。
    const QByteArray changedBytes = dotLottieArchive(QByteArrayLiteral("main"),
                                                     animationJson(QByteArrayLiteral("changed")));
    QVERIFY(writeFile(firstPath, changedBytes));
    {
        const UkuiQuick::LottieAsset changedAsset = loadAsset(manager, firstPath);
        QVERIFY(changedAsset.isValid());
        QVERIFY(changedAsset.revision().isValid());
        QVERIFY(changedAsset.revision() != firstRevision);
    }

    // 即使恢复相同字节，文件的 stat 代际已经变化，也不能复用旧 revision。
    QVERIFY(writeFile(firstPath, originalBytes));
    QVERIFY(loadAsset(manager, firstPath).revision() != firstRevision);

    // 普通 JSON 源同样携带内容派生 revision。
    const QString jsonPath = QDir(sourceDir.path()).filePath(QStringLiteral("plain.json"));
    QVERIFY(writeFile(jsonPath, animationJson()));
    {
        const UkuiQuick::LottieAsset jsonAsset = loadAsset(manager, jsonPath);
        QVERIFY(jsonAsset.isValid());
        QVERIFY(jsonAsset.revision().isValid());
        QVERIFY(jsonAsset.revision() != firstRevision);
    }
}

void lottie_cache_manager_test::sourceChangeDuringCopyRetriesWithStableBytes()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    // 第一次临时复制完成后原地改写源文件。加载器会发现同一个已打开文件的
    // stat 版本变化，丢弃第一次副本并完整重试一次。
    const QString source = QDir(temporaryHome.path()).filePath(QStringLiteral("isolation.lottie"));
    const QByteArray originalArchive = dotLottieArchive(QByteArrayLiteral("main"),
                                                        animationJson(QByteArrayLiteral("original")));
    const QByteArray replacementArchive = dotLottieArchive(QByteArrayLiteral("main"),
                                                           animationJson(QByteArrayLiteral("replaced")));
    QVERIFY(writeFile(source, originalArchive));
    bool archiHookFired = false;
    UkuiQuick::LottieAsset asset;
    {
        TemporaryArchiveCommitHookGuard hookGuard([&] {
            if (!archiHookFired) {
                archiHookFired = true;
                writeFile(source, replacementArchive);
            }
        });
        asset = loadAsset(manager, source);
    }
    QVERIFY(archiHookFired);
    QVERIFY(asset.isValid());
    QCOMPARE(readFile(source), replacementArchive);
    QCOMPARE(readFile(asset.animationJsonPath()), animationJson(QByteArrayLiteral("replaced")));
    QCOMPARE(asset.frameCount(), 60);
    QCOMPARE(asset.frameRate(), 30.0);

    // revision 来自重试后稳定观察到的 source 版本；不同路径即使内容相同，
    // 也不会合并为同一个本地文件身份。
    const QString referencePath = QDir(temporaryHome.path()).filePath(QStringLiteral("reference.lottie"));
    QVERIFY(writeFile(referencePath, originalArchive));
    const UkuiQuick::LottieAsset referenceAsset = loadAsset(manager, referencePath);
    QVERIFY(referenceAsset.isValid());
    QVERIFY(asset.revision() != referenceAsset.revision());
    const QString replacedPath = QDir(temporaryHome.path()).filePath(QStringLiteral("replaced.lottie"));
    QVERIFY(writeFile(replacedPath, replacementArchive));
    const UkuiQuick::LottieAsset replacedAsset = loadAsset(manager, replacedPath);
    QVERIFY(replacedAsset.isValid());
    QVERIFY(replacedAsset.revision() != asset.revision());

    // JSON 不写快照，但交付的资产自己持有读取时的不可变字节。
    const QString jsonSource = QDir(temporaryHome.path()).filePath(QStringLiteral("isolation.json"));
    const QByteArray originalJson = animationJson(QByteArrayLiteral("json-original"));
    QVERIFY(writeFile(jsonSource, originalJson));
    const UkuiQuick::LottieAsset jsonAsset = loadAsset(manager, jsonSource);
    QVERIFY(jsonAsset.isValid());
    QVERIFY(jsonAsset.animationJsonPath().isEmpty());
    QCOMPARE(jsonAsset.animationJsonData(), originalJson);
    QVERIFY(writeFile(jsonSource, QByteArrayLiteral("{}")));
    QCOMPARE(readFile(jsonSource), QByteArrayLiteral("{}"));
    QCOMPARE(jsonAsset.animationJsonData(), originalJson);
    QCOMPARE(jsonAsset.frameCount(), 60);
    QCOMPARE(jsonAsset.frameRate(), 30.0);
}

void lottie_cache_manager_test::continuouslyChangingSourceFailsAfterOneRetry()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    const QString source = writeLottieArchive(temporaryHome.path(),
                                              QStringLiteral("unstable.lottie"),
                                              QByteArrayLiteral("main"),
                                              animationJson(QByteArrayLiteral("unstable")));
    QVERIFY(!source.isEmpty());

    int changes = 0;
    temporaryArchiveCommitCount = 0;
    UkuiQuick::LottieAsset asset;
    {
        TemporaryArchiveCommitHookGuard hookGuard([&] {
            QFile changingSource(source);
            if (changingSource.open(QIODevice::WriteOnly | QIODevice::Append)) {
                QCOMPARE(changingSource.write("x", 1), 1LL);
                ++changes;
            }
        });
        asset = loadAsset(manager, source);
    }

    QVERIFY(!asset.isValid());
    QCOMPARE(changes, 2);
    QCOMPARE(temporaryArchiveCommitCount, 2);
    {
        QMutexLocker locker(&manager->d->parsedAssets.mutex);
        QVERIFY(manager->d->parsedAssets.assetCache.isEmpty());
        QVERIFY(manager->d->parsedAssets.failedLoads.isEmpty());
    }
    QCOMPARE(publishedGenerationCount(manager->cacheRootDir()), 0);
}

void lottie_cache_manager_test::growingSourceIsRejectedBeforeTemporaryAppend()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    const QString source = writeLottieArchive(temporaryHome.path(),
                                              QStringLiteral("growing.lottie"),
                                              QByteArrayLiteral("main"),
                                              animationJson(QByteArrayLiteral("growing")));
    QVERIFY(!source.isEmpty());
    QFile paddedSource(source);
    QVERIFY(paddedSource.open(QIODevice::WriteOnly | QIODevice::Append));
    const qint64 paddingSize = UkuiQuick::DotLottieParser::maxArchiveInputBytes() -
        paddedSource.size();
    QVERIFY(paddingSize > 0);
    QCOMPARE(paddedSource.write(QByteArray(static_cast<int>(paddingSize), '\0')),
             paddingSize);
    paddedSource.close();

    bool grew = false;
    temporaryArchiveBytesWritten = 0;
    UkuiQuick::LottieAsset asset;
    {
        TemporaryArchiveWriteHookGuard hookGuard([&] {
            if (grew) {
                return;
            }
            grew = true;
            QFile sourceWriter(source);
            if (sourceWriter.open(QIODevice::WriteOnly | QIODevice::Append)) {
                sourceWriter.write("x", 1);
            }
        });
        asset = loadAsset(manager, source);
    }

    QVERIFY(grew);
    QVERIFY(!asset.isValid());
    QCOMPARE(temporaryArchiveBytesWritten,
             UkuiQuick::DotLottieParser::maxArchiveInputBytes());
    QCOMPARE(publishedGenerationCount(manager->cacheRootDir()), 0);
}

void lottie_cache_manager_test::oldAndNewRevisionsCoexistWhileLeased()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    const QString source = QDir(temporaryHome.path()).filePath(QStringLiteral("coexist.lottie"));
    const QByteArray firstBytes = dotLottieArchive(QByteArrayLiteral("main"),
                                                    animationJson(QByteArrayLiteral("first")));
    QVERIFY(writeFile(source, firstBytes));
    UkuiQuick::LottieAsset oldAsset = loadAsset(manager, source);
    QVERIFY(oldAsset.isValid());
    const UkuiQuick::LottieAssetRevision oldRevision = oldAsset.revision();
    const QString oldExtractDir = oldAsset.extractDir();

    // 持有旧资产时，同一路径写入新内容：新 revision 使用新的解压代际目录。
    const QByteArray secondBytes = dotLottieArchive(QByteArrayLiteral("main"),
                                                     animationJson(QByteArrayLiteral("second")));
    QVERIFY(writeFile(source, secondBytes));
    UkuiQuick::LottieAsset newAsset = loadAsset(manager, source);
    QVERIFY(newAsset.isValid());
    QVERIFY(newAsset.revision().isValid());
    QVERIFY(newAsset.revision() != oldRevision);
    QVERIFY(newAsset.extractDir() != oldExtractDir);

    // 两者均可读：旧 revision 的内容不受新内容影响。
    QCOMPARE(readFile(oldAsset.animationJsonPath()), animationJson(QByteArrayLiteral("first")));
    QCOMPARE(readFile(newAsset.animationJsonPath()), animationJson(QByteArrayLiteral("second")));
    QVERIFY(UkuiQuick::extractGenerationIsPublished(oldExtractDir));

    // 旧资产释放并清理内存状态后，旧代际才能被回收逻辑移除。
    const QString oldExtractParent = QFileInfo(oldExtractDir).absolutePath();
    oldAsset = UkuiQuick::LottieAsset();
    newAsset = UkuiQuick::LottieAsset();
    resetManagerState(manager);
    QVERIFY(manager->removeExtractGeneration(oldExtractParent, oldExtractDir));
    QVERIFY(!QFileInfo::exists(oldExtractDir));
}

void lottie_cache_manager_test::concurrentRequestAfterReplacementJoinsStableRevision()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    const QString source = QDir(temporaryHome.path()).filePath(QStringLiteral("race.lottie"));
    const QByteArray firstBytes = dotLottieArchive(QByteArrayLiteral("main"),
                                                    animationJson(QByteArrayLiteral("race-first")));
    const QByteArray secondBytes = dotLottieArchive(QByteArrayLiteral("main"),
                                                     animationJson(QByteArrayLiteral("race-second")));
    QVERIFY(writeFile(source, firstBytes));

    // 第一个加载完成临时复制后，用 rename 原子替换原路径。第一个请求继续使用
    // 已打开且自洽的 A；第二个请求观察到新 inode，并得到 B 的新 revision。
    std::thread secondRequestThread;
    UkuiQuick::LottieAsset secondAsset;
    bool raceHookFired = false;
    UkuiQuick::LottieAsset firstAsset;
    {
        TemporaryArchiveCommitHookGuard hookGuard([&] {
            if (!raceHookFired) {
                raceHookFired = true;
                const QString replacement = source + QStringLiteral(".replacement");
                writeFile(replacement, secondBytes);
                QFile::remove(source);
                QFile(replacement).rename(source);
                secondRequestThread = std::thread([&] {
                    secondAsset = loadAsset(manager, source);
                });
            }
        });
        firstAsset = loadAsset(manager, source);
    }
    QVERIFY(raceHookFired);
    secondRequestThread.join();

    QVERIFY(firstAsset.isValid());
    QVERIFY(secondAsset.isValid());
    QVERIFY(firstAsset.revision().isValid());
    QVERIFY(secondAsset.revision().isValid());
    QCOMPARE(firstAsset.revision(), secondAsset.revision());
    QCOMPARE(readFile(firstAsset.animationJsonPath()), animationJson(QByteArrayLiteral("race-second")));
    QCOMPARE(readFile(secondAsset.animationJsonPath()), animationJson(QByteArrayLiteral("race-second")));
    // 两个请求最终收敛到重试后稳定观察到的同一代际。
    QCOMPARE(firstAsset.extractDir(), secondAsset.extractDir());
}

void lottie_cache_manager_test::sameContentDifferentSourcesKeepOwnImageRoots()
{
    QTemporaryDir temporaryHome;
    QTemporaryDir directoryA;
    QTemporaryDir directoryB;
    QVERIFY(temporaryHome.isValid());
    QVERIFY(directoryA.isValid());
    QVERIFY(directoryB.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    // 两个目录中完全相同的 JSON，但各自带有不同的外部 images 资源。
    const QByteArray sharedAnimation = imageAnimationJson();
    const QString sourceA = QDir(directoryA.path()).filePath(QStringLiteral("anim.json"));
    const QString sourceB = QDir(directoryB.path()).filePath(QStringLiteral("anim.json"));
    QVERIFY(writeFile(sourceA, sharedAnimation));
    QVERIFY(writeFile(sourceB, sharedAnimation));
    const QByteArray imageA = pngImage(QSize(2, 2), qRgb(255, 0, 0));
    const QByteArray imageB = pngImage(QSize(2, 2), qRgb(0, 0, 255));
    QVERIFY(!imageA.isEmpty());
    QVERIFY(!imageB.isEmpty());
    QVERIFY(writeFile(QDir(directoryA.path()).filePath(QStringLiteral("images/icon.png")),
                      imageA));
    QVERIFY(writeFile(QDir(directoryB.path()).filePath(QStringLiteral("images/icon.png")),
                      imageB));

    const UkuiQuick::LottieAsset assetA = loadAsset(manager, sourceA);
    const UkuiQuick::LottieAsset assetB = loadAsset(manager, sourceB);
    QVERIFY(assetA.isValid());
    QVERIFY(assetB.isValid());

    // JSON 虽然相同，但两份不可变资源快照不同，不能共享帧或栅格会话身份。
    QVERIFY(assetA.revision() != assetB.revision());
    QVERIFY(assetA.revision().isValid());
    QCOMPARE(assetA.animationJsonData(), assetB.animationJsonData());
    QCOMPARE(assetA.animationJsonData(), sharedAnimation);
    const UkuiQuick::LottieBoundImageResource boundA =
        assetA.boundImageResource(QStringLiteral("images/icon.png"));
    const UkuiQuick::LottieBoundImageResource boundB =
        assetB.boundImageResource(QStringLiteral("images/icon.png"));
    QVERIFY(boundA.isValid());
    QVERIFY(boundB.isValid());
    QCOMPARE(boundA.data, imageA);
    QCOMPARE(boundB.data, imageB);
    QCOMPARE(assetA.boundImageResource(QStringLiteral("icon.png")).data, imageA);
    QCOMPARE(assetB.boundImageResource(QStringLiteral("icon.png")).data, imageB);

    // ……但各自的 SourceBinding 解析到自己的图片搜索根，不发生 A -> B 串用。
    const QString rootA = QFileInfo(sourceA).absolutePath();
    const QString rootB = QFileInfo(sourceB).absolutePath();
    QVERIFY(assetA.resourceSearchDirs().contains(rootA));
    QVERIFY(!assetA.resourceSearchDirs().contains(rootB));
    QVERIFY(assetB.resourceSearchDirs().contains(rootB));
    QVERIFY(!assetB.resourceSearchDirs().contains(rootA));

    const QStringList candidatesA = assetA.imagePathCandidates(QStringLiteral("icon.png"));
    const QStringList candidatesB = assetB.imagePathCandidates(QStringLiteral("icon.png"));
    QCOMPARE(candidatesA.size(), 1);
    QCOMPARE(candidatesB.size(), 1);
    QCOMPARE(candidatesA.first(),
             canonicalPath(QDir(directoryA.path()).filePath(QStringLiteral("images/icon.png"))));
    QCOMPARE(candidatesB.first(),
             canonicalPath(QDir(directoryB.path()).filePath(QStringLiteral("images/icon.png"))));
    QCOMPARE(readFile(candidatesA.first()), imageA);
    QCOMPARE(readFile(candidatesB.first()), imageB);
}

void lottie_cache_manager_test::replacingExternalImageCreatesNewAssetRevision()
{
    QTemporaryDir temporaryHome;
    QTemporaryDir sourceDirectory;
    QVERIFY(temporaryHome.isValid());
    QVERIFY(sourceDirectory.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    const QString source = QDir(sourceDirectory.path()).filePath(QStringLiteral("anim.json"));
    const QString imagePath = QDir(sourceDirectory.path()).filePath(
        QStringLiteral("images/icon.png"));
    QVERIFY(writeFile(source, imageAnimationJson()));
    const QByteArray firstImage = pngImage(QSize(2, 2), qRgb(255, 0, 0));
    const QByteArray secondImage = pngImage(QSize(2, 2), qRgb(0, 0, 255));
    QVERIFY(writeFile(imagePath, firstImage));

    const UkuiQuick::LottieAsset first = loadAsset(manager, source);
    QVERIFY(first.isValid());
    QVERIFY(first.revision().isValid());

    QVERIFY(writeFile(imagePath, secondImage));
    const UkuiQuick::LottieAsset second = loadAsset(manager, source);
    QVERIFY(second.isValid());
    QVERIFY(second.revision().isValid());
    QVERIFY(second.revision() != first.revision());
    QCOMPARE(first.boundImageResource(QStringLiteral("images/icon.png")).data,
             firstImage);
    QCOMPARE(second.boundImageResource(QStringLiteral("images/icon.png")).data,
             secondImage);
}

void lottie_cache_manager_test::oversizedExternalImageIsRejectedBeforeRasterization()
{
    QTemporaryDir temporaryHome;
    QTemporaryDir sourceDirectory;
    QVERIFY(temporaryHome.isValid());
    QVERIFY(sourceDirectory.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    const QString source = QDir(sourceDirectory.path()).filePath(QStringLiteral("anim.json"));
    const QString imagePath = QDir(sourceDirectory.path()).filePath(
        QStringLiteral("images/icon.png"));
    QVERIFY(writeFile(source, imageAnimationJson()));
    const QByteArray oversizedImage = pngImage(QSize(8193, 1), qRgb(255, 0, 0));
    QVERIFY(!oversizedImage.isEmpty());
    QVERIFY(writeFile(imagePath, oversizedImage));

    QVERIFY(!loadAsset(manager, source).isValid());

    QByteArray oversizedEncodedImage = pngImage(QSize(2, 2), qRgb(255, 0, 0));
    oversizedEncodedImage.resize(static_cast<int>(16 * kMiB + 1));
    QVERIFY(writeFile(imagePath, oversizedEncodedImage));
    QVERIFY(!loadAsset(manager, source).isValid());

    // 同一预算也适用于 qrc 图片，不能因为资源来自可执行文件就跳过检查。
    QTemporaryDir resourceDirectory;
    QVERIFY(resourceDirectory.isValid());
    QString registeredRcc;
    QVERIFY(registerJsonImageResource(resourceDirectory.path(),
                                      oversizedImage,
                                      &registeredRcc));
    QVERIFY(!loadAsset(manager,
                       QStringLiteral("qrc:/lottie-image-budget/anim.json")).isValid());
    QVERIFY(QResource::unregisterResource(registeredRcc));

    // 内嵌图片也先解析头部和像素预算，再交给 ThorVG。
    QByteArray embeddedAnimation = imageAnimationJson();
    embeddedAnimation.replace(
        QByteArrayLiteral("\"u\":\"images/\",\"p\":\"icon.png\""),
        QByteArrayLiteral("\"u\":\"\",\"p\":\"data:image/png;base64,") +
            oversizedImage.toBase64() + QByteArrayLiteral("\""));
    const QString embeddedSource = QDir(sourceDirectory.path()).filePath(
        QStringLiteral("embedded.json"));
    QVERIFY(writeFile(embeddedSource, embeddedAnimation));
    QVERIFY(!loadAsset(manager, embeddedSource).isValid());
}

void lottie_cache_manager_test::repeatedImageDeclarationsRespectDecodeBudget()
{
    QTemporaryDir temporaryHome;
    QTemporaryDir sourceDirectory;
    QVERIFY(temporaryHome.isValid());
    QVERIFY(sourceDirectory.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    const QString source = QDir(sourceDirectory.path()).filePath(QStringLiteral("anim.json"));
    const QString imagePath = QDir(sourceDirectory.path()).filePath(
        QStringLiteral("images/icon.png"));
    const QByteArray image = pngImage(QSize(1024, 1024), qRgb(255, 0, 0));
    QVERIFY(!image.isEmpty());
    QVERIFY(writeFile(imagePath, image));

    // 唯一资源快照只保留一份，但 16 个 Picture 各自解码 1024x1024 像素，
    // 恰好用完 16 Mi 像素预算。
    QVERIFY(writeFile(source, repeatedImageAnimationJson(16)));
    const UkuiQuick::LottieAsset accepted = loadAsset(manager, source);
    QVERIFY(accepted.isValid());
    QCOMPARE(accepted.imageResourcePaths().size(), 1);
    QCOMPARE(accepted.boundImageResourceBytes(), static_cast<qint64>(image.size()));

    // 第 17 个声明会让 ThorVG 再创建并解码一个 Picture，必须在进入渲染前拒绝。
    QVERIFY(writeFile(source, repeatedImageAnimationJson(17)));
    QVERIFY(!loadAsset(manager, source).isValid());

    // 声明数量和唯一文件数量是两个预算。即使所有声明共用一张 1x1 图片，
    // 第 257 个声明也不能借助路径去重绕过数量上限。
    const QByteArray tinyImage = pngImage(QSize(1, 1), qRgb(255, 0, 0));
    QVERIFY(writeFile(imagePath, tinyImage));
    QVERIFY(writeFile(source, repeatedImageAnimationJson(256)));
    const UkuiQuick::LottieAsset maximumDeclarations = loadAsset(manager, source);
    QVERIFY(maximumDeclarations.isValid());
    QCOMPARE(maximumDeclarations.imageResourcePaths().size(), 1);
    QCOMPARE(maximumDeclarations.boundImageResourceBytes(),
             static_cast<qint64>(tinyImage.size()));

    QVERIFY(writeFile(source, repeatedImageAnimationJson(257)));
    QVERIFY(!loadAsset(manager, source).isValid());
}

void lottie_cache_manager_test::repeatedCancelAfterTemporaryCopyLeavesNoPersistentArtifacts()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();
    const QString cacheRoot = manager->cacheRootDir();
    QVERIFY(!cacheRoot.isEmpty());
    const int baselineCount = publishedGenerationCount(cacheRoot);
    const QString extractedRoot = QDir(cacheRoot).filePath(QStringLiteral("extracted"));
    const QStringList baselineSourceDirectories = QDir(extractedRoot).entryList(
        QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    const QString temporaryRoot = UkuiQuick::temporaryStorageRoot();
    const QStringList baselineTemporaryArchives = QDir(temporaryRoot).entryList(
        QStringList{QStringLiteral("ukui-quick-lottie-*.lottie")},
        QDir::Files | QDir::Hidden);

    for (int iteration = 0; iteration < 5; ++iteration) {
        std::atomic<bool> cancelFlag(false);
        bool hookFired = false;
        const auto isCancelled = [&cancelFlag] { return cancelFlag.load(); };

        const QString source = QDir(temporaryHome.path()).filePath(
            QStringLiteral("cancel-%1.lottie").arg(iteration));
        QVERIFY(writeFile(source,
                          dotLottieArchive(QByteArrayLiteral("main"),
                                           animationJson(QByteArray::number(iteration)))));

        // 临时归档复制完成后取消：不发布解压代际，临时文件由 RAII 删除。
        {
            TemporaryArchiveCommitHookGuard hookGuard([&] {
                hookFired = true;
                cancelFlag.store(true);
            });
            QVERIFY(!loadAsset(manager, source, QString(), isCancelled).isValid());
        }
        QVERIFY(hookFired);
        QCOMPARE(publishedGenerationCount(cacheRoot), baselineCount);
        QCOMPARE(QDir(extractedRoot).entryList(
                     QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks),
                 baselineSourceDirectories);
        QCOMPARE(QDir(temporaryRoot).entryList(
                     QStringList{QStringLiteral("ukui-quick-lottie-*.lottie")},
                     QDir::Files | QDir::Hidden),
                 baselineTemporaryArchives);
        QVERIFY(!QFileInfo(QDir(cacheRoot).filePath(QStringLiteral("snapshots"))).exists());
        QVERIFY(!QFileInfo(QDir(cacheRoot).filePath(QStringLiteral("source-index"))).exists());
    }
}

void lottie_cache_manager_test::jsonInputBudgetIsEnforcedBeforeParsing()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    const QString oversizedJson = QDir(temporaryHome.path()).filePath(QStringLiteral("oversized.json"));
    {
        QFile file(oversizedJson);
        QVERIFY(file.open(QIODevice::WriteOnly));
        const QByteArray chunk(1024 * 1024, ' ');
        for (int index = 0; index < 17; ++index) {
            QCOMPARE(file.write(chunk), static_cast<qint64>(chunk.size()));
        }
    }

    // 超过普通 JSON 固定上限在复制阶段（QJsonDocument::fromJson 之前）失败。
    FailedLoadWarningCounter warningCounter;
    QVERIFY(!loadAsset(manager, oversizedJson).isValid());
    QCOMPARE(warningCounter.budgetCount(), 1);
    QCOMPARE(warningCounter.count(), 0);

    // 失败不留下已发布的解压代际或解析产物。
    QCOMPARE(publishedGenerationCount(manager->cacheRootDir()), 0);

    // 恰好不超过上限的普通 JSON 仍可加载。
    const QString boundedJson = QDir(temporaryHome.path()).filePath(QStringLiteral("bounded.json"));
    QByteArray boundedContent = animationJson(QByteArrayLiteral("bounded"));
    boundedContent.append(QByteArray(static_cast<int>(UkuiQuick::DotLottieParser::maxJsonInputBytes()) -
                                         boundedContent.size(),
                                     ' '));
    QCOMPARE(boundedContent.size(), static_cast<int>(UkuiQuick::DotLottieParser::maxJsonInputBytes()));
    QVERIFY(writeFile(boundedJson, boundedContent));
    const UkuiQuick::LottieAsset boundedAsset = loadAsset(manager, boundedJson);
    QVERIFY(boundedAsset.isValid());
}

void lottie_cache_manager_test::cancelledCacheHitDoesNotDeliverAsset()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();
    const QString cacheRoot = manager->cacheRootDir();
    QVERIFY(!cacheRoot.isEmpty());

    const QString source = QDir(temporaryHome.path()).filePath(QStringLiteral("preheated.json"));
    QVERIFY(writeFile(source, animationJson(QByteArrayLiteral("preheated"))));

    // 预热缓存。
    const UkuiQuick::LottieAsset warmup = loadAsset(manager, source);
    QVERIFY(warmup.isValid());
    const QString cacheKey = warmup.revision().isValid()
        ? QString::fromLatin1(QCryptographicHash::hash(animationJson(QByteArrayLiteral("preheated")),
                                                       QCryptographicHash::Sha256).toHex()) + QStringLiteral(":json")
        : QString();

    // 取消请求不能交付预热结果，也不能删除仍然有效的缓存条目。Cached 分支的
    // 取消时序由 coversPrivateCacheStateTransitions 直接覆盖，不依赖复制 hook。
    const auto isCancelled = [] { return true; };
    const bool result = loadAsset(manager, source, QString(), isCancelled).isValid();
    QVERIFY(!result);

    // 取消不删除缓存条目：后续正常请求仍可命中。
    {
        QMutexLocker locker(&manager->d->parsedAssets.mutex);
        QVERIFY(manager->d->parsedAssets.assetCache.contains(cacheKey));
        QVERIFY(manager->d->parsedAssets.failedLoads.isEmpty());
    }
    const UkuiQuick::LottieAsset afterCancel = loadAsset(manager, source);
    QVERIFY(afterCancel.isValid());
    QCOMPARE(afterCancel.revision(), warmup.revision());

    // JSON 缓存条目保留内联字节，不依赖任何磁盘快照。
    QCOMPARE(afterCancel.animationJsonData(), animationJson(QByteArrayLiteral("preheated")));
    QVERIFY(afterCancel.animationJsonPath().isEmpty());
    QVERIFY(!QFileInfo(QDir(cacheRoot).filePath(QStringLiteral("snapshots"))).exists());
}

void lottie_cache_manager_test::sameBytesJsonDoesNotBypassDotLottieValidation()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    // 同一字节序列既是合法 JSON 动画，又不是合法 .lottie 归档。
    const QByteArray sharedBytes = animationJson(QByteArrayLiteral("shared"));
    const QString jsonSource = QDir(temporaryHome.path()).filePath(QStringLiteral("same.json"));
    const QString lottieSource = QDir(temporaryHome.path()).filePath(QStringLiteral("same.lottie"));
    QVERIFY(writeFile(jsonSource, sharedBytes));
    QVERIFY(writeFile(lottieSource, sharedBytes));

    // 先以 JSON 格式加载：成功。
    const UkuiQuick::LottieAsset jsonAsset = loadAsset(manager, jsonSource);
    QVERIFY(jsonAsset.isValid());
    QVERIFY(jsonAsset.revision().isValid());

    // 再以 .lottie 格式加载同一字节：不得复用 JSON 缓存条目，必须走归档验证并失败。
    FailedLoadWarningCounter warningCounter;
    QVERIFY(!loadAsset(manager, lottieSource).isValid());
    QVERIFY(QDir(QDir(manager->cacheRootDir()).filePath(QStringLiteral("extracted")))
                .entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks)
                .isEmpty());

    // JSON 的 revision 仍是内容摘要；扩展名为 .lottie 的同一字节必须独立走
    // 本地归档身份和归档验证，不能命中 JSON 资产。
    QCOMPARE(jsonAsset.revision(), UkuiQuick::LottieAssetRevision::fromIdentityDigest(
        QCryptographicHash::hash(sharedBytes, QCryptographicHash::Sha256)));
}

void lottie_cache_manager_test::wrongFormatCooldownDoesNotBlockCorrectFormat()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    // 同一字节序列：.lottie 加载失败（不是 zip），其冷却键来自本地 source
    // 版本；随后 .json 的内容键不同，不能被该冷却阻止。
    const QByteArray sharedBytes = animationJson(QByteArrayLiteral("cross-format"));
    const QString lottieSource = QDir(temporaryHome.path()).filePath(QStringLiteral("cross.lottie"));
    const QString jsonSource = QDir(temporaryHome.path()).filePath(QStringLiteral("cross.json"));
    QVERIFY(writeFile(lottieSource, sharedBytes));
    QVERIFY(writeFile(jsonSource, sharedBytes));

    QVERIFY(!loadAsset(manager, lottieSource).isValid());
    // .json 加载必须成功，不受 .lottie 失败冷却的影响。
    const UkuiQuick::LottieAsset jsonAsset = loadAsset(manager, jsonSource);
    QVERIFY(jsonAsset.isValid());
    QVERIFY(jsonAsset.revision().isValid());
}

void lottie_cache_manager_test::cancelledLoadRecordsNeitherFailureNorCooldown()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();
    const QString cacheRoot = manager->cacheRootDir();
    const auto cancelled = [] { return true; };

    // .lottie 与 JSON 的取消加载都不是失败。
    const QString source = writeLottieArchive(temporaryHome.path(),
                                              QStringLiteral("cancel.lottie"),
                                              QByteArrayLiteral("cancel"),
                                              animationJson(QByteArrayLiteral("cancel")));
    QVERIFY(!source.isEmpty());
    const QString jsonSource = QDir(temporaryHome.path()).filePath(QStringLiteral("cancel.json"));
    QVERIFY(writeFile(jsonSource, animationJson(QByteArrayLiteral("cancel-json"))));

    QVERIFY(!loadAsset(manager, source, QString(), cancelled).isValid());
    QVERIFY(!loadAsset(manager, jsonSource, QString(), cancelled).isValid());
    {
        QMutexLocker locker(&manager->d->parsedAssets.mutex);
        // 取消不是失败：既没有发布资产，也没有写入失败冷却。
        QVERIFY(manager->d->parsedAssets.assetCache.isEmpty());
        QVERIFY(manager->d->parsedAssets.failedLoads.isEmpty());
    }
    // 请求在入口处取消，不创建持久解压代际。
    QCOMPARE(publishedGenerationCount(cacheRoot), 0);

    // 紧接着（仍在失败冷却窗口内）的同一 source 加载必须成功。
    const UkuiQuick::LottieAsset asset = loadAsset(manager, source);
    QVERIFY(asset.isValid());
    QVERIFY(asset.revision().isValid());
    const UkuiQuick::LottieAsset jsonAsset = loadAsset(manager, jsonSource);
    QVERIFY(jsonAsset.isValid());
    QVERIFY(jsonAsset.revision().isValid());
}

void lottie_cache_manager_test::rejectsOversizedArchiveInputBudget()
{
    QTemporaryDir temporaryHome;
    QVERIFY(temporaryHome.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    const QString oversized = QDir(temporaryHome.path()).filePath(QStringLiteral("oversized.lottie"));
    {
        QFile file(oversized);
        QVERIFY(file.open(QIODevice::WriteOnly));
        const QByteArray chunk(1024 * 1024, '\0');
        for (int index = 0; index < 17; ++index) {
            QCOMPARE(file.write(chunk), static_cast<qint64>(chunk.size()));
        }
    }

    FailedLoadWarningCounter warningCounter;
    QVERIFY(!loadAsset(manager, oversized).isValid());
    QCOMPARE(warningCounter.count(), 0);

    // 超预算输入不会留下解压目录。
    const QString extractDir = manager->cacheExtractDirForRevision(oversized, QString(64, QLatin1Char('0')));
    QVERIFY(!extractDir.isEmpty());
    QVERIFY(!QFileInfo(extractDir).exists());
    // 新布局不会创建归档副本目录或 source 索引。
    QVERIFY(!QFileInfo(QDir(manager->cacheRootDir())
                           .filePath(QStringLiteral("snapshots"))).exists());
    QVERIFY(!QFileInfo(QDir(manager->cacheRootDir())
                           .filePath(QStringLiteral("source-index"))).exists());
}

void lottie_cache_manager_test::rejectsOverBudgetArchivesUniformlyForQrcAndLocalSources()
{
    QTemporaryDir temporaryHome;
    QTemporaryDir resourceDirectory;
    QVERIFY(temporaryHome.isValid());
    QVERIFY(resourceDirectory.isValid());
    HomeEnvironment homeEnvironment(temporaryHome.path());
    auto *manager = UkuiQuick::LottieCacheManager::instance();

    QString registeredRcc;
    QByteArray archiveBytes;
    QVERIFY(registerBudgetArchiveResource(resourceDirectory.path(), &registeredRcc, &archiveBytes));

    // qrc 来源与本地来源执行同一预算。
    QVERIFY(!loadAsset(
                 manager,
                 QStringLiteral("qrc:/lottie-cache-manager-budget/budget.lottie"))
                 .isValid());
    const QString localPath = QDir(temporaryHome.path()).filePath(QStringLiteral("budget.lottie"));
    QVERIFY(writeFile(localPath, archiveBytes));
    QVERIFY(!loadAsset(manager, localPath).isValid());

    QVERIFY(QResource::unregisterResource(registeredRcc));
}

QTEST_GUILESS_MAIN(lottie_cache_manager_test)

#include "test-lottie-cache-manager.moc"
