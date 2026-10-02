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

#include "monitorinfo.h"
#include "dataworker.h"

#include <QDebug>
#include <QTimer>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
//#include <qx11info_x11.h>
#include <QApplication>
//#include <X11/extensions/Xrandr.h>
#include <QScrollBar>

#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/edid.h>
#include <KF5/KScreen/kscreen/mode.h>
#include <KF5/KScreen/kscreen/config.h>
#include <KF5/KScreen/kscreen/getconfigoperation.h>
#include <KF5/KScreen/kscreen/configmonitor.h>
#include <KF5/KScreen/kscreen/setconfigoperation.h>
#include <KF5/KScreen/kscreen/edid.h>
#include <KF5/KScreen/kscreen/types.h>

MonitorInfo::MonitorInfo(QWidget *parent)
    : KABaseInfoPage(parent)
{
    m_strInfoJson = "";
    initUI();
    initConnections();
}

void MonitorInfo::initUI()
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
    QDesktopWidget *desktop = QApplication::desktop();
    QScreen *screen = QGuiApplication::primaryScreen();

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

void MonitorInfo::mousePressEvent(QMouseEvent *event)
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

void MonitorInfo::onUpdateInfo(QString strInfoJson)
{
    m_strInfoJson = strInfoJson;
    qInfo()<<"Monitor Info:"<<m_strInfoJson;
    QTimer::singleShot(0, this, [&, this, strInfoJson](){
        this->updateInfoItems(strInfoJson);
        this->chkNoDevice();
    });
}

void MonitorInfo::initConnections()
{
//    //注册监听xrr事件
//    // XRRQueryExtension(QX11Info::display(), &m_xrrEventBase, &m_xrrErrorBase);
//    // XRRSelectInput(QX11Info::display(), QX11Info::appRootWindow(), RRScreenChangeNotifyMask);
//    qApp->installNativeEventFilter(this); //监听全局x事件，调用了这个才能收到nativeEventFilter

    connect(DataWorker::getInstance(), SIGNAL(updateMonitorInfo(uint,QString)), this, SLOT(onUpdateInfo(uint,QString)));
    QObject::connect(new KScreen::GetConfigOperation(), &KScreen::GetConfigOperation::finished,
                    [&](KScreen::ConfigOperation *op) {
        this->setConfig(qobject_cast<KScreen::GetConfigOperation *>(op)->config());
    });

}

bool MonitorInfo::nativeEventFilter(const QByteArray &eventType, void *message, long *result) //监听全局x事件
{
    // if (qstrcmp(eventType, "xcb_generic_event_t") != 0) {
    //     return false;
    // }

    // xcb_generic_event_t *event = reinterpret_cast<xcb_generic_event_t*>(message);
    // const uint8_t responseType = event->response_type & ~0x80;
    // if(responseType == m_xrrEventBase + RRScreenChangeNotify){
    //     RRScreenChangeEvent();  //屏幕改变事件
    // }
    return false;
}

//void MonitorInfo::RRScreenChangeEvent()
//{
//    XRRScreenResources *screen;
//    screen = XRRGetScreenResources(QX11Info::display(), QX11Info::appRootWindow());
//    XRROutputInfo *info;
//    int nCurCount = 0;
//    static int nLastCount = -1;

//    //这里只是屏幕改变事件，通过屏幕改变事件遍历所有
//    for (int i = 0; i < screen->noutput; i++) {
//        info = XRRGetOutputInfo(QX11Info::display(), screen, screen->outputs[i]);
//        if (info->connection == RR_Connected) {
//            nCurCount++;
//        }
//        XRRFreeOutputInfo(info);
//    }
//    /*
//        count为当前连接的显示器个数，每次记录并比较就可以得到显示器的热插拔事件
//    */
//    XRRFreeScreenResources(screen);

//    // 屏幕数发生变化，获取最新屏幕信息
//    if (nCurCount != nLastCount) {
//        nLastCount = nCurCount;
//        DataWorker::getInstance()->getMonitorInfo(true);
//    }
//}

void MonitorInfo::clearInfoItems()
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

void MonitorInfo::addInfoItems(unsigned uGroup, unsigned uId, QString strTitle, QString strContent, bool bMulti)
{
    if (strContent.isEmpty())
        return ;
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
            KAInfoTitle *infoTitle = new KAInfoTitle(QString("%1%2").arg(tr("Monitor")).arg(m_mapInfoItems.size()+1));
            infoTitle->setMinimumHeight(50);
            infoTitle->setFixedWidth(666);
            m_listLayout->addWidget(infoTitle);
            mapItems[MONITORINFO_TITLE] = infoTitle;
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
            m_strPageInfo.append(QString("%1%2").arg(tr("Monitor")).arg(j+1)+"\n");
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

void MonitorInfo::updateInfoItems(QString strInfoJson)
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
                    QJsonValue subItemValue = subItemObj.value(MNI_MANUFACTURER);
                    if (subItemValue.isString()) {
                        addInfoItems(n, MONITORINFO_MANUFACTURER, tr("Manufacturer"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(MNI_NAME);
                    if (subItemValue.isString()) {
                        addInfoItems(n, MONITORINFO_NAME, tr("Name"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(MNI_SIZE);
                    if (subItemValue.isString()) {
                        addInfoItems(n, MONITORINFO_SIZE, tr("Size"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(MNI_RATIO);
                    if (subItemValue.isString()) {
                        addInfoItems(n, MONITORINFO_RATIO, tr("Ratio"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(MNI_RESOLUTION);
                    if (subItemValue.isString()) {
                        addInfoItems(n, MONITORINFO_RESOLUTION, tr("Resolution"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(MNI_MAXAVARES);
                    if (subItemValue.isString()) {
                        addInfoItems(n, MONITORINFO_MAXAVARES, tr("Optimal Resolution"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(MNI_ISMAIN);
                    if (subItemValue.isString()) {
                        QString strYesOrNo = "";
                        if (subItemValue.toString() == "1") {
                            strYesOrNo = tr("Yes");
                        } else {
                            strYesOrNo = tr("No");
                        }
                        addInfoItems(n, MONITORINFO_ISMAIN, tr("Main Screen"), strYesOrNo, bMulti);
                    }
                    subItemValue = subItemObj.value(MNI_INTERFACE);
                    if (subItemValue.isString()) {
                        addInfoItems(n, MONITORINFO_INTERFACE, tr("Interface"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(MNI_VISIBLEAREA);
                    if (subItemValue.isString()) {
                        addInfoItems(n, MONITORINFO_VISIBLEAREA, tr("Visible Area"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(MNI_WEEK);
                    if (subItemValue.isString()) {
                        addInfoItems(n, MONITORINFO_WEEK, tr("Product Week"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(MNI_YEAR);
                    if (subItemValue.isString()) {
                        addInfoItems(n, MONITORINFO_YEAR, tr("Product Year"), subItemValue.toString(), bMulti);
                    }
                    subItemValue = subItemObj.value(MNI_GAMMA);
                    if (subItemValue.isString()) {
                        addInfoItems(n, MONITORINFO_GAMMA, tr("Gamma"), subItemValue.toString(), bMulti);
                    }
                }
            }
         }
    }
}

void MonitorInfo::onRefreshInfo()
{
    DataWorker::getInstance()->getMonitorInfo(false);
}

void MonitorInfo::chkNoDevice()
{
    unsigned infoSize = 0;
    QMap<unsigned,QMap<unsigned, QWidget*>>::iterator itMapInfoItems = m_mapInfoItems.begin();
    for (;itMapInfoItems != m_mapInfoItems.end(); itMapInfoItems++) {
        infoSize += itMapInfoItems.value().size();
    }
    if (infoSize == 0) {
        addInfoItems(0, MONITORINFO_INVALID, "", tr("Device not exitst or Get Device is Empty"), false);
    }
}

void MonitorInfo::setConfig(const KScreen::ConfigPtr &config)
{
    qDebug() << Q_FUNC_INFO;
    if (mConfig) {
            KScreen::ConfigMonitor::instance()->removeConfig(mConfig);
            for (const KScreen::OutputPtr &output : mConfig->outputs()) {
                output->disconnect(this);
            }
            mConfig->disconnect(this);
        }

    mConfig = config;

    KScreen::ConfigMonitor::instance()->addConfig(mConfig);

    for (const KScreen::OutputPtr &output : mConfig->outputs()) {
        outputAdded(output, false);
        connect(output.data(), &KScreen::Output::isConnectedChanged,
            this, &MonitorInfo::slotOutputConnectedChanged);

    }
    
    connect(mConfig.data(), &KScreen::Config::outputAdded,
            this, [=](const KScreen::OutputPtr &output){
        DataWorker::getInstance()->getMonitorInfo(true);
    });
    
    connect(mConfig.data(), &KScreen::Config::outputRemoved,
            this, [=](int outputId){
        DataWorker::getInstance()->getMonitorInfo(true);
    });

}


void MonitorInfo::outputAdded(const KScreen::OutputPtr &output, bool connectChanged)
{
    if (true) {
        qDebug()<<output;
    connect(output.data(), &KScreen::Output::isConnectedChanged,
            this, &MonitorInfo::slotOutputConnectedChanged);

    connect(output.data(), &KScreen::Output::isEnabledChanged,
             this, [=](){
          DataWorker::getInstance()->getMonitorInfo(true);
     });
    //  connect(output.data(), &KScreen::Output::sizeChanged,
    //          this, [=](){
    //  });
    }
}

void MonitorInfo::slotOutputConnectedChanged()
{
    DataWorker::getInstance()->getMonitorInfo(true);
//    const KScreen::OutputPtr output(qobject_cast<KScreen::Output *>(sender()), [](void *){});
//    if (!output) {
//        return;
//    }

//    if (output->isConnected()) {
//        outputAdded(output, true);
//    } else {
//        // outputRemoved(output->id(), true);
//    }
}

void MonitorInfo::contextMenuEvent(QContextMenuEvent *event)
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

