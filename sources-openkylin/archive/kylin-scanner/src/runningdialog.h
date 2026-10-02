/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#ifndef RUNNINGDIALOG_H
#define RUNNINGDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QFileInfoList>
#include <QString>
#include <QTimer>
#include <QDir>

#include "include/common.h"
#include "include/theme.h"
#include "svghandler.h"
#include "custom_push_button.h"

#define WINDOW_WIDTH    380
#define WINDOW_HEIGHT  162

class RunningDialog : public QDialog
{
    Q_OBJECT
public:
    CustomPushButton *btnCancel = nullptr;
    QPushButton *m_closeButton = nullptr;
    explicit RunningDialog(QWidget *parent = nullptr);
    explicit RunningDialog(QWidget *parent = nullptr, QString text="");

    void getFileListNum();
    QFileInfoList GetFileList(QString path);
    void setWaitText(QString text);
    void disconnectCancelButton();
    void hideCancelButton();
    bool isDarkTheme();

private:
    int num = 0;
    int count = 0;

    SVGHandler *svghandler = nullptr;

    QFileInfo fileinfo;
    QString path;
    QTimer *time = nullptr;

    QHBoxLayout *m_titleHBoxLayout = nullptr;

    QLabel *waitImage = nullptr;
    QLabel *waitText = nullptr;
    QHBoxLayout *hLayoutInfo = nullptr;

    QString waitMsgText;

    QHBoxLayout *hLayoutCancel = nullptr;

    QVBoxLayout *vLayout = nullptr;



public slots:
    void runningDialogStyleChanged();
    void m_closeButtondisconnect();
    void showPictures();

};

#endif // RUNNINGDIALOG_H
