/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include "chat_search_filter.h"
#include "model/chatmsgmodel.h"

#include <QFileInfo>
#include <QDateTime>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/wait.h>
#include <QProcess>

#define MEM_BUF 1024 * 4

ChatSearchFilter::ChatSearchFilter() {}

ChatSearchFilter::~ChatSearchFilter() {}

/* 按照时间顺序 从小到大 */
bool ChatSearchFilter::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QString dateLeft = source_left.data(ChatMsgModel::ChatMsgRoles::MsgTime).toString();
    QString dateRight = source_right.data(ChatMsgModel::ChatMsgRoles::MsgTime).toString();

    if (dateLeft > dateRight) {
        return false;
    }

    return true;
}


/*
 * pat : all:xx , file : xx , image : xx , link : xx
 */
bool ChatSearchFilter::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    int msgType = index.data(ChatMsgModel::ChatMsgRoles::MsgType).toInt();
    QString chatText = index.data(ChatMsgModel::ChatMsgRoles::MsgContent).toString();
    QString filePath = index.data(ChatMsgModel::ChatMsgRoles::FilePath).toString();
    QString fileName = QFileInfo(filePath).fileName();

    /* 过滤时间 item */
    if (msgType == ChatMsgModel::MessageType::TimeMsg) {
        return false;
    }

    /* 如果过滤的内容为空 , 全部返回 , 不进行过滤 */
    QRegExp a = this->filterRegExp();
    QString pat = a.pattern();
    if (pat.isEmpty()) {
        return true;
    }

    QStringList res = pat.split(':');
    if (res.count() != 2) {
        qDebug() << "Error : filter pattern error";
        return false;
    }

    /* 判断是否为 all */
    if (res.at(0) == QString("all")) {
        if (res.at(1) == QString("")) {
            return true;
        } else {
            if (chatText.contains(res.at(1)) || fileName.contains(res.at(1))) {
                return true;
            }
        }

        return false;
    }

    /* 判断是否为 file */
    if (res.at(0) == QString("file")) {
        if (msgType == ChatMsgModel::MessageType::FileMsg) {
            if (!this->judgeWhetherPicture(filePath)) {
                if (res.at(1) == QString("")) {
                    return true;
                } else {
                    if (fileName.contains(res.at(1))) {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    /* 判断是否为 image */
    if (res.at(0) == QString("image")) {
        if (msgType == ChatMsgModel::MessageType::FileMsg) {
            if (this->judgeWhetherPicture(filePath)) {
                if (res.at(1) == QString("")) {
                    return true;
                } else {
                    if (fileName.contains(res.at(1))) {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    /* 判断是否为 link */
    if (res.at(0) == QString("link")) {
        if (msgType != ChatMsgModel::MessageType::FileMsg) {
            if (this->judgeWhetherLink(chatText)) {
                if (res.at(1) == QString("")) {
                    return true;
                } else {
                    if (chatText.contains(res.at(1))) {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    return false;
}

/*
 * PRO
 * if file is image return true , other return false
 */
bool ChatSearchFilter::judgeWhetherPicture(QString path) const
{
    QString command = QString("/usr/bin/file -i ") + '"' + path + '"';

    QProcess process;
    process.start(command);

    if (!process.waitForFinished()) {
        return false;
    }

    QString ret = process.readAll();
    
    if (ret.contains("image/bmp") || ret.contains("image/jpeg") || ret.contains("image/tiff") || ret.contains("image/png") || ret.contains("image/svg+xml")
        || ret.contains("image/gif") || ret.contains( "image/vnd.microsoft.icon") || ret.contains("application/x-shockwave-flash") || ret.contains("application/octet-stream") 
        || ret.contains("application/vnd.rn-realmedia") || ret.contains("image/x-tga") || ret.contains("video/3gpp2") || ret.contains("video/3gpp") || ret.contains( "video/webm")
        || ret.contains("video/x-ms-asf") || ret.contains("video/x-msvideo") || ret.contains("video/x-flv") || ret.contains("video/x-matroska") || ret.contains("video/mp4")
        || ret.contains( "video/quicktime") || ret.contains("video/mpeg") || ret.contains("video/MP2T")) {
        return true;
    }

    QStringList list;
    list = path.split(".");
    if ((list.at(list.size() - 1) == "ts") || (list.at(list.size() - 1) == "m2t")
        || (list.at(list.size() - 1) == "m2ts")) {
        return true;
    }
    return false;
}

bool ChatSearchFilter::judgeWhetherLink(QString str) const
{
    QString pat("(https?|ftp|file)://[-A-Za-z0-9+&@#/%?=~_|!:,.;]+[-A-Za-z0-9+&@#/%=~_|]");

    QRegExp reg(pat);

    if (reg.exactMatch(str)) {
        return true;
    }

    return false;
}

int ChatSearchFilter::verify_file(char *filename) const
{
	/* Get /etc/passwd entry for current user */
	struct passwd *pwd = getpwuid(getuid());
	if (pwd == NULL) {
		/* Handle error */
		return 0;
	}
	const size_t len = strlen( pwd->pw_dir);
	if (strncmp( filename, pwd->pw_dir, len) != 0) {
		return 0;
	}
	/* Make sure there is only one '/', immediately after homedir */
	if (strrchr( filename, '/') == filename + len) {
		return 1;
	}
	return 0;
}
