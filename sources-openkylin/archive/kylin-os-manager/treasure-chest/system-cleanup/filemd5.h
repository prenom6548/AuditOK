#ifndef FILEMD5_H
#define FILEMD5_H

#include <QObject>
#include <QHash>

class FileMd5 : public QObject
{
    Q_OBJECT

public:
    explicit FileMd5(QObject *parent = nullptr);
    QHash<QByteArray,QStringList> filemd5;
    QHash <long, QStringList> m_fileinfo;

Q_SIGNALS:
    void signalFileInfo(const QHash<QByteArray,QStringList>&p);
    void signalProgressValue(int current,int total);

public Q_SLOTS:
    void slotGetFileInfo(const QString &path);
    bool *slotGetCancelHandle();
    int *slotGetSameFileNum();

private:
    QStringList getFiles(const QString &path);
    // void getFiles(const QString &path);
    QByteArray getFileMd5(const QString &path);
    QHash<QByteArray, QStringList> filtr(long size ,const QStringList &list);

    bool *m_cancel = nullptr;
    int *m_sameFileNum = nullptr;
};

#endif // FILEMD5_H