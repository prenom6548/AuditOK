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

#include "voicecardinfo.h"
#include "dataworker.h"

#include <QDebug>
#include <QTimer>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QScrollBar>
#include <QProcess>
#include <unistd.h>
#include <QGuiApplication>
#include <QScreen>
#include "../../src/kaaboutdialog.h"
#include "ukuistylehelper/ukuistylehelper.h"

VoiceCardInfo::VoiceCardInfo(QWidget *parent)
    : KABaseInfoPage(parent)
{
    m_strInfoJson = "";
    m_strInfoLastJson = "";
    initUI();
    initConnections();
}

void VoiceCardInfo::initUI()
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

void VoiceCardInfo::mousePressEvent(QMouseEvent *event)
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

void VoiceCardInfo::onUpdateInfo(QString strInfoJson)
{
    m_strInfoJson = strInfoJson;
    qInfo()<<"VoiceCardInfo Info:"<<m_strInfoJson;
    if (m_strInfoJson.isEmpty()) {
        DataWorker::getInstance()->getVoiceCardExtInfo();
    } else {
        QTimer::singleShot(0, this, [&, this, strInfoJson](){
            this->updateInfoItems(strInfoJson);
            this->chkNoDevice();
        });
    }
}

void VoiceCardInfo::onUpdateExtInfo(uint uStatus, QString strInfoJson)
{
    Q_UNUSED(uStatus);
    m_strInfoJson = strInfoJson;
    qInfo()<<"VoiceCardExtInfo Info:"<<m_strInfoJson;
    QTimer::singleShot(0, this, [&, this, strInfoJson](){
        this->updateInfoItems(strInfoJson);
        this->chkNoDevice();
    });
}

void VoiceCardInfo::initConnections()
{
    connect(DataWorker::getInstance(), SIGNAL(updateVoiceCardInfo(uint,QString)), this, SLOT(onUpdateInfo(uint,QString)));
    connect(DataWorker::getInstance(), SIGNAL(updateVoiceCardExtInfo(uint,QString)), this, SLOT(onUpdateExtInfo(uint,QString)));
}

void VoiceCardInfo::clearInfoItems()
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

void VoiceCardInfo::addInfoItems(unsigned uGroup, unsigned uId, QString strTitle, QString strContent, bool bMulti)
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
            KAInfoTitle *infoTitle = new KAInfoTitle(QString("%1%2").arg(tr("Sound Card")).arg(m_mapInfoItems.size()+1));
            infoTitle->setMinimumHeight(50);
            infoTitle->setFixedWidth(666);
            m_listLayout->addWidget(infoTitle);
            mapItems[VOICECARDINFO_TITLE] = infoTitle;
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
            m_strPageInfo.append(QString("%1%2").arg(tr("Sound Card")).arg(j+1)+"\n");
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

void VoiceCardInfo::updateInfoItems(QString strInfoJson)
{
    if (strInfoJson.isEmpty())
        return;
    QStringList disableList = soundCardStatus();
    if(disableList.size() > 0 ){
        for(int i = 0 ; i < disableList.size() ; i++){
            if(disableList[i].contains("|")){
                QString strProduct = disableList[i].split("|")[0];
                QString strPciPath = disableList[i].split("|")[1];
                if(!strProduct.isEmpty()){
                    bool needAdd = true;
                    QAction *disableAction = new QAction(tr("Enable ") + strProduct, this);
                    connect(disableAction, &QAction::triggered, this, [=](){enableVoiceCard(disableList[i]);});
                    QList<QAction *> actList = m_rkeyMenu->actions();
                    for(int i = 0 ; i < actList.length() ; i++){
                        if((tr("Enable ") + strProduct) == actList[i]->text()){
                            needAdd = false;
                        }
                        if((tr("Disabled ") + strProduct) == actList[i]->text()){
                            m_rkeyMenu->removeAction(actList[i]);
                        }
                    }
                    if(needAdd){
                        m_rkeyMenu->addAction(disableAction);
                    }
                    // onRefreshInfo();
                }
            }
        }
    }
    qDebug()<<"Cur info:"<<strInfoJson;
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
                    QJsonValue subItemValue = subItemObj.value(VCI_MODEL);
                    if (subItemValue.isString()) {
                        bool needAdd = true;
                        // bool isDisable = false;
                        QString strTitle= tr("Model");
                        QString strContent= subItemValue.toString();
                        // QStringList disableList = KABaseInfoPage::soundCardStatus();
                        // if(disableList.size() > 0 ){
                        //     for(int i = 0 ; i < disableList.size() ; i++){
                        //         if(disableList[i].contains("|")){
                        //             QString strProduct = disableList[i].split("|")[0];
                        //             QString strPciPath = disableList[i].split("|")[1];
                        //             if(strProduct==strTitle){
                        //                 needAdd = false;
                        //                 break;
                        //             }
                        //         }
                        //     }
                        // }    
                        if(tr("Model")==strTitle){
                                QAction *disableAction = new QAction(tr("Disabled ")+strContent, this);
                                connect(disableAction, &QAction::triggered, this, [=](){disableVoiceCard(strContent);});
                                QList<QAction *> actList = m_rkeyMenu->actions();

                                for(int i = 0; i < actList.length() ; i++){
                                    if((tr("Disabled ")+strContent)==actList[i]->text()){
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
                        addInfoItems(n, VOICECARDINFO_MODEL, tr("Model"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(VCI_BUSADDR);
                    if (subItemValue.isString()) {
                        addInfoItems(n, VOICECARDINFO_BUSADDR, tr("Bus Address"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(VCI_DRIVE);
                    if (subItemValue.isString()) {
                        addInfoItems(n, VOICECARDINFO_DRIVE, tr("Drive"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(VCI_NAME);
                    if (subItemValue.isString()) {
                        addInfoItems(n, VOICECARDINFO_NAME, tr("Name"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(VCI_MANUFACTURER);
                    if (subItemValue.isString()) {
                        addInfoItems(n, VOICECARDINFO_MANUFACTURER, tr("Manufacurer"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(VCI_CLOCK);
                    if (subItemValue.isString()) {
                        addInfoItems(n, VOICECARDINFO_CLOCK, tr("Clock"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(VCI_BITWIDTH);
                    if (subItemValue.isString()) {
                        addInfoItems(n, VOICECARDINFO_BITWIDTH, tr("Bit Width"), subItemValue.toString(), bMulti);
                    }
                }
            }
         }
    }
}

void VoiceCardInfo::onRefreshInfo()
{
    DataWorker::getInstance()->getVoiceCardInfo();
}

void VoiceCardInfo::chkNoDevice()
{
    unsigned infoSize = 0;
    QMap<unsigned,QMap<unsigned, QWidget*>>::iterator itMapInfoItems = m_mapInfoItems.begin();
    for (;itMapInfoItems != m_mapInfoItems.end(); itMapInfoItems++) {
        infoSize += itMapInfoItems.value().size();
    }
    if (infoSize == 0) {
        QStringList disableList = soundCardStatus();
        qDebug()<<disableList;
        if(disableList.size() > 0 ){
            for(int i = 0 ; i < disableList.size() ; i++){
                if(disableList[i].contains("|")){
                    QString strProduct = disableList[i].split("|")[0];
                    QString strPciPath = disableList[i].split("|")[1];
                    if(!strProduct.isEmpty()){
                        bool needAdd = true;
                        QAction *disableAction = new QAction(tr("Enable ") + strProduct, this);
                        connect(disableAction, &QAction::triggered, this, [=](){enableVoiceCard(disableList[i]);});
                        QList<QAction *> actList = m_rkeyMenu->actions();
                        for(int i = 0 ; i < actList.length() ; i++){
                            if((tr("Enable ") + strProduct) == actList[i]->text()){
                                needAdd = false;
                            }
                            if((tr("Disabled ") + strProduct) == actList[i]->text()){
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
            addInfoItems(0, VOICECARDINFO_INVALID, "", tr("Sound card device is disabled"), false);
        }else{
            addInfoItems(0, VOICECARDINFO_INVALID, "", tr("Device not exitst or Get Device is Empty"), false);
        }
    }
}

void VoiceCardInfo::enterLoadingPage()
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

void VoiceCardInfo::exitLoadingPage()
{
    m_refreshInfoTimer->stop();
    m_scrollFrame->show();
    m_loadWidget->close();
}

void VoiceCardInfo::enableVoiceCard(QString strProductAndPci)
{
    m_strInfoLastJson = m_strInfoJson;
    m_strDevStatus = "enable";
    this->enterLoadingPage();

    if(!strProductAndPci.isEmpty()){
       DataWorker::getInstance()->enableSoundCard(strProductAndPci);
        // DevControl *dev = new DevControl();
        // dev->setMethod("enableSoundCard");
        // dev->setProductAndPci(strProductAndPci);
        // dev->start();
        // dev->quit();
        // delete dev;
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
        qDebug() << m_strInfoLastJson <<endl;
        qDebug() << m_strInfoJson <<endl;
        onRefreshInfo();  
    });
    m_refreshInfoTimer->start(5000); // Start timer with 1 second interval
}

/*
    返回禁用的声卡设备
*/
QStringList VoiceCardInfo::soundCardStatus()
{
    QStringList disableList;
    QProcess p(0);
    p.start("cat /etc/youker-assistant/soundCard");
    p.waitForStarted();
    p.waitForFinished();
    QString strTemp=QString::fromLocal8Bit(p.readAllStandardOutput());  //获得输出
    qDebug()<<strTemp<<endl;
    if(strTemp.isEmpty() || strTemp.contains("/etc/youker-assistant/soundCard")){
        return disableList;
    }
    disableList = strTemp.split("\n");
    return disableList;
}

void VoiceCardInfo::disableVoiceCard(QString strProduct)
{
    m_strInfoLastJson = m_strInfoJson;
    m_strDevStatus = "disable";
    // 拉起加载中后，重置status
    this->enterLoadingPage();
    m_strDevStatus = "";
    qDebug()<<strProduct<<endl;
    QProcess p(0);
    p.start("hwinfo --sound");
    p.waitForStarted();
    p.waitForFinished();
    QString sysPciPath = "";
    QString strTemp = QString::fromLocal8Bit(p.readAllStandardOutput());  //获得输出
    QStringList strList = strTemp.split("\n");
    for(int i = 0; i < strList.length(); i++){
        if(strList.at(i).contains(strProduct)){
            //  bus info 获取
            qDebug()<<strList.at(i-3)<<endl;
            if(strList.at(i-3).contains("SysFS ID")){
                sysPciPath = ((QString)strList.at(i-3));
                // /devices/pci0000:00/0000:00:14.3
                if(sysPciPath.contains("/devices/pci0000:00") and !(sysPciPath.contains("usb"))){
                    sysPciPath = sysPciPath.replace("  SysFS ID: /devices/pci0000:00/","");
                    DataWorker::getInstance()->disableSoundCard(strProduct,sysPciPath);
                    m_strDevStatus = "disable";
                    // DevControl *dev = new DevControl();
                    // dev->setMethod("disableSoundCard");
                    // dev->setProduct(strProduct);
                    // dev->setSysPciPath(sysPciPath);
                    // dev->start();
                    // dev->quit();
                    // delete dev;
                    // onRefreshInfo();
                }else{
                    // disableTooltip();
                    // exitLoadingPage();
                    // m_strDevStatus = "";
                    break;
                }
            }else{
                if(strList.at(i-4).contains("SysFS ID")){
                    sysPciPath = ((QString)strList.at(i-4));
                    // /devices/pci0000:00/0000:00:14.3
                    if(sysPciPath.contains("/devices/pci0000:00") and !(sysPciPath.contains("usb"))){
                        sysPciPath = sysPciPath.replace("  SysFS ID: /devices/pci0000:00/","");
                        DataWorker::getInstance()->disableSoundCard(strProduct,sysPciPath);
                        m_strDevStatus = "disable";
                        // DevControl *dev = new DevControl();
                        // dev->setMethod("disableSoundCard");
                        // dev->setProduct(strProduct);
                        // dev->setSysPciPath(sysPciPath);
                        // dev->start();
                        // dev->quit();
                        // delete dev;
                        // m_strDevStatus = "disable";
                        // onRefreshInfo();
                    }else{
                        // disableTooltip();
                        // m_strDevStatus = "";
                        // exitLoadingPage();
                        break;
                    }
                }else{
                    // disableTooltip();
                    // m_strDevStatus = "";
                    // exitLoadingPage();
                    break;
                }
            }
            qDebug()<<sysPciPath<<endl;
            break;
        }
    }
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
    }else{
        disableTooltip();
        this->exitLoadingPage();
    }

    //exitLoadingPage();
}

void VoiceCardInfo::contextMenuEvent(QContextMenuEvent *event)
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
