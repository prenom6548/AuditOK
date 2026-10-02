/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
 */

#ifndef MUSICFILEINFORMATION_H
#define MUSICFILEINFORMATION_H

#include <QDir>
#include <QFileInfo>
#include <QByteArray>
#include <QString>
#include <QSqlTableModel>
#include <QMessageBox>
#include <QDebug>
#include <QTime>
#include <QEventLoop>
#include <QProcess>

//taglib
#include <taglib/fileref.h>
#include <taglib/tpropertymap.h>
#include <taglib/tag.h>
#include <taglib/taglib.h>
//#include <taglib/tpropertymap.h>
////#include <taglib/toolkit/tpropertymap.h>
#include "musicDataBase.h"
#include "UIControl/player/coreplayer/mpvcore.h"

enum SearchType {
    None = 0,
    TitleType,
    SingerType,
    AlbumType
};

class MusicFileInformation : public QObject
{
    Q_OBJECT

    friend class unitMainWindow;

public:
    explicit MusicFileInformation(QObject *parent = nullptr);
    static MusicFileInformation& getInstance()
    {
        static MusicFileInformation instance;
        return instance;
    }

    QStringList getMusicType();
    QStringList updateSongInfoFromLocal(const QString &filepath);
    void addFile(const QStringList &addFile);
    int findIndexFromPlayList(QString listname, QString filepath);
    int getCount();
    int getFailCount();
    QPixmap getCoverPhotoPixmap(QString filepath); //获取封面图片
#ifndef WHITE_TEST
    QString processStart(QString cmd, QStringList args); // 开始执行命令
#endif
    QString processComand(const QString &);

    QStringList m_musicType;
    QList<musicDataStruct> resList;

public Q_SLOTS:
    void durationChange(qint64 duration);

Q_SIGNALS:
    void durations();

private:
    MpvCore *music = nullptr;
    QProcess *m_processGetTagInfo = nullptr; // 通过FFmpeg、ffprobe获取音频tag信息
//    QList<musicDataStruct> resList;
    musicDataStruct musicdataStruct;
    QStringList AllDirList;
    QStringList songFiles;
    QString Dir;
    QStringList matchMp3Files;

//    QFileInfo fileInfo;
    QString MD5Str;
    QStringList MD5List;
    QDir matchDir;

    QString fileType(QFileInfo fileInfo);
    QString fileSize(QFileInfo fileInfo);
    bool checkFileIsDamaged(QString filepath);
    QStringList fileInformation(QString filepath);
    QString filterTextCode(QString str);
    int preNum(unsigned char byte);
    bool isValidFtpPath(const QString &filepath);

    qint64 dur;
    bool success = false;
    int musicCount;
    int m_failCount;
};

#endif // MUSICFILEINFORMATION_H
