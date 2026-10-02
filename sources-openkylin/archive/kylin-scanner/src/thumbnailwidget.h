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
#ifndef THUMBNAILWIDGET_H
#define THUMBNAILWIDGET_H

#include "thumbnaildelegate.h"
#include "globalsignal.h"
#include "saneobject.h"
#include "include/common.h"

#include <QWidget>
#include <QListView>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QListData>
#include <QListIterator>
#include <QSize>
#include <QImage>
#include <QPixmap>
#include <QIcon>
#include <QGraphicsDropShadowEffect>
#include <QScrollBar>
#include <QFile>
#include <QFileInfo>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>
#include <QStyle>
#include <QThread>


class ScanStandardItem: public QStandardItem
{
public:
    void setPath(const QString &path);
    QString getPath();

    int getRowCountLocation() const;
    void setRowCountLocation(int value);

private:
    QString m_path;
    int rowCountLocation;

};

class ThumbnailWidget : public QListView
{
    Q_OBJECT
public:
    explicit ThumbnailWidget(QWidget *parent = nullptr);

    static int scanPictureCount;
    static bool scanPictureIsEmpty;
    void setupGui();
    void initConnect();
    void initSettings();
    void themeChange();
    bool isDarkTheme();
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

protected:


private:
    ThumbnailDelegate *m_thumbnailDelegate = nullptr;
    QStandardItemModel *m_thumbnailItemModel = nullptr;

public slots:
    void showThumbnailIcon();

    void showNormalImageAfterScan(QString, QString);
    void showNormalImagesAfterScan(QStringList, QStringList);
    void clickedItemSlot(const QModelIndex &index);

};

#endif // THUMBNAILWIDGET_H
