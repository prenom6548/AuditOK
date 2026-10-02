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
#include "thumbnailwidget.h"
#include <QDebug>
#include <kysdk/applications/gsettingmonitor.h>
#include <CImg.h>
#include "include/theme.h"
#include <QProcess>

int ThumbnailWidget::scanPictureCount = 0;
bool ThumbnailWidget::scanPictureIsEmpty = true;
ThumbnailWidget::ThumbnailWidget(QWidget *parent) : QListView(parent)
{
    setupGui();
    initConnect();
    initSettings();
    themeChange();
}
void ThumbnailWidget::setupGui()
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    bool isPCMode = GlobalUserSignal::getInstance()->getCurrentMode();
    if(isPCMode){
        this->setFixedWidth(ThumbnailWidgetMinimumWidth);
    }else{
        this->setFixedWidth(ThumbnailWidgetTabletMinimumWidth);
    }

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->verticalScrollBar()->setProperty("drawScrollBarGroove",false);

    setDragEnabled(false);
    setSpacing(8);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setIconSize(ThumbnailIconSize);
    setViewMode(QListView::IconMode);
    setMovement(QListWidget::Static);

    setAcceptDrops(false);
    setFocusPolicy(Qt::NoFocus);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void ThumbnailWidget::initConnect()
{

    connect(g_user_signal, &GlobalUserSignal::scanThreadFinishedImageLoadSignal, this, &ThumbnailWidget::showNormalImagesAfterScan);
    connect(this, &ThumbnailWidget::clicked, this, &ThumbnailWidget::clickedItemSlot);
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, &ThumbnailWidget::themeChange);

}

void ThumbnailWidget::themeChange()
{

}

bool ThumbnailWidget::isDarkTheme()
{
    QString systemTheme = kdk::GsettingMonitor::getSystemTheme().toString();

    if (systemTheme == STYLE_NAME_KEY_DARK || systemTheme == STYLE_NAME_KEY_BLACK) {
        return true;
    } else {
        return false;
    }
}

void ThumbnailWidget::resizeEvent(QResizeEvent *event)
{
    bool isPCMode = GlobalUserSignal::getInstance()->getCurrentMode();
    if(isPCMode){
        this->setFixedWidth(ThumbnailWidgetMinimumWidth);
    }else{
        this->setFixedWidth(ThumbnailWidgetTabletMinimumWidth);
    }
    QListView::resizeEvent(event);
}

void ThumbnailWidget::initSettings()
{
    showThumbnailIcon();
}


void ThumbnailWidget::showThumbnailIcon()
{
    m_thumbnailDelegate = new ThumbnailDelegate();
    this->setItemDelegate(m_thumbnailDelegate);

    m_thumbnailItemModel = new QStandardItemModel();

}

void ThumbnailWidget::showNormalImageAfterScan(QString loadPath, QString savePath)
{
    QString scanFullName = loadPath;
    if (g_config_signal->m_kylinScannerImageDebug) {
        scanFullName = g_config_signal->m_openFileName;
    }
    QFileInfo scanFullNameFileInfo(scanFullName);
    QFileInfo saveNameFileInfo(savePath);
    QString tooltipName = saveNameFileInfo.baseName();

    int rowcount = m_thumbnailItemModel->rowCount();
    qDebug() << "rowcount = " << rowcount
             << "scanFullName = " << scanFullName;

    QString newScanFullName = QFileInfo(scanFullName).absolutePath() + "/" +  QFileInfo(scanFullName).baseName() + ".jpg";

    QPixmap iconPix(newScanFullName);

    QPixmap fitpixmap = iconPix.scaled(QSize(40, 40), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    qDebug() << "image mesg:" << fitpixmap;
    ScanStandardItem *item = new ScanStandardItem();
    item->setIcon(QIcon(fitpixmap));
    item->setToolTip(tooltipName);
    item->setPath(newScanFullName);
    item->setRowCountLocation(rowcount);
    m_thumbnailItemModel->insertRow(rowcount, item);

    // Default choose index
    QModelIndex index = m_thumbnailItemModel->index(rowcount, 0);
    this->setCurrentIndex(index);

    scanPictureCount = rowcount;

    this->setModel(m_thumbnailItemModel);

}

void ThumbnailWidget::showNormalImagesAfterScan(QStringList loadPathes, QStringList savepathes)
{
    for(int i = 0; i < loadPathes.length(); i++){
        showNormalImageAfterScan(loadPathes[i], savepathes[i]);
    }
}

void ThumbnailWidget::clickedItemSlot(const QModelIndex &index)
{
    if(g_sane_object->cropFlag == 1){
        g_user_signal->cancleCorpSignal();
    }

     ScanStandardItem *item = dynamic_cast<ScanStandardItem *>(m_thumbnailItemModel->itemFromIndex(index));
     if (item) {
        int scanIconNumber = item->getRowCountLocation();
        QString loadPath = item->getPath();

        qDebug() << "scanIconPath = " << scanIconNumber << loadPath;

        g_user_signal->showImageAfterClickedThumbnail(loadPath);
        g_user_signal->resetNavigatorsignal();
     }
     g_user_signal->exitWindowWithSaveFlag = false;
}


void ScanStandardItem::setPath(const QString &path)
{
    m_path = path;
}

QString ScanStandardItem::getPath()
{
    return m_path;
}

int ScanStandardItem::getRowCountLocation() const
{
    return rowCountLocation;
}

void ScanStandardItem::setRowCountLocation(int value)
{
    rowCountLocation = value;
}
