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

#include "lottie-asset.h"

#include <QDir>
#include <QBitArray>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QSharedData>
#include <QSet>
#include <QTemporaryDir>
#include <QAtomicInt>
#include <QWeakPointer>
#include <utility>

#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

namespace UkuiQuick {

namespace {

// 校验图片资源引用只能是受限深度的相对路径。
bool safeRelativeResourcePath(const QString &path, QString *relativePath)
{
    if (path.isEmpty() ||
        path.size() > 1024 ||
        path.contains(QChar::Null) ||
        path.contains(QLatin1Char('\\')) ||
        path.contains(QLatin1Char(':')) ||
        QDir::isAbsolutePath(path)) {
        return false;
    }

    const QString normalized = QDir::cleanPath(path);
    const QStringList components = normalized.split(QLatin1Char('/'), Qt::KeepEmptyParts);
    if (normalized == QStringLiteral(".") || components.size() > 16) {
        return false;
    }
    for (const QString &component : components) {
        if (component.isEmpty() || component == QStringLiteral("..")) {
            return false;
        }
    }
    *relativePath = normalized;
    return true;
}

// 返回位于可信资源根目录内的常规文件，拒绝符号链接和越界路径。
QString safeResourceFile(const QString &rootPath, const QString &candidatePath)
{
    if (rootPath.startsWith(QLatin1Char(':'))) {
        const QFileInfo candidateInfo(candidatePath);
        return candidateInfo.exists() && candidateInfo.isFile() ? QDir::cleanPath(candidatePath) : QString();
    }

    const QFileInfo rootInfo(rootPath);
    const QFileInfo candidateInfo(candidatePath);
    if (!rootInfo.exists() ||
        !rootInfo.isDir() ||
        rootInfo.isSymLink() ||
        !candidateInfo.exists() ||
        !candidateInfo.isFile() ||
        candidateInfo.isSymLink()) {
        return {};
    }

    const QString canonicalRoot = rootInfo.canonicalFilePath();
    const QString canonicalCandidate = candidateInfo.canonicalFilePath();
    const QString prefix = canonicalRoot.endsWith(QLatin1Char('/'))
        ? canonicalRoot
        : canonicalRoot + QLatin1Char('/');
    return !canonicalRoot.isEmpty() && canonicalCandidate.startsWith(prefix)
        ? canonicalCandidate
        : QString();
}

}

LottieAssetCacheLease::LottieAssetCacheLease(
    const QString &extractDir,
    const QSharedPointer<QTemporaryDir> &storageOwner)
    : m_storageOwner(storageOwner)
{
    const QString leasePath = QDir(extractDir).filePath(QStringLiteral(".lease"));
    m_fileDescriptor = ::open(QFile::encodeName(leasePath).constData(),
                              O_CREAT | O_RDWR | O_CLOEXEC | O_NOFOLLOW,
                              S_IRUSR | S_IWUSR);
    if (m_fileDescriptor < 0) {
        return;
    }

    struct stat info = {};
    if (::fstat(m_fileDescriptor, &info) != 0 ||
        !S_ISREG(info.st_mode) ||
        ::flock(m_fileDescriptor, LOCK_SH) != 0) {
        ::close(m_fileDescriptor);
        m_fileDescriptor = -1;
    }
}

LottieAssetCacheLease::~LottieAssetCacheLease()
{
    if (m_fileDescriptor >= 0) {
        ::flock(m_fileDescriptor, LOCK_UN);
        ::close(m_fileDescriptor);
    }
}

bool LottieAssetCacheLease::isValid() const
{
    return m_fileDescriptor >= 0;
}

class LottieThemeFailureState
{
public:
    static QSharedPointer<LottieThemeFailureState> acquire(
        const QByteArray &revisionDigest, int themeCount)
    {
        QMutexLocker locker(&registryMutex);
        const auto existing = registry.value(revisionDigest).toStrongRef();
        if (existing) {
            return existing;
        }

        auto state = QSharedPointer<LottieThemeFailureState>(
            new LottieThemeFailureState(revisionDigest, themeCount));
        registry.insert(revisionDigest, state.toWeakRef());
        return state;
    }

    LottieThemeFailureState(const QByteArray &revisionDigest, int themeCount)
        : revisionDigest(revisionDigest)
        , failedThemes(qBound(0, themeCount, 256))
    {
    }

    ~LottieThemeFailureState()
    {
        QMutexLocker locker(&registryMutex);
        const auto it = registry.find(revisionDigest);
        if (it != registry.end() && it.value().isNull()) {
            registry.erase(it);
        }
    }

    bool contains(int ordinal) const
    {
        QMutexLocker locker(&mutex);
        return ordinal >= 0 && ordinal < failedThemes.size() &&
            failedThemes.testBit(ordinal);
    }

    void insert(int ordinal)
    {
        QMutexLocker locker(&mutex);
        if (ordinal >= 0 && ordinal < failedThemes.size()) {
            failedThemes.setBit(ordinal);
        }
    }

    int capacity() const
    {
        return failedThemes.size();
    }

    static QMutex registryMutex;
    static QHash<QByteArray, QWeakPointer<LottieThemeFailureState>> registry;

private:
    QByteArray revisionDigest;
    mutable QMutex mutex;
    QBitArray failedThemes;
};

QMutex LottieThemeFailureState::registryMutex;
QHash<QByteArray, QWeakPointer<LottieThemeFailureState>>
    LottieThemeFailureState::registry;

class LottieAssetPrivate : public QSharedData
{
public:
    QString m_animationJsonPath;
    QByteArray m_animationJsonData;
    QString m_resourceDir;
    QString m_extractDir;
    QString m_theme;
    QString m_manifestInitialThemeId;
    QString m_animationInitialThemeId;
    QStringList m_availableThemes;
    QString m_activeAnimationId;
    QStringList m_activeAnimationThemes;
    bool m_activeAnimationRestrictsThemes = false;
    QStringList imageResourcePaths;
    QHash<QString, int> imageResourceReferenceCounts;
    int imageResourceDeclarationCount = 0;
    QStringList resourceSearchDirs;
    QHash<QString, QList<LottieBoundImageResource>> boundImageResources;
    QHash<QString, QByteArray> themeSlotJsonCache;
    QSharedPointer<LottieAssetCacheLease> diskCacheLease;
    QSharedPointer<LottieThemeFailureState> themeFailureState;
    LottieAssetRevision revision;
    int frameCount = 0;
    qreal frameRate = 0.0;
};

bool LottieAssetRevision::isValid() const
{
    return !m_identityDigest.isEmpty();
}

bool LottieAssetRevision::operator==(const LottieAssetRevision &other) const
{
    return m_identityDigest == other.m_identityDigest;
}

bool LottieAssetRevision::operator!=(const LottieAssetRevision &other) const
{
    return m_identityDigest != other.m_identityDigest;
}

LottieAssetRevision LottieAssetRevision::fromIdentityDigest(const QByteArray &identityDigest)
{
    LottieAssetRevision revision;
    revision.m_identityDigest = identityDigest;
    return revision;
}

LottieAsset::LottieAsset()
    : d(new LottieAssetPrivate)
{
}

LottieAsset::LottieAsset(const LottieAsset &other) = default;

LottieAsset &LottieAsset::operator=(const LottieAsset &other) = default;

LottieAsset::~LottieAsset() = default;

void LottieAsset::setDiskCacheLease(const QSharedPointer<LottieAssetCacheLease> &lease)
{
    d->diskCacheLease = lease;
}

bool LottieAsset::isValid() const
{
    return !d->m_animationJsonPath.isEmpty() || !d->m_animationJsonData.isEmpty();
}

LottieAssetRevision LottieAsset::revision() const
{
    return d->revision;
}

void LottieAsset::setRevision(const LottieAssetRevision &revision)
{
    d->revision = revision;
    d->themeFailureState = revision.isValid()
        ? LottieThemeFailureState::acquire(
              revision.m_identityDigest, availableThemes().size())
        : QSharedPointer<LottieThemeFailureState>();
}

int LottieAsset::frameCount() const
{
    return d->frameCount;
}

void LottieAsset::setFrameCount(int frameCount)
{
    d->frameCount = frameCount;
}

qreal LottieAsset::frameRate() const
{
    return d->frameRate;
}

void LottieAsset::setFrameRate(qreal frameRate)
{
    d->frameRate = frameRate;
}

QString LottieAsset::animationJsonPath() const
{
    return d->m_animationJsonPath;
}

void LottieAsset::setAnimationJsonPath(const QString &animationJsonPath)
{
    d->m_animationJsonPath = animationJsonPath;
}

QByteArray LottieAsset::animationJsonData() const
{
    return d->m_animationJsonData;
}

void LottieAsset::setAnimationJsonData(const QByteArray &animationJsonData)
{
    d->m_animationJsonData = animationJsonData;
}

QString LottieAsset::resourceDir() const
{
    return d->m_resourceDir;
}

void LottieAsset::setResourceDir(const QString &resourceDir)
{
    d->m_resourceDir = resourceDir;
    addResourceSearchDir(resourceDir);
}

QString LottieAsset::extractDir() const
{
    return d->m_extractDir;
}

void LottieAsset::setExtractDir(const QString &extractDir)
{
    d->m_extractDir = extractDir;
}

QString LottieAsset::theme() const
{
    return d->m_theme;
}

void LottieAsset::setTheme(const QString &theme)
{
    d->m_theme = theme;
}

QString LottieAsset::manifestInitialThemeId() const
{
    return d->m_manifestInitialThemeId;
}

void LottieAsset::setManifestInitialThemeId(const QString &themeId)
{
    d->m_manifestInitialThemeId = themeId;
}

QString LottieAsset::animationInitialThemeId() const
{
    return d->m_animationInitialThemeId;
}

void LottieAsset::setAnimationInitialThemeId(const QString &themeId)
{
    d->m_animationInitialThemeId = themeId;
}

QStringList LottieAsset::availableThemes() const
{
    if (!d->m_activeAnimationRestrictsThemes) {
        return d->m_availableThemes;
    }

    QStringList selectableThemes;
    for (const QString &theme : d->m_availableThemes) {
        if (d->m_activeAnimationThemes.contains(theme)) {
            selectableThemes.append(theme);
        }
    }
    return selectableThemes;
}

void LottieAsset::setAvailableThemes(const QStringList &themes)
{
    d->m_availableThemes = themes;
    d->m_availableThemes.removeAll(QString());
    d->m_availableThemes.removeDuplicates();
}

QString LottieAsset::activeAnimationId() const
{
    return d->m_activeAnimationId;
}

void LottieAsset::setActiveAnimationId(const QString &animationId)
{
    d->m_activeAnimationId = animationId;
}

QStringList LottieAsset::activeAnimationThemes() const
{
    return d->m_activeAnimationThemes;
}

void LottieAsset::setActiveAnimationThemes(const QStringList &themes)
{
    d->m_activeAnimationThemes = themes;
    d->m_activeAnimationThemes.removeAll(QString());
    d->m_activeAnimationThemes.removeDuplicates();
}

bool LottieAsset::activeAnimationRestrictsThemes() const
{
    return d->m_activeAnimationRestrictsThemes;
}

void LottieAsset::setActiveAnimationRestrictsThemes(bool restricts)
{
    d->m_activeAnimationRestrictsThemes = restricts;
}

QStringList LottieAsset::imageResourcePaths() const
{
    return d->imageResourcePaths;
}

void LottieAsset::setImageResourcePaths(const QStringList &paths)
{
    d->imageResourcePaths.clear();
    d->imageResourceReferenceCounts.clear();
    d->imageResourceDeclarationCount = 0;
    for (const QString &path : paths) {
        addImageResourcePath(path);
    }
}

void LottieAsset::addImageResourcePath(const QString &path)
{
    if (path.isEmpty()) {
        return;
    }
    ++d->imageResourceDeclarationCount;
    ++d->imageResourceReferenceCounts[path];
    if (!d->imageResourcePaths.contains(path)) {
        d->imageResourcePaths.append(path);
    }
}

int LottieAsset::imageResourceReferenceCount(const QString &path) const
{
    return d->imageResourceReferenceCounts.value(path);
}

int LottieAsset::imageResourceDeclarationCount() const
{
    return d->imageResourceDeclarationCount;
}

QStringList LottieAsset::resourceSearchDirs() const
{
    return d->resourceSearchDirs;
}

void LottieAsset::setResourceSearchDirs(const QStringList &dirs)
{
    d->resourceSearchDirs = dirs;
    d->resourceSearchDirs.removeDuplicates();
}

void LottieAsset::addResourceSearchDir(const QString &dir)
{
    if (!dir.isEmpty() && !d->resourceSearchDirs.contains(dir)) {
        d->resourceSearchDirs.append(dir);
    }
}

void LottieAsset::setThemeSlotJson(const QString &themeId, const QByteArray &slotJson)
{
    if (!themeId.isEmpty()) {
        d->themeSlotJsonCache.insert(themeId, slotJson);
    }
}

QByteArray LottieAsset::themeSlotJson(const QString &themeId) const
{
    return d->themeSlotJsonCache.value(themeId);
}

bool LottieAsset::canUseTheme(const QString &themeId) const
{
    if (themeId.isEmpty()) {
        return true;
    }

    bool found = false;
    for (const QString &theme : d->m_availableThemes) {
        if (theme == themeId) {
            found = true;
            break;
        }
    }
    if (!found) {
        return false;
    }

    if (d->m_activeAnimationRestrictsThemes && !d->m_activeAnimationThemes.contains(themeId)) {
        return false;
    }
    return true;
}

QString LottieAsset::preferredTheme(const QString &requestedTheme) const
{
    if (!requestedTheme.isEmpty() && canUseTheme(requestedTheme)) {
        return requestedTheme;
    }
    if (!d->m_manifestInitialThemeId.isEmpty() && canUseTheme(d->m_manifestInitialThemeId)) {
        return d->m_manifestInitialThemeId;
    }
    if (!d->m_animationInitialThemeId.isEmpty() && canUseTheme(d->m_animationInitialThemeId)) {
        return d->m_animationInitialThemeId;
    }
    for (const QString &theme : d->m_availableThemes) {
        if (canUseTheme(theme)) {
            return theme;
        }
    }
    return {};
}

QStringList LottieAsset::imagePathCandidates(const QString &sourcePath) const
{
    QStringList candidates;
    QString relativePath;
    if (!safeRelativeResourcePath(sourcePath, &relativePath)) {
        return candidates;
    }

    const QFileInfo sourceInfo(relativePath);
    const QString resourceFileName = sourceInfo.fileName();

    QStringList searchDirs = d->resourceSearchDirs;
    if (!d->m_resourceDir.isEmpty()) {
        searchDirs.prepend(d->m_resourceDir);
    }
    searchDirs.removeDuplicates();

    for (const QString &dirPath : std::as_const(searchDirs)) {
        if (dirPath.isEmpty()) {
            continue;
        }
        const QDir resourceDir(dirPath);
        const QStringList relativeCandidates = {
            relativePath,
            resourceFileName,
            QStringLiteral("images/") + resourceFileName,
            QStringLiteral("i/") + resourceFileName
        };
        for (const QString &candidatePath : relativeCandidates) {
            const QString safePath = safeResourceFile(dirPath, resourceDir.filePath(candidatePath));
            if (!safePath.isEmpty()) {
                candidates << safePath;
            }
        }
    }

    for (const QString &path : d->imageResourcePaths) {
        const QFileInfo info(path);
        if (info.fileName() == resourceFileName || QDir::cleanPath(path).endsWith(relativePath)) {
            for (const QString &dirPath : std::as_const(searchDirs)) {
                const QString safePath = safeResourceFile(dirPath, path);
                if (!safePath.isEmpty()) {
                    candidates << safePath;
                    break;
                }
            }
        }
    }

    candidates.removeDuplicates();
    return candidates;
}

LottieBoundImageResource LottieAsset::boundImageResource(const QString &sourcePath) const
{
    const QList<LottieBoundImageResource> candidates =
        boundImageResourceCandidates(sourcePath);
    return candidates.isEmpty() ? LottieBoundImageResource()
                                : candidates.constFirst();
}

QList<LottieBoundImageResource> LottieAsset::boundImageResourceCandidates(
    const QString &sourcePath) const
{
    const auto exact = d->boundImageResources.constFind(sourcePath);
    if (exact != d->boundImageResources.constEnd()) {
        return exact.value();
    }

    // ThorVG 有时会把资源根前缀一并传给 resolver。只接受以完整逻辑名结尾
    // 的最长匹配，不能只按文件名随意命中另一张图片。
    QString matchedName;
    for (auto it = d->boundImageResources.constBegin();
         it != d->boundImageResources.constEnd(); ++it) {
        if (sourcePath.endsWith(QLatin1Char('/') + it.key()) &&
            it.key().size() > matchedName.size()) {
            matchedName = it.key();
        }
    }
    return matchedName.isEmpty()
        ? QList<LottieBoundImageResource>()
        : d->boundImageResources.value(matchedName);
}

qint64 LottieAsset::boundImageResourceBytes() const
{
    qint64 total = 0;
    QSet<const void *> seenData;
    for (auto it = d->boundImageResources.constBegin();
         it != d->boundImageResources.constEnd(); ++it) {
        for (const LottieBoundImageResource &resource : it.value()) {
            const void *identity = resource.data.constData();
            if (!seenData.contains(identity)) {
                seenData.insert(identity);
                total += resource.data.size();
            }
        }
    }
    return total;
}

void LottieAsset::clearBoundImageResources()
{
    d->boundImageResources.clear();
}

void LottieAsset::setBoundImageResources(
    const QString &sourcePath,
    const QList<LottieBoundImageResource> &resources)
{
    if (sourcePath.isEmpty() || resources.isEmpty()) {
        return;
    }
    for (const LottieBoundImageResource &resource : resources) {
        if (!resource.isValid()) {
            return;
        }
    }
    d->boundImageResources.insert(sourcePath, resources);
}

int LottieAsset::selectableThemeOrdinal(const QString &themeId) const
{
    if (!canUseTheme(themeId)) {
        return -1;
    }
    return availableThemes().indexOf(themeId);
}

bool LottieAsset::hasDeterministicThemeFailure(int themeOrdinal) const
{
    return d->themeFailureState && d->themeFailureState->contains(themeOrdinal);
}

void LottieAsset::rememberDeterministicThemeFailure(int themeOrdinal) const
{
    if (d->themeFailureState) {
        d->themeFailureState->insert(themeOrdinal);
    }
}

int LottieAsset::themeFailureStateCapacity() const
{
    return d->themeFailureState ? d->themeFailureState->capacity() : 0;
}

} // namespace UkuiQuick
