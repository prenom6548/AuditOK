/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
/*** 
 * @Author: pengcheng
 * @Date: 2021-06-24 10:27:50
 * @LastEditTime: 2021-08-24 14:16:10
 * @LastEditors: pengcheng
 * @Description: 
 * @FilePath: /kylin-daq/src/DataCollection.h
 */
#ifndef DATACOLLECTIONMODULE_H
#define DATACOLLECTIONMODULE_H

#include <QtCore/QtCore>
#include <QtDBus/QtDBus>
#include <QtNetwork/QtNetwork>

#include "TransmitModule.h"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#include <iostream>
#include "BaseModule.h"
#include "KylinDaqCommon.h"


class DBusModule: public BaseModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.daq.interface")
public:
    /**
     * @brief dbus连接状态枚举
     * 
     */
    enum ConnectStatus
    {
        ConnectFailded = -1,
        ConnectSuccess = 1
    };
    /**
     * @brief UploadMessage返回值
     * 
     */
    enum UploadMessageReturnState{
        OK = 0,
        InvalidArgumentFormat = 1,
        InvalidTid = 2,
        InvalidUploadedMessageSha256 = 3,
        InvalidUploadedMessageSha256Decryption = 4,
        InvalidCreateTimeStamp = 5,
        DisableKey = 6,
        PrimaryKeyQueryStatementIsNull = 7,
        UploadTypeInvalid = 8
    };
    /**
     * @brief 设置上传状态放回值
     * 
     */
    enum SetState{
        inValid = 0,
        Valid = 1
    };



private:
    // dbus server
    /**
     * @brief dbus服务名
     * 
     */
    static constexpr const char *s_serverName = "com.kylin.daq";
    /**
     * @brief dbus路径名
     * 
     */
    static constexpr const char *s_objectPath = "/com/kylin/daq";
    /**
     * @brief dbus接口名
     * 
     */
    static constexpr const char *s_interfaceName = "com.kylin.daq.interface";
    /**
     * @brief 模块名
     * 
     */
    static constexpr const char *s_moduleName = "DBusModule";
    /**
     * @brief 传输模块名
     * 
     */
    static constexpr const char *s_transmitModuleName = "TransmitModule";
    /**
     * @brief 删除状态键
     * 
     */
    static constexpr const char *s_transmitModuleUploadStateKey = "UploadState";

    static constexpr const char *s_filterStateKey="FilterState";
    static constexpr const char *s_validPackageNameKey="ValidPackageName";

    /**
     * @brief RSA私钥
     * 
     */
    static constexpr const char *privateKey =
        "";
    /**
     * @brief 单例对象
     * 
     */
    static QScopedPointer<DBusModule> ms_instance;
    /**
     * @brief 被禁止的公共键
     *
     */
    QList<QString> m_disableCommonKey;
    /**
     * @brief 初始化被禁止的公共键
     *
     */
    void initDisableCommonKey();
    //	function
    //	rsa加密解密
    /**
     * @brief rsa加密
     * 
     * @param hasedValue 待加密值
     * @return QByteArray 加密结果
     */
    QByteArray encrypt(QByteArray hasedValue);
    /**
     * @brief rsa解密
     * 
     * @param hasedValue 待解密值
     * @return QByteArray 解密结果
     */
    QByteArray decrypt(QByteArray hasedValue);
    /**
     * @brief 打印加密错误
     * 
     */
    void evpError(void);
    /**
     * @brief 检测包信息
     * 
     * @param packInfo 包信息
     * @return UploadMessageReturnState 检测结果
     */
    UploadMessageReturnState checkPackageInfo(const QString &packInfo);
    /**
     * @brief  检测packageInfo中是否有无效字段
     * @param QJsonObject 包信息的json对象
     * @return UploadMessageReturnState 检测结果
     *
     */
    UploadMessageReturnState checkPackageInfoInvalidKey(const QJsonObject &packageInfoJsonObj);
    /**
     * @brief 检测hase值是否正常
     * 
     * @param information information值
     * @param hasedValue hase值
     * @return UploadMessageReturnState 检测结果
     */
    UploadMessageReturnState checkHashedValue(const QString &information,const QString &hasedValue);
    /**
     * @brief 检测information中是否包含必要字段
     * 
     * @param uploadedMessage 上传消息
     * @return UploadMessageReturnState 检测结果
     */
    UploadMessageReturnState checkUploadedMessage(const QString &uploadedMessage);
    /**
     * @brief  检测uploadedMessage中是否有无效字段
     * @param QJsonObject 待上传消息的json对象
     * @return UploadMessageReturnState 检测结果
     *
     */
    UploadMessageReturnState checkUploadedMessageInvalidKey(const QJsonObject &uploadedMessageJsonObj);
    /**
      * @brief  检测uploadedMessage中时间戳是否有效
      * @param QJsonObject uploadedMessage的json对象
      * @return UploadMessageReturnState 检测结果
      *
      */
    UploadMessageReturnState checkUploadEventMessage(const QString &uploadedMessage);
    UploadMessageReturnState checkUploadedMessageTimeStampState(const QJsonObject &uploadedMessageJsonObj);

    /**
     *  @brief 检测uploadType
     *  @param uploadType
     *  @return UploadMessageReturnState  检测结果
    */
    UploadMessageReturnState checkUploadType(const int32_t &uploadType);
    /**
      * @brief  检测UploadMessage参数是否有效
      * @param packageInfo 包信息
      * @param uploadedMessage 被上传信息
      * @param uploadedMessageSha256 被上传信息的sha256的加密值
      * @return UploadMessageReturnState 检测结果
      *
    */
    UploadMessageReturnState checkUploadMessage(const QString &packageInfo, const QString &uploadedMessage, const QString &uploadedMessageSha256);

    /**
      * @brief  检测UploadStateMessage参数是否有效
      * @param packageInfo 包信息
      * @param uploadedMessage 被上传信息
      * @param uploadedMessageSha256 被上传信息的sha256的加密值
      * @param uploadType
      * @return UploadMessageReturnState 检测结果
      *
    */
    UploadMessageReturnState checkUploadStateMessage(const QString &packageInfo, const QString &uploadedMessage, const QString &uploadedMessageSha256, const KylinDaqCommon::UploadStateEnum &uploadType);
    /**
     * @brief 依据packInfo创建Tid
     * 
     * @param packInfo 包信息
     * @return QString tid
     */
    QString createPackInfoId(const QString &packInfo);
    /**
     * @brief 获取packInfo中的tid
     * 
     * @param packageInfo 包信息
     * @return QString tid
     */
    QString getPackageInfoTid(const QString &packageInfo);
    /**
     * @brief 填充source信息
     * 
     * @param packageInfo 包信息
     * @param tid tid
     * @param newTid 新的tid
     * @return QString 填充完的结果
     */
    QString fillSource(const QString &packageInfo, const QString &tid, const QString &newTid);
    /**
     * @brief Create a Valid Message object
     * 
     */
    void createValidMessage();
    /**
     * @brief 
     * 
     * @param source 
     * @param uploadedMessage 
     * @return QString 
     */
    QString fillValidInfo(const QString &source,const QString &uploadedMessage);
    /**
     * @brief Construct a new DBusModule object
     * 
     * @param parent 
     */
    explicit DBusModule(QObject *parent = nullptr);
    /**
     * @brief 检测包信息
     *
     * @param packInfo 包信息
     * @return UploadMessageReturnState 检测结果
     */
    void getPackageInfo(const QString &packInfo, QString &packageName, QString &messageType);

    /*
    * @brief 
    */
    UploadMessageReturnState createStateMessageInfo(const QString &packageInfo, const QString &uploadedMessage, KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo);
    QString createMessage(KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo);

    void saveStateMessageInfo(KylinDaqCommon::UploadStateMessageInfo_t &uploadStateMessageInfo,KylinDaqCommon::UploadStateMessageInfo_t &oldUploadStateMessageInfo,const KylinDaqCommon::KylinDaqCommon::UploadTypeEnum &uploadType);
public:
    /**
     * @brief Destroy the DBusModule object
     * 
     */
    virtual ~DBusModule();
    /**
     * @brief instance function
     * 
     * @return DBusModule* 
     */
    static DBusModule *instance(void);
public slots:
    /**
     * @brief 获取上传状态
     * 
     * @return int 上传状态
     */
    Q_SCRIPTABLE int GetUploadState();
    /**
     * @brief 设置上传状态
     * 
     * @param setState 被设置的上传状态
     * @return int 设置结果
     */
    Q_SCRIPTABLE int SetUploadState(int setState);
    /**
     * @brief 上传信息
     * 
     * @param packageInfo 包信息
     * @param uploadedMessage 待上传信息
     * @param uploadedMessageSha256 待上传信息散列值
     * @param retTid 返回的tid
     * @return int 返回状态
     */
    Q_SCRIPTABLE int UploadMessage(const QString &packageInfo, const QString &uploadedMessage, const QString &uploadedMessageSha256, QString &retTid);
    /**
     * @brief 上传事件信息
     *
     * @param uploadedMessage 待上传信息
     * @param uploadedMessageSha256 待上传信息散列值
     * @return int 返回状态
     */
    Q_SCRIPTABLE int UploadEventMessage(const QString &uploadedMessage, const QString &uploadedMessageSha256);

    /**
     * @brief 上传信息
     * 
     * @param packageInfo 包信息
     * @param uploadedMessage 待上传信息
     * @param uploadedMessageSha256 待上传信息散列值
     * @param uploadType 上传类型
     * @param retTid 返回的tid
     * @return int 返回状态
     */
    Q_SCRIPTABLE int UploadStateMessage(const QString &packageInfo, const QString &uploadedMessage, const QString &uploadedMessageSha256, const qint32 &uploadType, QString &retTid);
    /**
     * @brief 依据包名和消息类型在数据库中查询tid
     * 
     * @param packageName 包名
     * @param messageType 消息类型
     * @return QString tid
     */
    Q_SCRIPTABLE QString GetTid(const QString &packageName, const QString &messageType);
    virtual void updateProperty(const QJsonObject &conf,const QJsonObject &defaultConf);
signals:
    /**
     * @brief 发送埋点数据
     * 
     * @param info 埋点数据
     */
    void saveInfo(const QString &info);
    /**
     * @brief 发送事件埋点数据
     *
     * @param info 埋点数据
     */
    void saveEventInfo(const QString &info);
    /**
     * @brief 发送tid数据
     * 
     * @param info tid
     */
    void saveTid(const QString &info);
    /**
     * @brief 检测tid是否存在
     * 
     * @param info tid
     * @param existState tid是否存在于数据库
     */
    void checkTid(const QString &info, bool &existState);
    /**
     * @brief 存储上传状态配置
     * 
     * @param moduleName 模块名
     * @param key 键
     * @param value 值
     */
    void saveUploadState(const QString &moduleName, const QString &key, const QVariant &value);

    /**
     * @brief 上传状态设置成功后发送给 D-Bus 客户端的通知信号
     *
     * @param uploadState 当前上传状态
     */
    Q_SCRIPTABLE void UploadStateChanged(int uploadState);

    void getHisotryStateMessage(const QString &packageInfo, QString &uploadedMessage, QString &tids, QString &createTimeStamp, KylinDaqCommon::UploadStateEnum &uploadState);
    void saveStateMessage(const QString &packageInfo, const QString &uploadedMessage, const QString &tids, const QString &createTimeStamp,const KylinDaqCommon::UploadStateEnum &uploadState);
    void updateStateMessage(const QString &packageInfo, const QString &uploadedMessage, const QString &tids, const QString &createTimeStamp,KylinDaqCommon::UploadStateEnum &uploadState);
private:
    Q_PROPERTY(KylinDaqCommon::UploadState uploadState READ uploadState WRITE setUploadState NOTIFY uploadStateChanged)
    Q_PROPERTY(SetState filterState READ filterState WRITE setFilterState NOTIFY filterStateChanged)
    Q_PROPERTY(QList<QString> validPackageName READ validPackageName WRITE setValidPackageName NOTIFY validPackageNameChanged)

    KylinDaqCommon::UploadState m_uploadState;
    SetState m_filterState;

    QList<QString> m_validPackageName;

public:
    KylinDaqCommon::UploadState uploadState() const
    {
        return m_uploadState;
    }
    SetState filterState() const
    {
        return m_filterState;
    }

    QList<QString> validPackageName() const
    {
        return m_validPackageName;
    }

public slots:
    void setUploadState(KylinDaqCommon::UploadState uploadState)
    {
        if (m_uploadState == uploadState)
            return;

        m_uploadState = uploadState;
        emit uploadStateChanged(m_uploadState);
        emit saveUploadState(QString(s_transmitModuleName),QString(s_transmitModuleUploadStateKey),KylinDaqCommon::UploadState(m_uploadState));
    }
    void setFilterState(SetState filterState)
    {
        if (m_filterState == filterState)
            return;

        m_filterState = filterState;
        emit filterStateChanged(m_filterState);
    }

    void setValidPackageName(QList<QString> validPackageName)
    {
        if (m_validPackageName == validPackageName)
            return;

        m_validPackageName = validPackageName;
        emit validPackageNameChanged(m_validPackageName);
    }

signals:
    void uploadStateChanged(KylinDaqCommon::UploadState uploadState);
    void filterStateChanged(SetState filterState);
    void validPackageNameChanged(QList<QString> validPackageName);
};

#endif // TEST_H