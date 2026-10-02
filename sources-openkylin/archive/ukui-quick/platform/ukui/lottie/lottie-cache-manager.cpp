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

#include "lottie-cache-manager.h"

#include "dot-lottie-parser.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QImageReader>
#include <QLockFile>
#include <QMutex>
#include <QMutexLocker>
#include <QSaveFile>
#include <QSharedPointer>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QUrl>
#include <QVector>
#include <QWeakPointer>
#include <algorithm>
#include <array>
#include <limits>
#include <utility>

#include <cerrno>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

namespace UkuiQuick {

namespace {

// Lottie 磁盘缓存允许占用的最大字节数。
constexpr qint64 kMaxDiskCacheBytes = 256LL * 1024LL * 1024LL;
// 计算解压目录用量时每个条目的最小计费字节数。
constexpr qint64 kMinExtractEntryChargeBytes = 4LL * 1024LL;
// 复制归档到自动删除的临时文件时，每次读取的字节数。
constexpr qint64 kResourceCopyChunkBytes = 64LL * 1024LL;
// 内存资产缓存允许保留的最大条目数。
constexpr int kMaxAssetCacheEntries = 128;
// 普通 JSON 会由内存资产持有；除条目数外再限制其总字节数。
constexpr qint64 kMaxInlineJsonAssetCacheBytes = 64LL * 1024LL * 1024LL;
// 单个图片候选允许保留的最大编码字节数。
constexpr qint64 kMaxImageResourceBytes = 16LL * 1024LL * 1024LL;
// 一个资产的全部候选图片快照允许保留的最大编码字节数。
constexpr qint64 kMaxTotalImageResourceBytes = 64LL * 1024LL * 1024LL;
// 图片任一边允许的最大像素数。
constexpr int kMaxImageResourceDimension = 8192;
// 按声明和候选解码次数累计的最大图片像素数。
constexpr qint64 kMaxTotalImageResourcePixels = 16LL * 1024LL * 1024LL;
// 失败加载记录允许保留的最大条目数。
constexpr int kMaxFailedLoadEntries = 128;
// 同一版本资源加载失败后的冷却时间，避免并发请求形成重试风暴。
constexpr int kFailedLoadCooldownMs = 5 * 1000;
// 获取解压缓存全局锁的最长等待时间。
constexpr int kDiskCacheLockTimeoutMs = 30 * 1000;
// 判断解压缓存全局锁已失效的超时时间。
constexpr int kDiskCacheStaleLockMs = 15 * 60 * 1000;
// 标记解压目录完整可用的文件名。
const QString kCompleteMarker = QStringLiteral(".complete");
// 记录解压目录最近访问时间的文件名。
const QString kAccessMarker = QStringLiteral(".access");
// 保护整个解压缓存目录的锁文件名。
const QString kDiskCacheLock = QStringLiteral(".cache.lock");
// 解压目录使用的固定后缀。
const QString kGenerationSuffix = QStringLiteral("_FILES");
const QString kExtractedDirectory = QStringLiteral("extracted");
bool isSha256Name(const QString &name);

// 本地文件的一次可观察版本。路径相同并不代表还是同一个文件，因此同时记录
// inode、大小以及纳秒级内容和元数据时间。
struct SourceVersionStamp
{
    quint64 device = 0;
    quint64 inode = 0;
    qint64 size = -1;
    qint64 modifiedSeconds = 0;
    qint64 modifiedNanoseconds = 0;
    qint64 changedSeconds = 0;
    qint64 changedNanoseconds = 0;

    bool operator==(const SourceVersionStamp &other) const
    {
        return device == other.device &&
               inode == other.inode &&
               size == other.size &&
               modifiedSeconds == other.modifiedSeconds &&
               modifiedNanoseconds == other.modifiedNanoseconds &&
               changedSeconds == other.changedSeconds &&
               changedNanoseconds == other.changedNanoseconds;
    }
};

bool sourceVersionStamp(const struct stat &info, SourceVersionStamp *stamp)
{
    if (!stamp || !S_ISREG(info.st_mode)) {
        return false;
    }
    stamp->device = static_cast<quint64>(info.st_dev);
    stamp->inode = static_cast<quint64>(info.st_ino);
    stamp->size = static_cast<qint64>(info.st_size);
#if defined(__linux__)
    stamp->modifiedSeconds = static_cast<qint64>(info.st_mtim.tv_sec);
    stamp->modifiedNanoseconds = static_cast<qint64>(info.st_mtim.tv_nsec);
    stamp->changedSeconds = static_cast<qint64>(info.st_ctim.tv_sec);
    stamp->changedNanoseconds = static_cast<qint64>(info.st_ctim.tv_nsec);
#else
    stamp->modifiedSeconds = static_cast<qint64>(info.st_mtime);
    stamp->changedSeconds = static_cast<qint64>(info.st_ctime);
#endif
    return true;
}

bool sourceVersionStamp(QFile &source, SourceVersionStamp *stamp)
{
    if (!source.isOpen() || source.handle() < 0) {
        return false;
    }
    struct stat info = {};
    return ::fstat(source.handle(), &info) == 0 &&
           sourceVersionStamp(info, stamp);
}

bool sourceVersionStamp(const QString &sourcePath, SourceVersionStamp *stamp)
{
    struct stat info = {};
    return ::lstat(QFile::encodeName(sourcePath).constData(), &info) == 0 &&
           sourceVersionStamp(info, stamp);
}

bool openLocalSource(const QString &sourcePath,
                     QFile *source,
                     SourceVersionStamp *stamp)
{
    if (!source || !stamp) {
        return false;
    }
    const int fileDescriptor = ::open(QFile::encodeName(sourcePath).constData(),
                                      O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fileDescriptor < 0) {
        return false;
    }
    if (!source->open(fileDescriptor,
                      QIODevice::ReadOnly,
                      QFileDevice::AutoCloseHandle)) {
        ::close(fileDescriptor);
        return false;
    }
    return sourceVersionStamp(*source, stamp);
}

// 清理端持有独占锁直到目录删除结束。没有 .lease 的代际尚未交付给调用方，
// 可以删除；已交付且仍被其他进程使用的代际会让 LOCK_EX | LOCK_NB 立即失败。
class ExclusiveCacheLease
{
public:
    explicit ExclusiveCacheLease(const QString &extractDir)
    {
        const QString leasePath = QDir(extractDir).filePath(QStringLiteral(".lease"));
        m_fileDescriptor = ::open(QFile::encodeName(leasePath).constData(),
                                  O_RDWR | O_CLOEXEC | O_NOFOLLOW);
        if (m_fileDescriptor < 0) {
            m_canRemove = errno == ENOENT;
            return;
        }

        struct stat info = {};
        m_canRemove = ::fstat(m_fileDescriptor, &info) == 0 &&
                      S_ISREG(info.st_mode) &&
                      ::flock(m_fileDescriptor, LOCK_EX | LOCK_NB) == 0;
    }

    ~ExclusiveCacheLease()
    {
        if (m_fileDescriptor >= 0) {
            if (m_canRemove) {
                ::flock(m_fileDescriptor, LOCK_UN);
            }
            ::close(m_fileDescriptor);
        }
    }

    bool canRemove() const
    {
        return m_canRemove;
    }

private:
    int m_fileDescriptor = -1;
    bool m_canRemove = false;
};

QString sourceAbsolutePath(const QFileInfo &sourceInfo)
{
    return QDir::cleanPath(sourceInfo.absoluteFilePath());
}

// 返回缓存根目录下经过检查的私有子目录。
QString cacheSubdirectory(const QString &cacheRoot,
                          const QString &name,
                          bool create)
{
    const QFileInfo rootInfo(cacheRoot);
    if (!rootInfo.exists() || !rootInfo.isDir() || rootInfo.isSymLink()) {
        return {};
    }
    const QString canonicalRoot = rootInfo.canonicalFilePath();
    if (canonicalRoot.isEmpty()) {
        return {};
    }

    const QString path = QDir(canonicalRoot).filePath(name);
    if (create && !QDir().mkpath(path)) {
        return {};
    }
    const QFileInfo pathInfo(path);
    if (!pathInfo.exists() ||
        !pathInfo.isDir() ||
        pathInfo.isSymLink() ||
        pathInfo.absolutePath() != canonicalRoot) {
        return {};
    }
    QFile(path).setPermissions(QFileDevice::ReadOwner |
                               QFileDevice::WriteOwner |
                               QFileDevice::ExeOwner);
    return pathInfo.canonicalFilePath();
}

// 描述一个可参与磁盘缓存淘汰的条目。
struct DiskCacheEntryInfo
{
    /// 条目的规范化路径。
    QString path;
    /// 条目的累计计费用量。
    qint64 bytes = 0;
    /// 最近一次访问时间；清理空间时优先删除最久未使用的条目。
    QDateTime lastAccess;
};

// 判断解压目录是否包含安全的完成标记。
bool extractGenerationIsPublished(const QString &path)
{
    const QFileInfo directoryInfo(path);
    const QFileInfo markerInfo(QDir(path).filePath(kCompleteMarker));
    return directoryInfo.exists() &&
           directoryInfo.isDir() &&
           !directoryInfo.isSymLink() &&
           markerInfo.exists() &&
           markerInfo.isFile() &&
           !markerInfo.isSymLink();
}

// 判断名称是否为 SHA-256 十六进制摘要。
bool isSha256Name(const QString &name)
{
    if (name.size() != 64) {
        return false;
    }
    for (const QChar character : name) {
        if (!character.isDigit() && (character < QLatin1Char('a') || character > QLatin1Char('f'))) {
            return false;
        }
    }
    return true;
}

// 判断目录名是否为当前缓存布局中的源路径哈希目录。
bool isSourceCacheDirectoryName(const QString &name)
{
    return isSha256Name(name);
}

// 判断目录名是否为已解压的 dotLottie 目录。
bool isExtractGenerationName(const QString &name)
{
    return name.size() == 64 + kGenerationSuffix.size() &&
           name.endsWith(kGenerationSuffix) &&
           isSha256Name(name.left(64));
}

QString diskCachePathForAsset(const LottieAsset &asset)
{
    const QString path = asset.extractDir();
    if (path.isEmpty()) {
        return {};
    }

    const QFileInfo pathInfo(path);
    return pathInfo.exists()
        ? pathInfo.canonicalFilePath()
        : QDir::cleanPath(pathInfo.absoluteFilePath());
}

// 递归统计一份解压目录的实际磁盘占用，并防止整数溢出。
qint64 extractGenerationBytes(const QString &path)
{
    qint64 totalBytes = kMinExtractEntryChargeBytes;
    QDirIterator iterator(path,
                          QDir::Files | QDir::Dirs | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot,
                          QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        const QString entryPath = iterator.next();
        const QFileInfo info = iterator.fileInfo();
        qint64 entryBytes = qMax(kMinExtractEntryChargeBytes, info.size());
        struct stat entryStat;
        if (lstat(QFile::encodeName(entryPath).constData(), &entryStat) == 0 && entryStat.st_blocks > 0) {
            constexpr qint64 blockBytes = 512;
            const qint64 allocatedBytes = entryStat.st_blocks > std::numeric_limits<qint64>::max() / blockBytes
                ? std::numeric_limits<qint64>::max()
                : static_cast<qint64>(entryStat.st_blocks) * blockBytes;
            entryBytes = qMax(entryBytes, allocatedBytes);
        }
        if (entryBytes < 0 || totalBytes > std::numeric_limits<qint64>::max() - entryBytes) {
            return std::numeric_limits<qint64>::max();
        }
        totalBytes += entryBytes;
    }
    return totalBytes;
}

// 执行饱和加法，防止缓存用量统计溢出。
qint64 saturatedAdd(qint64 left, qint64 right)
{
    if (right < 0 || left > std::numeric_limits<qint64>::max() - right) {
        return std::numeric_limits<qint64>::max();
    }
    return left + right;
}

// 返回解压目录的最近访问时间；旧缓存没有访问标记时回退到完成标记时间。
QDateTime extractGenerationLastAccess(const QString &extractDir)
{
    const QFileInfo accessInfo(QDir(extractDir).filePath(kAccessMarker));
    if (accessInfo.exists() && accessInfo.isFile() && !accessInfo.isSymLink()) {
        return accessInfo.lastModified();
    }
    const QFileInfo completeInfo(QDir(extractDir).filePath(kCompleteMarker));
    return completeInfo.exists() ? completeInfo.lastModified() : QFileInfo(extractDir).lastModified();
}

// 更新解压目录的最近访问时间。完成标记仍保留原时间，用来判断内容是否完整。
void touchExtractGeneration(const QString &extractDir)
{
    const QString accessPath = QDir(extractDir).filePath(kAccessMarker);
    const QFileInfo accessInfo(accessPath);
    if (accessInfo.exists() && (!accessInfo.isFile() || accessInfo.isSymLink())) {
        return;
    }

    QFile marker(accessPath);
    if (marker.open(QIODevice::WriteOnly | QIODevice::Append)) {
        marker.setFileTime(QDateTime::currentDateTimeUtc(), QFileDevice::FileModificationTime);
    }
}

void removeEmptyExtractSourceDirectory(const QString &extractDir)
{
    const QString sourceDirectory = QFileInfo(extractDir).absolutePath();
    if (isSourceCacheDirectoryName(QFileInfo(sourceDirectory).fileName())) {
        QDir().rmdir(sourceDirectory);
    }
}

// 返回调用方是否已请求取消当前加载。
bool cancellationRequested(const LottieCacheManager::CancellationCheck &isCancelled)
{
    return isCancelled && isCancelled();
}

QString temporaryStorageRoot()
{
    const QStringList candidates{
        QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation),
        QStandardPaths::writableLocation(QStandardPaths::TempLocation),
        QDir::tempPath()
    };
    for (const QString &candidate : candidates) {
        const QFileInfo info(candidate);
        if (!candidate.isEmpty() &&
            info.exists() &&
            info.isDir() &&
            !info.isSymLink() &&
            info.isWritable()) {
            const QString canonical = info.canonicalFilePath();
            if (!canonical.isEmpty()) {
                return canonical;
            }
        }
    }
    return {};
}

// 即使身份摘要相同，普通 JSON 和 dotLottie 也必须视为两种不同的加载任务。
enum class ParseFormat { Json, DotLottie };

// 用已验证身份摘要和文件格式生成加载任务键。
QString assetLoadKey(const QByteArray &digest, ParseFormat format)
{
    return QString::fromLatin1(digest.toHex()) + QLatin1Char(':') +
           (format == ParseFormat::DotLottie ? QStringLiteral("lottie") : QStringLiteral("json"));
}

struct BoundedSourceData
{
    QByteArray data;
    QByteArray contentDigest;
    bool budgetExceeded = false;
};

QByteArray normalizedRasterFormat(QByteArray format)
{
    format = format.toLower();
    if (format == QByteArrayLiteral("jpg")) {
        return QByteArrayLiteral("jpeg");
    }
    return format;
}

bool isAllowedRasterFormat(const QByteArray &format)
{
    const QByteArray normalized = normalizedRasterFormat(format);
    return normalized == QByteArrayLiteral("png") ||
        normalized == QByteArrayLiteral("jpeg") ||
        normalized == QByteArrayLiteral("webp") ||
        normalized == QByteArrayLiteral("gif") ||
        normalized == QByteArrayLiteral("bmp");
}

struct CheckedImageResource
{
    LottieBoundImageResource resource;
    QByteArray contentDigest;
    qint64 decodedPixels = 0;
};

bool checkImageResource(const QByteArray &bytes,
                        const QByteArray &declaredFormat,
                        CheckedImageResource *checked)
{
    if (!checked || bytes.isEmpty() ||
        bytes.size() > kMaxImageResourceBytes) {
        return false;
    }

    QBuffer buffer;
    buffer.setData(bytes);
    if (!buffer.open(QIODevice::ReadOnly)) {
        return false;
    }
    QImageReader reader(&buffer);
    reader.setDecideFormatFromContent(true);
    const QByteArray format = normalizedRasterFormat(reader.format());
    const QByteArray expectedFormat = normalizedRasterFormat(declaredFormat);
    if (!isAllowedRasterFormat(format) ||
        (!expectedFormat.isEmpty() && expectedFormat != format)) {
        return false;
    }

    const QSize size = reader.size();
    int frameCount = reader.imageCount();
    if (frameCount <= 0) {
        // 有些插件无法预先给出动画帧数。此时不能把未知数量的解码帧当成一帧
        // 计入预算；静态格式则按一帧处理。
        if (reader.supportsAnimation()) {
            return false;
        }
        frameCount = 1;
    }
    if (!size.isValid() ||
        size.width() > kMaxImageResourceDimension ||
        size.height() > kMaxImageResourceDimension) {
        return false;
    }
    const qint64 pixelsPerFrame = static_cast<qint64>(size.width()) * size.height();
    if (pixelsPerFrame <= 0 ||
        pixelsPerFrame > kMaxTotalImageResourcePixels / frameCount) {
        return false;
    }

    checked->resource.data = bytes;
    checked->resource.format = format;
    checked->resource.pixelSize = size;
    checked->resource.frameCount = frameCount;
    checked->contentDigest = QCryptographicHash::hash(bytes, QCryptographicHash::Sha256);
    checked->decodedPixels = pixelsPerFrame * frameCount;
    return checked->resource.isValid();
}

bool decodeEmbeddedImage(const QString &source, CheckedImageResource *checked)
{
    const int comma = source.indexOf(QLatin1Char(','));
    if (comma <= 0) {
        return false;
    }
    const QString header = source.left(comma);
    if (!header.startsWith(QStringLiteral("data:image/"), Qt::CaseInsensitive) ||
        !header.endsWith(QStringLiteral(";base64"), Qt::CaseInsensitive)) {
        return false;
    }

    const QString formatText = header.mid(QStringLiteral("data:image/").size(),
                                          header.size() -
                                              QStringLiteral("data:image/").size() -
                                              QStringLiteral(";base64").size());
    const QString payloadText = source.mid(comma + 1);
    const QByteArray payload = payloadText.toLatin1();
    if (payload.size() != payloadText.size() ||
        payload.size() > ((kMaxImageResourceBytes + 2) / 3) * 4) {
        return false;
    }
    const QByteArray bytes = QByteArray::fromBase64(
        payload, QByteArray::AbortOnBase64DecodingErrors);
    return checkImageResource(bytes, formatText.toLatin1(), checked);
}

// 普通 JSON 只读取一次：读取时执行大小和取消检查，同时生成内容版本摘要。
BoundedSourceData readBoundedSourceData(
    const QString &sourcePath,
    qint64 maxBytes,
    const LottieCacheManager::CancellationCheck &isCancelled)
{
    BoundedSourceData result;
    if (cancellationRequested(isCancelled)) {
        return result;
    }

    QFile source(sourcePath);
    if (!source.open(QIODevice::ReadOnly)) {
        return result;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    std::array<char, static_cast<size_t>(kResourceCopyChunkBytes)> buffer;
    while (true) {
        if (cancellationRequested(isCancelled)) {
            return {};
        }
        const qint64 bytesRead = source.read(buffer.data(), static_cast<qint64>(buffer.size()));
        if (bytesRead < 0) {
            return {};
        }
        if (bytesRead == 0) {
            break;
        }
        // 先拒绝再追加，内存中不会暂存超过 JSON 输入预算的数据。
        if (maxBytes >= 0 &&
            (result.data.size() > maxBytes - bytesRead)) {
            result.data.clear();
            result.budgetExceeded = true;
            return result;
        }
        result.data.append(buffer.data(), static_cast<int>(bytesRead));
        hash.addData(buffer.data(), bytesRead);
    }
    if (source.error() != QFileDevice::NoError ||
        result.data.isEmpty() ||
        cancellationRequested(isCancelled)) {
        result.data.clear();
        return result;
    }
    result.contentDigest = hash.result();
    return result;
}

// 分块复制动画文件，不把完整文件放进内存。qrc 调用方会在复制时计算内容
// SHA-256；本地文件只统计字节数。maxBytes 大于等于 0 时，越界前立即停止。
bool copyOpenFileToTemporaryArchive(QFile &source,
                                    const QString &temporaryPath,
                                    const LottieCacheManager::CancellationCheck &isCancelled = {},
                                    QCryptographicHash *contentHash = nullptr,
                                    qint64 *copiedBytes = nullptr,
                                    qint64 maxBytes = -1)
{
    if (copiedBytes) {
        *copiedBytes = 0;
    }
    QSaveFile output(temporaryPath);
    output.setDirectWriteFallback(false);
    if (!output.open(QIODevice::WriteOnly)) {
        return false;
    }

    qint64 totalBytes = 0;
    std::array<char, static_cast<size_t>(kResourceCopyChunkBytes)> buffer;
    while (true) {
        if (cancellationRequested(isCancelled)) {
            output.cancelWriting();
            return false;
        }
        const qint64 bytesRead = source.read(buffer.data(), static_cast<qint64>(buffer.size()));
        if (bytesRead < 0) {
            output.cancelWriting();
            return false;
        }
        if (bytesRead == 0) {
            break;
        }
        // 输入预算是“拒绝后不追加”：读到会越界的块时，既不写临时文件，
        // 也不把它加入摘要。
        if (maxBytes >= 0 && totalBytes > maxBytes - bytesRead) {
            if (copiedBytes) {
                *copiedBytes = maxBytes + 1;
            }
            output.cancelWriting();
            return false;
        }
        if (output.write(buffer.data(), bytesRead) != bytesRead) {
            output.cancelWriting();
            return false;
        }
        if (contentHash) {
            contentHash->addData(buffer.data(), bytesRead);
        }
        totalBytes += bytesRead;
        if (copiedBytes) {
            *copiedBytes = totalBytes;
        }
    }
    if (source.error() != QFileDevice::NoError || !output.commit()) {
        return false;
    }
    return QFile(temporaryPath).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
}

// 以可协作取消的方式获取解压缓存锁；取消或超时时返回 false。
bool acquireCacheLockWithCancellation(QLockFile &lockFile,
                                      const LottieCacheManager::CancellationCheck &isCancelled)
{
    QElapsedTimer timer;
    timer.start();
    while (true) {
        constexpr int kLockPollMs = 50;
        if (lockFile.tryLock(kLockPollMs)) {
            return true;
        }
        if (cancellationRequested(isCancelled) || timer.elapsed() >= kDiskCacheLockTimeoutMs) {
            return false;
        }
    }
}

} // namespace

namespace {

void appendBigEndian64(QByteArray &bytes, quint64 value)
{
    for (int shift = 56; shift >= 0; shift -= 8) {
        bytes.append(static_cast<char>((value >> shift) & 0xff));
    }
}

QByteArray localArchiveVersionDigest(const QString &canonicalPath,
                                     const SourceVersionStamp &version)
{
    if (canonicalPath.isEmpty()) {
        return {};
    }

    const QByteArray canonicalUri = QUrl::fromLocalFile(canonicalPath)
                                        .toEncoded(QUrl::FullyEncoded);
    QByteArray identity = QByteArrayLiteral("ukui-quick:lottie-source-version:1");
    appendBigEndian64(identity, static_cast<quint64>(canonicalUri.size()));
    identity.append(canonicalUri);
    appendBigEndian64(identity, version.device);
    appendBigEndian64(identity, version.inode);
    appendBigEndian64(identity, static_cast<quint64>(version.size));
    appendBigEndian64(identity, static_cast<quint64>(version.modifiedSeconds));
    appendBigEndian64(identity, static_cast<quint64>(version.modifiedNanoseconds));
    appendBigEndian64(identity, static_cast<quint64>(version.changedSeconds));
    appendBigEndian64(identity, static_cast<quint64>(version.changedNanoseconds));
    return QCryptographicHash::hash(identity, QCryptographicHash::Sha256);
}

QString archiveExtractDir(const QString &cacheRoot,
                          const QByteArray &canonicalUri,
                          const QString &revisionHex)
{
    if (canonicalUri.isEmpty() || !isSha256Name(revisionHex)) {
        return {};
    }
    const QString extractedRoot = cacheSubdirectory(cacheRoot,
                                                    kExtractedDirectory,
                                                    true);
    if (extractedRoot.isEmpty()) {
        return {};
    }

    const QString sourceHash = QString::fromLatin1(
        QCryptographicHash::hash(canonicalUri, QCryptographicHash::Sha256).toHex());
    const QString sourceRoot = QDir(extractedRoot).filePath(sourceHash);
    const QFileInfo sourceRootInfo(sourceRoot);
    if (sourceRootInfo.isSymLink()) {
        return {};
    }
    if (!sourceRootInfo.exists()) {
        // 只计算目标路径。真正需要解压时，parser 才创建 U 目录；磁盘命中和
        // 复制前取消都不会留下空的 source 目录。
        return QFileInfo(extractedRoot).isWritable()
            ? QDir(sourceRoot).filePath(revisionHex + kGenerationSuffix)
            : QString();
    }
    if (!sourceRootInfo.isDir() || !sourceRootInfo.isWritable()) {
        return {};
    }
    const QString canonicalSourceRoot = sourceRootInfo.canonicalFilePath();
    return canonicalSourceRoot.isEmpty()
        ? QString()
        : QDir(canonicalSourceRoot).filePath(revisionHex + kGenerationSuffix);
}

} // namespace

// 缓存已经解析完成的资产。实际加载由 RuntimeService 的唯一 asset worker
// 串行执行；相同来源的排队请求也已经在服务层合并，因此这里不再维护第二套
// “内容正在加载”等待状态。
class ParsedAssetCache
{
public:
    // 一条内存缓存。JSON/qrc 按内容身份索引，本地归档按 source 版本索引。
    struct CachedAssetData {
        // 已解析、可复用的动画数据。普通 JSON 直接持有内存字节，
        // dotLottie 则引用已验证的解压目录。
        LottieAsset asset;
        // 最近一次访问的顺序号。
        quint64 lastAccessTick = 0;
    };

    // 同一内容最近失败的记录。短暂冷却期内不重复加载，直接返回失败。
    struct FailedLoadData {
        // true 表示已经按精确版本区分文件，只需判断冷却时间是否结束。
        bool versionKeyed = false;
        qint64 sourceFileSize = -1;
        QDateTime sourceLastModified;
        QDateTime sourceLastMetadataChange;
        QDateTime retryAfter;
    };

    enum class AssetLoadStatus {
        Cached,
        Failed,
        NeedsLoading,
        Cancelled
    };

    // 尝试复用缓存；也可能发现近期失败、请求已取消，或需要执行加载。
    AssetLoadStatus acquireAssetLoad(const QString &cacheKey,
                                     QFileInfo &sourceInfo,
                                     bool isResourceSource,
                                     LottieAsset *cachedAsset,
                                     const LottieCacheManager::CancellationCheck &isCancelled = {});
    // 保存加载结果。取消既不写缓存，也不记为失败。
    void finishAssetLoad(const QString &cacheKey,
                         LottieAsset &loadedAsset,
                         bool cancelled = false);

    // 生成新的缓存访问序号，调用时需已持有互斥锁。
    quint64 nextCacheAccessTickLocked();
    // 更新资产缓存条目的访问时间。
    void touchAssetLocked(CachedAssetData &data);
    // 按数量和内联 JSON 字节数限制淘汰资产缓存。
    void trimAssetCacheLocked();
    // 清理过期和超出数量限制的失败加载记录。
    void trimFailedLoadsLocked();
    // 清理已经没有动画资产使用的磁盘缓存占用标记。
    void trimDiskCacheLeasesLocked();
    // 标记某个磁盘缓存正在被动画资产使用。调用时必须已持有互斥锁。
    bool attachDiskCacheLeaseLocked(
        LottieAsset &asset,
        const QSharedPointer<QTemporaryDir> &storageOwner = {});
    // 移除指定磁盘条目的内存缓存，并检查是否还有动画资产正在使用它。
    bool retireCachedAssetsUsingDiskPathLocked(const QString &diskPath);
    // 写入或更新资产缓存。
    void storeAssetLocked(const QString &cacheKey, LottieAsset &asset);
    // 检查缓存资产的磁盘产物是否仍可用。
    bool assetIsUsable(const CachedAssetData &data) const;
    // 检查失败记录是否仍适用且尚在冷却期。
    bool failedLoadIsCoolingDown(const FailedLoadData &data,
                                 const QFileInfo &sourceInfo,
                                 bool isResourceSource) const;

    // 保护所有内存缓存。测试可能直接检查缓存，生产加载由单 worker 串行执行。
    mutable QMutex mutex;
    // 按已验证身份索引的解析后动画资产缓存。
    QHash<QString, CachedAssetData> assetCache;
    // 已失败的资源版本及其下次可重试时间；键为身份摘要或源路径。
    QHash<QString, FailedLoadData> failedLoads;
    // 按磁盘路径记录“仍在使用”，但不反向持有动画资产。
    QHash<QString, QWeakPointer<LottieAssetCacheLease>> diskCacheLeases;

    // 递增的访问顺序号；数值越小表示越久没有使用。
    quint64 cacheAccessTick = 0;
};

class LottieCacheManagerPrivate
{
public:
    QString resourceCacheRoot()
    {
        if (!resourceCache) {
            const QString temporaryRoot = temporaryStorageRoot();
            if (temporaryRoot.isEmpty()) {
                return {};
            }
            resourceCache = QSharedPointer<QTemporaryDir>::create(
                QDir(temporaryRoot).filePath(
                    QStringLiteral("ukui-quick-lottie-qrc-XXXXXX")));
            resourceCache->setAutoRemove(true);
        }
        return resourceCache->isValid()
            ? resourceCache->path()
            : QString();
    }

    QSharedPointer<QTemporaryDir> resourceCache;
    ParsedAssetCache parsedAssets;
};

LottieCacheManager::LottieCacheManager()
    : d(new LottieCacheManagerPrivate)
{
}

LottieCacheManager::~LottieCacheManager() = default;

LottieCacheManager *LottieCacheManager::instance()
{
    static LottieCacheManager manager;
    return &manager;
}

LottieAssetRequestResult LottieCacheManager::requestAsset(
    const LottieSourceReference &source,
    const CancellationCheck &isCancelled)
{
    if (cancellationRequested(isCancelled)) {
        return {};
    }
    const LottieResolvedSource resolvedSource = LottieSourceResolver().resolve(source);
    if (!resolvedSource.isValid()) {
        return {};
    }
    const QString sourcePath = resolvedSource.path;

    const bool isResourceSource = sourcePath.startsWith(QLatin1Char(':'));
    QFileInfo sourceInfo(sourcePath);
    QFile resourceSource(sourcePath);
    if (!sourceInfo.exists()) {
        if (source.kind() != LottieSourceKind::MotionIcon) {
            qWarning() << "LottieCacheManager: animation source does not exist or is not a regular file:"
                       << sourcePath;
        }
        return {{}, true};
    }
    if ((!isResourceSource && (!sourceInfo.isFile() || sourceInfo.isSymLink())) ||
        (isResourceSource && !resourceSource.open(QIODevice::ReadOnly))) {
        qWarning() << "LottieCacheManager: animation source does not exist or is not a regular file:" << sourcePath;
        return {};
    }

    const QString pathKey = isResourceSource
        ? QDir::cleanPath(sourcePath)
        : sourceInfo.canonicalFilePath();
    const bool isArchive = resolvedSource.format == LottieSourceFormat::DotLottie;

    // 读取前先检查该路径是否刚加载失败。随后 JSON 按内容区分，本地归档按
    // URI + stat 版本区分，因此新版本不会被旧版本的失败记录误拦。
    {
        QMutexLocker locker(&d->parsedAssets.mutex);
        const auto failed = d->parsedAssets.failedLoads.constFind(pathKey);
        if (failed != d->parsedAssets.failedLoads.constEnd() &&
            d->parsedAssets.failedLoadIsCoolingDown(failed.value(), sourceInfo, isResourceSource)) {
            return {};
        }
    }

    if (isArchive) {
        return isResourceSource
            ? requestResourceArchive(sourcePath, sourceInfo, isCancelled)
            : requestLocalArchive(sourcePath, pathKey, sourceInfo, isCancelled);
    }

    // 普通 JSON 已经必须读入内存交给解析器，因此在这次读取中一并限额和
    // 计算摘要，不再额外写一份磁盘快照。
    const BoundedSourceData readResult = readBoundedSourceData(
        sourcePath,
        DotLottieParser::maxJsonInputBytes(),
        isCancelled);
    if (readResult.data.isEmpty() || readResult.contentDigest.isEmpty()) {
        if (!cancellationRequested(isCancelled)) {
            if (readResult.budgetExceeded) {
                qWarning() << "LottieCacheManager: source exceeds the input budget:" << sourcePath;
            } else {
                qWarning() << "LottieCacheManager: Failed to read immutable source data:" << sourcePath;
            }
            recordFailedLoad(pathKey, sourceInfo, false);
        }
        return {};
    }

    const QString cacheKey = assetLoadKey(readResult.contentDigest, ParseFormat::Json);
    LottieAsset cachedAsset;
    const auto loadStatus = d->parsedAssets.acquireAssetLoad(cacheKey,
                                                             sourceInfo,
                                                             isResourceSource,
                                                             &cachedAsset,
                                                             isCancelled);
    if (loadStatus == ParsedAssetCache::AssetLoadStatus::Cached) {
        // 即使找到缓存，也要再次检查取消；已经取消的请求不能收到动画资产。
        if (cancellationRequested(isCancelled)) {
            return {};
        }
        // 内容相同可以共享解析结果，但外部图片目录等信息仍按本次来源重新设置。
        return deliverBoundAsset(cachedAsset, sourcePath, isCancelled);
    }
    if (loadStatus != ParsedAssetCache::AssetLoadStatus::NeedsLoading) {
        return {};
    }

    LottieAsset loadedAsset = loadJsonAsset(sourcePath,
                                            readResult.data,
                                            readResult.contentDigest,
                                            isCancelled);
    // 被取消的加载不作为结果发布，也不写入失败冷却，以免拖累后续请求。
    const bool cancelled = cancellationRequested(isCancelled);
    if (!cancelled && !loadedAsset.isValid()) {
        recordFailedLoad(cacheKey, sourceInfo, true);
    }
    d->parsedAssets.finishAssetLoad(cacheKey, loadedAsset, cancelled);

    if (cancelled || !loadedAsset.isValid()) {
        return {};
    }
    return deliverBoundAsset(loadedAsset, sourcePath, isCancelled);
}

LottieAssetRequestResult LottieCacheManager::requestLocalArchive(
    const QString &sourcePath,
    const QString &canonicalPath,
    QFileInfo sourceInfo,
    const CancellationCheck &isCancelled)
{
    const QString cacheRoot = cacheRootDir();
    if (cacheRoot.isEmpty()) {
        qWarning() << "LottieCacheManager: Cannot prepare Lottie cache root";
        return {};
    }

    QLockFile cacheLock(QDir(cacheRoot).filePath(kDiskCacheLock));
    cacheLock.setStaleLockTime(kDiskCacheStaleLockMs);
    if (!acquireCacheLockWithCancellation(cacheLock, isCancelled)) {
        if (!cancellationRequested(isCancelled)) {
            qWarning() << "LottieCacheManager: Failed to lock Lottie cache:" << cacheRoot;
        }
        return {};
    }

    // 先用路径 stat 定位解压代际；已有代际不需要打开源文件。只有缓存未命中
    // 才打开并复制归档。复制期间若路径被替换或文件被改写，完整重试一次；
    // 连续两次都变化时安全失败，等待调用方稍后重试。
    constexpr int kMaximumSourceVersionAttempts = 2;
    for (int attempt = 0; attempt < kMaximumSourceVersionAttempts; ++attempt) {
        if (cancellationRequested(isCancelled)) {
            return {};
        }

        SourceVersionStamp observedVersion;
        if (!sourceVersionStamp(canonicalPath, &observedVersion)) {
            qWarning() << "LottieCacheManager: Failed to stat local dotLottie source:" << sourcePath;
            return {};
        }
        const QByteArray revisionDigest = localArchiveVersionDigest(
            canonicalPath, observedVersion);
        if (revisionDigest.isEmpty()) {
            return {};
        }
        const QString cacheKey = assetLoadKey(revisionDigest, ParseFormat::DotLottie);
        sourceInfo.setFile(canonicalPath);
        sourceInfo.refresh();
        if (observedVersion.size < 0 ||
            observedVersion.size > DotLottieParser::maxArchiveInputBytes()) {
            qWarning() << "LottieCacheManager: source exceeds the input budget:" << sourcePath;
            recordFailedLoad(cacheKey, sourceInfo, true);
            return {};
        }

        LottieAsset cachedAsset;
        const auto loadStatus = d->parsedAssets.acquireAssetLoad(cacheKey,
                                                                 sourceInfo,
                                                                 false,
                                                                 &cachedAsset,
                                                                 isCancelled);
        if (loadStatus == ParsedAssetCache::AssetLoadStatus::Cached) {
            if (cancellationRequested(isCancelled)) {
                return {};
            }
            SourceVersionStamp versionAtDelivery;
            if (!sourceVersionStamp(canonicalPath, &versionAtDelivery) ||
                !(versionAtDelivery == observedVersion)) {
                if (attempt + 1 < kMaximumSourceVersionAttempts) {
                    continue;
                }
                qWarning() << "LottieCacheManager: local dotLottie source changed while being observed:"
                           << sourcePath;
                return {};
            }
            return deliverBoundAsset(cachedAsset, sourcePath, isCancelled);
        }
        if (loadStatus != ParsedAssetCache::AssetLoadStatus::NeedsLoading) {
            return {};
        }

        const QString digestHex = QString::fromLatin1(revisionDigest.toHex());
        const QString extractDir = cacheExtractDirForRevision(canonicalPath, digestHex);
        if (extractDir.isEmpty()) {
            qWarning() << "LottieCacheManager: Cannot prepare Lottie extract directory";
            return {};
        }

        QString archiveInputPath;
        QTemporaryFile temporaryArchive;
        if (!extractGenerationIsPublished(extractDir)) {
            QFile source;
            SourceVersionStamp openedVersion;
            if (!openLocalSource(canonicalPath, &source, &openedVersion)) {
                qWarning() << "LottieCacheManager: Failed to open local dotLottie source:"
                           << sourcePath;
                return {};
            }
            if (!(openedVersion == observedVersion)) {
                if (attempt + 1 < kMaximumSourceVersionAttempts) {
                    continue;
                }
                qWarning() << "LottieCacheManager: local dotLottie source changed before reading:"
                           << sourcePath;
                return {};
            }

            const QString temporaryRoot = temporaryStorageRoot();
            if (temporaryRoot.isEmpty()) {
                return {};
            }
            temporaryArchive.setFileTemplate(
                QDir(temporaryRoot)
                    .filePath(QStringLiteral("ukui-quick-lottie-XXXXXX.lottie")));
            temporaryArchive.setAutoRemove(true);
            if (!temporaryArchive.open()) {
                qWarning() << "LottieCacheManager: Cannot create temporary archive input";
                return {};
            }
            archiveInputPath = temporaryArchive.fileName();
            temporaryArchive.close();

            qint64 copiedBytes = 0;
            if (!copyOpenFileToTemporaryArchive(source,
                                                archiveInputPath,
                                                isCancelled,
                                                nullptr,
                                                &copiedBytes,
                                                DotLottieParser::maxArchiveInputBytes())) {
                if (!cancellationRequested(isCancelled)) {
                    if (copiedBytes > DotLottieParser::maxArchiveInputBytes()) {
                        qWarning() << "LottieCacheManager: source exceeds the input budget:" << sourcePath;
                    } else {
                        qWarning() << "LottieCacheManager: Failed to copy local dotLottie source:" << sourcePath;
                    }
                }
                return {};
            }

            SourceVersionStamp versionAfterCopy;
            SourceVersionStamp pathVersionAfterCopy;
            if (!sourceVersionStamp(source, &versionAfterCopy) ||
                !(versionAfterCopy == observedVersion) ||
                !sourceVersionStamp(canonicalPath, &pathVersionAfterCopy) ||
                !(pathVersionAfterCopy == observedVersion)) {
                if (attempt + 1 < kMaximumSourceVersionAttempts) {
                    continue;
                }
                if (!cancellationRequested(isCancelled)) {
                    qWarning() << "LottieCacheManager: local dotLottie source changed while being read:"
                               << sourcePath;
                }
                return {};
            }
        }

        LottieAsset loadedAsset = loadDotLottieAsset(sourcePath,
                                                      archiveInputPath,
                                                      revisionDigest,
                                                      extractDir,
                                                      cacheRoot,
                                                      {},
                                                      isCancelled);
        const bool cancelled = cancellationRequested(isCancelled);
        SourceVersionStamp versionAtDelivery;
        if (!cancelled &&
            (!sourceVersionStamp(canonicalPath, &versionAtDelivery) ||
             !(versionAtDelivery == observedVersion))) {
            loadedAsset = LottieAsset();
            if (attempt + 1 < kMaximumSourceVersionAttempts) {
                continue;
            }
            qWarning() << "LottieCacheManager: local dotLottie source changed before delivery:"
                       << sourcePath;
            return {};
        }
        if (!cancelled && !loadedAsset.isValid()) {
            recordFailedLoad(cacheKey, sourceInfo, true);
        }
        d->parsedAssets.finishAssetLoad(cacheKey, loadedAsset, cancelled);
        if (cancelled || !loadedAsset.isValid()) {
            return {};
        }

        return deliverBoundAsset(loadedAsset, sourcePath, isCancelled);
    }

    return {};
}

LottieAssetRequestResult LottieCacheManager::requestResourceArchive(
    const QString &sourcePath,
    QFileInfo sourceInfo,
    const CancellationCheck &isCancelled)
{
    const QString privateCacheRoot = d->resourceCacheRoot();
    if (privateCacheRoot.isEmpty() || cancellationRequested(isCancelled)) {
        return {};
    }

    QFile source(sourcePath);
    if (!source.open(QIODevice::ReadOnly)) {
        return {};
    }

    QTemporaryFile temporaryArchive(
        QDir(privateCacheRoot).filePath(QStringLiteral(".archive-XXXXXX.lottie")));
    temporaryArchive.setAutoRemove(true);
    if (!temporaryArchive.open()) {
        return {};
    }
    const QString archiveInputPath = temporaryArchive.fileName();
    temporaryArchive.close();

    QCryptographicHash contentHash(QCryptographicHash::Sha256);
    qint64 copiedBytes = 0;
    if (!copyOpenFileToTemporaryArchive(source,
                                        archiveInputPath,
                                        isCancelled,
                                        &contentHash,
                                        &copiedBytes,
                                        DotLottieParser::maxArchiveInputBytes())) {
        if (!cancellationRequested(isCancelled)) {
            if (copiedBytes > DotLottieParser::maxArchiveInputBytes()) {
                qWarning() << "LottieCacheManager: source exceeds the input budget:" << sourcePath;
            } else {
                qWarning() << "LottieCacheManager: Failed to copy qrc dotLottie source:" << sourcePath;
            }
        }
        return {};
    }

    const QByteArray revisionDigest = contentHash.result();
    const QString cacheKey = assetLoadKey(revisionDigest, ParseFormat::DotLottie);
    LottieAsset cachedAsset;
    const auto loadStatus = d->parsedAssets.acquireAssetLoad(cacheKey,
                                                             sourceInfo,
                                                             true,
                                                             &cachedAsset,
                                                             isCancelled);
    if (loadStatus == ParsedAssetCache::AssetLoadStatus::Cached) {
        if (cancellationRequested(isCancelled)) {
            return {};
        }
        return deliverBoundAsset(cachedAsset, sourcePath, isCancelled);
    }
    if (loadStatus != ParsedAssetCache::AssetLoadStatus::NeedsLoading) {
        return {};
    }

    const QByteArray canonicalUri = QUrl(QStringLiteral("qrc") + sourcePath)
                                        .toEncoded(QUrl::FullyEncoded);
    const QString extractDir = archiveExtractDir(
        privateCacheRoot,
        canonicalUri,
        QString::fromLatin1(revisionDigest.toHex()));
    if (extractDir.isEmpty()) {
        return {};
    }

    LottieAsset loadedAsset = loadDotLottieAsset(sourcePath,
                                                  archiveInputPath,
                                                  revisionDigest,
                                                  extractDir,
                                                  privateCacheRoot,
                                                  d->resourceCache,
                                                  isCancelled);
    const bool cancelled = cancellationRequested(isCancelled);
    if (!cancelled && !loadedAsset.isValid()) {
        recordFailedLoad(cacheKey, sourceInfo, true);
    }
    d->parsedAssets.finishAssetLoad(cacheKey, loadedAsset, cancelled);
    if (cancelled || !loadedAsset.isValid()) {
        return {};
    }

    return deliverBoundAsset(loadedAsset, sourcePath, isCancelled);
}

LottieAsset LottieCacheManager::loadJsonAsset(const QString &sourcePath,
                                              const QByteArray &animationData,
                                              const QByteArray &contentDigest,
                                              const CancellationCheck &isCancelled)
{
    if (cancellationRequested(isCancelled)) {
        return {};
    }
    DotLottieParser parser;
    // 验证、后续栅格和内容版本都使用同一份不可变内存数据。
    LottieAsset asset = parser.createJsonAsset(sourcePath, animationData, isCancelled);
    if (!asset.isValid()) {
        return asset;
    }
    if (cancellationRequested(isCancelled)) {
        return {};
    }

    asset.setRevision(LottieAssetRevision::fromIdentityDigest(contentDigest));
    return asset;
}

LottieAsset LottieCacheManager::loadDotLottieAsset(const QString &sourcePath,
                                                    const QString &archiveInputPath,
                                                    const QByteArray &revisionDigest,
                                                    const QString &extractDir,
                                                    const QString &cacheRoot,
                                                    const QSharedPointer<QTemporaryDir> &storageOwner,
                                                    const CancellationCheck &isCancelled)
{
    if (cancellationRequested(isCancelled)) {
        return {};
    }

    if (extractDir.isEmpty()) {
        qWarning() << "LottieCacheManager: Cannot prepare Lottie extract directory";
        return {};
    }

    DotLottieParser parser;
    QString parseError;
    LottieAsset asset;
    if (extractGenerationIsPublished(extractDir)) {
        asset = parser.parseExtractedDirectory(sourcePath, extractDir, QString(), &parseError, isCancelled);
    } else {
        if (archiveInputPath.isEmpty()) {
            return {};
        }
        const QFileInfo extractInfo(extractDir);
        if (extractInfo.exists() &&
            (extractInfo.isSymLink() ||
             !removeExtractGeneration(extractInfo.absolutePath(), extractDir))) {
            qWarning() << "LottieCacheManager: Failed to remove incomplete Lottie extract directory:" << extractDir;
            return {};
        }
        // 读取归档目录、检查大小和解压都只操作本次请求的稳定临时副本。
        asset = parser.extractArchiveAndParse(sourcePath,
                                              archiveInputPath,
                                              extractDir,
                                              QString(),
                                              &parseError,
                                              isCancelled);
    }
    if (!asset.isValid()) {
        removeEmptyExtractSourceDirectory(extractDir);
        if (!cancellationRequested(isCancelled)) {
            qWarning() << "LottieCacheManager: Failed to prepare dotLottie asset:" << sourcePath
                       << "error:" << parseError;
        }
        return {};
    }
    if (cancellationRequested(isCancelled)) {
        return {};
    }

    asset.setRevision(LottieAssetRevision::fromIdentityDigest(revisionDigest));
    if (!attachDiskCacheLease(asset, storageOwner)) {
        return {};
    }
    touchExtractGeneration(extractDir);
    if (!cacheRoot.isEmpty() && !maintainDiskCache(cacheRoot, extractDir)) {
        qWarning() << "LottieCacheManager: Failed to maintain disk cache:" << cacheRoot;
    }
    return asset;
}

bool LottieCacheManager::attachDiskCacheLease(
    LottieAsset &asset,
    const QSharedPointer<QTemporaryDir> &storageOwner)
{
    QMutexLocker locker(&d->parsedAssets.mutex);
    return d->parsedAssets.attachDiskCacheLeaseLocked(asset, storageOwner);
}

void LottieCacheManager::recordFailedLoad(const QString &key,
                                          const QFileInfo &sourceInfo,
                                          bool versionKeyed)
{
    QMutexLocker locker(&d->parsedAssets.mutex);
    ParsedAssetCache::FailedLoadData failedLoad;
    failedLoad.versionKeyed = versionKeyed;
    failedLoad.sourceFileSize = sourceInfo.size();
    failedLoad.sourceLastModified = sourceInfo.lastModified();
    failedLoad.sourceLastMetadataChange = sourceInfo.metadataChangeTime();
    failedLoad.retryAfter = QDateTime::currentDateTimeUtc().addMSecs(kFailedLoadCooldownMs);
    d->parsedAssets.failedLoads.insert(key, failedLoad);
    d->parsedAssets.trimFailedLoadsLocked();
}

void LottieCacheManager::rebindAssetToSource(LottieAsset &asset, const QString &sourcePath) const
{
    const QFileInfo sourceInfo(sourcePath);
    if (asset.extractDir().isEmpty()) {
        // 普通 JSON 的外部图片仍从原动画文件所在目录查找。
        asset.setResourceDir(sourceInfo.absolutePath());
        asset.setResourceSearchDirs(QStringList{
            sourceInfo.absolutePath(),
            sourceInfo.absoluteDir().filePath(QStringLiteral("images")),
            sourceInfo.absoluteDir().filePath(QStringLiteral("i"))
        });
        return;
    }

    // dotLottie 内部资源来自解压目录，可以随内容共享；额外的外部图片目录仍按
    // 本次请求的原来源重新设置。
    const QDir extractDir(asset.extractDir());
    const QFileInfo animationInfo(asset.animationJsonPath());
    QStringList searchDirs{
        animationInfo.absolutePath(),
        asset.extractDir(),
        extractDir.filePath(QStringLiteral("images")),
        extractDir.filePath(QStringLiteral("i")),
        extractDir.filePath(QStringLiteral("a")),
        extractDir.filePath(QStringLiteral("animations"))
    };
    if (!sourcePath.startsWith(QLatin1Char(':')) &&
        sourceInfo.exists() &&
        sourceInfo.isFile() &&
        !sourceInfo.isSymLink()) {
        searchDirs << sourceInfo.absolutePath()
                   << sourceInfo.absoluteDir().filePath(QStringLiteral("images"))
                   << sourceInfo.absoluteDir().filePath(QStringLiteral("i"));
    }
    asset.setResourceSearchDirs(searchDirs);
}

LottieAssetRequestResult LottieCacheManager::deliverBoundAsset(
    const LottieAsset &parsedAsset,
    const QString &sourcePath,
    const CancellationCheck &isCancelled) const
{
    if (!parsedAsset.isValid() || cancellationRequested(isCancelled)) {
        return {};
    }

    LottieAsset asset = parsedAsset;
    return bindImageResources(asset, sourcePath, isCancelled)
        ? LottieAssetRequestResult{asset, false}
        : LottieAssetRequestResult{};
}

bool LottieCacheManager::bindImageResources(
    LottieAsset &asset,
    const QString &sourcePath,
    const CancellationCheck &isCancelled) const
{
    if (!asset.isValid() || !asset.revision().isValid() ||
        cancellationRequested(isCancelled)) {
        return false;
    }

    rebindAssetToSource(asset, sourcePath);
    asset.clearBoundImageResources();
    QStringList logicalPaths = asset.imageResourcePaths();
    logicalPaths.removeAll(QString());
    logicalPaths.removeDuplicates();
    if (asset.imageResourceDeclarationCount() > DotLottieParser::maxImageResources()) {
        return false;
    }
    if (logicalPaths.isEmpty()) {
        return true;
    }

    QHash<QString, int> fileNameCounts;
    for (const QString &logicalPath : std::as_const(logicalPaths)) {
        if (!logicalPath.startsWith(QStringLiteral("data:"), Qt::CaseInsensitive)) {
            ++fileNameCounts[QFileInfo(logicalPath).fileName()];
        }
    }

    struct RevisionResource
    {
        QString logicalPath;
        QList<CheckedImageResource> candidates;
    };
    QList<RevisionResource> revisionResources;
    qint64 totalBytes = 0;
    qint64 totalPixels = 0;
    for (const QString &logicalPath : std::as_const(logicalPaths)) {
        if (cancellationRequested(isCancelled)) {
            return false;
        }

        QList<CheckedImageResource> checkedCandidates;
        if (logicalPath.startsWith(QStringLiteral("data:"), Qt::CaseInsensitive)) {
            CheckedImageResource checked;
            if (!decodeEmbeddedImage(logicalPath, &checked)) {
                return false;
            }
            checkedCandidates.append(checked);
        } else {
            const QStringList candidates = asset.imagePathCandidates(logicalPath);
            if (candidates.isEmpty()) {
                return false;
            }
            for (const QString &candidate : candidates) {
                const BoundedSourceData imageData = readBoundedSourceData(
                    candidate,
                    kMaxImageResourceBytes,
                    isCancelled);
                if (cancellationRequested(isCancelled)) {
                    return false;
                }
                CheckedImageResource checked;
                if (!imageData.data.isEmpty() && !imageData.budgetExceeded &&
                    checkImageResource(imageData.data, {}, &checked)) {
                    checkedCandidates.append(checked);
                }
            }
            if (checkedCandidates.isEmpty()) {
                return false;
            }
        }

        const int referenceCount = asset.imageResourceReferenceCount(logicalPath);
        if (referenceCount <= 0) {
            return false;
        }
        QList<LottieBoundImageResource> resources;
        for (const CheckedImageResource &checked : std::as_const(checkedCandidates)) {
            if (checked.decodedPixels >
                kMaxTotalImageResourcePixels / referenceCount) {
                return false;
            }
            const qint64 decodedPixelCost = checked.decodedPixels * referenceCount;
            if (totalBytes > kMaxTotalImageResourceBytes -
                                 checked.resource.data.size() ||
                totalPixels > kMaxTotalImageResourcePixels -
                                  decodedPixelCost) {
                return false;
            }
            totalBytes += checked.resource.data.size();
            totalPixels += decodedPixelCost;
            resources.append(checked.resource);
        }
        asset.setBoundImageResources(logicalPath, resources);

        // ThorVG 的不同版本可能传回 u+p，也可能只传 p。只有文件名在本动画
        // 中唯一时才加这个短别名，避免两个目录中的同名图片互相覆盖。
        const QString fileName = QFileInfo(logicalPath).fileName();
        if (!logicalPath.startsWith(QStringLiteral("data:"), Qt::CaseInsensitive) &&
            fileName != logicalPath && fileNameCounts.value(fileName) == 1) {
            asset.setBoundImageResources(fileName, resources);
        }
        revisionResources.append({logicalPath, checkedCandidates});
    }
    if (cancellationRequested(isCancelled)) {
        return false;
    }

    std::sort(revisionResources.begin(), revisionResources.end(),
              [](const RevisionResource &left, const RevisionResource &right) {
                  return left.logicalPath < right.logicalPath;
              });
    QCryptographicHash revisionHash(QCryptographicHash::Sha256);
    revisionHash.addData(QByteArrayLiteral("ukui-quick:lottie-render-revision:2"));
    const auto addField = [&revisionHash](const QByteArray &field) {
        QByteArray sizeBytes;
        appendBigEndian64(sizeBytes, static_cast<quint64>(field.size()));
        revisionHash.addData(sizeBytes);
        revisionHash.addData(field);
    };
    addField(asset.revision().m_identityDigest);
    for (const RevisionResource &entry : std::as_const(revisionResources)) {
        const QByteArray logicalIdentity = entry.logicalPath.startsWith(
                                                QStringLiteral("data:"),
                                                Qt::CaseInsensitive)
            ? QByteArrayLiteral("embedded:") +
                QCryptographicHash::hash(entry.logicalPath.toUtf8(),
                                         QCryptographicHash::Sha256)
            : entry.logicalPath.toUtf8();
        addField(logicalIdentity);
        QByteArray candidateCount;
        appendBigEndian64(candidateCount,
                          static_cast<quint64>(entry.candidates.size()));
        addField(candidateCount);
        for (const CheckedImageResource &candidate : entry.candidates) {
            addField(candidate.contentDigest);
            addField(candidate.resource.format);
            QByteArray dimensions;
            appendBigEndian64(dimensions,
                              static_cast<quint64>(candidate.resource.pixelSize.width()));
            appendBigEndian64(dimensions,
                              static_cast<quint64>(candidate.resource.pixelSize.height()));
            appendBigEndian64(dimensions,
                              static_cast<quint64>(candidate.resource.frameCount));
            addField(dimensions);
        }
    }
    asset.setRevision(LottieAssetRevision::fromIdentityDigest(revisionHash.result()));
    return true;
}

QString LottieCacheManager::cacheRootDir() const
{
    QString genericCacheRoot = QStandardPaths::writableLocation(
        QStandardPaths::GenericCacheLocation);
    if (genericCacheRoot.isEmpty()) {
        return {};
    }
    // Qt 5 会原样返回相对的 XDG_CACHE_HOME。XDG 要求它必须是绝对路径；相对值
    // 不能落到进程工作目录，按规范改用 HOME 下的默认缓存目录。
    if (!QDir::isAbsolutePath(genericCacheRoot)) {
        const QString homePath = QDir::homePath();
        if (!QDir::isAbsolutePath(homePath)) {
            return {};
        }
        genericCacheRoot = QDir(homePath).filePath(QStringLiteral(".cache"));
    }
    // GenericCacheLocation 保持跨宿主应用共享，同时遵守 XDG_CACHE_HOME。
    const QString cacheRoot = QDir(genericCacheRoot).filePath(
        QStringLiteral("ukui-quick/lottie"));
    if (cacheRoot.isEmpty() || !QDir().mkpath(cacheRoot)) {
        return {};
    }
    const QFileInfo rootInfo(cacheRoot);
    if (!rootInfo.exists() ||
        !rootInfo.isDir() ||
        rootInfo.isSymLink() ||
        !rootInfo.isWritable()) {
        return {};
    }
    QFile(cacheRoot).setPermissions(QFileDevice::ReadOwner |
                                    QFileDevice::WriteOwner |
                                    QFileDevice::ExeOwner);
    return rootInfo.canonicalFilePath();
}

QString LottieCacheManager::cacheExtractDirForRevision(const QString &sourcePath,
                                                       const QString &revisionHex) const
{
    const QString root = cacheRootDir();
    if (root.isEmpty()) {
        return {};
    }
    const QFileInfo sourceInfo(sourcePath);
    if (sourceInfo.fileName().isEmpty() || !isSha256Name(revisionHex)) {
        return {};
    }

    // U 只区分规范化 source；R（参数中的摘要）区分该 source 的文件代际。
    // 目录名不再包含原文件名，避免同一身份存在第二套字符串规则。
    const QByteArray canonicalUri = sourcePath.startsWith(QLatin1Char(':'))
        ? QUrl(QStringLiteral("qrc") + sourcePath).toEncoded(QUrl::FullyEncoded)
        : QUrl::fromLocalFile(sourceInfo.canonicalFilePath())
              .toEncoded(QUrl::FullyEncoded);
    return archiveExtractDir(root, canonicalUri, revisionHex);
}

bool LottieCacheManager::maintainDiskCache(const QString &cacheRoot,
                                           const QString &protectedPath)
{
    const QFileInfo rootInfo(cacheRoot);
    if (!rootInfo.exists() || !rootInfo.isDir() || rootInfo.isSymLink()) {
        return false;
    }
    const QString canonicalRoot = rootInfo.canonicalFilePath();
    if (canonicalRoot.isEmpty()) {
        return false;
    }
    const QFileInfo protectedInfo(protectedPath);
    const QString cleanProtectedPath = protectedInfo.exists()
        ? protectedInfo.canonicalFilePath()
        : QDir::cleanPath(protectedInfo.absoluteFilePath());

    const auto removeTemporaryDirs = [](const QString &parentPath) {
        const QFileInfoList temporaryDirs = QDir(parentPath).entryInfoList(
            QStringList{QStringLiteral(".extract-*")},
            QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        for (const QFileInfo &temporaryDir : temporaryDirs) {
            QDir(temporaryDir.absoluteFilePath()).removeRecursively();
        }
    };

    QVector<DiskCacheEntryInfo> entries;
    qint64 totalBytes = 0;
    const auto appendGeneration = [&](const QFileInfo &generationInfo) {
        if (!isExtractGenerationName(generationInfo.fileName()) ||
            !extractGenerationIsPublished(generationInfo.absoluteFilePath())) {
            return;
        }

        const QString generationPath = generationInfo.canonicalFilePath();
        if (generationPath.isEmpty()) {
            return;
        }
        DiskCacheEntryInfo entry;
        entry.path = generationPath;
        entry.bytes = saturatedAdd(extractGenerationBytes(generationPath),
                                   kMinExtractEntryChargeBytes);
        entry.lastAccess = extractGenerationLastAccess(generationPath);
        totalBytes = saturatedAdd(totalBytes, entry.bytes);
        entries.append(entry);
    };

    const QString extractedRoot = cacheSubdirectory(canonicalRoot,
                                                    kExtractedDirectory,
                                                    false);
    if (!extractedRoot.isEmpty()) {
        const QFileInfoList sourceRoots = QDir(extractedRoot).entryInfoList(
            QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        for (const QFileInfo &sourceRootInfo : sourceRoots) {
            if (!isSourceCacheDirectoryName(sourceRootInfo.fileName())) {
                continue;
            }
            const QString sourceRoot = sourceRootInfo.canonicalFilePath();
            if (sourceRoot.isEmpty()) {
                continue;
            }
            removeTemporaryDirs(sourceRoot);
            const QFileInfoList generations = QDir(sourceRoot).entryInfoList(
                QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
            for (const QFileInfo &generation : generations) {
                appendGeneration(generation);
            }
            // 崩溃或取消可能只留下空的 U 目录；它不应绕过容量管理无限累积。
            QDir().rmdir(sourceRoot);
        }
    }

    std::sort(entries.begin(), entries.end(), [](const DiskCacheEntryInfo &left,
                                                  const DiskCacheEntryInfo &right) {
        if (left.lastAccess == right.lastAccess) {
            return left.path < right.path;
        }
        return left.lastAccess < right.lastAccess;
    });

    for (const DiskCacheEntryInfo &entry : std::as_const(entries)) {
        if (totalBytes <= kMaxDiskCacheBytes) {
            break;
        }
        if (entry.path == cleanProtectedPath) {
            continue;
        }
        const bool removed = removeExtractGeneration(
            QFileInfo(entry.path).absolutePath(), entry.path);
        if (removed) {
            totalBytes = entry.bytes >= totalBytes ? 0 : totalBytes - entry.bytes;
        }
    }
    return totalBytes <= kMaxDiskCacheBytes;
}

bool LottieCacheManager::removeExtractGeneration(const QString &generationParent,
                                                 const QString &extractDir)
{
    const QFileInfo rootInfo(generationParent);
    const QFileInfo generationInfo(extractDir);
    if (!rootInfo.exists() ||
        !rootInfo.isDir() ||
        rootInfo.isSymLink() ||
        !generationInfo.exists() ||
        !generationInfo.isDir() ||
        generationInfo.isSymLink() ||
        generationInfo.absolutePath() != rootInfo.canonicalFilePath() ||
        !isExtractGenerationName(generationInfo.fileName())) {
        return false;
    }

    if (!retireCachedAssetsUnderDirectory(extractDir)) {
        return false;
    }
    ExclusiveCacheLease crossProcessLease(extractDir);
    if (!crossProcessLease.canRemove()) {
        return false;
    }
    return QDir(extractDir).removeRecursively();
}

bool LottieCacheManager::retireCachedAssetsUnderDirectory(const QString &dirPath)
{
    const QString cleanDir = QDir::cleanPath(QFileInfo(dirPath).absoluteFilePath());
    QMutexLocker locker(&d->parsedAssets.mutex);
    return d->parsedAssets.retireCachedAssetsUsingDiskPathLocked(cleanDir);
}

ParsedAssetCache::AssetLoadStatus ParsedAssetCache::acquireAssetLoad(
    const QString &cacheKey,
    QFileInfo &sourceInfo,
    bool isResourceSource,
    LottieAsset *cachedAsset,
    const LottieCacheManager::CancellationCheck &isCancelled)
{
    QMutexLocker locker(&mutex);
    if (!isResourceSource) {
        sourceInfo.refresh();
    }

    auto cached = assetCache.find(cacheKey);
    if (cached != assetCache.end() && assetIsUsable(cached.value())) {
        // 找到缓存后仍要检查取消；取消时不交付资产，也不修改磁盘缓存。
        if (cancellationRequested(isCancelled)) {
            return AssetLoadStatus::Cancelled;
        }
        touchAssetLocked(cached.value());
        *cachedAsset = cached.value().asset;
        if (!cachedAsset->extractDir().isEmpty()) {
            touchExtractGeneration(cachedAsset->extractDir());
        }
        return AssetLoadStatus::Cached;
    }
    if (cached != assetCache.end()) {
        assetCache.erase(cached);
    }

    auto failed = failedLoads.find(cacheKey);
    if (failed != failedLoads.end()) {
        if (failedLoadIsCoolingDown(failed.value(), sourceInfo, isResourceSource)) {
            return AssetLoadStatus::Failed;
        }
        failedLoads.erase(failed);
    }

    return cancellationRequested(isCancelled)
        ? AssetLoadStatus::Cancelled
        : AssetLoadStatus::NeedsLoading;
}

void ParsedAssetCache::finishAssetLoad(
    const QString &cacheKey,
    LottieAsset &loadedAsset,
    bool cancelled)
{
    QMutexLocker locker(&mutex);
    // 取消不是加载失败：既不发布结果，也不记录失败冷却。
    // 真正的失败由调用方在结束这项加载前记录短暂冷却。
    if (cancelled) {
        loadedAsset = LottieAsset();
    } else if (loadedAsset.isValid()) {
        storeAssetLocked(cacheKey, loadedAsset);
        failedLoads.remove(cacheKey);
    }
}

bool ParsedAssetCache::attachDiskCacheLeaseLocked(
    LottieAsset &asset,
    const QSharedPointer<QTemporaryDir> &storageOwner)
{
    const QString diskPath = diskCachePathForAsset(asset);
    if (diskPath.isEmpty()) {
        return false;
    }

    QSharedPointer<LottieAssetCacheLease> lease = diskCacheLeases.value(diskPath).toStrongRef();
    if (lease.isNull()) {
        lease = QSharedPointer<LottieAssetCacheLease>::create(diskPath, storageOwner);
        if (!lease->isValid()) {
            return false;
        }
        diskCacheLeases.insert(diskPath, lease.toWeakRef());
    }
    asset.setDiskCacheLease(lease);
    return true;
}

bool ParsedAssetCache::retireCachedAssetsUsingDiskPathLocked(const QString &diskPath)
{
    QStringList assetKeys;
    for (auto it = assetCache.constBegin(); it != assetCache.constEnd(); ++it) {
        if (diskCachePathForAsset(it.value().asset) == diskPath) {
            assetKeys.append(it.key());
        }
    }
    for (const QString &key : std::as_const(assetKeys)) {
        assetCache.remove(key);
    }

    auto lease = diskCacheLeases.find(diskPath);
    if (lease == diskCacheLeases.end()) {
        return true;
    }
    const QSharedPointer<LottieAssetCacheLease> activeLease = lease.value().toStrongRef();
    if (!activeLease.isNull()) {
        return false;
    }
    diskCacheLeases.erase(lease);
    return true;
}

void ParsedAssetCache::trimDiskCacheLeasesLocked()
{
    for (auto it = diskCacheLeases.begin(); it != diskCacheLeases.end();) {
        if (it.value().toStrongRef().isNull()) {
            it = diskCacheLeases.erase(it);
        } else {
            ++it;
        }
    }
}

bool ParsedAssetCache::assetIsUsable(const CachedAssetData &data) const
{
    // 缓存已经按已验证身份索引；这里只需要确认内存字节或解压目录仍然完整。
    const LottieAsset &asset = data.asset;
    if (!asset.isValid() || !asset.revision().isValid()) {
        return false;
    }
    if (!asset.animationJsonData().isEmpty()) {
        return true;
    }
    if (asset.extractDir().isEmpty() ||
        !extractGenerationIsPublished(asset.extractDir())) {
        return false;
    }
    const QFileInfo animationInfo(asset.animationJsonPath());
    if (!animationInfo.exists() ||
        !animationInfo.isFile() ||
        animationInfo.isSymLink()) {
        return false;
    }
    const QString canonicalRoot = QFileInfo(asset.extractDir()).canonicalFilePath();
    const QString canonicalAnimation = animationInfo.canonicalFilePath();
    const QString prefix = canonicalRoot.endsWith(QLatin1Char('/'))
        ? canonicalRoot
        : canonicalRoot + QLatin1Char('/');
    if (canonicalRoot.isEmpty() || !canonicalAnimation.startsWith(prefix)) {
        return false;
    }
    return true;
}

bool ParsedAssetCache::failedLoadIsCoolingDown(const FailedLoadData &data,
                                               const QFileInfo &sourceInfo,
                                               bool isResourceSource) const
{
    if (data.retryAfter <= QDateTime::currentDateTimeUtc()) {
        return false;
    }
    // 失败已经按精确版本区分，因此这里只需要判断冷却时间是否结束。
    if (data.versionKeyed || isResourceSource) {
        return true;
    }
    return sourceInfo.exists() &&
           sourceInfo.isFile() &&
           !sourceInfo.isSymLink() &&
           data.sourceFileSize == sourceInfo.size() &&
           data.sourceLastModified == sourceInfo.lastModified() &&
           data.sourceLastMetadataChange == sourceInfo.metadataChangeTime();
}

quint64 ParsedAssetCache::nextCacheAccessTickLocked()
{
    return ++cacheAccessTick;
}

void ParsedAssetCache::touchAssetLocked(CachedAssetData &data)
{
    data.lastAccessTick = nextCacheAccessTickLocked();
}

void ParsedAssetCache::trimAssetCacheLocked()
{
    const auto inlineJsonBytes = [this] {
        qint64 total = 0;
        for (auto it = assetCache.constBegin(); it != assetCache.constEnd(); ++it) {
            total = saturatedAdd(total, it.value().asset.animationJsonData().size());
        }
        return total;
    };

    qint64 retainedInlineBytes = inlineJsonBytes();
    while (assetCache.size() > kMaxAssetCacheEntries ||
           retainedInlineBytes > kMaxInlineJsonAssetCacheBytes) {
        QString victimKey;
        bool foundVictim = false;
        quint64 oldestTick = std::numeric_limits<quint64>::max();
        for (auto it = assetCache.constBegin(); it != assetCache.constEnd(); ++it) {
            if (!foundVictim || it.value().lastAccessTick < oldestTick) {
                foundVictim = true;
                oldestTick = it.value().lastAccessTick;
                victimKey = it.key();
            }
        }
        if (!foundVictim) {
            break;
        }
        retainedInlineBytes = qMax<qint64>(
            0,
            retainedInlineBytes - assetCache.value(victimKey).asset.animationJsonData().size());
        assetCache.remove(victimKey);
    }
}

void ParsedAssetCache::trimFailedLoadsLocked()
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    for (auto it = failedLoads.begin(); it != failedLoads.end();) {
        if (it.value().retryAfter <= now) {
            it = failedLoads.erase(it);
        } else {
            ++it;
        }
    }

    while (failedLoads.size() > kMaxFailedLoadEntries) {
        auto victim = failedLoads.end();
        for (auto it = failedLoads.begin(); it != failedLoads.end(); ++it) {
            if (victim == failedLoads.end() || it.value().retryAfter < victim.value().retryAfter) {
                victim = it;
            }
        }
        if (victim == failedLoads.end()) {
            break;
        }
        failedLoads.erase(victim);
    }
}

void ParsedAssetCache::storeAssetLocked(const QString &cacheKey, LottieAsset &asset)
{
    trimDiskCacheLeasesLocked();
    attachDiskCacheLeaseLocked(asset);
    CachedAssetData data;
    data.asset = asset;
    data.lastAccessTick = nextCacheAccessTickLocked();
    assetCache.insert(cacheKey, data);
    trimAssetCacheLocked();
}

} // namespace UkuiQuick
