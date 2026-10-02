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

#include "gamelistview.h"
#include "sys/syslog.h"

#define GAME_JSON_FILE "/usr/share/kmre/games.json"
GameListView::GameListView(QWidget *parent) : QListWidget(parent)
{
    connect(this, &GameListView::itemClicked, [=] (QListWidgetItem *item) {
            GameListItem *appItem = static_cast<GameListItem *>(itemWidget(item));
            Q_UNUSED(appItem);
            pkgname = appItem->pkgName();
            appname = appItem->appName();
    //        char *pic;
    //        QByteArray ba = pkgname.toLatin1();
    //        pic = ba.data();
    //        syslog(LOG_DEBUG,"####YANG pkgname = :%s",pic);
            emit this->Clickeditem(pkgname,appname);
    });
    this->setAlternatingRowColors(true);
    setStyleSheet("QListWidget{border:none;} QListWidget::item{padding-left:8px;}");
}

void GameListView::removeItem(GameListItem *item)
{
    Q_ASSERT(item);

    //TODO
//    QMapIterator<QString, GameListItem *> i(m_map);
//    while (i.hasNext()) {
//        i.next();
//    }
    m_map.remove(item->pkgName());
    int row = currentRow();
    takeItem(row);
//    m_listLayout->removeWidget(item);
    item->removeEventFilter(this);
    item->setParent(nullptr);
    item->deleteLater();
//    delete item;//Attention: on wayland, it has exception: double free or corruption (out)
//    item = nullptr;
}

void GameListView::removeItemByName(const QString &pkgName)
{
    GameListItem *item = m_map.value(pkgName);
    if (item == Q_NULLPTR) {
        return;
    }

    this->removeItem(item);
}

void GameListView::loadItem(const QString &pkgName, const QString &appName)
{
    GameListItem *item = new GameListItem(0, pkgName, appName);
    addItem(item->getItem());
    item->getItem()->setSizeHint(QSize(400, 36));
    setItemWidget(item->getItem(), item);
    m_map.insert(pkgName, item);

    connect(this, SIGNAL(requestRestoreBtnStatus(QString)), item, SLOT(onRestoreBtnStatus(QString)));
}

void GameListView::onResponseClearGame(const QString &pkgName)
{
//    syslog(LOG_DEBUG,"####YANG onResonseClearGame");
    QDBusInterface interface("cn.kylinos.Kmre.Pref", "/cn/kylinos/Kmre/Pref", "cn.kylinos.Kmre.Pref", QDBusConnection::systemBus());
    QDBusReply<bool> reply = interface.call("removeGameFromWhiteList", pkgName);
    if (reply.isValid()) {
        bool ret = reply.value();
        if (ret) {
            this->removeItemByName(pkgName);
        }
        else {
            emit this->requestRestoreBtnStatus(pkgName);
        }
    }
    else {
        emit this->requestRestoreBtnStatus(pkgName);
    }
}

void GameListView::refreshList()
{
    bool hadItems = false;
    m_map.clear();
    this->clear();
//    m_refreshBtn->setEnabled(false);

    QString content;
    QFile file;
    file.setFileName(GAME_JSON_FILE);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    content = file.readAll();
    file.close();

    QJsonParseError jsonError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(content.toUtf8(), &jsonError);
    if (!jsonDocument.isNull() && (jsonError.error == QJsonParseError::NoError)) {
        if (jsonDocument.isObject()) {
            QJsonObject obj = jsonDocument.object();
            if (obj.contains("games")) {
                QJsonValue arrayVaule = obj.value("games");
                if (arrayVaule.isArray()) {
                    QJsonArray array = arrayVaule.toArray();
                    for (int i = 0;i<array.size();i++) {
                        QJsonValue value = array.at(i);
                        QJsonObject child = value.toObject();
                        QString pkgname = child["pkgname"].toString();
                        QString appname = child["appname"].toString();
                        this->loadItem(pkgname, appname);
                        hadItems = true;
                    }
                }
            }
        }
    }
}
