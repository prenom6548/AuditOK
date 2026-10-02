/*
 *
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

#include "bookmarks-manager.h"

#include <QDomDocument>
#include <QDomNamedNodeMap>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QReadWriteLock>
#include <QSaveFile>
#include <QSet>
#include <QThread>
#include <QUrl>

namespace UkuiFileMetadata {

static const QString KDE_OWNER_HOST = QStringLiteral("kde.org");
static const QString FREEDESKTOP_OWNER_HOST = QStringLiteral("freedesktop.org");

static QString normalizeOwnerHost(const QString &host)
{
    QString normalized = host.trimmed().toLower();
    if (normalized.startsWith(QLatin1String("www."))) {
        normalized = normalized.mid(4);
    }
    return normalized;
}

static QString normalizedOwnerHost(const QString &owner)
{
    const QString trimmed = owner.trimmed();
    if (trimmed.isEmpty()) {
        return QString();
    }

    const QUrl url(trimmed);
    if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
        return normalizeOwnerHost(url.host());
    }

    QString fallback = trimmed;
    if (fallback.startsWith(QLatin1String("http://"), Qt::CaseInsensitive)) {
        fallback = fallback.mid(7);
    } else if (fallback.startsWith(QLatin1String("https://"), Qt::CaseInsensitive)) {
        fallback = fallback.mid(8);
    }

    int pathStart = fallback.indexOf(QLatin1Char('/'));
    const int queryStart = fallback.indexOf(QLatin1Char('?'));
    const int fragmentStart = fallback.indexOf(QLatin1Char('#'));
    if (queryStart >= 0 && (pathStart < 0 || queryStart < pathStart)) {
        pathStart = queryStart;
    }
    if (fragmentStart >= 0 && (pathStart < 0 || fragmentStart < pathStart)) {
        pathStart = fragmentStart;
    }
    if (pathStart >= 0) {
        fallback.truncate(pathStart);
    }

    const int portSeparator = fallback.indexOf(QLatin1Char(':'));
    if (portSeparator >= 0) {
        fallback.truncate(portSeparator);
    }

    return normalizeOwnerHost(fallback);
}

static bool ownerMatchesHost(const QString &owner, const QString &host)
{
    return normalizedOwnerHost(owner) == host;
}

static QString elementName(const QDomElement &element)
{
    QString name = element.localName();
    if (!name.isEmpty()) {
        return name;
    }

    name = element.tagName();
    const int colon = name.indexOf(QLatin1Char(':'));
    return colon < 0 ? name : name.mid(colon + 1);
}

static bool isStandardAttributeForElement(const QString &elementKind, const QString &attributeName)
{
    if (elementKind == QLatin1String("xbel")) {
        static const QSet<QString> xbelAttrs = {
            QStringLiteral("version"), QStringLiteral("id"),
            QStringLiteral("added"), QStringLiteral("dbusName")
        };
        return xbelAttrs.contains(attributeName);
    }
    if (elementKind == QLatin1String("folder")) {
        static const QSet<QString> folderAttrs = {
            QStringLiteral("id"), QStringLiteral("added"),
            QStringLiteral("modified"), QStringLiteral("visited"),
            QStringLiteral("folded"), QStringLiteral("toolbar")
        };
        return folderAttrs.contains(attributeName);
    }
    if (elementKind == QLatin1String("bookmark")) {
        static const QSet<QString> bookmarkAttrs = {
            QStringLiteral("id"), QStringLiteral("added"),
            QStringLiteral("modified"), QStringLiteral("visited"),
            QStringLiteral("href"), QStringLiteral("showintoolbar")
        };
        return bookmarkAttrs.contains(attributeName);
    }
    if (elementKind == QLatin1String("alias")) {
        return attributeName == QLatin1String("ref");
    }
    return false;
}

static bool parseFreedesktopBooleanValue(const QString &value, bool &result)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QLatin1String("true") || normalized == QLatin1String("yes") || normalized == QLatin1String("1")) {
        result = true;
        return true;
    }
    if (normalized == QLatin1String("false") || normalized == QLatin1String("no") || normalized == QLatin1String("0")) {
        result = false;
        return true;
    }
    return false;
}

class BookMarksTextCollector
{
public:
    void reset();
    void appendRawTextLine(const QString &value);
    void appendLabeledTextLine(const QString &label, const QString &value);
    void appendCustomMetaLine(const QString &owner, const QString &href, const QString &nodePath, const QString &path, const QString &value);
    void appendExtendedAttributeLine(const QString &elementKind, const QString &attributeName, const QString &value);
    QStringList textLines(const QString &dbusName) const;
    QStringList customMetaLines() const;
    QStringList extendedAttributeLines() const;

private:
    QStringList m_textLines;
    QStringList m_customMetaLines;
    QStringList m_extendedAttributeLines;
    QSet<QString> m_seenCustomMetaKeys;
    QSet<QString> m_seenExtendedAttributeLines;
};

class BookMarksManagerPrivate
{
public:
    explicit BookMarksManagerPrivate(BookMarksManager *q);

    bool isValid() const;
    QString filePath() const;
    void setFilePath(const QString &filePath);
    bool save();
    bool saveAs(const QString &filePath);
    QString rootTitle() const;
    QString dbusName() const;
    BookMarksGroup rootGroup() const;
    QStringList textLines() const;
    QStringList customMetaLines() const;
    QStringList extendedAttributeLines() const;
    QHash<QString, QStringList> freedesktopGroups() const;
    QStringList freedesktopGroups(const QString &href) const;
    QList<FreedesktopApplicationInfo> freedesktopApplications() const;
    QList<FreedesktopApplicationInfo> freedesktopApplications(const QString &href) const;
    QString lastError() const;
    int errorLine() const;
    int errorColumn() const;

private:
    // Watcher
    void initializeFileWatcher();
    void updateWatchedFile(const QString &filePath);
    void handleWatchedFileChanged(const QString &changedPath);
    void handleWatchedDirectoryChanged(const QString &changedPath);
    bool sendError(const QString &message, int line = -1, int column = -1);
    void emitParseError();
    void emitChanged();

    // Parser
    bool parse(bool &contentChanged);
    bool parseFile(const QString &filePath);
    void reset();
    void parseXbelAttributes(const QDomElement &root);
    void collectExtendedAttributes(const QDomElement &element, const QString &elementKind);
    void parseInfoElement(const QDomElement &element, BookMarksGroup &group, const QString &nodePath);
    void parseInfoElement(const QDomElement &element, BookMark &bookmark, const QString &nodePath);
    void parseMetaDataElement(const QDomElement &element, BookMarksGroup &group, const QString &nodePath);
    void parseMetaDataElement(const QDomElement &element, BookMark &bookmark, const QString &nodePath);
    BookMarksGroup parseGroupElement(const QDomElement &element, const QString &groupPath);
    BookMark parseBookmarkElement(const QDomElement &element, const QString &bookmarkPath);
    void collectMetadataContent(const QDomElement &metadataElement, const QString &owner, const QString &href, const QString &nodePath, const QString &basePath);
    void collectMetadataElementAttributes(const QDomElement &element, const QString &owner, const QString &href, const QString &nodePath, const QString &path);
    void parseFreedesktopMetadataNode(const QDomElement &element, BookMark &bookmark);
    void addFreedesktopGroup(const QString &href, const QString &group);
    void addFreedesktopApplication(const FreedesktopApplicationInfo &appInfo);

    // State
    // Watcher state
    QFileSystemWatcher m_fileWatcher;

    // Parse state
    BookMarksManager *q = nullptr;
    mutable QReadWriteLock m_lock;
    QString m_filePath;
    bool m_valid = false;
    QString m_lastError;
    int m_errorLine = -1;
    int m_errorColumn = -1;

    QString m_rootTitle;
    QString m_dbusName;
    BookMarksGroup m_rootGroup;
    QDomDocument m_document;
    QByteArray m_contentDigest;
    BookMarksTextCollector m_textCollector;
    QHash<QString, QStringList> m_freedesktopGroupsByHref;
    QList<FreedesktopApplicationInfo> m_freedesktopApplications;
};

void BookMarksTextCollector::reset()
{
    m_textLines.clear();
    m_customMetaLines.clear();
    m_extendedAttributeLines.clear();
    m_seenCustomMetaKeys.clear();
    m_seenExtendedAttributeLines.clear();
}

void BookMarksTextCollector::appendRawTextLine(const QString &value)
{
    if (!value.isEmpty()) {
        m_textLines.append(value);
    }
}

void BookMarksTextCollector::appendLabeledTextLine(const QString &label, const QString &value)
{
    if (!label.isEmpty() && !value.isEmpty()) {
        m_textLines.append(label + QLatin1String(": ") + value);
    }
}

void BookMarksTextCollector::appendCustomMetaLine(
    const QString &owner, const QString &href, const QString &nodePath, const QString &path, const QString &value)
{
    if (path.isEmpty() || value.isEmpty()) {
        return;
    }

    const QString ownerPart = owner.isEmpty() ? QStringLiteral("unknown-owner") : owner;
    const QString line = !href.isEmpty()
        ? QStringLiteral("metadata[%1][%2].%3: %4").arg(ownerPart, href, path, value)
        : QStringLiteral("metadata[%1].%2: %3").arg(ownerPart, path, value);
    const QString dedupeKey = ownerPart + QLatin1Char('|') + href + QLatin1Char('|') + nodePath + QLatin1Char('|')
        + path + QLatin1Char('|') + value;
    if (m_seenCustomMetaKeys.contains(dedupeKey)) {
        return;
    }
    m_seenCustomMetaKeys.insert(dedupeKey);
    m_customMetaLines.append(line);
}

void BookMarksTextCollector::appendExtendedAttributeLine(const QString &elementKind, const QString &attributeName, const QString &value)
{
    if (elementKind.isEmpty() || attributeName.isEmpty() || value.isEmpty()) {
        return;
    }

    const QString line = QStringLiteral("attribute[%1].%2: %3").arg(elementKind, attributeName, value);
    if (m_seenExtendedAttributeLines.contains(line)) {
        return;
    }

    m_seenExtendedAttributeLines.insert(line);
    m_extendedAttributeLines.append(line);
}

QStringList BookMarksTextCollector::textLines(const QString &dbusName) const
{
    QStringList lines;
    if (!dbusName.isEmpty()) {
        lines.append(QStringLiteral("dbusName: ") + dbusName);
    }
    lines.append(m_textLines);
    lines.append(m_customMetaLines);
    lines.append(m_extendedAttributeLines);
    return lines;
}

QStringList BookMarksTextCollector::customMetaLines() const
{
    return m_customMetaLines;
}

QStringList BookMarksTextCollector::extendedAttributeLines() const
{
    return m_extendedAttributeLines;
}

static void applyBookmarkIconFields(BookMark &bookmark, BookMarksTextCollector &collector, const QDomElement &element, bool onlyIfUnset)
{
    const QString iconName = element.attribute(QLatin1String("name")).trimmed();
    const QString iconHref = element.attribute(QLatin1String("href")).trimmed();
    const QString iconType = element.attribute(QLatin1String("type")).trimmed();

    if (!iconName.isEmpty() && (!onlyIfUnset || bookmark.iconName().isEmpty())) {
        bookmark.setIconName(iconName);
        collector.appendLabeledTextLine(QStringLiteral("icon"), iconName);
    }
    if (!iconHref.isEmpty() && (!onlyIfUnset || bookmark.iconHref().isEmpty())) {
        bookmark.setIconHref(iconHref);
        collector.appendLabeledTextLine(QStringLiteral("icon-href"), iconHref);
    }
    if (!iconType.isEmpty() && (!onlyIfUnset || bookmark.iconType().isEmpty())) {
        bookmark.setIconType(iconType);
        collector.appendLabeledTextLine(QStringLiteral("icon-type"), iconType);
    }
}

static void applyBookmarkMimeType(BookMark &bookmark, BookMarksTextCollector &collector, const QString &mimeType, bool onlyIfUnset)
{
    if (mimeType.isEmpty() || (onlyIfUnset && !bookmark.mimeType().isEmpty())) {
        return;
    }

    bookmark.setMimeType(mimeType);
    collector.appendLabeledTextLine(QStringLiteral("mime-type"), mimeType);
}

static void applyBookmarkPrivateFlag(BookMark &bookmark, BookMarksTextCollector &collector, bool isPrivate)
{
    bookmark.setPrivateFlag(isPrivate);
    collector.appendLabeledTextLine(QStringLiteral("private"), isPrivate ? QStringLiteral("true") : QStringLiteral("false"));
}

BookMarksManagerPrivate::BookMarksManagerPrivate(BookMarksManager *manager)
    : q(manager)
{
    initializeFileWatcher();
}

// Watcher
void BookMarksManagerPrivate::initializeFileWatcher()
{
    QObject::connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged, &m_fileWatcher,
        [this](const QString &changedPath) { handleWatchedFileChanged(changedPath); });
    QObject::connect(&m_fileWatcher, &QFileSystemWatcher::directoryChanged, &m_fileWatcher,
        [this](const QString &changedPath) { handleWatchedDirectoryChanged(changedPath); });
}

void BookMarksManagerPrivate::updateWatchedFile(const QString &filePath)
{
    QWriteLocker locker(&m_lock);

    const QStringList watchedFiles = m_fileWatcher.files();
    const QStringList watchedDirectories = m_fileWatcher.directories();
    if (!watchedFiles.isEmpty()) {
        m_fileWatcher.removePaths(watchedFiles);
    }
    if (!watchedDirectories.isEmpty()) {
        m_fileWatcher.removePaths(watchedDirectories);
    }

    if (filePath.isEmpty()) {
        return;
    }

    const QFileInfo fileInfo(filePath);
    const QString parentPath = fileInfo.absolutePath();
    if (!QFileInfo::exists(parentPath)) {
        return;
    }
    if (!m_fileWatcher.directories().contains(parentPath)) {
        m_fileWatcher.addPath(parentPath);
    }

    if (fileInfo.exists() && fileInfo.isFile() && !m_fileWatcher.files().contains(filePath)) {
        m_fileWatcher.addPath(filePath);
    }
}

void BookMarksManagerPrivate::handleWatchedFileChanged(const QString &changedPath)
{
    const QString currentPath = filePath();
    if (currentPath.isEmpty() || changedPath != currentPath || !QFileInfo::exists(currentPath)) {
        return;
    }

    bool contentChanged = false;
    const bool parseOk = parse(contentChanged);
    if (contentChanged) {
        emitChanged();
    }
    if (!parseOk) {
        emitParseError();
    }
}

void BookMarksManagerPrivate::handleWatchedDirectoryChanged(const QString &changedPath)
{
    const QString currentPath = filePath();
    if (currentPath.isEmpty()) {
        return;
    }

    const QFileInfo fileInfo(currentPath);
    if (fileInfo.absolutePath() != changedPath) {
        return;
    }

    updateWatchedFile(currentPath);
    bool contentChanged = false;
    const bool parseOk = parse(contentChanged);
    if (contentChanged) {
        emitChanged();
    }
    if (!parseOk) {
        emitParseError();
    }
}

bool BookMarksManagerPrivate::sendError(const QString &message, int line, int column)
{
    {
        QWriteLocker locker(&m_lock);
        m_lastError = message;
        m_errorLine = line;
        m_errorColumn = column;
    }
    emitParseError();
    return false;
}

void BookMarksManagerPrivate::emitParseError()
{
    if (q) {
        QString error;
        {
            QReadLocker locker(&m_lock);
            error = m_lastError;
        }
        Q_EMIT q->errorOccurred(error);
    }
}

void BookMarksManagerPrivate::emitChanged()
{
    if (q) {
        Q_EMIT q->changed();
    }
}

// Public API
bool BookMarksManagerPrivate::isValid() const
{
    QReadLocker locker(&m_lock);
    return m_valid;
}

QString BookMarksManagerPrivate::filePath() const
{
    QReadLocker locker(&m_lock);
    return m_filePath;
}

void BookMarksManagerPrivate::setFilePath(const QString &filePath)
{
    {
        QWriteLocker locker(&m_lock);
        // Allow retry on the same path when current state is invalid.
        if (m_filePath == filePath && m_valid) {
            return;
        }
        m_filePath = filePath;
    }

    bool contentChanged = false;
    const bool parseOk = parse(contentChanged);
    if (contentChanged) {
        emitChanged();
    }
    if (!parseOk) {
        emitParseError();
    }
}

bool BookMarksManagerPrivate::save()
{
    QString path;
    {
        QReadLocker locker(&m_lock);
        path = m_filePath;
    }
    return saveAs(path);
}

bool BookMarksManagerPrivate::saveAs(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return sendError(QStringLiteral("stage=saveAs, reason=Target file path is empty"));
    }

    QByteArray content;
    {
        QReadLocker locker(&m_lock);
        if (!m_valid || m_document.isNull()) {
            locker.unlock();
            return sendError(QStringLiteral("stage=saveAs, path=%1, reason=No valid parsed document to save").arg(filePath));
        }
        content = m_document.toByteArray(2);
    }

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return sendError(QStringLiteral("stage=saveAs/open, path=%1, reason=%2").arg(filePath, file.errorString()));
    }

    if (file.write(content) < 0) {
        return sendError(QStringLiteral("stage=saveAs/write, path=%1, reason=%2").arg(filePath, file.errorString()));
    }
    if (!file.commit()) {
        return sendError(QStringLiteral("stage=saveAs/commit, path=%1, reason=%2").arg(filePath, file.errorString()));
    }

    QString watchedPath;
    {
        QWriteLocker locker(&m_lock);
        if (m_filePath != filePath) {
            m_filePath = filePath;
            watchedPath = m_filePath;
        }
    }

    if (!watchedPath.isEmpty()) {
        updateWatchedFile(watchedPath);
    }
    return true;
}

QString BookMarksManagerPrivate::rootTitle() const
{
    QReadLocker locker(&m_lock);
    return m_rootTitle;
}

QString BookMarksManagerPrivate::dbusName() const
{
    QReadLocker locker(&m_lock);
    return m_dbusName;
}

BookMarksGroup BookMarksManagerPrivate::rootGroup() const
{
    QReadLocker locker(&m_lock);
    return m_rootGroup;
}

QStringList BookMarksManagerPrivate::textLines() const
{
    QReadLocker locker(&m_lock);
    return m_textCollector.textLines(m_dbusName);
}

QStringList BookMarksManagerPrivate::customMetaLines() const
{
    QReadLocker locker(&m_lock);
    return m_textCollector.customMetaLines();
}

QStringList BookMarksManagerPrivate::extendedAttributeLines() const
{
    QReadLocker locker(&m_lock);
    return m_textCollector.extendedAttributeLines();
}

QHash<QString, QStringList> BookMarksManagerPrivate::freedesktopGroups() const
{
    QReadLocker locker(&m_lock);
    return m_freedesktopGroupsByHref;
}

QStringList BookMarksManagerPrivate::freedesktopGroups(const QString &href) const
{
    QReadLocker locker(&m_lock);
    return m_freedesktopGroupsByHref.value(href);
}

QList<FreedesktopApplicationInfo> BookMarksManagerPrivate::freedesktopApplications() const
{
    QReadLocker locker(&m_lock);
    return m_freedesktopApplications;
}

QList<FreedesktopApplicationInfo> BookMarksManagerPrivate::freedesktopApplications(const QString &href) const
{
    QReadLocker locker(&m_lock);
    QList<FreedesktopApplicationInfo> filtered;
    for (const FreedesktopApplicationInfo &item : m_freedesktopApplications) {
        if (item.href() == href) {
            filtered.append(item);
        }
    }
    return filtered;
}

QString BookMarksManagerPrivate::lastError() const
{
    QReadLocker locker(&m_lock);
    return m_lastError;
}

int BookMarksManagerPrivate::errorLine() const
{
    QReadLocker locker(&m_lock);
    return m_errorLine;
}

int BookMarksManagerPrivate::errorColumn() const
{
    QReadLocker locker(&m_lock);
    return m_errorColumn;
}

// Parser
bool BookMarksManagerPrivate::parse(bool &contentChanged)
{
    QWriteLocker locker(&m_lock);
    const QString sourceFilePath = m_filePath;
    const bool oldValid = m_valid;
    const QByteArray oldDigest = m_contentDigest;

    reset();

    const bool parseOk = parseFile(sourceFilePath);

    contentChanged = (oldValid != m_valid) || (oldDigest != m_contentDigest);
    locker.unlock();
    updateWatchedFile(sourceFilePath);
    return parseOk;
}

bool BookMarksManagerPrivate::parseFile(const QString &filePath)
{
    if (filePath.isEmpty()) {
        m_lastError = QStringLiteral("stage=parse/open, reason=Source file path is empty");
        m_errorLine = -1;
        m_errorColumn = -1;
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QStringLiteral("stage=parse/open, path=%1, reason=%2").arg(filePath, file.errorString());
        m_errorLine = -1;
        m_errorColumn = -1;
        return false;
    }

    QCryptographicHash hash(QCryptographicHash::Sha1);
    if (!hash.addData(&file)) {
        m_lastError = QStringLiteral("stage=parse/digest, path=%1, reason=Failed to read file for digest").arg(filePath);
        m_errorLine = -1;
        m_errorColumn = -1;
        return false;
    }
    m_contentDigest = hash.result();
    if (!file.seek(0)) {
        m_lastError = QStringLiteral("stage=parse/rewind, path=%1, reason=Failed to rewind file after digest").arg(filePath);
        m_errorLine = -1;
        m_errorColumn = -1;
        return false;
    }

    QDomDocument doc;
    QString errorMessage;
    int errorLine = 0;
    int errorColumn = 0;
    if (!doc.setContent(&file, true, &errorMessage, &errorLine, &errorColumn)) {
        const QString reason = errorMessage.isEmpty() ? QStringLiteral("XML parse error") : errorMessage;
        m_lastError = QStringLiteral("stage=parse/xml, path=%1, line=%2, column=%3, reason=%4")
                          .arg(filePath)
                          .arg(errorLine)
                          .arg(errorColumn)
                          .arg(reason);
        m_errorLine = errorLine;
        m_errorColumn = errorColumn;
        return false;
    }

    const QDomElement root = doc.documentElement();
    if (root.isNull() || elementName(root) != QLatin1String("xbel")) {
        m_lastError = QStringLiteral("stage=parse/validate, path=%1, reason=Root element is not <xbel>").arg(filePath);
        m_errorLine = -1;
        m_errorColumn = -1;
        return false;
    }

    parseXbelAttributes(root);

    int rootFolderIndex = 0;
    int rootBookmarkIndex = 0;

    for (QDomNode node = root.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (!node.isElement()) {
            continue;
        }

        const QDomElement element = node.toElement();
        const QString tagName = elementName(element);

        if (tagName == QLatin1String("title")) {
            const QString title = element.text().trimmed();
            if (!title.isEmpty()) {
                m_rootTitle = title;
                m_rootGroup.setTitle(title);
                m_textCollector.appendRawTextLine(title);
            }
            continue;
        }

        if (tagName == QLatin1String("desc") || tagName == QLatin1String("description")) {
            const QString description = element.text().trimmed();
            if (!description.isEmpty()) {
                m_rootGroup.setDescription(description);
                m_textCollector.appendRawTextLine(description);
            }
            continue;
        }

        if (tagName == QLatin1String("info")) {
            parseInfoElement(element, m_rootGroup, QStringLiteral("root"));
            continue;
        }

        if (tagName == QLatin1String("metadata")) {
            parseMetaDataElement(element, m_rootGroup, QStringLiteral("root"));
            continue;
        }

        if (tagName == QLatin1String("folder")) {
            m_rootGroup.addGroup(parseGroupElement(element, QStringLiteral("root.folder[%1]").arg(rootFolderIndex++)));
            continue;
        }

        if (tagName == QLatin1String("bookmark")) {
            m_rootGroup.addBookmark(parseBookmarkElement(element, QStringLiteral("root.bookmark[%1]").arg(rootBookmarkIndex++)));
            continue;
        }

        if (tagName == QLatin1String("alias") || tagName == QLatin1String("separator")) {
            collectExtendedAttributes(element, tagName);
            continue;
        }
    }

    m_valid = true;
    m_document = doc;
    return true;
}

void BookMarksManagerPrivate::reset()
{
    m_valid = false;
    m_lastError.clear();
    m_errorLine = -1;
    m_errorColumn = -1;
    m_rootTitle.clear();
    m_dbusName.clear();
    m_rootGroup = BookMarksGroup();
    m_document = QDomDocument();
    m_contentDigest.clear();

    m_textCollector.reset();
    m_freedesktopGroupsByHref.clear();
    m_freedesktopApplications.clear();
}

void BookMarksManagerPrivate::parseXbelAttributes(const QDomElement &root)
{
    const QDomNamedNodeMap attrs = root.attributes();
    for (int i = 0; i < attrs.size(); ++i) {
        const QDomAttr attr = attrs.item(i).toAttr();
        if (attr.isNull()) {
            continue;
        }

        const QString attributeName = attr.name();
        const QString value = attr.value().trimmed();
        if (attributeName == QLatin1String("id")) {
            m_rootGroup.setId(value);
            continue;
        }
        if (attributeName == QLatin1String("added")) {
            m_rootGroup.setAdded(value);
            continue;
        }
        if (attributeName == QLatin1String("modified")) {
            m_rootGroup.setModified(value);
            continue;
        }
        if (attributeName == QLatin1String("visited")) {
            m_rootGroup.setVisited(value);
            continue;
        }
        if (attributeName == QLatin1String("dbusName")) {
            m_dbusName = value;
            continue;
        }
    }

    collectExtendedAttributes(root, QStringLiteral("xbel"));
}

void BookMarksManagerPrivate::collectExtendedAttributes(const QDomElement &element, const QString &elementKind)
{
    const QDomNamedNodeMap attrs = element.attributes();
    for (int i = 0; i < attrs.size(); ++i) {
        const QDomAttr attr = attrs.item(i).toAttr();
        if (attr.isNull()) {
            continue;
        }

        const QString attributeName = attr.name();
        if (attributeName == QLatin1String("xmlns")
            || attributeName.startsWith(QLatin1String("xmlns:"))
            || isStandardAttributeForElement(elementKind, attributeName)) {
            continue;
        }

        const QString value = attr.value().trimmed();
        if (!value.isEmpty()) {
            m_textCollector.appendExtendedAttributeLine(elementKind, attributeName, value);
        }
    }
}

void BookMarksManagerPrivate::parseInfoElement(const QDomElement &element, BookMarksGroup &group, const QString &nodePath)
{
    for (QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (!node.isElement()) {
            continue;
        }

        const QDomElement child = node.toElement();
        if (elementName(child) == QLatin1String("metadata")) {
            parseMetaDataElement(child, group, nodePath);
        }
    }
}

void BookMarksManagerPrivate::parseInfoElement(const QDomElement &element, BookMark &bookmark, const QString &nodePath)
{
    for (QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (!node.isElement()) {
            continue;
        }

        const QDomElement child = node.toElement();
        if (elementName(child) == QLatin1String("metadata")) {
            parseMetaDataElement(child, bookmark, nodePath);
        }
    }
}

void BookMarksManagerPrivate::parseMetaDataElement(const QDomElement &element, BookMarksGroup &group, const QString &nodePath)
{
    const QString owner = element.attribute(QLatin1String("owner")).trimmed();
    const bool isKdeOwner = ownerMatchesHost(owner, KDE_OWNER_HOST);

    for (QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (!node.isElement()) {
            continue;
        }

        const QDomElement child = node.toElement();
        const QString key = elementName(child);
        const QString value = child.text().trimmed();
        if (isKdeOwner && !value.isEmpty()) {
            group.addKdeMetaValue(key, value);
            m_textCollector.appendLabeledTextLine(key, value);
        }

        if (!isKdeOwner) {
            collectMetadataContent(child, owner, QString(), nodePath, key);
        }
    }
}

void BookMarksManagerPrivate::parseMetaDataElement(const QDomElement &element, BookMark &bookmark, const QString &nodePath)
{
    const QString owner = element.attribute(QLatin1String("owner")).trimmed();
    const bool isKdeOwner = ownerMatchesHost(owner, KDE_OWNER_HOST);
    const bool isFreedesktopOwner = ownerMatchesHost(owner, FREEDESKTOP_OWNER_HOST);

    for (QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (!node.isElement()) {
            continue;
        }

        const QDomElement child = node.toElement();
        const QString key = elementName(child);
        const QString value = child.text().trimmed();
        if (isKdeOwner && !value.isEmpty()) {
            bookmark.addKdeMetaValue(key, value);
            m_textCollector.appendLabeledTextLine(key, value);
        }

        if (!isKdeOwner) {
            collectMetadataContent(child, owner, bookmark.href(), nodePath, key);
        }

        if (isFreedesktopOwner) {
            parseFreedesktopMetadataNode(child, bookmark);
        }
    }
}

BookMarksGroup BookMarksManagerPrivate::parseGroupElement(const QDomElement &element, const QString &groupPath)
{
    BookMarksGroup group;

    const QDomNamedNodeMap attrs = element.attributes();
    for (int i = 0; i < attrs.size(); ++i) {
        const QDomAttr attr = attrs.item(i).toAttr();
        if (attr.isNull()) {
            continue;
        }

        const QString attributeName = attr.name();
        const QString value = attr.value().trimmed();
        if (attributeName == QLatin1String("id")) {
            group.setId(value);
            continue;
        }
        if (attributeName == QLatin1String("added")) {
            group.setAdded(value);
            continue;
        }
        if (attributeName == QLatin1String("modified")) {
            group.setModified(value);
            continue;
        }
        if (attributeName == QLatin1String("visited")) {
            group.setVisited(value);
            continue;
        }
        if (attributeName == QLatin1String("folded")) {
            if (!value.isEmpty()) {
                group.setFolded(value);
                m_textCollector.appendLabeledTextLine(QStringLiteral("folded"), value);
            }
            continue;
        }
        if (attributeName == QLatin1String("toolbar")) {
            if (!value.isEmpty()) {
                group.setToolbar(value);
                m_textCollector.appendLabeledTextLine(QStringLiteral("toolbar"), value);
            }
            continue;
        }
    }

    collectExtendedAttributes(element, QStringLiteral("folder"));

    int subgroupIndex = 0;
    int bookmarkIndex = 0;

    for (QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (!node.isElement()) {
            continue;
        }

        const QDomElement child = node.toElement();
        const QString tagName = elementName(child);

        if (tagName == QLatin1String("title")) {
            const QString title = child.text().trimmed();
            if (!title.isEmpty()) {
                group.setTitle(title);
                m_textCollector.appendRawTextLine(title);
            }
            continue;
        }

        if (tagName == QLatin1String("desc") || tagName == QLatin1String("description")) {
            const QString description = child.text().trimmed();
            if (!description.isEmpty()) {
                group.setDescription(description);
                m_textCollector.appendRawTextLine(description);
            }
            continue;
        }

        if (tagName == QLatin1String("info")) {
            parseInfoElement(child, group, groupPath);
            continue;
        }

        if (tagName == QLatin1String("metadata")) {
            parseMetaDataElement(child, group, groupPath);
            continue;
        }

        if (tagName == QLatin1String("folder")) {
            group.addGroup(parseGroupElement(child, QStringLiteral("%1.folder[%2]").arg(groupPath).arg(subgroupIndex++)));
            continue;
        }

        if (tagName == QLatin1String("bookmark")) {
            group.addBookmark(parseBookmarkElement(child, QStringLiteral("%1.bookmark[%2]").arg(groupPath).arg(bookmarkIndex++)));
            continue;
        }

        if (tagName == QLatin1String("alias") || tagName == QLatin1String("separator")) {
            collectExtendedAttributes(child, tagName);
            continue;
        }
    }

    return group;
}

BookMark BookMarksManagerPrivate::parseBookmarkElement(const QDomElement &element, const QString &bookmarkPath)
{
    BookMark bookmark;

    const QDomNamedNodeMap attrs = element.attributes();
    for (int i = 0; i < attrs.size(); ++i) {
        const QDomAttr attr = attrs.item(i).toAttr();
        if (attr.isNull()) {
            continue;
        }

        const QString attributeName = attr.name();
        const QString value = attr.value().trimmed();
        if (attributeName == QLatin1String("id")) {
            bookmark.setId(value);
            continue;
        }
        if (attributeName == QLatin1String("added")) {
            bookmark.setAdded(value);
            continue;
        }
        if (attributeName == QLatin1String("modified")) {
            bookmark.setModified(value);
            continue;
        }
        if (attributeName == QLatin1String("visited")) {
            bookmark.setVisited(value);
            continue;
        }
        if (attributeName == QLatin1String("href")) {
            if (!value.isEmpty()) {
                bookmark.setHref(value);
                m_textCollector.appendRawTextLine(value);
            }
            continue;
        }
        if (attributeName == QLatin1String("showintoolbar")) {
            if (!value.isEmpty()) {
                bookmark.setShowInToolbar(value);
                m_textCollector.appendLabeledTextLine(QStringLiteral("showintoolbar"), value);
            }
            continue;
        }
    }

    collectExtendedAttributes(element, QStringLiteral("bookmark"));

    for (QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (!node.isElement()) {
            continue;
        }

        const QDomElement child = node.toElement();
        const QString tagName = elementName(child);
        const QString qualifiedName = child.tagName();

        if (tagName == QLatin1String("title")) {
            const QString title = child.text().trimmed();
            if (!title.isEmpty()) {
                bookmark.setTitle(title);
                m_textCollector.appendRawTextLine(title);
            }
            continue;
        }

        if (tagName == QLatin1String("desc") || tagName == QLatin1String("description")) {
            const QString description = child.text().trimmed();
            if (!description.isEmpty()) {
                bookmark.setDescription(description);
                m_textCollector.appendRawTextLine(description);
            }
            continue;
        }

        if (tagName == QLatin1String("info")) {
            parseInfoElement(child, bookmark, bookmarkPath);
            continue;
        }

        if (tagName == QLatin1String("metadata")) {
            parseMetaDataElement(child, bookmark, bookmarkPath);
            continue;
        }

        if (qualifiedName == QLatin1String("bookmark:icon")
            || (tagName == QLatin1String("icon")
                && child.namespaceURI() == QLatin1String("http://www.freedesktop.org/standards/desktop-bookmarks"))) {
            applyBookmarkIconFields(bookmark, m_textCollector, child, true);
            continue;
        }

        if (qualifiedName == QLatin1String("mime:mime-type")
            || (tagName == QLatin1String("mime-type")
                && child.namespaceURI() == QLatin1String("http://www.freedesktop.org/standards/shared-mime-info"))) {
            applyBookmarkMimeType(bookmark, m_textCollector, child.attribute(QLatin1String("type")).trimmed(), true);
            continue;
        }
    }

    return bookmark;
}

void BookMarksManagerPrivate::collectMetadataContent(const QDomElement &metadataElement, const QString &owner, const QString &href, const QString &nodePath, const QString &basePath)
{
    bool hasElementChild = false;
    for (QDomNode node = metadataElement.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isElement()) {
            hasElementChild = true;
            break;
        }
    }

    if (!hasElementChild) {
        const QString value = metadataElement.text().trimmed();
        if (!value.isEmpty() && !basePath.isEmpty()) {
            m_textCollector.appendCustomMetaLine(owner, href, nodePath, basePath, value);
        }
    }

    collectMetadataElementAttributes(metadataElement, owner, href, nodePath, basePath);
}

void BookMarksManagerPrivate::collectMetadataElementAttributes(const QDomElement &element, const QString &owner, const QString &href, const QString &nodePath, const QString &path)
{
    const QDomNamedNodeMap attrs = element.attributes();
    for (int i = 0; i < attrs.size(); ++i) {
        const QDomAttr attr = attrs.item(i).toAttr();
        if (attr.isNull() || attr.name() == QLatin1String("owner")) {
            continue;
        }
        const QString attrValue = attr.value().trimmed();
        if (!attrValue.isEmpty()) {
            m_textCollector.appendCustomMetaLine(owner, href, nodePath, path + QLatin1Char('@') + attr.name(), attrValue);
        }
    }

    for (QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (!node.isElement()) {
            continue;
        }
        const QDomElement child = node.toElement();
        const QString childName = elementName(child);
        const QString childPath = path.isEmpty() ? childName : path + QLatin1Char('.') + childName;
        collectMetadataContent(child, owner, href, nodePath, childPath);
    }
}

void BookMarksManagerPrivate::parseFreedesktopMetadataNode(const QDomElement &element, BookMark &bookmark)
{
    const QString tagName = elementName(element);
    const QString href = bookmark.href();
    if (tagName == QLatin1String("groups")) {
        for (QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
            if (!node.isElement()) {
                continue;
            }
            const QDomElement child = node.toElement();
            if (elementName(child) != QLatin1String("group")) {
                continue;
            }
            const QString group = child.text().trimmed();
            if (!group.isEmpty()) {
                addFreedesktopGroup(href, group);
            }
        }
        return;
    }

    if (tagName == QLatin1String("icon")) {
        applyBookmarkIconFields(bookmark, m_textCollector, element, false);
        return;
    }

    if (tagName == QLatin1String("mime-type")) {
        applyBookmarkMimeType(bookmark, m_textCollector, element.attribute(QLatin1String("type")).trimmed(), false);
        return;
    }

    if (tagName == QLatin1String("private")) {
        bool isPrivate = false;
        if (parseFreedesktopBooleanValue(element.text(), isPrivate)) {
            applyBookmarkPrivateFlag(bookmark, m_textCollector, isPrivate);
        }
        return;
    }

    if (tagName == QLatin1String("applications")) {
        for (QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
            if (!node.isElement()) {
                continue;
            }
            const QDomElement child = node.toElement();
            if (elementName(child) != QLatin1String("application")) {
                continue;
            }

            FreedesktopApplicationInfo info;
            info.setHref(href);
            const QString name = child.attribute(QLatin1String("name")).trimmed();
            QString exec = child.attribute(QLatin1String("exec")).trimmed();
            info.setName(name);
            if (exec.isEmpty() && !name.isEmpty()) {
                exec = name + QStringLiteral(" %u");
            }
            info.setExec(exec);
            info.setModified(child.attribute(QLatin1String("modified")).trimmed());
            bool ok = false;
            const QString countText = child.attribute(QLatin1String("count")).trimmed();
            const int parsedCount = countText.toInt(&ok);
            info.setCount(countText.isEmpty() ? 1 : (ok ? parsedCount : -1));
            addFreedesktopApplication(info);
        }
    }
}

void BookMarksManagerPrivate::addFreedesktopGroup(const QString &href, const QString &group)
{
    if (href.isEmpty() || group.isEmpty()) {
        return;
    }
    if (m_freedesktopGroupsByHref.value(href).contains(group)) {
        return;
    }
    m_freedesktopGroupsByHref[href].append(group);
}

void BookMarksManagerPrivate::addFreedesktopApplication(const FreedesktopApplicationInfo &appInfo)
{
    if (appInfo.href().isEmpty()) {
        return;
    }
    if (appInfo.name().isEmpty() && appInfo.exec().isEmpty() && appInfo.modified().isEmpty() && appInfo.count() < 0) {
        return;
    }
    m_freedesktopApplications.append(appInfo);
}

BookMarksManager::BookMarksManager(const QString &filePath, QObject *parent)
    : QObject(parent)
    , d(new BookMarksManagerPrivate(this))
{
    if (!filePath.isEmpty()) {
        d->setFilePath(filePath);
    }
}

BookMarksManager::~BookMarksManager()
{
    delete d;
}

bool BookMarksManager::isValid() const
{
    return d->isValid();
}

QString BookMarksManager::filePath() const
{
    return d->filePath();
}

void BookMarksManager::setFilePath(const QString &filePath)
{
    d->setFilePath(filePath);
}

bool BookMarksManager::save()
{
    return d->save();
}

bool BookMarksManager::saveAs(const QString &filePath)
{
    return d->saveAs(filePath);
}

QString BookMarksManager::lastError() const
{
    return d->lastError();
}

int BookMarksManager::errorLine() const
{
    return d->errorLine();
}

int BookMarksManager::errorColumn() const
{
    return d->errorColumn();
}

QString BookMarksManager::rootTitle() const
{
    return d->rootTitle();
}

QString BookMarksManager::dbusName() const
{
    return d->dbusName();
}

BookMarksGroup BookMarksManager::rootGroup() const
{
    return d->rootGroup();
}

QStringList BookMarksManager::textLines() const
{
    return d->textLines();
}

QStringList BookMarksManager::customMetaLines() const
{
    return d->customMetaLines();
}

QStringList BookMarksManager::extendedAttributeLines() const
{
    return d->extendedAttributeLines();
}

QHash<QString, QStringList> BookMarksManager::freedesktopGroups() const
{
    return d->freedesktopGroups();
}

QStringList BookMarksManager::freedesktopGroups(const QString &href) const
{
    return d->freedesktopGroups(href);
}

QList<FreedesktopApplicationInfo> BookMarksManager::freedesktopApplications() const
{
    return d->freedesktopApplications();
}

QList<FreedesktopApplicationInfo> BookMarksManager::freedesktopApplications(const QString &href) const
{
    return d->freedesktopApplications(href);
}

} // namespace UkuiFileMetadata
