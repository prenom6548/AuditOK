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


#ifndef DATACDROM_H
#define DATACDROM_H

#include "datasetting.h"
#include "src/frame/signals.h"
#include "src/status-fix/discread.h"

#include <QObject>
#include <QIcon>
#include <QProcess>
#include <QTimer>
#include <QDebug>

enum MEDIUM_SYSTEM_TYPE {
    ISO9660 = 1,
    UDF102 = 2,
    UDF201 = 3,
    UNKNOWN = 4
};

class DataCDROM;
class DataMedium : public QObject
{
    friend class MainWindowTest;
    Q_OBJECT
Q_SIGNALS:
    // 当第一次执行m_discread->work()获取信息失败后，再执行一次
    void secondGetDiscStatus();
public:
    enum MEDIUM_TYPE
    {
        MEDIUM_UNKOWN = 0x0000,
        MEDIUM_CD_ROM = 0x0001,
        MEDIUM_CD_R = 0x0002,
        MEDIUM_CD_RW = 0x0004,
        MEDIUM_DVD_ROM = 0x0008,
        MEDIUM_DVD_R = 0x0010,
        MEDIUM_DVD_RAM = 0x0020,
        MEDIUM_DVD_R_DL_SEQ = 0x0040,
        MEDIUM_DVD_R_DL_JUMP = 0x0080,
        MEDIUM_DVD_PLUS_RW = 0x0100,
        MEDIUM_DVD_PLUS_R = 0x0101,
        MEDIUM_DVD_PLUS_RW_DL = 0x0102,
        MEDIUM_DVD_PLUS_R_DL = 0x0104,
        MEDIUM_DVD_RW_OVERWRITE = 0x0108,
        MEDIUM_DVD_RW_SEQ = 0x0110,
        MEDIUM_CD_RW_S0 = 0x0120, // in MMC-5 CD-RW has 8 subtypes.
        MEDIUM_CD_RW_S1 = 0x0140,
        MEDIUM_CD_RW_S2 = 0x0180,
        MEDIUM_CD_RW_S3 = 0x0200,
        MEDIUM_CD_RW_S4 = 0x0201,
        MEDIUM_CD_RW_S5 = 0x0202,
        MEDIUM_CD_RW_S6 = 0x0204,
        MEDIUM_CD_RW_S7 = 0x0208,
        MEDIUM_DVD_RW = 0x0210,
        MEDIA_BD_ROM = 0x400000,    /** Read-only Blu-ray Disc (BD) */
        MEDIA_BD_R = 0x800000,      /** Writable Blu-ray Disc (BD-R) */
        MEDIA_BD_R_SRM = 0x1000000, /** Writable Blu-ray Disc (BD-R) */
        MEDIA_BD_R_SRM_POW = 0x2000000,/** Writable Blu-ray Disc (BD-R) */
        MEDIA_BD_R_RRM = 0x4000000, /** Writable Blu-ray Disc (BD-R) */
        MEDIA_BD_RE = 0x8000000,    /** Rewritable Blu-ray Disc (BD-RE) */
    };
    enum MEDIUM_STATUS
    {
        MEDIUM_EMPTY = 0,
        MEDIUM_INCOMPLETE,
        MEDIUM_FINALIZED,
        MEDIUM_OTHER
    };
public:
    explicit DataMedium(QString, QString, QObject *);
    DataMedium(DataMedium &, QObject *);
    ~DataMedium();
public:
    bool isRWType();
    bool isCDType();
    bool isDVDType();

    bool isErasable() const { return m_bErasable; }
    bool isEnableUDF() const { return m_bEnableUDF; }
    QString getCapacity() const { return m_oCapacity; }
    unsigned long  get32Capacity() const { return m_u64Capacity; }
    QString getUsedCapacity() const { return m_oUsedCapacity; }
    unsigned long  get32UsedCapacity() const { return m_u64UsedCapacity; }
    QString getFreeCapacity() const { return m_oFreeCapacity; }
    unsigned long  get32FreeCapacity() const { return m_u64FreeCapacity; }
    QList<int>  getNumSpeed() const { return m_iSpeeds; }
    QStringList speeds() const { return m_oSpeeds; }
    QString display() const { return m_oDisplay; }
    int numDataTracks() const { return m_iNumDataTrack; }
    MEDIUM_STATUS status() const { return (MEDIUM_STATUS)m_u8Status; }
    QIcon icon() const { return m_oIcon; }
    QString getFileSystemType();
    QString getFileSystemExtendType();
    QString getType() const { return m_oMediumType; }
    QString getMountPoint() const { return m_oMountPoint; }
    QString getBlockName() const { return m_oBlockName; }
    void setUsedSize(unsigned long s) { m_u64UsedCapacity = s; }
    QString getMediumName() {return m_oMediumName;}
    QString getVolumeID() {return m_oVolumeID;}
    void load();

protected:
    void type();
    void capacity();
    void speed();
protected:
    void EnableUDF();
    QString toUnitString(unsigned long);
protected slots:
    void slotsChangeBurnFormat_CDROM();
    void fixDiscStatus();
    void slotsSecondGetDiscStatus(); //第一次获取光盘状态失败后，再次执行m_discread->work()
    void secondFixDiscStatus(); //检测到挂载点后，再次修正光盘状态
    void slotsMountPoint();
protected:
    unsigned char    m_u8Status;
    bool             m_bAppRunning;
    bool             m_bErasable;
    bool             m_bUDF;
    //DVD光盘 且 空状态
    //m_bEnableUDF为真表示可以使用UDF格式刻录
    bool             m_bEnableUDF;
    DataCDROM       *m_oCDROM;
    int              m_iNumDataTrack;
    unsigned int     m_u32TrackNumber;
    unsigned long    m_u64UsedCapacity;
    unsigned long    m_u64FreeCapacity;
    unsigned long    m_u64Capacity;
    QString          m_oVolumeID;
    QString          m_oBlockName;
    QString          m_oMediumType;
    QString          m_oUsedCapacity;
    QString          m_oFreeCapacity;
    QString          m_oCapacity;
    QString          m_oMountPoint;
    QString          m_oMediumName;
    QString          m_oDisplay;
    QIcon            m_oIcon;
    QList<int>       m_iSpeeds;
    QStringList      m_oSpeeds;
    DiscRead        *m_discread;
    QTimer          *m_mountPointTimer;
};

class DataMedium;
class DataCDROM : public QObject
{
    friend class MainWindowTest;
    Q_OBJECT
public:
    enum BURN_MODE
    {
        BURN_CD_TAO = 0x01,
        BURN_CD_SAO = 0x02,
        BURN_CD_DAO = 0x04 // close session/track when TAO over.
    };
public:
    explicit DataCDROM(QString blockName, QObject *parent = nullptr);
    DataCDROM(DataCDROM &, QString blockName, QObject *parent = nullptr);
    static bool umountMedium(QString);
    static bool mountMedium(QString);
    ~DataCDROM();
public:
    bool isOpen();
    bool isUdf();
    MEDIUM_SYSTEM_TYPE fileSystemTypeWithExtendName();
    bool update();
    void sendMediumStatusToOtherView();
    bool umount();
    bool doorIsOpen();
    void setVolumnID(QString);

    //释放销毁Medium
    void destroyMedium();

    DataCDROM * clone();
public:
    void use() { ++m_is_using; }
    void release() {--m_is_using;}
    int getUsedTimes(){return m_is_using;}
public slots:
    bool eject();
    void earse();
public:
    int getHandle() const {return m_iHandle;}
    bool isCDROM() const { return m_bCDROM; }
    bool isSupportBD() const { return m_bSupportBD; }
    bool isEjected();
    bool checkUsingOfOtherApp(bool isHideModalDialog = false);
    bool isEnableUDF() const {return m_oMedium ? m_oMedium->isEnableUDF() : false ;}
    QString device() const { return m_oBlockName; }
    QString display() const { return m_oDisplay; }
    void updateDisplay(QString s) { if(!m_bCDROM) m_oBlockName = s; }
    QString mDisplay();
    QString getBurnFormat() const {  return m_oSetting ? m_oSetting->getBurnFormat():"ISO9660"; }
    QString mediumType() const { return m_oMedium ? m_oMedium->getType() : m_oMediumType; }
    QIcon mediumIcon();
    int numDataTracks() const {return m_oMedium ? m_oMedium->numDataTracks() : 0;}
    QIcon icon() { return QIcon::fromTheme("drive-optical"); }
    unsigned long mediumFreeSize() const { return m_oMedium ? m_oMedium->get32FreeCapacity() : 0; }
    unsigned long mediumUsedSize() const { return m_oMedium ? m_oMedium->get32UsedCapacity() : 0; }
    int mediumStatus() const { return m_oMedium ? m_oMedium->status() : -1; }
    QStringList mediumSpeed() const { return m_oMedium ? m_oMedium->speeds() : QStringList(); }
    bool mediumIsErasable() const { return m_oMedium ? m_oMedium->isErasable() : false; }
    QString mediumMountPoint() const { return m_oMedium ? m_oMedium->getMountPoint() : QString(); }

    QString mediumName() const { return m_oMedium ? m_oMedium->getMediumName() : QString(); }
public:
    void setting() { if(m_oSetting) m_oSetting->load();}
    DataSetting* getSettings();

    // 从m_oSetting中获取值
    int settingSpeed() const { return m_oSetting ? m_oSetting->getSpeed() : 0; }
    bool settingClean() const { return m_oSetting ? m_oSetting->getClean() : false; }
    bool settingCreateISO() const { return m_oSetting ? m_oSetting->getCreateISO() : false; }
    bool settingCloseSession() const { return m_oSetting ? m_oSetting->getCloseSession() : false; }
    bool settingVerficationData() const { return m_oSetting ? m_oSetting->getVerficationData() : false; }
    bool settingOpen() const { return m_oSetting ? m_oSetting->getOpen() : false; }
    bool settingIsShowBeforBurn() const { return m_oSetting ? m_oSetting->getIsShowBeforBurn() : true; }
    QString settingPreparer() const { return m_oSetting ? m_oSetting->getPreparer() : QString(); }
    QString settingPublisher() const { return m_oSetting ? m_oSetting->getPublisher() : QString(); }
    QString settingVolumnID() const { return m_oSetting ? m_oSetting->getVolumnID() : QString(); }
    QString settingCopyright() const { return m_oSetting ? m_oSetting->getCopyright() : QString(); }
    QString settingTempPath() const { return m_oSetting ? m_oSetting->getTempPath() : QString(); }

    void updateVerficationData(bool b);


public: // scsi command
    bool open();
    void close();
    bool execSCSI(const unsigned char *, const int, unsigned char *, const int);
    bool execSCSI(int, const unsigned char *, const int, unsigned char *, const int);
    bool read10( unsigned char* data,
                  unsigned int dataLen,
                  unsigned long startAdress,
                  unsigned int length,
                  bool fua=false);


    void setSettingSpeed(int);
    void setSettingClean(bool);
    void setSettingCreateISO(bool);
    void setSettingCloseSession(bool);
    void setSettingVerficationData(bool);
    void setSettingSetOpen(bool);
    void setSettingCopyright(QString);
    void setSettingPreparer(QString);
    void setSettingPublisher(QString);
    void setSettingVolumnID(QString);
    void setSettingTempPath(QString);
    void setSettingBurnFormat(QString);
    void setSettingIsShowBeforBurn(bool);

public slots:
    void updateMediumUsedSize(unsigned long);
    void receiveDiscStatusChange();
    void changeISOPath(QString);
private:
    void prepare();
    bool loading();
    bool checkCDROM();
protected:
    QString display(QString);
    void support();
    void checkMedium();
protected slots:
    void changeISOName(QString);
    void slotsUpdateCDROMBurnStart(bool); //更新m_bStart状态

protected:
    int            m_iHandle;
    unsigned int   m_u32MediumRSupport;
    unsigned int   m_u32MediumWSupport;
    unsigned int   m_u32BurnModeSupport;
    bool           m_bEjected;
    bool           m_bCDROM;   //is cdrom
    bool           m_bAppRunning;
    bool           m_bSupportBD;
    DataSetting   *m_oSetting;
    DataMedium    *m_oMedium;
    QString        m_oDisplay;
    QString        m_oVendor;
    QString        m_oProductor;
    QString        m_oBlockName;
    QString        m_oMediumType;
    QString        m_unique_uuid;

public:
    bool           m_bUpdateStart;//用于控制在LogincMain中addDevice和changeDevice多次调用update导致错误的成员变量
    bool           m_bStart;//用于控制在光盘刻录过程中物理弹出按键打开光盘门导致刻录流程终端的成员变量
    int            m_is_using;//测试用，应该是私有的

    QString getUnique_uuid() const;

Q_SIGNALS:
    // medium
    void empty(); void appendale(); void finished();
    // setting
    void volumnID(QString);
    void tempPath(QString);
    // other
    void eject(bool);
    void updateCDROMBurnStart(bool); // 发送m_bStart状态
};

#endif // DATACDROM_H
