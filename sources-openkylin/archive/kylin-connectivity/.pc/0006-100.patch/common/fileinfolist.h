#ifndef FILEINFOLIST_H
#define FILEINFOLIST_H

#include <QString>
#include <QVariant>
#include <QtDBus>

#include "fileinfo.h"

class FileInfoList
{
public:
    FileInfoList();
    void addFileInfo(FileInfo fileInfo);
    const QList<FileInfo> getFileList();
    const FileInfo value(int index);
    const FileInfo value(QString key);
    const int size();
    void clear();

    friend QDBusArgument &operator<<(QDBusArgument &argument, const FileInfoList &fileInfoList);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, FileInfoList &fileInfoList);
    static void registerMetaType();

private:
    QList<FileInfo> m_fileList;
    int m_length = 0;
};
Q_DECLARE_METATYPE(FileInfoList)
#endif // FILEINFOLIST_H
