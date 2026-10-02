/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "filepathedit.h"

FilePathEdit::FilePathEdit(QWidget *parent /*= nullptr*/) :
    QLineEdit(parent)
{}

FilePathEdit::~FilePathEdit()
{}

void FilePathEdit::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        for (QUrl url : event->mimeData()->urls()) {
            QString file = url.toString();
            if (file.startsWith("file://")) {
                file.replace("file://", "");
                setText(file);
            }
        }
    }
}
