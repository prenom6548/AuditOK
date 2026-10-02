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
    bool isCanceled = false;

signals:
    void signalFileInfo(const QHash<QByteArray,QStringList>&p);
    void signalProgressValue(int current,int total);

public slots:
    void slotGetFileInfo(const QString &path);
    void slotCannelGetMd5();

protected:
    QStringList getFiles(const QString &path);
    QByteArray getFileMd5(const QString &path);

};

#endif // FILEMD5_H