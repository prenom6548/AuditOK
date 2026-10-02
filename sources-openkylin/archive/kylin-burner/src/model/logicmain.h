/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/


#ifndef LOGICMAIN_H
#define LOGICMAIN_H

#include <QObject>
#include <QMutex>
#include <QHash>
#include <QGSettings/QGSettings>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QThread>

#ifdef signals
#undef signals
#endif
class DiscMonitor;
class DiscRead;
class LogicMain : public QObject
{
    Q_OBJECT
    friend class LogicMainTestFriend;
    friend class LogicMainTest;
public:
    enum Busness
    {
        CreateISO = 0,
        BurnISO9660Data,
        BurnUDFData,
        AddBurn,
        BurnImage,
        CopyData,
        GetISO

    };
public:
    static LogicMain *instance();
    static QThread * getThread();
public:
    LogicMain(const LogicMain*)=delete;
    LogicMain *operator =(const LogicMain*)=delete;

public slots:
    void slotsSetBusness(int);
    void init();
    void initISO();
public:
    void destroy();
    QString theme();
    QString icon();
    int GetSystemFontSize();
private:
    explicit LogicMain(QObject *parent = nullptr);
    ~LogicMain();
public slots:
    void slotsDoBusness();

private slots:
    void createCDROM(QString);
    //void changeCDROM(QString);
    //void removeCDROM(QString);
    void slotsEmptyMedium();
    void slotsAppendableMedium();
    void slotsFinishedMedium();
    void slotsMenuBurnSetting();
    void currentDiscs();                       //查询当前所有的有光盘的光驱设备
private:
    static LogicMain     *m_oInstance;
    static QMutex         m_oMutex;
private:
    int                   m_iBusness;
    QString               m_oCurrentIcon;
    QString               m_oCurrnetThemeBackground;
    QGSettings           *m_oTheme;
    QHash<QString, void*> m_oCDROMS;
    DiscMonitor          *m_oDiscMonitor;       //光盘设备实时监控
    QStringList           m_oDiscList;          //电脑上实时连接的光盘列表

private Q_SLOTS:
    void discAddedSlot(QString);                //检测到光盘插入
    void discRemovedSlot(QString);              //监测到光盘移除
    void driveAddedSlot(QString);               //检测到光驱插入
    void driveRemovedSlot(QString);             //监测到光驱移除

Q_SIGNALS:
    // logic
    void newCDromArrived(void *);
    void createISO(void *);
    void createCopyImage(void *);

    void mediumRmoved(void *);


    void emptyMedium(void *);
    void appendableMedium(void *);
    void finishedMedium(void *);
    void CDRomRemoved(void *);
    // busness
    void createISO();
    void burnData();
    void addBurn();
    void burnImage();
    void copyData();
    void getISO();
    // menu
    void functionMenuChanged(void*);
    void burnDataSetting();
    void burnImageSetting();
    void burnCopySetting();

    // other
    void storagePath(QString);
    void iconChange(QIcon);
    void exit();

    void systemFontSizeChanged(int);
    void systemFontnameChanged(QString fontName);
};

#endif // LOGICMAIN_H
