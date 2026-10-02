#ifndef UDISKDETECTOR_H
#define UDISKDETECTOR_H

#include <QObject>
#include <QStorageInfo>
#include <QString>

class QTimer;
class QFileSystemWatcher;

class UdiskDetector : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QStorageInfo> udiskList READ getDiskList WRITE setDiskList NOTIFY udiskListChanged)
public:
    UdiskDetector(QObject* parent = nullptr);

    void getStorageInfo();
    void refreshDiskList();

    QList<QStorageInfo> getDiskList() const
    {
        return m_udiskList;
    }

public slots:
    void setDiskList(QList<QStorageInfo> udiskList);

signals:
    void udiskListChanged(QList<QStorageInfo> udiskList);

private:
    QTimer* diskRefreshDelay;
    QFileSystemWatcher* udiskplugwatcher;
    QList<QStorageInfo> m_udiskList;
};

#endif // UDISKDETECTOR_H
