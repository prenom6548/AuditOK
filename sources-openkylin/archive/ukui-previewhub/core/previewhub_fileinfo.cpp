#include "previewhub_fileinfo.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QUrl>

static bool isOfficeExtension(const QString &suffix)
{
    static const char *const kExts[] = {
        "doc", "docx", "dot", "dotx", "wps",
        "xls", "xlsx", "xlt", "xltx", "csv", "et",
        "ppt", "pptx", "pot", "potx", "dps",
        "odt", "ods", "odp", "odg", "odf",
        "rtf"
    };
    const QString s = suffix.toLower();
    for (const char *e : kExts) {
        if (s == QLatin1String(e))
            return true;
    }
    return false;
}

std::shared_ptr<PreviewHubFileInfo> PreviewHubFileInfo::fromUri(const QString &uri)
{
    auto p = std::shared_ptr<PreviewHubFileInfo>(new PreviewHubFileInfo);
    p->m_uri = uri;
    const QUrl u(uri);
    if (u.isLocalFile())
        p->m_path = u.toLocalFile();
    p->recomputeFromDisk();
    return p;
}

void PreviewHubFileInfo::recomputeFromDisk()
{
    m_accessSecs = 0;
    m_modifiedSecs = 0;
    m_iconName.clear();
    m_desktopName.clear();

    if (m_path.isEmpty()) {
        m_displayName.clear();
        m_fileType = QStringLiteral("unknown");
        m_mimeType.clear();
        return;
    }

    QFileInfo fi(m_path);
    if (!fi.exists() || !fi.isFile()) {
        m_displayName.clear();
        m_fileType = QStringLiteral("unknown");
        m_mimeType.clear();
        return;
    }

    m_displayName = fi.fileName();
    m_desktopName = fi.completeBaseName().isEmpty() ? fi.fileName() : fi.completeBaseName();

    QMimeDatabase db;
    const QMimeType mime = db.mimeTypeForFile(fi);
    m_mimeType = mime.name();
    m_iconName = mime.iconName();

    const QString comment = mime.comment();
    if (!comment.isEmpty())
        m_fileType = comment;
    else
        m_fileType = m_mimeType;

    if (m_mimeType == QLatin1String("application/octet-stream")
        || m_mimeType == QLatin1String("inode/x-empty")) {
        m_fileType = QStringLiteral("unknown");
    }

    const QDateTime md = fi.lastModified();
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    const QDateTime ar = fi.lastRead();
    m_accessSecs = ar.isValid() ? ar.toSecsSinceEpoch() : md.toSecsSinceEpoch();
#else
    m_accessSecs = md.isValid() ? md.toSecsSinceEpoch() : 0;
#endif
    m_modifiedSecs = md.isValid() ? md.toSecsSinceEpoch() : 0;
}

void PreviewHubFileInfo::syncRefresh()
{
    recomputeFromDisk();
}

bool PreviewHubFileInfo::isVideoFile() const
{
    if (m_fileType == QLatin1String("unknown"))
        return false;
    const QString suf = QFileInfo(m_path).suffix().toLower();
    if (suf == QLatin1String("ncm"))
        return false;
    return m_mimeType.startsWith(QLatin1String("video/"));
}

bool PreviewHubFileInfo::isImageFile() const
{
    return m_mimeType.startsWith(QLatin1String("image/"));
}

bool PreviewHubFileInfo::isPdfFile() const
{
    if (m_mimeType == QLatin1String("application/pdf"))
        return true;
    return QFileInfo(m_path).suffix().compare(QLatin1String("pdf"), Qt::CaseInsensitive) == 0;
}

bool PreviewHubFileInfo::isOfficeFile() const
{
    const QString suf = QFileInfo(m_path).suffix();
    if (isOfficeExtension(suf))
        return true;
    const QString m = m_mimeType.toLower();
    return m.contains(QLatin1String("officedocument"), Qt::CaseInsensitive)
        || m.contains(QLatin1String("msword"), Qt::CaseInsensitive)
        || m.contains(QLatin1String("ms-powerpoint"), Qt::CaseInsensitive)
        || m.contains(QLatin1String("ms-excel"), Qt::CaseInsensitive)
        || m.contains(QLatin1String("opendocument"), Qt::CaseInsensitive)
        || m == QLatin1String("application/rtf");
}

bool PreviewHubFileInfo::canWrite() const
{
    if (m_path.isEmpty())
        return false;
    QFileInfo fi(m_path);
    return fi.isWritable();
}
