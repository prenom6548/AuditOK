/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <kswitchbutton.h>
#include "config_win.h"
#include "ui_config_win.h"
#include "size_para.h"
#include "toolutils.h"
#include <QButtonGroup>

const static int ITEM_HEIGHT = 60;

ConfigWin::ConfigWin(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ConfigWin)
{
    ui->setupUi(this);

    setWindowTitle(tr("IntraNetSet"));
    setWin();
    ui->titleBar->setTitleName(tr("IntraNetSet"));
    ui->quadBtns->setShowBtnsMode(QuadBtnsShowMode::ONLY_CLOSE_BTN, this);
    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::Tool);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    QString platform = QGuiApplication::platformName();
    if (platform.startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        ;
    } else {
        MotifWmHints hints1;
        hints1.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
        hints1.functions = MWM_FUNC_ALL;
        hints1.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints1);
    }

    connect(ui->switchButton, SIGNAL(stateChanged(bool)), this, SLOT(setInnerCheckShow(bool)));
    connect(ui->ipWidget, SIGNAL(addWinSize()), this, SLOT(resizeWinSize()));
    connect(ui->webWidget, SIGNAL(addWinSize()), this, SLOT(resizeWinSize()));
    connect(ui->ipWidget, SIGNAL(minWinSize()), this, SLOT(resizeWinSize()));
    connect(ui->webWidget, SIGNAL(minWinSize()), this, SLOT(resizeWinSize()));

    ui->cancelBtn->setText(tr("Cancel"));
    ui->saveBtn->setText(tr("Save"));
    ui->saveBtn->setProperty("isImportant", true);


    connect(ui->cancelBtn, SIGNAL(pressed()), this, SLOT(cancelPress()));
    connect(ui->saveBtn, SIGNAL(pressed()), this, SLOT(savePress()));

    connect(ui->ipWidget, SIGNAL(changedEvent()), this, SLOT(recordChange()));
    connect(ui->webWidget, SIGNAL(changedEvent()), this, SLOT(recordChange()));
}

ConfigWin::~ConfigWin()
{
    delete ui;
}
void ConfigWin::setWin()
{
    qDebug() << "开始构造设置窗口！";
    ToolUtils utils;
    bool isOn = utils.getInnerCheckSetting();
    m_isON = isOn;
    QMap<QString, QVector<QString>> settingsMap = utils.getConfigFile();
    QList<QString> iplist;
    iplist.clear();
    QList<QString> weblist;
    weblist.clear();
    if (settingsMap.contains("config-ip")) {
        QVector<QString> ipVector = settingsMap.value("config-ip");
        iplist = ipVector.toList();
    }
    if (settingsMap.contains("config-web")) {
        QVector<QString> webVector = settingsMap.value("config-web");
        weblist = webVector.toList();
    }
    int ipNum = iplist.size();
    int webNum = weblist.size();
    if (isOn) {
        qDebug() << "内网检测开！";
        ui->switchButton->setChecked(true);
        qDebug() << "内网检测IP数量为：" << ipNum;
        qDebug() << "内网检测IP为：" << iplist;
        qDebug() << "内网检测WEB数量为：" << webNum;
        qDebug() << "内网检测WEB为：" << weblist;
        //经过分析，发现当前代码的两个对0的判断是为了后边的一种取巧的操作
        if (ipNum == 0) {
            ui->ipWidget->setItemNums(1, iplist, IPWebWidgetType::IP_WIDGET);
        } else {
            ui->ipWidget->setItemNums(ipNum, iplist, IPWebWidgetType::IP_WIDGET);
        }
        if (webNum == 0) {
            ui->webWidget->setItemNums(1, weblist, IPWebWidgetType::WEB_WIDGET);
        } else {
            ui->webWidget->setItemNums(webNum, weblist, IPWebWidgetType::WEB_WIDGET);
        }


        ui->ipWidget->showListWidget(true, IPWebWidgetType::IP_WIDGET);
        ui->webWidget->showListWidget(true, IPWebWidgetType::WEB_WIDGET);
        int totalItemNum = webNum + ipNum;
        if (totalItemNum == 0 || totalItemNum == 1) {
            this->setFixedSize(420, 184 + 2 * ITEM_HEIGHT);
        } else if (ipNum == 0 || webNum == 0) {
            this->setFixedSize(420, 184 + totalItemNum * ITEM_HEIGHT + ITEM_HEIGHT);
        } else {
            this->setFixedSize(420, 184 + totalItemNum * ITEM_HEIGHT);
        }

    } else {
        qDebug() << "内网检测未开启！";
        ui->ipWidget->setItemNums(0, iplist, IPWebWidgetType::IP_WIDGET);

        ui->webWidget->setItemNums(0, weblist, IPWebWidgetType::WEB_WIDGET);
        ui->switchButton->setChecked(false);
        this->setFixedSize(420, 184);
        ui->webWidget->hide();
        ui->ipWidget->hide();
    }
}
void ConfigWin::showWin()
{
    ui->saveBtn->setEnabled(false);
    if (!m_isShow) {
        this->exec();
    }
}

// void ConfigWin::initSettings()
//{
//     const QByteArray idd(NET_CHECK_SCHEMA);
//     if(QGSettings::isSchemaInstalled(idd)) {
//         m_checkSettings = new QGSettings(idd);
//     }
//     if (m_checkSettings) {
//         connect(m_checkSettings, &QGSettings::changed, this, [=](const QString &key) {
//             if (key == "innercheck") {
//                 auto isInnerCheck = m_checkSettings->get("innercheck");
//                 if (isInnerCheck.isValid()) {
//                     m_innerCheckArgSettings.isInnerCheck = isInnerCheck.toBool();
//                 }
//             }
//             if (key == "ip") {
//                 auto ipListVar = m_checkSettings->get("ip");
//                 if (ipListVar.isValid()) {
//                     QString ipStr = ipListVar.toString();
//                     QStringList ipStrList = ipStr.split(";");
//                     if(ipStrList.size()>0){
//                         m_innerCheckArgSettings.ipNum = ipStrList.size() - 1;
//                         for (int i = 0; i < ipStrList.size()-1; ++i) //此处size-1,修改bug，当size=5时，会出现数组越界
//                         {
//                             m_innerCheckArgSettings.ip[i] = ipStrList.at(i);
//                         }
//                     }
//                 }
//             }
//             if (key == "web") {
//                 auto webListVar = m_checkSettings->get("web");
//                 if (webListVar.isValid()) {
//                     QString webStr = webListVar.toString();
//                     QStringList webStrList = webStr.split(";");
//                     if(webStrList.size()>0){
//                         m_innerCheckArgSettings.webNum = webStrList.size() - 1;
//                         for (int i = 0; i < webStrList.size()-1; ++i)
//                         //此处size-1,修改bug，当size=5时，会出现数组越界
//                         {
//                             m_innerCheckArgSettings.web[i] = webStrList.at(i);
//                         }
//                     }
//                 }
//             }

//            setWin();
//        });
//        auto isInnerCheck = m_checkSettings->get("innercheck");
//        if (isInnerCheck.isValid()) {
//            m_innerCheckArgSettings.isInnerCheck = isInnerCheck.toBool();
//        }
//        auto ipListVar = m_checkSettings->get("ip");
//        if (ipListVar.isValid()) {
//            QString ipStr = ipListVar.toString();
//            QStringList ipStrList = ipStr.split(";");
//            qDebug() << "ConfigWin::initSettings ipStrList.size:" <<ipStrList.size();
//            if(ipStrList.size()>0){
//                m_innerCheckArgSettings.ipNum = ipStrList.size() - 1;
//                //最后一个分号后边是空的，所以要减一，会多一个14.15.16.19;14.15.16.20; for (int i = 0; i <
//                ipStrList.size()-1; ++i)    //此处size-1,修改bug，当size=5时，会出现数组越界
//                {
//                    m_innerCheckArgSettings.ip[i] = ipStrList.at(i);
//                }
//            }

//        }
//        auto webListVar = m_checkSettings->get("web");
//        if (webListVar.isValid()) {
//            QString webStr = webListVar.toString();
//            QStringList webStrList = webStr.split(";");
//            if(webStrList.size()>0){
//                m_innerCheckArgSettings.webNum = webStrList.size() - 1;
//                for (int i = 0; i < webStrList.size()-1; ++i)  //此处size-1,修改bug，当size=5时，会出现数组越界
//                {
//                    m_innerCheckArgSettings.web[i] = webStrList.at(i);
//                }
//            }
//        }
//    }
//    qDebug() << "读取内网检测开关信息为：" << m_innerCheckArgSettings.isInnerCheck;
//    qDebug() << "读取内网ip信息为：" << m_innerCheckArgSettings.ip;
//    qDebug() << "读取内网网站信息为：" << m_innerCheckArgSettings.web;
//    setWin();
//}
bool ConfigWin::saveSettings()
{
    QStringList ipList = ui->ipWidget->getAllSettings().split(";", QString::SkipEmptyParts);
    QVector<QString> ipVector = ipList.toVector();
    QStringList webList = ui->webWidget->getAllSettings().split(";", QString::SkipEmptyParts);
    QVector<QString> webVector = webList.toVector();
    bool isOn = true;
    if (ui->switchButton->isChecked()) {
        isOn = true;
    } else {
        isOn = false;
    }
    qDebug() << "ConfigWin::saveSettings" << ipVector << ipVector.isEmpty();
    if (ipVector.isEmpty() && webVector.isEmpty()) {
        isOn = false;
    }
    QMap<QString, QVector<QString>> map;
    map.clear();
    map.insert("config-ip", ipVector);
    map.insert("config-web", webVector);
    ToolUtils utils;
    bool result = utils.writeConfigFile(isOn, map);
    emit sigUpdateConfigFile();
    return result;
}
void ConfigWin::setInnerCheckShow(bool isCheck)
{
    m_isON = isCheck;
    if (isCheck) {
        ui->ipWidget->showListWidget(true, IPWebWidgetType::IP_WIDGET);
        ui->webWidget->showListWidget(true, IPWebWidgetType::WEB_WIDGET);
    } else {
        ui->ipWidget->hide();
        ui->webWidget->hide();
    }
    ui->saveBtn->setEnabled(true);
    resizeWinSize();
}

void ConfigWin::resizeWinSize()
{
    if (m_isON) {
        int ipItemNum = ui->ipWidget->getWidgetItemNums();
        int webItemNum = ui->webWidget->getWidgetItemNums();
        qDebug() << "ConfigWin::resizeWinSize size:" << ipItemNum << webItemNum;
        this->setFixedSize(420, 184 + (ipItemNum + webItemNum) * ITEM_HEIGHT);
    } else {
        this->setFixedSize(420, 184);
    }

    this->update();
}

void ConfigWin::cancelPress()
{
    this->hide();
}

void ConfigWin::savePress()
{
    if (saveSettings()) {
        this->hide();
    } else {
        // TODO:此处应有UI提示
        qCritical() << "保存失败！";
        this->hide();
    }
}

void ConfigWin::recordChange()
{
    ui->saveBtn->setEnabled(true);
    if (!ui->ipWidget->getAllFormatStatus() || !ui->webWidget->getAllFormatStatus()) {
        ui->saveBtn->setEnabled(false);
    }

    //    for(int i = 0; i < 5; ++i)
    //    {
    //        m_innerCheckArgSettings.ip[i] = "";
    //    }
    //    for(int i = 0; i < 5; ++i)
    //    {
    //        m_innerCheckArgSettings.web[i] = "";
    //    }
    //    QStringList ipList = ui->ipWidget->getAllSettings().split(";");
    //    m_innerCheckArgSettings.ipNum = ipList.size() - 1;

    //    QStringList webList = ui->webWidget->getAllSettings().split(";");
    //    m_innerCheckArgSettings.webNum = webList.size() - 1;


    //    for(int i = 0; i < m_innerCheckArgSettings.ipNum;++i)
    //    {
    //        m_innerCheckArgSettings.ip[i] = ipList.at(i);
    //    }
    //    for(int i = 0; i < m_innerCheckArgSettings.webNum;++i)
    //    {
    //        m_innerCheckArgSettings.web[i] = webList.at(i);
    //    }
    //    return;
}
