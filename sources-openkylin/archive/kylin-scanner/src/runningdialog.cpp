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

#include "runningdialog.h"
#include <kysdk/applications/gsettingmonitor.h>
#include "Qt/windowmanage.hpp"

RunningDialog::RunningDialog(QWidget *parent)
    : QDialog(parent)
    , svghandler (new SVGHandler())
    , time (new QTimer())
    , m_closeButton(new QPushButton())
    , m_titleHBoxLayout(new QHBoxLayout())
    , waitImage (new QLabel())
    , waitText (new QLabel())
    , hLayoutInfo (new QHBoxLayout())
    , btnCancel (new CustomPushButton())
    , hLayoutCancel (new QHBoxLayout())
    , vLayout (new QVBoxLayout())
{
    setWindowModality(Qt::ApplicationModal);
    WindowManage::removeHeader(this);


    setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    if (isDarkTheme()) {
        QPalette pal(palette());
        pal.setColor(QPalette::Background, QColor(0, 0, 0));
        setAutoFillBackground(true);
        setPalette(pal);
    } else {
        QPalette pal(palette());
        pal.setColor(QPalette::Background, QColor(255, 255, 255));
        setAutoFillBackground(true);
        setPalette(pal);
    }

    m_closeButton->setIcon(QIcon::fromTheme ("window-close-symbolic"));
    m_closeButton->setToolTip(tr("Close"));
    m_closeButton->setFixedSize(30, 30);
    m_closeButton->setIconSize (QSize(16, 16));
    m_closeButton->setProperty("isWindowButton", 0x2);
    m_closeButton->setProperty("useIconHighlightEffect", 0x8);
    m_closeButton->setFlat(true);

    m_titleHBoxLayout->setSpacing(0);
    m_titleHBoxLayout->addStretch();
    m_titleHBoxLayout->addWidget(m_closeButton);
    m_titleHBoxLayout->setContentsMargins(0, 4, 4, 4);
    m_titleHBoxLayout->setAlignment(Qt::AlignCenter);


    getFileListNum();
    waitImage->show();
    time->start(200);
    count = 0;

    waitImage->setFixedSize(20, 20);
    waitText->setText(waitMsgText);
    hLayoutInfo->setSpacing(0);
    hLayoutInfo->addWidget(waitImage);
    hLayoutInfo->addSpacing(8);
    hLayoutInfo->addWidget(waitText);
    hLayoutInfo->setAlignment(Qt::AlignLeft);
    hLayoutInfo->setContentsMargins(24, 0, 0, 0);

    btnCancel->setFixedSize(100, 36);
    btnCancel->setText(tr("Cancel"));
    hLayoutCancel->setSpacing(0);
    hLayoutCancel->addStretch();
    hLayoutCancel->addWidget(btnCancel);
    hLayoutCancel->setAlignment(Qt::AlignCenter);
    hLayoutCancel->setContentsMargins(248, 0, 24, 0);

    vLayout->setSpacing(0);
    vLayout->addLayout(m_titleHBoxLayout);
    vLayout->addStretch();
    vLayout->addLayout(hLayoutInfo);
    vLayout->addSpacing(32);
    vLayout->addStretch();
    vLayout->addLayout(hLayoutCancel);
    vLayout->addSpacing(24);
    vLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(vLayout);

    connect(time, SIGNAL(timeout()), this, SLOT(showPictures()));
    connect(m_closeButton, &QPushButton::clicked, this, &RunningDialog::accept);
//    connect(btnCancel, SIGNAL(clicked()), this, SLOT(accepted()));
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, &RunningDialog::runningDialogStyleChanged);
}

RunningDialog::RunningDialog(QWidget *parent, QString text)
    : QDialog(parent)
    , svghandler (new SVGHandler())
    , time (new QTimer())
    , m_closeButton(new QPushButton())
    , m_titleHBoxLayout(new QHBoxLayout())
    , waitImage (new QLabel())
    , waitText (new QLabel())
    , hLayoutInfo (new QHBoxLayout())
    , btnCancel (new CustomPushButton())
    , hLayoutCancel (new QHBoxLayout())
    , vLayout (new QVBoxLayout())

{
    WindowManage::removeHeader(this);
    setWindowModality(Qt::ApplicationModal);

    setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    if (isDarkTheme()) {
        QPalette pal(palette());
        pal.setColor(QPalette::Background, QColor(0, 0, 0));
        setAutoFillBackground(true);
        setPalette(pal);
    } else {
        QPalette pal(palette());
        pal.setColor(QPalette::Background, QColor(255, 255, 255));
        setAutoFillBackground(true);
        setPalette(pal);
    }

    m_closeButton->setIcon (QIcon::fromTheme ("window-close-symbolic"));
    m_closeButton->setToolTip(tr("Close"));
    m_closeButton->setFixedSize(30, 30);
    m_closeButton->setIconSize (QSize(16, 16));
    m_closeButton->setProperty("isWindowButton", 0x2);
    m_closeButton->setProperty("useIconHighlightEffect", 0x8);
    m_closeButton->setFlat(true);

    m_titleHBoxLayout->setSpacing(0);
    m_titleHBoxLayout->addStretch();
    m_titleHBoxLayout->addWidget(m_closeButton);
    m_titleHBoxLayout->setContentsMargins(0, 4, 4, 4);
    m_titleHBoxLayout->setAlignment(Qt::AlignCenter);


    getFileListNum();
    waitImage->show();
    time->start(200);
    count = 0;

    waitImage->setFixedSize(20, 20);
    waitText->setText(text);
    hLayoutInfo->setSpacing(0);
    hLayoutInfo->addWidget(waitImage);
    hLayoutInfo->addSpacing(8);
    hLayoutInfo->addWidget(waitText);
    hLayoutInfo->setAlignment(Qt::AlignLeft);
    hLayoutInfo->setContentsMargins(24, 0, 0, 0);

    btnCancel->setFixedSize(100, 36);
    btnCancel->setText(tr("Cancel"));
    hLayoutCancel->setSpacing(0);
    hLayoutCancel->addStretch();
    hLayoutCancel->addWidget(btnCancel);
    hLayoutCancel->setAlignment(Qt::AlignCenter);
    hLayoutCancel->setContentsMargins(248, 0, 24, 0);


    vLayout->setSpacing(0);
    vLayout->addLayout(m_titleHBoxLayout);
    vLayout->addStretch();
    vLayout->addLayout(hLayoutInfo);
    vLayout->addSpacing(32);
    vLayout->addStretch();
    vLayout->addLayout(hLayoutCancel);
    vLayout->addSpacing(24);
    vLayout->setContentsMargins(0, 0, 0, 0);


    setLayout(vLayout);

    connect(time, SIGNAL(timeout()), this, SLOT(showPictures()));
    connect(m_closeButton, &QPushButton::clicked, this, &RunningDialog::reject);
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, &RunningDialog::runningDialogStyleChanged);
}

bool RunningDialog::isDarkTheme()
{
    QString systemTheme = kdk::GsettingMonitor::getSystemTheme().toString();

    if (systemTheme == STYLE_NAME_KEY_DARK || systemTheme == STYLE_NAME_KEY_BLACK) {
        return true;
    } else {
        return false;
    }
}

void RunningDialog::getFileListNum()
{
    path = ":/loading/";
    num = GetFileList(path).size();
    count = 0;
}

QFileInfoList RunningDialog::GetFileList(QString path)
{
    QDir dir(path);
    QFileInfoList file_list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (int i = 0; i != folder_list.size(); i++) {
        QString name = folder_list.at(i).absoluteFilePath();
        QFileInfoList child_file_list = GetFileList(name);
        file_list.append(child_file_list);
    }

    return file_list;
}

void RunningDialog::setWaitText(QString text)
{
    waitText->setText(text);
}

void RunningDialog::disconnectCancelButton()
{
    disconnect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

void RunningDialog::hideCancelButton()
{
    btnCancel->hide();
}

void RunningDialog::runningDialogStyleChanged()
{
    if (isDarkTheme()) {
        QPalette pal(palette());
        pal.setColor(QPalette::Background, QColor(0, 0, 0));
        setAutoFillBackground(true);
        setPalette(pal);

    } else {
        QPalette pal(palette());
        pal.setColor(QPalette::Background, QColor(255, 255, 255));
        setAutoFillBackground(true);
        setPalette(pal);
    }
}

void RunningDialog::m_closeButtondisconnect()
{
    disconnect(m_closeButton, &QPushButton::clicked, this, &RunningDialog::reject);
}

void RunningDialog::showPictures()
{
    QImage image;
    fileinfo = GetFileList(path).at(count);
    waitImage->setScaledContents(true);
    if (!isDarkTheme()) {
        waitImage->setPixmap(svghandler->loadSvgColor(fileinfo.filePath(), "white", 30));
    } else {
        waitImage->setPixmap(svghandler->loadSvgColor(fileinfo.filePath(), "black", 30));
    }
    ++count;
    if (count == num)
        count = 0;
}
