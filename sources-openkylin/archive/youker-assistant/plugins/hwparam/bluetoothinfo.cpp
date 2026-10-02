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

#include "bluetoothinfo.h"
#include "dataworker.h"
#include "../../src/util.h"

#include <QDebug>
#include <QTimer>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QScrollBar>
#include <QScreen>
#include <QGuiApplication>
#include "ukuistylehelper/ukuistylehelper.h"

BluetoothInfo::BluetoothInfo(QWidget *parent)
    : KABaseInfoPage(parent)
{
    m_strInfoJson = "";
    initUI();
    initConnections();
}

void BluetoothInfo::initUI()
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

void BluetoothInfo::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        m_rkeyMenu->popup(event->globalPos());
        KALabel *label = qobject_cast<KALabel*>(childAt(event->pos()));
        if (label) {
            // 保存鼠标右键的KAlabel
            m_selectedLabel = label;
        }
    }
    KABaseInfoPage::mousePressEvent(event);
}

void BluetoothInfo::onUpdateInfo(QString strInfoJson)
{
    m_strInfoJson = strInfoJson;
    qInfo()<<"Bluetooth Info:"<<m_strInfoJson;
    QTimer::singleShot(0, this, [&, this, strInfoJson](){
        this->updateInfoItems(strInfoJson);
        this->chkNoDevice();
    });
}

void BluetoothInfo::initConnections()
{
    connect(DataWorker::getInstance(), SIGNAL(updateBluetoothInfo(uint,QString)), this, SLOT(onUpdateInfo(uint,QString)));
}

void BluetoothInfo::clearInfoItems()
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

void BluetoothInfo::addInfoItems(unsigned uGroup, unsigned uId, QString strTitle, QString strContent, bool bMulti)
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
            KAInfoTitle *infoTitle = new KAInfoTitle(QString("%1%2").arg(tr("Bluetooth")).arg(m_mapInfoItems.size()+1));
            infoTitle->setMinimumHeight(50);
            infoTitle->setFixedWidth(666);
            m_listLayout->addWidget(infoTitle);
            mapItems[BLUETOOTHINFO_TITLE] = infoTitle;
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
            m_strPageInfo.append(QString("%1%2").arg(tr("Bluetooth")).arg(j+1)+"\n");
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

void BluetoothInfo::updateInfoItems(QString strInfoJson)
{
    if (strInfoJson.isEmpty())
        return;
    // m_rkeyMenu->clear();
    if(KABaseInfoPage::bluetoothExist() && !Util::judgmentHW()){
        // 禁/启用状态判断
        bool needAdd = true;
        if(KABaseInfoPage::bluetoothStatus().contains("disable")){
            QAction *enableAction = new QAction(tr("Enable "), this);
            connect(enableAction, &QAction::triggered, this, [=](){enableBluetooth();});
            QList<QAction *> actList = m_rkeyMenu->actions();

            for(int i = 0; i < actList.length(); i++){
                if((tr("Enable "))==actList[i]->text()){
                    needAdd = false;
                }
                if((tr("Disable "))==actList[i]->text()){
                    m_rkeyMenu->removeAction(actList[i]);
                }
            }
            if(needAdd){
                m_rkeyMenu->addAction(enableAction);
            }
        }else{
            QAction *disableAction = new QAction(tr("Disable "), this);
            connect(disableAction, &QAction::triggered, this, [=](){disableBluetooth();});
            QList<QAction *> actList = m_rkeyMenu->actions();

            for(int i = 0; i < actList.length(); i++){
                if((tr("Disable "))==actList[i]->text()){
                    needAdd = false;
                }
                if((tr("Enable "))==actList[i]->text()){
                    m_rkeyMenu->removeAction(actList[i]);
                }
            }
            if(needAdd){
                m_rkeyMenu->addAction(disableAction);
            }
        }
    }
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
                    QJsonValue subItemValue = subItemObj.value(BLI_BUSADDR);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_BUSADDR, tr("Bus Address"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_FUNCTION);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_FUNCTION, tr("Function"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_FREQ);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_FREQ, tr("Frequency"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_CONF);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_CONF, tr("Configuration"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_DEVTYPE);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_DEVTYPE, tr("Type"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_ID);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_ID, tr("ID"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_DEVMODEL);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_DEVMODEL, tr("Model"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_RESOURCE);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_RESOURCE, tr("Resource"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_MANUFACTURER);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_MANUFACTURER, tr("Manufacturer"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_DEVVERSION);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_DEVVERSION, tr("Version"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_DATAWIDTH);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_DATAWIDTH, tr("Data Width"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_NAME);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_NAME, tr("Name"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_DRIVER);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_DRIVER, tr("Driver"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_SPEED);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_SPEED, tr("Speed"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_SERIALNUM);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_SERIALNUM, tr("Serial Number"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_ADDRESS);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_ADDRESS, tr("Address"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_LINKMODE);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_LINKMODE, tr("Link Mode"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_LINKPOLICY);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_LINKPOLICY, tr("Link Policy"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_CAPABILITIES);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_CAPABILITIES, tr("Capabilities"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_BUS);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_BUS, tr("Bus"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_SCOMTU);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_SCOMTU, tr("SCO MTU"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_ALCMTU);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_ALCMTU, tr("ACL MTU"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_PACKETTYPE);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_PACKETTYPE, tr("Packet Type"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(BLI_FEATURES);
                    if (subItemValue.isString()) {
                        addInfoItems(n, BLUETOOTHINFO_FEATURES, tr("Features"), subItemValue.toString(), bMulti);
                    }
                }
            }
         }
    }
}

void BluetoothInfo::onRefreshInfo()
{
    DataWorker::getInstance()->getBluetoothInfo();
}

void BluetoothInfo::chkNoDevice()
{
    unsigned infoSize = 0;
    QMap<unsigned,QMap<unsigned, QWidget*>>::iterator itMapInfoItems = m_mapInfoItems.begin();
    for (;itMapInfoItems != m_mapInfoItems.end(); itMapInfoItems++) {
        infoSize += itMapInfoItems.value().size();
    }
    if (infoSize == 0) {
        QList<QAction *> actList = m_rkeyMenu->actions();
        // 0表示禁用，1表示启用，2表示启用中
        int btStatus = 1;
        for(int i = 0;i<actList.length();i++){
            if((tr("Enable "))==actList[i]->text()){
                btStatus = 0;
                break;
            }else if ((tr("Disable "))==actList[i]->text())
            {
                btStatus = 2;
                break;
            }
        }
        if(0 == btStatus || 2 == btStatus){
            if(!KABaseInfoPage::bluetoothExist()){
                addInfoItems(0, BLUETOOTHINFO_INVALID, "", tr("Device not exitst or Get Device is Empty"), false);
            }else{
                addInfoItems(0, BLUETOOTHINFO_INVALID, "", tr("Bluetooth Disable"), false);
            }
        }else{
            addInfoItems(0, BLUETOOTHINFO_INVALID, "", tr("Device not exitst or Get Device is Empty"), false);
        }
        
    }
}

void BluetoothInfo::enterLoadingPage()
{
    m_scrollFrame->hide();
    m_loadWidget = new LoadWidget(m_strDevStatus);
    m_loadWidget->setModal(true);
    m_loadWidget->show();
    m_loadWidget->move(this->window()->frameGeometry().topLeft() + this->window()->rect().center() - m_loadWidget->rect().center());
    m_refreshInfoTimer->stop();
}

void BluetoothInfo::exitLoadingPage()
{
    m_scrollFrame->show();
    m_loadWidget->close();
    m_refreshInfoTimer->stop();
}

void BluetoothInfo::disableBluetooth(){
    m_strInfoLastJson = m_strInfoJson;
    m_strDevStatus = "disable";
    if(nullptr == m_refreshInfoTimer){
        m_refreshInfoTimer =  new QTimer();
    }
    enterLoadingPage();
    DataWorker::getInstance()->disableBluetooth();
    QObject::connect(m_refreshInfoTimer, &QTimer::timeout, [&]() {
        if("" == m_strDevStatus || nullptr == m_strDevStatus || m_strInfoLastJson != m_strInfoJson ){
            exitLoadingPage();
            m_strInfoLastJson = m_strInfoJson;
            m_strDevStatus = "";
        }
        onRefreshInfo();
    });
    m_refreshInfoTimer->start(1000); // Start timer with 1 second interval
}

void BluetoothInfo::enableBluetooth(){
    m_strInfoLastJson = m_strInfoJson;
    m_strDevStatus = "enable";
    if(nullptr == m_refreshInfoTimer){
        m_refreshInfoTimer =  new QTimer();
    }
    enterLoadingPage();
    DataWorker::getInstance()->enableBluetooth();
    QObject::connect(m_refreshInfoTimer, &QTimer::timeout, [&]() {
        if("" == m_strDevStatus || nullptr == m_strDevStatus || m_strInfoLastJson != m_strInfoJson ){
            exitLoadingPage();
            m_strInfoLastJson = m_strInfoJson;
            m_strDevStatus = "";
        }
        onRefreshInfo();  
    });
    m_refreshInfoTimer->start(1000); // Start timer with 1 second interval
    
}

void BluetoothInfo::contextMenuEvent(QContextMenuEvent *event)
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
