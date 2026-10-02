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

#include "kabaseinfopage.h"
#include <QClipboard>
#include <QApplication>
#include <QDebug>
#include "dataworker.h"
#include <QRegExp>
#include <QFile>

KABaseInfoPage::KABaseInfoPage(QWidget *parent)
    : QWidget(parent)
{

}

void KABaseInfoPage::onRefreshInfo()
{

}

void KABaseInfoPage::onUpdateInfo(QString strInfoJson)
{

}

void KABaseInfoPage::onUpdateInfo(unsigned uStatus, QString strInfoJson)
{
    Q_UNUSED(uStatus);
    onUpdateInfo(strInfoJson);
}

void KABaseInfoPage::onCopyContent(QString strPageInfo)
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(strPageInfo);
}

void KABaseInfoPage::onSelectAllLabel(KALabel* selectedLabel)
{
    if(selectedLabel){
        // KAlabel选中所有文字
        selectedLabel->setSelection(0,selectedLabel->text().length());
    }
}


void KABaseInfoPage::disableMouse(QString strProduct)
{
    QString mouseId = queryMouseId(strProduct);
    if(""!=mouseId){
        QProcess p(0);
        p.start("xinput disable "+mouseId);
        p.waitForStarted();
        p.waitForFinished();
    }else{
        disableTooltip();
//        该设备禁用失败
    }
//        刷新页面
    onRefreshInfo();
}

void KABaseInfoPage::enableMouse(QString strProduct)
{
    QString mouseId = queryMouseId(strProduct);
    if(""!=mouseId){
        QProcess p(0);
        p.start("xinput enable "+mouseId);
        p.waitForStarted();
        p.waitForFinished();
    }else{
//        该设备启用失败
        disableTooltip();
    }
//        刷新页面
    onRefreshInfo();
}

QString KABaseInfoPage::mouseStatus(QString strProduct)
 {
    QString status = "";
    QString mouseId = queryMouseId(strProduct);
    if(""!=mouseId){
            QProcess p(0);
            p.start("xinput list-props "+mouseId);
            p.waitForStarted();
            p.waitForFinished();
            QString strTemp=QString::fromLocal8Bit(p.readAllStandardOutput());  //获得输出
            QStringList strList = strTemp.split("\n");
            for(int i=0;i<strList.length();i++){
                if(strList.at(i).contains("Device Enabled")){
                    //  设备device id获取
                    if(strList.at(i).contains(":")){
                        //  Device Enabled (169):	1
                        status = strList.at(i).split(":")[1];
                        break;
                    }
                }
            }
    }
    return status;
 }

QString KABaseInfoPage::queryMouseId(QString strProduct)
{
    QClipboard *clipboard = QApplication::clipboard();
    QProcess p(0);
    QString cmd = "xinput list";
    QString mouseId = "";
    p.start(cmd);
    p.waitForStarted();
    p.waitForFinished();
    QString strTemp=QString::fromLocal8Bit(p.readAllStandardOutput());  //获得输出
    QStringList strList = strTemp.split("\n");
    for(int i=0;i<strList.length();i++){
        if(strList.at(i).contains(strProduct)){
            //  设备device id获取
            if(strList.at(i).contains("id=")){
                //  "⎜   ↳ USB OPTICAL MOUSE                       \tid=10\t[slave  pointer  (2)]"
                mouseId = strList.at(i).split("id=")[1].split("\t")[0];
                break;
            }
        }
    }
    return mouseId;
}

bool KABaseInfoPage::bluetoothExist()
{
    QProcess p(0);
    p.start("lsusb -vv");
    p.waitForStarted();
    p.waitForFinished();
    QString strTemp=QString::fromLocal8Bit(p.readAllStandardOutput());  //获得输出
    if(strTemp.contains("Bluetooth")){
        return true;
    }
    return false;
}

QString KABaseInfoPage::bluetoothStatus()
{
    QString status = "enable";
    QProcess p(0);
    p.start("hciconfig -a");
    p.waitForStarted();
    p.waitForFinished();
    QString strTemp=QString::fromLocal8Bit(p.readAllStandardOutput());  //获得输出
    if("" == strTemp){
        status = "disable";
    }
    return status;
}


void KABaseInfoPage::disableTooltip()
{
    QDBusInterface iface("org.freedesktop.Notifications",
                        "/org/freedesktop/Notifications",
                        "org.freedesktop.Notifications",
                        QDBusConnection::sessionBus());
    QList<QVariant> args;
    QStringList actions;  //跳转动作
    QMap<QString, QVariant> hints;
    args<<QString(tr("kylin-assistant"))         //应用名
        <<((unsigned int) 0)              //替换ID：若不替换则填0
        <<QString("kylin-assistant")                 //应用主题图标名
        <<QString(tr("kylin-assistant info"))    //通知主题
        <<QString(tr("Disable failed"))    //通知正文
        <<actions                      //跳转动作
        <<hints
        <<(int)-1;                      //驻留时间： -1：默认显示时间（由通知中心决定） 0：消息常驻  其他：显示相应时长(由应用决定，单位ms)
    QDBusMessage response = iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
    if (response.type() == QDBusMessage::ReplyMessage)//判断method的返回值
    {
        qDebug() << "发送成功!通知ID:" << response.arguments().takeFirst().toString();
        uint m_notifyId = response.arguments().takeFirst().toUInt();  //返回的是通知中心为该条消息分配的ID
    } else {
        qDebug() << "发送失败：" << response.type();
    }
}

int KABaseInfoPage::getBlankNum(){
    QString locale = QLocale::system().name();
    if(locale == "zh_CN"){
        return BLANKNUM;
    }else{
        return BLANKNUM_EN;
    }
}
