/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
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

#include "appview.h"
#include "appitem.h"
#include "utils.h"

#include <QListWidgetItem>
#include <QMouseEvent>
#include <QScrollBar>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QFile>
#include <QDebug>

AppView::AppView(QWidget *parent) 
    : QListWidget(parent)
{
    this->setMouseTracking(true);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setFixedSize(540, 520);
    this->clear();
    this->verticalScrollBar()->setValue(0);
    this->setAlternatingRowColors(true);
    connect(this, SIGNAL(rightClick(QPoint)), this, SLOT(onRightClick(QPoint)));
    connect(this, &AppView::itemDoubleClicked, [=] (QListWidgetItem *item) {
        AppItem *appItem = static_cast<AppItem *>(itemWidget(item));
        Q_UNUSED(appItem);
    });
}

void AppView::reloadItems(const std::vector<std::pair<QString, QString>> &appList, const QStringList& enableList)
{
    clear();

    for (const auto &app : appList) {
        loadItem(app.second, app.first, enableList.contains(app.first));
    }

    if (count() == 0) {
        emit sigClear();
    }
}

void AppView::loadItem(const QString &appName, const QString &pkgName, bool checked)
{
    AppItem *item = new AppItem();
    item->setInfo(appName, pkgName, checked);
    connect(item, SIGNAL(checkedChanged(QString,bool)), this, SIGNAL(sigCheckedChanged(QString,bool)));
    connect(item, SIGNAL(enter()), this, SLOT(handleEnter()));
    addItem(item->getItem());
    item->getItem()->setSizeHint(QSize(100, 46));
    setItemWidget(item->getItem(), item);
}

void AppView::onRightClick(QPoint pos)
{
    m_rightSelectItem = itemAt(pos);
    if (m_rightSelectItem != 0) {
        //qDebug() << this->mapToGlobal(pos);
    }
}

void AppView::deleteItem()
{
    if (m_rightSelectItem != 0) {
        AppItem *appItem = static_cast<AppItem *>(itemWidget(m_rightSelectItem));
        Q_UNUSED(appItem);
        delete takeItem(row(m_rightSelectItem));
    }
}

void AppView::handleEnter()
{
//    for (int i = 0; i < count(); i++) {
//        QListWidgetItem* matchItem = item(i);
//        TrayAppItem *appItem = static_cast<TrayAppItem *>(itemWidget(matchItem));

//        if (appItem->getPkgName() == ((TrayAppItem*) sender())->getPkgName()) {
//            appItem->highlight();
//        } else {
//            appItem->unhighlight();
//        }
//    }
}

void AppView::selectItemWithPkgName(QString pkgName)
{
    for(int i = 0; i < count(); i++) {
        QListWidgetItem* matchItem = item(i);
        AppItem *appItem = static_cast<AppItem *>(itemWidget(matchItem));

        if (appItem->getPkgName() == pkgName) {
            setCurrentItem(matchItem);
            break;
        }
    }
}

void AppView::mousePressEvent(QMouseEvent *event)
{
    QListWidget::mousePressEvent(event);

    if (event->button() == Qt::RightButton){
        emit rightClick(event->pos());
    }
}
