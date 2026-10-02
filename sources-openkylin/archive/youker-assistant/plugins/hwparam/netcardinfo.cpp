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

#include "netcardinfo.h"
#include "dataworker.h"
#include "../../src/util.h"
#include "ukuistylehelper/ukuistylehelper.h"

#include <QDebug>
#include <QTimer>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QScrollBar>
#include <QScreen>
#include <QGuiApplication>
#include <kysdk/kysdk-base/kyutils.h>

NetCardInfo::NetCardInfo(QWidget *parent)
    : KABaseInfoPage(parent)
{
    m_strInfoJson = "";
    m_strInfoLastJson = "";
    initUI();
    initConnections();
}

void NetCardInfo::initUI()
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
    m_refreshInfoTimer = new QTimer(this);
}

void NetCardInfo::mousePressEvent(QMouseEvent *event)
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

void NetCardInfo::onUpdateInfo(QString strInfoJson)
{
    m_strInfoJson = strInfoJson;
    qInfo()<<"NetCardInfo Info:"<<m_strInfoJson;
    QTimer::singleShot(0, this, [&, this, strInfoJson](){
        this->updateInfoItems(strInfoJson);
        this->chkNoDevice();
    });
}

void NetCardInfo::initConnections()
{
    connect(DataWorker::getInstance(), SIGNAL(updateNetworkCardInfo(uint,QString)), this, SLOT(onUpdateInfo(uint,QString)));
}

void NetCardInfo::clearInfoItems()
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

void NetCardInfo::addInfoItems(unsigned uGroup, unsigned uId, QString strTitle, QString strContent, bool bMulti)
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
            KAInfoTitle *infoTitle = new KAInfoTitle(QString("%1%2").arg(tr("Network Card")).arg(m_mapInfoItems.size()+1));
            infoTitle->setMinimumHeight(50);
            infoTitle->setFixedWidth(666);
            m_listLayout->addWidget(infoTitle);
            mapItems[NETCARDINFO_TITLE] = infoTitle;
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

void NetCardInfo::updateInfoItems(QString strInfoJson)
{
    if (strInfoJson.isEmpty())
        return;
    QStringList disableList = NetCardInfo::netCardStatus();
    if(disableList.size() > 0 ){
        for(int i = 0 ; i < disableList.size() ; i++){
            if(disableList[i].contains("|")){
                QString strProduct = disableList[i].split("|")[0];
                QString strPciPath = disableList[i].split("|")[1];
                if(!strProduct.isEmpty()){
                    bool needAdd = true;
                    QAction *disableAction = new QAction(tr("Enable ") + strProduct, this);
                    qDebug()<<Q_FUNC_INFO;
                    qDebug()<<strProduct;
                    connect(disableAction, &QAction::triggered, this, [=](){NetCardInfo::enableNetCard(disableList[i]);});
                    QList<QAction *> actList = m_rkeyMenu->actions();
                    for(int i = 0 ; i < actList.length() ; i++){
                        if((tr("Enable ") + strProduct) == actList[i]->text()){
                            needAdd = false;
                        }
                        if((tr("Disable ") + strProduct) == actList[i]->text()){
                            m_rkeyMenu->removeAction(actList[i]);
                        }
                    }
                    if(needAdd){
                        m_rkeyMenu->addAction(disableAction);
                    }
//                        addInfoItems(0, VOICECARDINFO_NAME, tr("Name"), strProduct, true);
                    // onRefreshInfo();
                }
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
                    QJsonValue subItemValue = subItemObj.value(NWI_NAME);
                    if (subItemValue.isString()) {
                        QString strTitle= tr("Name");
                        QString strContent= subItemValue.toString();
                            if(tr("Name")==strTitle && !Util::judgmentHW()){
                                bool needAdd = true;
                                QAction *disableAction = new QAction(tr("Disable ")+strContent, this);
                                connect(disableAction, &QAction::triggered, this, [=](){NetCardInfo::disableNetCard(strContent);});
                                QList<QAction *> actList = m_rkeyMenu->actions();

                                for(int i=0;i<actList.length();i++){
                                    if((tr("Disable ")+strContent)==actList[i]->text()){
                                        needAdd = false;
                                    }
                                    if((tr("Enable ")+strContent)==actList[i]->text()){
                                        m_rkeyMenu->removeAction(actList[i]);
                                    }
                                }
                                if(needAdd){
                                    m_rkeyMenu->addAction(disableAction);
                                }
                            }
                        addInfoItems(n, NETCARDINFO_NAME, tr("Name"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(NWI_TYPE);
                    if (subItemValue.isString()) {
                        QString strType = subItemValue.toString();
                        if (strType.contains("Ethernet", Qt::CaseInsensitive)) {
                            addInfoItems(n, NETCARDINFO_ISWIFI, tr("Type"), tr("Wired"), bMulti);
                        } else if (strType.contains("Wireless", Qt::CaseInsensitive)) {
                            addInfoItems(n, NETCARDINFO_ISWIFI, tr("Type"), tr("Wireless"), bMulti);
                        } else {
                            addInfoItems(n, NETCARDINFO_ISWIFI, tr("Type"), strType, bMulti);
                        }
                    }
                    subItemValue = subItemObj.value(NWI_MANUFACTURER);
                    if (subItemValue.isString()) {
                        addInfoItems(n, NETCARDINFO_MANUFACTURER, tr("Manufacturer"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(NWI_BUSADDR);
                    if (subItemValue.isString()) {
                        addInfoItems(n, NETCARDINFO_BUSADDR, tr("Bus Address"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(NWI_MACADDR);
                    if (subItemValue.isString()) {
                        addInfoItems(n, NETCARDINFO_MACADDR, tr("MAC"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(NWI_DRIVER);
                    if (subItemValue.isString()) {
                        addInfoItems(n, NETCARDINFO_DRIVER, tr("Driver"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(NWI_CONNSPEED);
                    if (subItemValue.isString()) {
                        addInfoItems(n, NETCARDINFO_CONNSPEED, tr("Link Speed"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(NWI_MTU);
                    if (subItemValue.isString()) {
                        addInfoItems(n, NETCARDINFO_MTU, tr("MTU"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(NWI_IPADDR);
                    if (subItemValue.isString()) {
                        addInfoItems(n, NETCARDINFO_IPADDR, tr("IP"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(NWI_MODEL);
                    if (subItemValue.isString()) {
                        addInfoItems(n, NETCARDINFO_MODEL, tr("Model"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(NWI_SUBNETMASK);
                    if (subItemValue.isString()) {
                        addInfoItems(n, NETCARDINFO_SUBNETMASK, tr("Subnet Mask"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(NWI_GATEWAY);
                    if (subItemValue.isString()) {
                        addInfoItems(n, NETCARDINFO_GATEWAY, tr("Gateway"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(NWI_DNSSERVER);
                    if (subItemValue.isString()) {
                        addInfoItems(n, NETCARDINFO_DNSSERVER, tr("DNS Server"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(NWI_RECVBYTES);
                    if (subItemValue.isString()) {
                        QString data = subItemValue.toString();
                        char result_data[128] = {0};
                        KDKVolumeBaseType unit = Util::getVolumeUnit(data);

                        char* volume = Util::getVolume(data,unit);
                        if(kdkVolumeBaseCharacterConvert(volume, unit, result_data) == KDK_NOERR){
                            addInfoItems(n, NETCARDINFO_RECVBYTES, tr("Bytes Received"), QString::fromUtf8(result_data), bMulti);
                        }else{
                            addInfoItems(n, NETCARDINFO_RECVBYTES, tr("Bytes Received"), subItemValue.toString(), bMulti);
                        }
                    }
                    subItemValue = subItemObj.value(NWI_SENDBYTES);
                    if (subItemValue.isString()) {
                        QString data = subItemValue.toString();
                        char result_data[128] = {0};
                        KDKVolumeBaseType unit = Util::getVolumeUnit(data);

                        char* volume = Util::getVolume(data,unit);
                        if(kdkVolumeBaseCharacterConvert(volume, unit, result_data) == KDK_NOERR){
                            addInfoItems(n, NETCARDINFO_SENDBYTES, tr("Bytes Sent"), QString::fromUtf8(result_data), bMulti);
                        }else{
                            addInfoItems(n, NETCARDINFO_SENDBYTES, tr("Bytes Sent"), subItemValue.toString(), bMulti);
                        } 
                    }
                }
            }
         }
    }
}

void NetCardInfo::onRefreshInfo()
{
    DataWorker::getInstance()->getNetworkCardInfo();
}

void NetCardInfo::chkNoDevice()
{
    unsigned infoSize = 0;
    QMap<unsigned,QMap<unsigned, QWidget*>>::iterator itMapInfoItems = m_mapInfoItems.begin();
    for (;itMapInfoItems != m_mapInfoItems.end(); itMapInfoItems++) {
        infoSize += itMapInfoItems.value().size();
    }
    if (infoSize == 0) {
        QStringList disableList = NetCardInfo::netCardStatus();
        if(disableList.size() > 0 ){
            for(int i = 0 ; i < disableList.size() ; i++){
                if(disableList[i].contains("|")){
                    QString strProduct = disableList[i].split("|")[0];
                    QString strPciPath = disableList[i].split("|")[1];
                    if(!strProduct.isEmpty() && !Util::judgmentHW()){
                        bool needAdd = true;
                        QAction *disableAction = new QAction(tr("Enable ") + strProduct, this);
                        connect(disableAction, &QAction::triggered, this, [=](){NetCardInfo::enableNetCard(disableList[i]);});
                        QList<QAction *> actList = m_rkeyMenu->actions();
                        for(int i = 0 ; i < actList.length() ; i++){
                            if((tr("Enable ") + strProduct) == actList[i]->text()){
                                needAdd = false;
                            }
                            if((tr("Disable ") + strProduct) == actList[i]->text()){
                                m_rkeyMenu->removeAction(actList[i]);
                            }   
                        }
                        if(needAdd){
                            m_rkeyMenu->addAction(disableAction);
                        }
//                        addInfoItems(0, VOICECARDINFO_NAME, tr("Name"), strProduct, true);
                        // onRefreshInfo();
                    }
                }
            }
            addInfoItems(0, NETCARDINFO_INVALID, "", tr("Network card device is disabled"), false);
        }else{
            addInfoItems(0, NETCARDINFO_INVALID, "", tr("Device not exitst or Get Device is Empty"), false);
        }
    }
}

void NetCardInfo::enterLoadingPage()
{
    m_refreshInfoTimer->stop();
    m_scrollFrame->hide();
    m_loadWidget = new LoadWidget(m_strDevStatus);
    m_loadWidget->setModal(true);
    kdk::UkuiStyleHelper::self()->removeHeader(m_loadWidget);
    m_loadWidget->show();
    // Center m_aboutDlg in the mainwindow
    if(QGuiApplication::platformName() == "wayland"){
        QRect rect = QGuiApplication::primaryScreen()->geometry();
        m_loadWidget->move(rect.center() - m_loadWidget->rect().center());
    }else{
        m_loadWidget->move(this->window()->frameGeometry().topLeft() + this->window()->rect().center() - m_loadWidget->rect().center());
    }
}

void NetCardInfo::exitLoadingPage()
{
    m_refreshInfoTimer->stop();
    m_scrollFrame->show();
    m_loadWidget->close();
}

void NetCardInfo::disableNetCard(QString strProduct)
{
    m_strInfoLastJson = m_strInfoJson;
    m_strDevStatus = "disable";
    // 拉起加载中后，重置status
    enterLoadingPage();
    m_strDevStatus = "";
    qDebug()<<Q_FUNC_INFO;
    qDebug()<<strProduct;
    QProcess p(0);
    p.start("hwinfo --network");
    p.waitForStarted();
    p.waitForFinished();
    QString sysPciPath = "";
    QString strTemp=QString::fromLocal8Bit(p.readAllStandardOutput());  //获得输出
    QStringList strList = strTemp.split("\n");
    for(int i=0;i < strList.length();i++){
        if(strList.at(i).contains(strProduct)){
            //  bus info 获取
            if(strList.at(i+1).contains("SysFS Device Link")){
                sysPciPath = ((QString)strList.at(i+1));
                // /devices/pci0000:00/0000:00:14.3
                if(sysPciPath.contains("/devices/pci0000:00") and !(sysPciPath.contains("usb"))){
                    sysPciPath = sysPciPath.replace("  SysFS Device Link: /devices/pci0000:00/","");
                    DataWorker::getInstance()->disableNetCard(strProduct,sysPciPath);
                    m_strDevStatus = "disable";
                }else{
                    // sw架构单独处理
                    QProcess p(0);
                    p.start("uname -a");
                    p.waitForStarted();
                    p.waitForFinished();
                    QString strArchTemp=QString::fromLocal8Bit(p.readAllStandardOutput()); 
                    if(strArchTemp.contains("sw_64") and sysPciPath.contains("/devices/pci0000:02") and !(sysPciPath.contains("usb"))){
                        sysPciPath = sysPciPath.replace("  SysFS Device Link: /devices/pci0000:02/","");
                        DataWorker::getInstance()->disableNetCard(strProduct,sysPciPath);
                        m_strDevStatus = "disable";
                    }else{
                        break;
                    }
                }
            }else{
                break;
            }
            break;
        }
    }
    qDebug() << Q_FUNC_INFO;
    qDebug() << m_strInfoLastJson;
    // 当数据和上一次数据不同时，表示刷新完成
    if(nullptr == m_refreshInfoTimer){
        m_refreshInfoTimer =  new QTimer();
    }
    if("" != m_strDevStatus){
        QObject::connect(m_refreshInfoTimer, &QTimer::timeout, [&]() {
            if( nullptr == m_strDevStatus || m_strInfoLastJson != m_strInfoJson ){
                exitLoadingPage();
                m_strInfoLastJson = m_strInfoJson;
                m_strDevStatus = "";
            }
            onRefreshInfo();  
        });

        m_refreshInfoTimer->start(5000); // Start timer with 1 second interval
    //exitLoadingPage();
    }else{
        disableTooltip();
        exitLoadingPage();
    }
}

void NetCardInfo::enableNetCard(QString strProductAndPci)
{
    m_strInfoLastJson = m_strInfoJson;
    m_strDevStatus = "enable";
    enterLoadingPage();
    if(!strProductAndPci.isEmpty()){
        DataWorker::getInstance()->enableNetCard(strProductAndPci);
        // m_strDevStatus = "enable";
    }
    // 当数据和上一次数据不同时，表示刷新完成
    // QTimer *timer =  new QTimer();
    if(nullptr == m_refreshInfoTimer){
        m_refreshInfoTimer =  new QTimer();
    }
    QObject::connect(m_refreshInfoTimer, &QTimer::timeout, [&]() {
        if("" == m_strDevStatus || nullptr == m_strDevStatus || m_strInfoLastJson != m_strInfoJson ){
            exitLoadingPage();
            m_strInfoLastJson = m_strInfoJson;
            m_strDevStatus = "";
        }
        onRefreshInfo();  
    });
    m_refreshInfoTimer->start(5000); // Start timer with 1 second interval
}

/*
    返回禁用的声卡设备
*/
QStringList NetCardInfo::netCardStatus()
{
    QStringList disableList;
    QProcess p(0);
    p.start("cat /etc/youker-assistant/netCard");
    p.waitForStarted();
    p.waitForFinished();
    QString strTemp=QString::fromLocal8Bit(p.readAllStandardOutput());  //获得输出
    if(strTemp.isEmpty() || strTemp.contains("/etc/youker-assistant/netCard")){
        return disableList;
    }
    disableList = strTemp.split("\n");
    return disableList;
}

void NetCardInfo::contextMenuEvent(QContextMenuEvent *event)
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
