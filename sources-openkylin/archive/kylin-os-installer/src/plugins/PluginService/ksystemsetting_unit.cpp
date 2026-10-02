/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "ksystemsetting_unit.h"
#include <QSettings>
#include <QFile>
#include <QDebug>
#include <QDir>

#define USER_INSTALL_CFG    "/var/log/installer/ky-installer.cfg"
#define CDROM_INSTALL_CFG   "/cdrom/ky-installer.cfg"
//#define TMP_INSTALL_CFG   "/tmp/ky-installer.cfg"
#define LANGUAGE_SUPPORT_CFG    "/usr/share/kylin-os-installer/language/ky-langsupport.cfg"
#define LANGUAGE_CURRENT_CFG    "/usr/share/kylin-os-installer/language/ky-langcurrent.cfg"

using namespace std;
namespace KServer {
QString strModeGlobel = USER_INSTALL_CFG;

void setModel(QString strModeG)
{
    if(strModeG.contains("oem2"))
        strModeGlobel = QDir::homePath() + "/ky-installer.cfg";
    qDebug() << "OOBE cfg:"<<strModeGlobel;
}

QString ReadSettingIni(QString group, QString key)
{
    QSettings setting(strModeGlobel, QSettings::IniFormat);
    setting.setIniCodec("UTF-8");
    setting.beginGroup(group);
    QString value =setting.value(key).toString();
    setting.endGroup();
    return (QString)value;
}

QString AutoReadSettingIni(QString group, QString key)
{
    QString path = QString("");
    path = "/var/log/installer/ky-installer.cfg";
    QSettings setting(path, QSettings::IniFormat);
    setting.setIniCodec("UTF-8");
    setting.beginGroup(group);
    QString value =setting.value(key).toString();
    setting.endGroup();
    return (QString)value;
}

QList<LanguageSetInINI> ReadLanguageIni(void)
{
    QString group("Language");
    QMap<QString,int> LanguageShowMap;//<语言，显示顺序号>
    QSettings setting1(LANGUAGE_CURRENT_CFG, QSettings::IniFormat);
    setting1.setIniCodec("UTF-8");
    setting1.beginGroup(group);
    QStringList keys = setting1.allKeys();//默认按ASCII排序
    QFile qfileCurrent(LANGUAGE_CURRENT_CFG);
    qfileCurrent.open(QIODevice::ReadOnly);
    QString strCurrentAll = qfileCurrent.readAll();
    qfileCurrent.close();
    int nGroupPos = strCurrentAll.indexOf("["+group+"]");
    if(-1 == nGroupPos)
        nGroupPos = 0;
    for (int i = 0; i < keys.size(); ++i)
    {
        QString strKey = keys.at(i);
        strKey.trimmed();
        if(strKey.indexOf("#") == 0)//去掉注释行，即过滤掉以#开头的行
        {
            continue;
        }

        int nP = keys.size()+i;
        int ntmp = strCurrentAll.indexOf(strKey, nGroupPos);
        if(-1 != ntmp)
            nP = ntmp;
        LanguageShowMap.insert(strKey,nP);//排序改为按文件中出现的顺序来显示对应按钮
    }
    setting1.endGroup();

    QMap<int, LanguageSetInINI> LanguageMap;//<显示顺序号，内容>
    QString LValue;
    QSettings setting2(LANGUAGE_SUPPORT_CFG, QSettings::IniFormat);
    setting2.setIniCodec("UTF-8");
    setting2.beginGroup(group);
    QStringList keys2 = setting2.allKeys();
    for (int i = 0; i < keys2.size(); ++i)
    {
        int n = keys2.size()*1000 + i;
        LanguageSetInINI langTmp;

        LValue =setting2.value(keys2.at(i)).toString();
        LValue.trimmed();
        if(LValue.isEmpty())
        {
            continue;
        }
        langTmp.display = LValue;
        langTmp.Language = keys2.at(i).trimmed();
        QMap<QString, int>::const_iterator inOrder = LanguageShowMap.find(langTmp.Language);
        if(inOrder != LanguageShowMap.end())
        {
            langTmp.lsInCurrentList = true;
            n = inOrder.value();
        }
        LanguageMap.insert(n, langTmp);
    }
    setting2.endGroup();

    //按LANGUAGE_CURRENT_CFG文件内标号顺序来重排
    QList<LanguageSetInINI> LanguageListOrdered;//已按显示顺序号排序的语言列表
    QMap<int, LanguageSetInINI>::const_iterator iOrder = LanguageMap.constBegin();
      while (iOrder != LanguageMap.constEnd()) {
          LanguageListOrdered.append(iOrder.value());
          ++iOrder;
      }

      //语言选择界面中，中文简体按钮是必须要提供的，不可屏蔽
      bool ch = false;
      int iLCN = 0;
      for (; iLCN < LanguageListOrdered.size(); ++iLCN)
      {
          if(LanguageListOrdered.at(iLCN).Language == "zh_CN")
          {
              ch = true;
              LanguageListOrdered[iLCN].lsInCurrentList = true;//不可屏蔽中文简体
              break;
          }
      }
      if(!ch)//如果没有中文简体，就新增中文简体，并且第一个显示
      {
          LanguageSetInINI langCNTmp;
          langCNTmp.display = "中文(简体)";
          langCNTmp.Language = "zh_CN";
          langCNTmp.lsInCurrentList = true;
          LanguageListOrdered.insert(0, langCNTmp);
      }

    return LanguageListOrdered;
}

//bool WritePwdToIni(QString group, QString key, QString value)
//{
//    FILE *file = fopen(GetKylinInstallPath() + "/ky-installer.cfg", "wb+");
//    char str[1024] = {0};
//    if(file == NULL) {
//        return false;
//    }
//    fgets(str, 1024, file);


//}

bool WriteSettingToIni(QString group, QString key, QString value)
{
    QSettings setting(strModeGlobel, QSettings::IniFormat);
    setting.setIniCodec("UTF-8");
    setting.beginGroup(group);
    if(key == "password" || key == "encryptyPWD" ) {
        QByteArray list = value.toLocal8Bit();
        setting.setValue(key, list);
    } else {
        setting.setValue(key, value);
    }

    setting.endGroup();
    return true;
}


bool getSettingBool(QString key)
{
    QVariant value = ReadSettingIni("setting", key);
    if(value.isValid()) {
        return value.toBool();
    }
    return false;
}

void writePartitionInfo(QString& rootdisk, QString& rootpart, QString& bootpath, QString& mountpoints)
{
    QSettings setting(strModeGlobel, QSettings::IniFormat);
    setting.setIniCodec("UTF-8");
    setting.beginGroup("partition");
    setting.setValue("KI_ROOT_DISK", rootdisk);
    setting.setValue("KI_ROOT_PARTITION", rootpart);
    setting.setValue("KI_BOOTLOADER", bootpath);
    setting.setValue("KI_MOUNTPOINTS", mountpoints);
    setting.endGroup();
}

void writeRequiringSwapFile(bool isRequired)
{
    QSettings setting(strModeGlobel, QSettings::IniFormat);
    setting.setIniCodec("UTF-8");
    setting.beginGroup("partition");
    setting.setValue("KI_SWAP_FILE", isRequired);
    setting.endGroup();
}

void writeUEFI(bool isEfi)
{
    QSettings setting(strModeGlobel, QSettings::IniFormat);
    setting.setIniCodec("UTF-8");
    setting.beginGroup("partition");
    setting.setValue("KI_UEFI", isEfi);
    setting.endGroup();
}

void writeDiskPartitionSetting(DiskPartitionSetting& set)
{
    writePartitionInfo(set.rootDisk, set.rootPartition, set.bootPartition, set.mountPoints);
    writeRequiringSwapFile(set.swapFileRequire);
    writeUEFI(set.uefiRequired);
}

void updataIni()
{
    //1.首先拷贝usr下的文件到tmp
   // QFile::copy(USER_INSTALL_CFG,TMP_INSTALL_CFG);

    //2.读取2个文件目录下的配置文件

   QSettings setting1(CDROM_INSTALL_CFG, QSettings::IniFormat);
   setting1.setIniCodec("UTF-8");

   QSettings setting2(strModeGlobel, QSettings::IniFormat);
   setting2.setIniCodec("UTF-8");


    //合并usr目录下的和cdrom目录下的文件内容，cdrom覆盖usr
   QStringList set1=setting1.allKeys();
   foreach (QString str, set1) {
       setting2.setValue(str,setting1.value(str));
   }
}

}
