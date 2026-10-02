#include "fileinfolist.h"

#include <QDebug>

FileInfoList::FileInfoList() {}

void FileInfoList::addFileInfo(FileInfo fileInfo)
{
    m_fileList.append(fileInfo);
    m_length++;
}

const QList<FileInfo> FileInfoList::getFileList()
{
    return m_fileList;
}


const FileInfo FileInfoList::value(int index)
{
    return m_fileList.value(index);
}

const FileInfo FileInfoList::value(QString key)
{
    for (int i = 0; i < m_fileList.size(); i++) {
        if (key == m_fileList.value(i).name()) {
            return m_fileList.value(i);
        }
    }
    return FileInfo();
}

const int FileInfoList::size()
{
    return m_length;
}

void FileInfoList::clear()
{
    m_fileList.clear();
    m_length = 0;
}

QDBusArgument &operator<<(QDBusArgument &argument, const FileInfoList &fileInfoList)
{
    argument.beginArray(qMetaTypeId<FileInfo>());
    for (int i = 0; i < fileInfoList.m_length; i++) {
        argument << fileInfoList.m_fileList.value(i);
    }
    argument.endArray();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, FileInfoList &fileInfoList)
{
    argument.beginArray();
    fileInfoList.clear();

    while (!argument.atEnd()) {
        FileInfo fileInfo;
        argument >> fileInfo;
        fileInfoList.addFileInfo(fileInfo);
    }

    argument.endArray();
    return argument;
}

void FileInfoList::registerMetaType()
{
    qRegisterMetaType<FileInfoList>("FileInfoList");
    qDBusRegisterMetaType<FileInfoList>();
}
