#ifndef PREVIEWHUB_FILEINFO_H
#define PREVIEWHUB_FILEINFO_H

#include <QString>
#include <memory>

/**
 * Local replacement for Peony::FileInfo used by the preview stack.
 * Only local file:// URIs are fully supported.
 */
class PreviewHubFileInfo
{
public:
    static std::shared_ptr<PreviewHubFileInfo> fromUri(const QString &uri);

    QString uri() const { return m_uri; }
    QString filePath() const { return m_path; }
    QString displayName() const { return m_displayName; }
    QString fileType() const { return m_fileType; }
    QString mimeType() const { return m_mimeType; }
    QString iconName() const { return m_iconName; }
    QString desktopName() const { return m_desktopName; }
    qint64 accessTime() const { return m_accessSecs; }
    qint64 modifiedTime() const { return m_modifiedSecs; }

    bool isVideoFile() const;
    bool isImageFile() const;
    bool isPdfFile() const;
    bool isOfficeFile() const;
    bool canWrite() const;

    /** Refresh metadata from disk (replaces Peony::FileInfoJob::querySync). */
    void syncRefresh();

private:
    PreviewHubFileInfo() = default;
    void recomputeFromDisk();

    QString m_uri;
    QString m_path;
    QString m_displayName;
    QString m_fileType;
    QString m_mimeType;
    QString m_iconName;
    QString m_desktopName;
    qint64 m_accessSecs = 0;
    qint64 m_modifiedSecs = 0;
};

#endif
