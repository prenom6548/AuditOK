#include "previewhub_filewatcher.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUrl>

#ifdef Q_OS_UNIX
#  include <sys/stat.h>
#endif

PreviewHubFileWatcher::PreviewHubFileWatcher(const QString &uri, QObject *parent)
    : QObject(parent)
    , m_uri(uri)
{
    const QUrl u(uri);
    if (u.isLocalFile())
        m_path = u.toLocalFile();
    if (!m_path.isEmpty()) {
        QFileInfo fi(m_path);
        m_parent = fi.absolutePath();
    }
}

void PreviewHubFileWatcher::captureInode()
{
    m_ino = 0;
    m_dev = 0;
#ifdef Q_OS_UNIX
    if (m_path.isEmpty())
        return;
    struct stat st;
    if (::stat(m_path.toUtf8().constData(), &st) == 0) {
        m_ino = static_cast<quint64>(st.st_ino);
        m_dev = static_cast<quint64>(st.st_dev);
    }
#endif
}

bool PreviewHubFileWatcher::rescanParentForInode()
{
#ifdef Q_OS_UNIX
    if (m_parent.isEmpty() || m_ino == 0)
        return false;
    QDir dir(m_parent);
    const auto names = dir.entryList(QDir::Files | QDir::System | QDir::Hidden | QDir::NoDotAndDotDot);
    for (const QString &name : names) {
        const QString fp = dir.filePath(name);
        struct stat st;
        if (::stat(fp.toUtf8().constData(), &st) != 0)
            continue;
        if (static_cast<quint64>(st.st_ino) == m_ino && static_cast<quint64>(st.st_dev) == m_dev) {
            const QString newUri = QUrl::fromLocalFile(fp).toString();
            if (newUri != m_uri) {
                const QString old = m_uri;
                m_uri = newUri;
                m_path = fp;
                QFileInfo fi(m_path);
                m_parent = fi.absolutePath();
                m_fs.removePaths(m_fs.files());
                if (QFile::exists(m_path))
                    m_fs.addPath(m_path);
                if (!m_parent.isEmpty())
                    m_fs.addPath(m_parent);
                captureInode();

                // A direct-connected receiver may delete this watcher while
                // handling the rename. Do not access any members after emit.
                emit locationChanged(old, newUri);
            }
            return true;
        }
    }
#endif
    return false;
}

void PreviewHubFileWatcher::startMonitor()
{
    m_fs.disconnect();
    if (m_path.isEmpty() || !QFile::exists(m_path)) {
        qDebug() << "PreviewHubFileWatcher: not a watchable local file" << m_uri;
        return;
    }
    captureInode();
    connect(&m_fs, &QFileSystemWatcher::directoryChanged, this, &PreviewHubFileWatcher::onDirectoryChanged);
    m_fs.addPath(m_path);
    if (!m_parent.isEmpty() && m_parent != m_path)
        m_fs.addPath(m_parent);
}

void PreviewHubFileWatcher::onDirectoryChanged(const QString &path)
{
    Q_UNUSED(path);
    if (QFile::exists(m_path))
        return;
    rescanParentForInode();
}
