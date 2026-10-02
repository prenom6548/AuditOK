#ifndef PREVIEWHUB_FILEWATCHER_H
#define PREVIEWHUB_FILEWATCHER_H

#include <QFileSystemWatcher>
#include <QObject>
#include <QString>

/**
 * Watches a local file and emits locationChanged when it is renamed in the same directory
 * (inode match). Replaces Peony::FileWatcher for local file:// URIs only.
 */
class PreviewHubFileWatcher : public QObject
{
    Q_OBJECT
public:
    explicit PreviewHubFileWatcher(const QString &uri, QObject *parent = nullptr);
    void startMonitor();

signals:
    void locationChanged(const QString &oldUri, const QString &newUri);

private slots:
    void onDirectoryChanged(const QString &path);

private:
    void captureInode();
    bool rescanParentForInode();

    QString m_uri;
    QString m_path;
    QString m_parent;
    QFileSystemWatcher m_fs;
    quint64 m_ino = 0;
    quint64 m_dev = 0;
};

#endif
