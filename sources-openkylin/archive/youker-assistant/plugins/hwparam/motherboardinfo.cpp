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

#include "motherboardinfo.h"
#include "dataworker.h"
#include "../../src/util.h"
#include "../../src/commondef.h"


#include <QDebug>
#include <QTimer>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QScrollBar>
#include <libkydate.h>
#include <QDBusInterface>


MotherBoardInfo::MotherBoardInfo(QWidget *parent)
    : KABaseInfoPage(parent)
{
    m_strInfoJson = "";
    initUI();
    initConnections();
}

void MotherBoardInfo::initUI()
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

    QAction *selectAllAction = new QAction(tr("Select All"), this);
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    connect(selectAllAction, &QAction::triggered, this, [=](){KABaseInfoPage::onSelectAllLabel(m_selectedLabel);});
    m_rkeyMenu->addAction(selectAllAction);

    QAction *copyAllAction = new QAction(tr("Export"), this);
    connect(copyAllAction, &QAction::triggered, this, [=](){KABaseInfoPage::onCopyContent(m_strPageInfo);});
    m_rkeyMenu->addAction(copyAllAction);

    m_mainLayout->addWidget(m_scrollFrame);
    this->setLayout(m_mainLayout);
}

void MotherBoardInfo::mousePressEvent(QMouseEvent *event)
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

void MotherBoardInfo::onUpdateInfo(QString strInfoJson)
{
    m_strInfoJson = strInfoJson;
    qInfo()<<"MotherBoardInfo Info:"<<m_strInfoJson;
    QTimer::singleShot(0, this, [&, this, strInfoJson](){
        this->updateInfoItems(strInfoJson);
        this->chkNoDevice();
    });
}

void MotherBoardInfo::initConnections()
{
    QDBusInterface *  iface = new QDBusInterface(   KYLIN_KYSDK_DATESERVER_SERVICE,
                                                    KYLIN_KYSDK_DATESERVER_PATH,
                                                    KYLIN_KYSDK_DATESERVER_INTERFACE,
                                                    QDBusConnection::sessionBus(),
                                                    this);
    connect(iface, SIGNAL(DateSigna(uint,QString)), this, SLOT(onUpdateInfo(uint,QString)));
    connect(DataWorker::getInstance(), SIGNAL(updateMotherBoardInfo(uint,QString)), this, SLOT(onRefreshInfo()));
}

void MotherBoardInfo::clearInfoItems()
{
    QMap<unsigned, KInfoListItem*>::iterator itInfoItem = m_mapItem.begin();
    for (; itInfoItem != m_mapItem.end(); itInfoItem++) {
        if (itInfoItem.value()) {
            delete itInfoItem.value();
        }
    }
    m_mapItem.clear();
}

void MotherBoardInfo::addInfoItems(unsigned uId, QString strTitle, QString strContent)
{
    if(strContent.isEmpty())
        return;
    if (m_mapItem.contains(uId)) {
        m_mapItem[uId]->updateDetailInfo(strContent);
    } else {
        KInfoListItem *listItem = new KInfoListItem(strTitle, "", strContent, m_mapItem.size()%2);
        listItem->setMinimumHeight(40);
        listItem->setFixedWidth(666);
        m_listLayout->addWidget(listItem);
        m_mapItem[uId] = listItem;
        mapPageInfo.insert(std::pair<QString, QString>( QString::number(uId,10),strTitle+"|"+strContent));
        if (!(std::find(vecIndex.begin(),vecIndex.end(),QString::number(uId,10))!=vecIndex.end())){
            vecIndex.push_back(QString::number(uId,10));
        }
    }
    // 利用UID实现有序
    m_strPageInfo.clear();
    for(int i=0;i<vecIndex.size();i++){
        if(mapPageInfo.count(vecIndex[i])!=0){
            QString tmpTitle = mapPageInfo[vecIndex[i]].split("|")[0];
            QString tmpContent = mapPageInfo[vecIndex[i]].split("|")[1];
            int length = tmpTitle.length();
            m_strPageInfo.append(tmpTitle);
            for(int i = length; i < KABaseInfoPage::getBlankNum(); i++){
                m_strPageInfo.append("　");
            }
            m_strPageInfo.append(tmpContent+"\n");
        }
    }
}

void MotherBoardInfo::updateInfoItems(QString strInfoJson)
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
         clearInfoItems();
         QJsonValue valJson = rootObj.value(MBI_NAME);
         if (valJson.isString()) {
             addInfoItems(MOTHERBOARDINFO_NAME, tr("Name"), valJson.toString());
         }
         valJson = rootObj.value(MBI_MANUFACTURER);
         if (valJson.isString()) {
             addInfoItems(MOTHERBOARDINFO_MANUFACTURER, tr("Manufacturer"), valJson.toString());
         }
         valJson = rootObj.value(MBI_VERSION);
         if (valJson.isString()) {
             addInfoItems(MOTHERBOARDINFO_VERSION, tr("Version"), valJson.toString());
         }
         valJson = rootObj.value(MBI_CHIPSET);
         if (valJson.isString()) {
             addInfoItems(MOTHERBOARDINFO_CHIPSET, tr("Chipset"), valJson.toString());
         }
         valJson = rootObj.value(MBI_SERIALNUM);
         if (valJson.isString()) {
             addInfoItems(MOTHERBOARDINFO_SERIALNUM, tr("Serial Num"), valJson.toString());
         }
         valJson = rootObj.value(MBI_PUBDATE);
         if (valJson.isString()) {
            QString publishDateStr = valJson.toString();
            if(Util::isValidDateFormat(publishDateStr)){
                char* publishDateChar;
                QByteArray ba = publishDateStr.toLatin1();
                publishDateChar = ba.data();
                char* convertDateChar = kdk_system_gjx_time(publishDateChar);
                addInfoItems(MOTHERBOARDINFO_PUBDATE, tr("Publish Date"), QString::fromUtf8(convertDateChar));
            }else{
                addInfoItems(MOTHERBOARDINFO_PUBDATE, tr("Publish Date"), publishDateStr);
            }

         }
         valJson = rootObj.value(MBI_BIOSMANUFACTURER);
         if (valJson.isString()) {
             addInfoItems(MOTHERBOARDINFO_BIOSMANUFACTURER, tr("BIOS Manufacturer"), valJson.toString());
         }
         valJson = rootObj.value(MBI_BIOSVERSION);
         if (valJson.isString()) {
             addInfoItems(MOTHERBOARDINFO_BIOSVERSION, tr("BIOS Version"), valJson.toString());
         }
    }
}

void MotherBoardInfo::onRefreshInfo()
{
    DataWorker::getInstance()->getMotherBoardInfo();
}

void MotherBoardInfo::chkNoDevice()
{
    if (m_mapItem.size() == 0) {
        addInfoItems(MOTHERBOARDINFO_INVALID, "", tr("Device not exitst or Get Device is Empty"));
    }
}

void MotherBoardInfo::contextMenuEvent(QContextMenuEvent *event)
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
