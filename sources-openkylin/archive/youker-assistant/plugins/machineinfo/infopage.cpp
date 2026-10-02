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

#include "infopage.h"
#include "dataworker.h"
#include "../../src/util.h"

#include <QPixmap>
#include <QLabel>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QTimer>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QScrollBar>
#include <QClipboard>
#include <QApplication>
#include <kysdk/kysdk-base/kyutils.h>

InfoPage::InfoPage(QWidget *parent)
    : QWidget(parent)
{
    m_strInfoJson = "";
    initUI();
    initConnections();
}

void InfoPage::initUI()
{
    m_mainLayout = new QHBoxLayout();
    m_mainLayout->setContentsMargins(0,0,0,24);
    m_mainLayout->setSpacing(0);
    m_listLayout = new QVBoxLayout();
    m_listLayout->setContentsMargins(32,0,0,0);
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
    connect(copyAction, &QAction::triggered, this, [=](){InfoPage::onCopyContent(m_strCopyInfo);});
    m_rkeyMenu->addAction(copyAction);

    QAction *selectAllAction = new QAction(tr("Select All"), this);
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    connect(selectAllAction, &QAction::triggered, this, [=](){InfoPage::onSelectAllLabel(m_selectedLabel);});
    m_rkeyMenu->addAction(selectAllAction);

    QAction *copyAllAction = new QAction(tr("Export"), this);
   connect(copyAllAction, &QAction::triggered, this, [=](){InfoPage::onCopyContent(m_strPageInfo);});
    m_rkeyMenu->addAction(copyAllAction);

    m_mainLayout->addWidget(m_scrollFrame);
    this->setLayout(m_mainLayout);
}

void InfoPage::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        KALabel *label = qobject_cast<KALabel*>(childAt(event->pos()));
        if (label) {
            // 保存鼠标右键的KAlabel
            m_selectedLabel = label;
        }
        m_rkeyMenu->popup(event->globalPos());
    }
    // KABaseInfoPage::mousePressEvent(event);
}

void InfoPage::onCopyContent(QString strPageInfo)
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(strPageInfo);
}

void InfoPage::onSelectAllLabel(KALabel* selectedLabel)
{
    if(selectedLabel){
        // KAlabel选中所有文字
        selectedLabel->setSelection(0,selectedLabel->text().length());
    }
}

int InfoPage::getBlankNum(){
    QString locale = QLocale::system().name();
    if(locale == "zh_CN"){
        return BLANKNUM;
    }else{
        return BLANKNUM_EN;
    }
}

void InfoPage::onUpdateInfo(QString strInfoJson)
{
    m_strInfoJson = strInfoJson;
    qInfo()<<"Machine Info11:"<<m_strInfoJson;
    QTimer::singleShot(0, this, [&, this, strInfoJson](){
        this->updateInfoItems(strInfoJson);
    });
}

void InfoPage::onUpdateInfo(unsigned uStatus, QString strInfoJson)
{
    if (strInfoJson.isEmpty()) {
        uStatus = 0;
    }
    if (uStatus == 1) {
        onUpdateInfo(strInfoJson);
    }
    Q_EMIT updateInfo(uStatus, strInfoJson);
}

void InfoPage::initConnections()
{
    // 连接更新信息信号
    connect(DataWorker::getInstance(), SIGNAL(updateOutline(uint,QString)), this, SLOT(onUpdateInfo(uint,QString)));
    DataWorker::getInstance()->connectOutlineSignal(true);
}

void InfoPage::addInfoItems(unsigned uId, QString strTitle, QString strIcon, QString strContent)
{
    if (strContent.isEmpty()) {
        return ;
    }
    if (m_mapItem.contains(uId)) {
        m_mapItem[uId]->updateDetailInfo(strContent);
    } else {
        KInfoListItem *listItem = new KInfoListItem(strTitle,
                                                    strIcon,
                                                    strContent, m_mapItem.size()%2);
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
            for(int i = length; i < InfoPage::getBlankNum(); i++){
                m_strPageInfo.append("　");
            }
            m_strPageInfo.append(tmpContent+"\n");
        }
    }
}

void InfoPage::clearInfoItems()
{
    QMap<unsigned, KInfoListItem*>::iterator itInfoItem = m_mapItem.begin();
    for (; itInfoItem != m_mapItem.end(); itInfoItem++) {
        if (itInfoItem.value()) {
            delete itInfoItem.value();
        }
    }
    m_mapItem.clear();
}

void InfoPage::updateInfoItems(QString strInfoJson)
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
         QJsonValue valJson = rootObj.value(MI_MANUFACTURER);
         if (valJson.isString()) {
             addInfoItems(MACHINEINFO_MANUFACTURER, tr("Manufacturer"),
                          "ukui-zs-manufacturer-symbolic",
                          valJson.toString());
         }
         valJson = rootObj.value(MI_MACHINEMODEL);
         if (valJson.isString()) {
             addInfoItems(MACHINEINFO_MACHINEMODEL, tr("Machine Model"),
                          "ukui-zs-machine-model-symbolic",
                          valJson.toString());
         }
         valJson = rootObj.value(MI_SERIALNUM);
         if (valJson.isString()) {
             addInfoItems(MACHINEINFO_SERIALNUM, tr("Serial Number"),
                          "ukui-zs-sn-symbolic",
                          valJson.toString());
         }
         valJson = rootObj.value(MI_SYSTEMBITS);
         if (valJson.isString()) {
             addInfoItems(MACHINEINFO_SYSTEMBITS, tr("System Bits"),
                          "ukui-zs-system-bits-symbolic",
                          valJson.toString());
         }
         valJson = rootObj.value(MI_KERNELARCH);
         if (valJson.isString()) {
             addInfoItems(MACHINEINFO_KERNELARCH, tr("Kernel Arch"),
                          "ukui-zs-kernel-architecture-symbolic",
                          valJson.toString());
         }
         valJson = rootObj.value(MI_HOSTNAME);
         if (valJson.isString()) {
             addInfoItems(MACHINEINFO_HOSTNAME, tr("Host Name"),
                          "ukui-zs-host-symbolic",
                          valJson.toString());
         }

         QString versionPath = "/etc/os-release";
         QStringList osRes = readFile(versionPath);
         QString version;

         for (QString str : osRes) {

             if (!QLocale::system().name().compare("zh_CN", Qt::CaseInsensitive)) {
                 if (str.contains("VERSION=")) {
                     QRegExp rx("VERSION=\"(.*)\"$");
                     int pos = rx.indexIn(str);
                     if (pos > -1) {
                         version = rx.cap(1);
                     }
                 }
             } else if (!QLocale::system().name().compare("en_US", Qt::CaseInsensitive)) {
                 if (str.contains("VERSION_US=")) {
                     QRegExp rx("VERSION_US=\"(.*)\"$");
                     int pos = rx.indexIn(str);
                     if (pos > -1) {
                         version = rx.cap(1);
                     }
                 }
             } else {
                 version = tr("Kylin Linux Desktop V10 (SP1)");
             }
         }

        if (QString(kdk_system_get_systemCategory()).compare("MaxTablet") == 0) {
            version = tr("Kylin Linux Desktop (Touch Screen) V10 (SP1)");
        } else if (QString(kdk_system_get_systemCategory()).compare("Tablet") == 0) {
            version = tr("Kylin Linux Desktop (Tablet) V10 (SP1)");
        }

         if (!version.isEmpty()) {
             addInfoItems(MACHINEINFO_OSVERSION, tr("OS Version"),
                          "ukui-zs-neicun-symbolic",
                          version);
         }

         valJson = rootObj.value(MI_KERNELVERSION);
         if (valJson.isString()) {
             addInfoItems(MACHINEINFO_KERNELVERSION, tr("Kernel Version"),
                          "ukui-zs-kernel-version-symbolic",
                          valJson.toString());
         }
         valJson = rootObj.value(MI_PROCESSOR);
         if (valJson.isString()) {
             addInfoItems(MACHINEINFO_PROCESSOR, tr("Processor"),
                          "ukui-zs-processor-symbolic",
                          valJson.toString());
         }
         valJson = rootObj.value(MI_MEMORY);
         if (valJson.isString()) {
            // 判断是否为多个内存
            if(valJson.toString().contains(" / ")){
                QStringList valList = valJson.toString().split(" / ");
                for (int i = 0; i < valList.size(); ++i){
                    QStringList itemList = valList[i].split(" ");
                    QString data;
                    int index = 1;
                    if(itemList.length() >= 2) {
                        if(Util::isVolumeUnit(itemList[1])) {
                            data = itemList[0]+itemList[1];
                            index = 2;
                        } else {
                            data = itemList[0];
                        }
                    } else {
                        data = itemList[0];
                    }
                    char result_data[128] = {0};
                    KDKVolumeBaseType unit = Util::getVolumeUnit(data);
                    char* volume = Util::getVolume(data,unit);
                    if(kdkVolumeBaseCharacterConvert(volume, unit, result_data) == KDK_NOERR){
                        QString tmpStr = QString::fromUtf8(result_data);
                        // 替换原数组值
                        for(int j = index ; j < itemList.size() ; j++) {
                            tmpStr += " "+itemList[j];
                        }
                        valList[i] = tmpStr;
                        addInfoItems(MACHINEINFO_MEMORY, tr("Memory"),
                            "ukui-zs-memory-symbolic",
                            valList.join(" / "));
                    }else{
                        addInfoItems(MACHINEINFO_MEMORY, tr("Memory"),
                            "ukui-zs-memory-symbolic",
                            valJson.toString());
                    }
                }
            }else{
                QString valStr = valJson.toString();
                QStringList itemList = valStr.split(" ");
                QString data;
                int index = 1;
                if(itemList.length() >= 2) {
                    if(Util::isVolumeUnit(itemList[1])) {
                        data = itemList[0]+itemList[1];
                        index = 2;
                    } else {
                        data = itemList[0];
                    }
                } else {
                    data = itemList[0];
                }
                char result_data[128] = {0};
                KDKVolumeBaseType unit = Util::getVolumeUnit(data);
                char* volume = Util::getVolume(data,unit);
                if(kdkVolumeBaseCharacterConvert(volume, unit, result_data) == KDK_NOERR){
                    QString tmpStr = QString::fromUtf8(result_data);
                    // 替换原数组值
                    for(int j = index; j < itemList.size(); j++) {
                        tmpStr += " "+itemList[j];
                    }
                    addInfoItems(MACHINEINFO_MEMORY, tr("Memory"),
                        "ukui-zs-memory-symbolic",
                        tmpStr);
                }else{
                    addInfoItems(MACHINEINFO_MEMORY, tr("Memory"),
                        "ukui-zs-memory-symbolic",
                        valJson.toString());
                }
            }
         }
         valJson = rootObj.value(MI_MAINBOARD);
         if (valJson.isString()) {
             addInfoItems(MACHINEINFO_MAINBOARD, tr("Mother Board"),
                          "ukui-mother-board-symbolic",
                          valJson.toString());
         }
         valJson = rootObj.value(MI_HARDDISK);
         if (valJson.isString()) {
            if(valJson.toString().contains("(") && valJson.toString().contains(")")){
                QString data = valJson.toString().split("(")[1].replace(")","");
                char result_data[128] = {0};
                KDKVolumeBaseType unit = Util::getVolumeUnit(data);

                char* volume = Util::getVolume(data,unit);
                if(kdkVolumeBaseCharacterConvert(volume, unit, result_data) == KDK_NOERR){
                    addInfoItems(MACHINEINFO_HARDDISK, tr("Hard Disk"),
                                "ukui-zs-hard-disk-symbolic",
                                valJson.toString().split("(")[0]+"("+QString::fromUtf8(result_data)+")");
                }
            }else{
                addInfoItems(MACHINEINFO_HARDDISK, tr("Hard Disk"),
                            "ukui-zs-hard-disk-symbolic",
                            valJson.toString());
            }
         }
         valJson = rootObj.value(MI_GRAPHICSCARD);
         if (valJson.isString()) {
             addInfoItems(MACHINEINFO_GRAPHICSCARD, tr("Graphics Card"),
                          "ukui-graphics-card-symbolic",
                          valJson.toString());
         }
         valJson = rootObj.value(MI_MONITOR);
         if (valJson.isString()) {
             addInfoItems(MACHINEINFO_MONITOR, tr("Monitor"),
                          "computer-symbolic",
                          valJson.toString());
         }
         valJson = rootObj.value(MI_NETWORKCARD);
         if (valJson.isString()) {
             addInfoItems(MACHINEINFO_NETWORKCARD, tr("Network Card"),
                          "ukui-zs-network-card-symbolic",
                          valJson.toString());
         }
         valJson = rootObj.value(MI_SOUNDCARD);
         if (valJson.isString()) {
             addInfoItems(MACHINEINFO_SOUNDCARD, tr("Sound Card"),
                          "audio-volume-high-symbolic",
                          valJson.toString());
         }
    }
}

/* 读取/etc/os-release文件的内容 */
QStringList InfoPage::readFile(QString filepath)
{
    QStringList fileCont;
    QFile file(filepath);
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "ReadFile() failed to open" << filepath;
            return QStringList();
        }
        QTextStream textStream(&file);
        while (!textStream.atEnd()) {
            QString line = textStream.readLine();
            line.remove('\n');
            fileCont<<line;
        }
        file.close();
        return fileCont;
    } else {
        qWarning() << filepath << " not found";
        return QStringList();
    }
}

void InfoPage::onRefreshInfo()
{
    DataWorker::getInstance()->getOutline();
}

void InfoPage::contextMenuEvent(QContextMenuEvent *event)
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
