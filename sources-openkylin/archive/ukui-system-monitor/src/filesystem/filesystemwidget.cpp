/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "filesystemwidget.h"
#include "filesysteminfoitem.h"

#include <QDebug>
#include <QAction>
#include <QDesktopServices>
#include <QScrollBar>

#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

#include "../macro.h"

FileSystemWidget::FileSystemWidget(QWidget *parent)
    : QWidget(parent)
    , m_strSearchKey("")
{
    installEventFilter(this);
    initUI();
    // init filesystem list provider
    m_fileSystemWorker = new FileSystemWorker(this);
}

FileSystemWidget::~FileSystemWidget()
{

}

void FileSystemWidget::initUI()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setContentsMargins(12,8,0,8);
    m_mainLayout->setSpacing(0);
    m_listLayout = new QVBoxLayout();
    m_listLayout->setContentsMargins(0,0,0,0);
    m_listLayout->setSpacing(12);
    m_listLayout->setAlignment(Qt::AlignTop);
    m_listFrame = new QFrame();
    m_listFrame->setLayout(m_listLayout);
    m_scrollFrame = new QScrollArea();
    m_scrollFrame->setContentsMargins(0,0,0,0);
    m_scrollFrame->setBackgroundRole(QPalette::Base);
    m_scrollFrame->setAutoFillBackground(true);
    m_scrollFrame->setFrameStyle(0);
    m_scrollFrame->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollFrame->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_scrollFrame->setWidgetResizable(true);
    m_scrollFrame->setWidget(m_listFrame);

    m_contextMenu = new QMenu(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_contextMenu, "FileSystemWidget", "", "");
    m_contextMenu->setObjectName("MonitorMenu");

    m_notFoundPixLabel = new QLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_notFoundPixLabel, "FileSystemWidget", "", "");

    m_notFoundPixLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    m_notFoundPixLabel->setVisible(false);

    const QByteArray idd(THEME_QT_SCHEMA);
    QGSettings *qtSettings  = new QGSettings(idd, QByteArray(), this);
    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode){
        m_notFoundPixLabel->setPixmap(QPixmap(QString(":/img/nosearchresult_black.svg")).scaled(96,96));
    } else {
        m_notFoundPixLabel->setPixmap(QPixmap(QString(":/img/nosearchresult_white.svg")).scaled(96,96));
    }
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "styleName") {
            QString currentThemeMode = qtSettings->get(key).toString();
            if ("ukui-black" == currentThemeMode || "ukui-dark" == currentThemeMode) {
                m_notFoundPixLabel->setPixmap(QPixmap(QString(":/img/nosearchresult_black.svg")).scaled(96,96));
            } else if("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode || "ukui-light" == currentThemeMode) {
                m_notFoundPixLabel->setPixmap(QPixmap(QString(":/img/nosearchresult_white.svg")).scaled(96,96));
            }
        }
    });

    m_notFoundTextLabel = new QLabel(tr("No search result"), this);
    m_notFoundTextLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_notFoundTextLabel->setVisible(false);

    QVBoxLayout *notFoundVLayout = new QVBoxLayout(m_scrollFrame);
    notFoundVLayout->setContentsMargins(0,0,0,0);
    notFoundVLayout->addWidget(m_notFoundPixLabel);
    notFoundVLayout->addWidget(m_notFoundTextLabel);

    m_mainLayout->addWidget(m_scrollFrame);
    this->setLayout(m_mainLayout);
}

void FileSystemWidget::initConnections()
{
    //update model's filesystem list cache on filesystem list updated signal
    m_fileSystemWatcher = FileSystemWatcher::instance();  //it will emit a signal to tell us if the device is added or removed
    connect(m_fileSystemWatcher, &FileSystemWatcher::deviceAdded, this, [=] (const QString &dev) {
        Q_UNUSED(dev);
        this->onUpdateInfo();
    });
    connect(m_fileSystemWatcher, &FileSystemWatcher::deviceRemoved, this, [=] (const QString &dev) {
        Q_UNUSED(dev);
        this->onUpdateInfo();
    });
    auto *refreshAction = new QAction(tr("Refresh"), this);
    connect(refreshAction, &QAction::triggered, this, &FileSystemWidget::onUpdateInfo);
    m_contextMenu->addAction(refreshAction);//刷新
}

void FileSystemWidget::clearInfoItems()
{
    QMap<QString, QWidget*>::iterator itMapInfoItems = m_mapInfoItems.begin();
    for (;itMapInfoItems != m_mapInfoItems.end(); itMapInfoItems++) {
        if (itMapInfoItems.value()) {
            delete itMapInfoItems.value();
        }
    }
    m_mapInfoItems.clear();
}

void FileSystemWidget::addInfoItems(FileSystemData& fsData)
{
    FileSystemInfoItem *infoItem = new FileSystemInfoItem(this);
    KDK_EXTEND_ALL_INFO_FORMAT(infoItem, "FileSystem", fsData.deviceName().split("/").last(), "FileSystemInfoItem of FileSystemWidget");
    infoItem->setFixedHeight(100);
    infoItem->setMinimumWidth(748);
    infoItem->onUpdateInfo(fsData);
    connect(infoItem, &FileSystemInfoItem::dblClicked, this, &FileSystemWidget::onItemDblClicked);
    m_listLayout->addWidget(infoItem);
    m_mapInfoItems[fsData.deviceName()] = infoItem;
}

void FileSystemWidget::onUpdateInfo()
{
    if (m_fileSystemWorker) {
        m_fileSystemWorker->onFileSystemListChanged();

        const QList<QString> &newDevNameList = m_fileSystemWorker->diskDevNameList();
        clearInfoItems();
        m_mapFileSystemData.clear();
        for (const auto &devname : newDevNameList) {
            FileSystemData fsData;
            m_fileSystemWorker->getDiskInfo(devname, fsData);
            if (!m_strSearchKey.isEmpty()) {
                if (!devname.contains(m_strSearchKey) && !fsData.mountDir().contains(m_strSearchKey)
                    && !fsData.diskType().contains(m_strSearchKey))
                    continue;
            }
            m_mapFileSystemData[devname] = fsData;
            addInfoItems(fsData);
            if (fsData.mountDir() == "/") {
                m_diskOccupancy = QString::number((fsData.usedCapacityValue() / fsData.totalCapacityValue())*100, 'f', 1) + "%";
            }
        }
        m_notFoundPixLabel->setVisible(m_mapFileSystemData.size() == 0);
        m_notFoundTextLabel->setVisible(m_mapFileSystemData.size() == 0);
    }
}

QString FileSystemWidget::getDiskOccupancy()
{
    if (m_firstGetOccupancy && m_fileSystemWorker) {
        m_firstGetOccupancy = false;
        m_fileSystemWorker->onFileSystemListChanged(true);

        const QList<QString> &newDevNameList = m_fileSystemWorker->diskDevNameList();
        for (const auto &devname : newDevNameList) {
            FileSystemData fsData;
            m_fileSystemWorker->getDiskInfo(devname, fsData);
            if (fsData.mountDir() == "/") {
                m_diskOccupancy = QString::number((fsData.usedCapacityValue() / fsData.totalCapacityValue())*100, 'f', 1) + "%";
            }
        }
    }

    return m_diskOccupancy;
}

void FileSystemWidget::onSearch(QString searchKey)
{
    if (m_strSearchKey == searchKey || !isVisible())
        return;
    m_strSearchKey = searchKey;
    onUpdateInfo();
}

void FileSystemWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        m_contextMenu->popup(event->globalPos());
    }
    QWidget::mousePressEvent(event);
}

void FileSystemWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
}

void FileSystemWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void FileSystemWidget::onItemDblClicked(QString strMountUrl)
{
    if (strMountUrl.isEmpty())
        return ;
    //open the mount Uri
    QString targetPath = QString("file://%1").arg(strMountUrl);
    QDesktopServices::openUrl(QUrl(targetPath));//xdg-open
}

void FileSystemWidget::onSwitchOnView()
{
    if (m_needFirstRefresh) {
        this->onUpdateInfo();
        initConnections(); // 切换到磁盘界面后才建立连接，刷新数据
        m_needFirstRefresh = false;
    }
}
