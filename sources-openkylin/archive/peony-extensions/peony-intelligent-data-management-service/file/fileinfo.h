/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef FILEINFO_H
#define FILEINFO_H

#include <QSharedPointer>
#include <QVariant>

namespace UKUI {
namespace IDM {

class FileInfoPrivate;

class FileInfo
{
    friend class FileInfoJob;
public:
    static QSharedPointer<FileInfo> fromPath(const QString &filePath, bool addWatch = false);
    static QSharedPointer<FileInfo> fromUri(const QString &uri, bool addWatch = false);
    static QSharedPointer<FileInfo> fromUrl(const QUrl &url, bool addWatch = false);

    bool isValid() const;
    QString uri() const;
    QString path() const;
    QString parentPath() const;
    QString displayName() const;
    QString iconName() const;
    QString createDateString() const;
    QString modifyDateString() const;
    QString openDateString() const;
    QString dateString() const;
    QString version() const;
    QString mimeType() const;
    QString fileType() const;
    quint64 timeModifiedMsec() const;
    quint64 timeAccessMsec() const;
    qint64 timeRecentModified() const;
    quint64 size() const;
    QString sizeFormat() const;
    quint64 openCount() const;
    QList<QVariant> labelColors() const;
    void setChecked(bool flag);
    bool isChecked() const;
    ~FileInfo();

private:
    FileInfo();
    QSharedPointer<FileInfo> getInfoByUri(const QString &uri);
    FileInfoPrivate *priv;
};
}
}

#endif // FILEINFO_H
