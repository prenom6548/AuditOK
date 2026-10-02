/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "clipboard.h"
#include "app_control_manager.h"

#include <QApplication>
#include <QMimeData>

#include <syslog.h>


Clipboard::Clipboard()
    : QObject(nullptr)
{
    mClipboard = QApplication::clipboard();
    if (mClipboard) {
        connect(mClipboard, SIGNAL(dataChanged()), this, SLOT(onClipboardDataChanged()));
    }
    else {
        syslog(LOG_ERR, "[%s] Get system clipboard failed!", __func__);
    }
}

void Clipboard::clear()
{
    if (mClipboard) {
        mClipboard->clear();
    }
}

bool Clipboard::getImage(QVariant &imageData)
{
    if (mClipboard) {
        const QMimeData *clipData = mClipboard->mimeData();
        if (clipData->hasImage()) {
            imageData = clipData->imageData();
            return true;
        }
    }
    return false;
}

void Clipboard::onClipboardDataChanged()
{
    if (mClipboard) {
        const QMimeData *mimeData = mClipboard->mimeData();
        if (!mimeData || mimeData->formats().isEmpty()) {
            return;
        }
        if (mimeData->hasFormat("uos/remote-copy")) {//解决深信服远程桌面问题
            return;
        }
        if (mimeData->hasText()/* && !mimeData->hasUrls()*/) {//urls由Ctrl+V进行文件分享处理，这里只做剪切板的文本内容处理
            QString text = mimeData->text();
            if (!text.isEmpty()) {
                //syslog(LOG_INFO, "Send clipboard to android: '%s'", text.toStdString().c_str());
                AppControlManager::getInstance()->sendClipboardData(text);
            }
        }
    }
}

void Clipboard::onSyncClipboardFromAndroid(const QString &content)
{
    if (!mClipboard) {
        return;
    }

    const QMimeData *mimeData = mClipboard->mimeData();
    if (mimeData && (!mimeData->formats().isEmpty())) {
        if (mimeData->hasText()) {
            if (mimeData->text() == content) {
                return;
            }
        }
    }
    
    //syslog(LOG_INFO, "Set system clipboard: '%s'", content.toStdString().c_str());
    mClipboard->clear();
    mClipboard->setText(content);
}