/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "faninfo.h"
#include "dataworker.h"

#include <QDebug>
#include <QTimer>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QScrollBar>

FanInfo::FanInfo(QWidget *parent)
    : KABaseInfoPage(parent)
{
    m_strInfoJson = "";
    initUI();
    initConnections();
}

void FanInfo::initUI()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setContentsMargins(0,0,0,0);
    m_mainLayout->setSpacing(0);
    m_listLayout = new QVBoxLayout();
    m_listLayout->setContentsMargins(0,0,0,0);
    m_listLayout->setSpacing(0);
    m_listLayout->setAlignment(Qt::AlignTop);
    m_listFrame = new QFrame();
    m_listFrame->setLayout(m_listLayout);
    m_scrollFrame = new QScrollArea();
    m_scrollFrame->setContentsMargins(0, 0, 0, 0);
    m_scrollFrame->setBackgroundRole(QPalette::Base);
    m_scrollFrame->setAutoFillBackground(true);
    m_scrollFrame->setFrameStyle(0);
    m_scrollFrame->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollFrame->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_scrollFrame->setWidgetResizable(true);
    m_scrollFrame->setWidget(m_listFrame);

    m_rkeyMenu = new QMenu(this);

    QAction *copyAction = new QAction(QIcon::fromTheme("edit-copy-symbolic"), tr("Copy"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, this, [=](){KABaseInfoPage::onCopyContent(m_strCopyInfo);});
    m_rkeyMenu->addAction(copyAction);

    // 风扇刷新较快，不适合全选
    // QAction *selectAllAction = new QAction(tr("Select All"), this);
    // selectAllAction->setShortcut(QKeySequence::SelectAll);
    // connect(selectAllAction, &QAction::triggered, this, [=](){KABaseInfoPage::onSelectAllLabel(m_selectedLabel);});
    // m_rkeyMenu->addAction(selectAllAction);

    QAction *copyAllAction = new QAction(tr("Export"), this);
    connect(copyAllAction, &QAction::triggered, this, [=](){KABaseInfoPage::onCopyContent(m_strPageInfo);});
    m_rkeyMenu->addAction(copyAllAction);

    m_mainLayout->addWidget(m_scrollFrame);
    this->setLayout(m_mainLayout);
    m_refreshInfoTimer = new QTimer(this);
}

void FanInfo::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        KALabel *label = qobject_cast<KALabel*>(childAt(event->pos()));
        if (label) {
            // 保存鼠标右键的KAlabel
            m_selectedLabel = label;
        }
        m_rkeyMenu->popup(event->globalPos());
    }
    KABaseInfoPage::mousePressEvent(event);
}

void FanInfo::onUpdateInfo(QString strInfoJson)
{
    m_strInfoJson = strInfoJson;
    qInfo()<<"Fan Info:"<<m_strInfoJson;
    QTimer::singleShot(0, this, [&, this, strInfoJson](){
        this->updateInfoItems(strInfoJson);
        chkNoDevice();
    });
}

void FanInfo::initConnections()
{
    connect(DataWorker::getInstance(), SIGNAL(updateFanInfo(uint,QString)), this, SLOT(onUpdateInfo(uint,QString)));
    connect(m_refreshInfoTimer, &QTimer::timeout, this, &FanInfo::onRefreshInfo);
    m_refreshInfoTimer->start(2000);
}

void FanInfo::clearInfoItems()
{
    QMap<unsigned,QMap<unsigned, QWidget*>>::iterator itMapInfoItems = m_mapInfoItems.begin();
    for (;itMapInfoItems != m_mapInfoItems.end(); itMapInfoItems++) {
        QMap<unsigned, QWidget*>::iterator itInfoItems = itMapInfoItems.value().begin();
        for (;itInfoItems != itMapInfoItems.value().end(); itInfoItems++) {
            if (itInfoItems.value()) {
                delete itInfoItems.value();
            }
        }
        itMapInfoItems.value().clear();
    }
    mapPageInfo.clear();
    vecItems.clear();
    vecMapItems.clear();
    m_mapInfoItems.clear();
}

void FanInfo::addInfoItems(unsigned uGroup, unsigned uId, QString strTitle, QString strContent, bool bMulti)
{
    if (m_mapInfoItems.contains(uGroup)) {
        QMap<unsigned,QMap<unsigned, QWidget*>>::iterator itMapInfoItems = m_mapInfoItems.find(uGroup);
        if (itMapInfoItems != m_mapInfoItems.end()) {
            if (itMapInfoItems.value().contains(uId)) {
                KInfoListItem *listItem = qobject_cast<KInfoListItem *>(itMapInfoItems.value()[uId]);
                if (listItem) {
                    listItem->updateDetailInfo(strContent);
                }
            } else {
                bool isSigleLine = true;
                if (bMulti) {
                    isSigleLine = (itMapInfoItems.value().size()-1)%2;
                } else {
                    isSigleLine = itMapInfoItems.value().size()%2;
                }
                KInfoListItem *listItem = new KInfoListItem(strTitle, "", strContent, isSigleLine);
                listItem->setMinimumHeight(40);
                listItem->setFixedWidth(666);
                m_listLayout->addWidget(listItem);
                itMapInfoItems.value()[uId] = listItem;
                m_strPageInfo.append("|"+strTitle+"|"+strContent+"\n");
                if (vecMapItems.size() <= int(uGroup)){
                    mapPageInfo[QString::number(uId,10)] = strTitle+"|"+strContent;
                    vecMapItems.push_back(mapPageInfo);
                }else{
                    vecMapItems[int(uGroup)][QString::number(uId,10)] = strTitle+"|"+strContent;
                }
                if(vecItems.size()==0){
                    vecItems.push_back(vecIndex);
                }
                if(vecItems.size() == int(uGroup)+1){
                    if (!(std::find(vecIndex.begin(),vecIndex.end(),QString::number(uId,10))!=vecIndex.end())){
                        vecIndex.push_back(QString::number(uId,10));
                    }
                    vecItems[int(uGroup)]=vecIndex;
                }else{
                    vecIndex.clear();
                    vecIndex.push_back(QString::number(uId,10));
                    vecItems.push_back(vecIndex); 
                }
            }
        }
    } else {
        QMap<unsigned, QWidget*> mapItems;
        if (bMulti) {
            KAInfoTitle *infoTitle = new KAInfoTitle(QString("%1%2").arg(tr("Fan")).arg(m_mapInfoItems.size()+1));
            infoTitle->setMinimumHeight(50);
            infoTitle->setFixedWidth(666);
            m_listLayout->addWidget(infoTitle);
            mapItems[FANINFO_TITLE] = infoTitle;
        }
        KInfoListItem *listItem = new KInfoListItem(strTitle, "", strContent, 0);
        listItem->setMinimumHeight(40);
        listItem->setFixedWidth(666);
        m_listLayout->addWidget(listItem);
        mapItems[uId] = listItem;
        m_mapInfoItems[uGroup] = mapItems;
        if (vecMapItems.size() <= int(uGroup)){
            mapPageInfo[QString::number(uId,10)] = strTitle+"|"+strContent;
            vecMapItems.push_back(mapPageInfo);
        }else{
            vecMapItems[int(uGroup)][QString::number(uId,10)] = strTitle+"|"+strContent;
        }
        if(vecItems.size()==0){
            vecItems.push_back(vecIndex);
        }
        if(vecItems.size() == int(uGroup)+1){
            if (!(std::find(vecIndex.begin(),vecIndex.end(),QString::number(uId,10))!=vecIndex.end())){
                vecIndex.push_back(QString::number(uId,10));
            }
            vecItems[int(uGroup)]=vecIndex;
        }else{
            vecIndex.clear();
            vecIndex.push_back(QString::number(uId,10));
            vecItems.push_back(vecIndex); 
        }
    }
   // 利用UID实现有序
   m_strPageInfo.clear();
   for(int j=0;j<vecMapItems.size();j++){
        if(vecMapItems.size()>1){
            m_strPageInfo.append(QString("%1%2").arg(tr("Network Card")).arg(j+1)+"\n");
        }
        std::map<QString, QString> temMap = vecMapItems[j];
        for(int i=0;i<vecItems[j].size();i++){
            if(temMap.count(vecItems[j][i])!=0){
                QString tmpTitle = temMap[vecItems[j][i]].split("|")[0];
                QString tmpContent = temMap[vecItems[j][i]].split("|")[1];
                int length = tmpTitle.length();
                m_strPageInfo.append(tmpTitle);
                for(int i = length; i < KABaseInfoPage::getBlankNum(); i++){
                    m_strPageInfo.append("　");
                }
                m_strPageInfo.append(tmpContent+"\n");
            }
        }
        m_strPageInfo.append("\n");
   }
   m_strPageInfo.chop(1);
}

void FanInfo::updateInfoItems(QString strInfoJson)
{
    if (strInfoJson.isEmpty())
        return;
    qInfo()<<"Cur info:"<<strInfoJson;
    QJsonParseError jsonParseErr;
    QJsonDocument  rootDoc = QJsonDocument::fromJson(strInfoJson.toUtf8(), &jsonParseErr);//字符串格式化为JSON
    if (jsonParseErr.error != QJsonParseError::NoError) {
        qWarning() << "JSON格式错误";
        return;
    } else {
         QJsonObject rootObj = rootDoc.object();
         if (rootObj.isEmpty()) {
             qWarning() << "JSON串为空";
             return;
         }
         QJsonValue valJson = rootObj.value("list");
         if (valJson.isArray()) {
            QJsonArray arrayJson = valJson.toArray();
            clearInfoItems();
            bool bMulti = false;
            if (arrayJson.size() > 1) {
                bMulti = true;
            }
            for (int n = 0; n < arrayJson.size(); n++) {
                if (arrayJson.at(n).isObject()) {
                    QJsonObject subItemObj = arrayJson.at(n).toObject();
                    QJsonValue subItemValue = subItemObj.value(FAI_SPEED);
                    if (subItemValue.isString()) {
                        addInfoItems(n, FANINFO_SPEED, tr("Speed"), subItemValue.toString(), bMulti);
                    }
                }
            }
         }
    }
}

void FanInfo::onRefreshInfo()
{
    DataWorker::getInstance()->getFanInfo();
}

void FanInfo::chkNoDevice()
{
    unsigned infoSize = 0;
    QMap<unsigned,QMap<unsigned, QWidget*>>::iterator itMapInfoItems = m_mapInfoItems.begin();
    for (;itMapInfoItems != m_mapInfoItems.end(); itMapInfoItems++) {
        infoSize += itMapInfoItems.value().size();
    }
    if (infoSize == 0) {
        addInfoItems(0, FANINFO_INVALID, "", tr("Device not exitst or Get Device is Empty"), false);
    }
}

void FanInfo::contextMenuEvent(QContextMenuEvent *event)
{
    KALabel *label = qobject_cast<KALabel*>(childAt(event->pos()));
    // 如果为qlabel则显示选择全部和复制
    if (!label) {
        // 复制的action
        m_rkeyMenu->actions().at(0)->setVisible(false);
        // 全选的action
        m_rkeyMenu->actions().at(1)->setVisible(false);
    }else{
        m_rkeyMenu->actions().at(0)->setVisible(true);
        m_rkeyMenu->actions().at(1)->setVisible(true);
        // 当选中了KAlabel的文字后才显示复制的
        if (label && !label->selectedText().isEmpty()) {
            m_rkeyMenu->actions().at(0)->setEnabled(true);
            // 保存鼠标选中的文字
            m_strCopyInfo = label->selectedText();
        } else {
            m_rkeyMenu->actions().at(0)->setEnabled(false);
        }
    }
}
