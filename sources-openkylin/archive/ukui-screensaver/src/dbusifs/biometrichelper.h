/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef BIOMETRICHELPER_H
#define BIOMETRICHELPER_H

#include <QtDBus>
#include <QDBusAbstractInterface>
#include <QDBusUnixFileDescriptor>
#include "../../src/common/biodefines.h"

class BiometricHelper : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    explicit BiometricHelper(QObject *parent = nullptr);

public Q_SLOTS:
    /**
     * @brief 使用指定id的设备进行用户认证
     * @param drvid         驱动（设备）id
     * @param uid           用户id
     * @param indexStart    用于认证的特征索引范围
     * @param indexEnd
     * @return  结果：<int result, int uid> （结果，用户id)
     */
    QDBusPendingCall Identify(int drvid, int uid, int indexStart = 0, int indexEnd = -1);
    /**
     * @brief 使用指定id的设备进行用户认证
     * @param drvid         驱动（设备）id
     * @param type          ukey的认证类型，2表示pin认证（需通过setExtraInfo设置pin码,3表示指纹认证）
     * @param uid           用户id
     * @return  结果：<int result, int uid> （结果，用户id)
     */
    QDBusPendingCall UkeyIdentify(int drvid, int type, int uid);
    /**
     * @brief 设置一些认证时所需的额外的信息
     * @param info_type         额外的信息类型，ukey pincode认证时传 "pincode"
     * @param extra_info        额外的信息内容，ukey pincode认证时传PIN码内容
     * @return  结果：<int result> （设置额外信息的结果)
     */
    int SetExtraInfo(QString info_type, QString extra_info);
    /**
     * @brief 终止设备上正在进行的操作
     * @param drvid     设备id
     * @param waiting   等待时间（秒）
     * @return
     */
    int StopOps(int drvid, int waiting = 3000);
    /**
     * @brief 获取当前用户已连接设备对应特征数目
     * @param uid              用户id
     * @param indexStart       用于认证的特征索引范围
     * @param indexEnd
     * @return
     */
    int GetFeatureCount(int uid, int indexStart = 0, int indexEnd = -1);
    /**
     * @brief 获取当前用户已连接设备对应特征数目
     * @param uid              用户id
     * @param indexStart       用于认证的特征索引范围
     * @param indexEnd
     * @return  返回是否存在ukey特征
     */
    bool GetHasUkeyFeature(int uid, int indexStart = 0, int indexEnd = -1);
    /**
     * @brief 获取已连接的设备列表
     * @return
     */
    DeviceList GetDevList();
    /**
     * @brief 获取设备数量
     * @return
     */
    int GetDevCount();
    /**
     * @brief 获取设备消息
     * @param drvid 驱动id
     * @return
     */
    QString GetDevMesg(int drvid);
    /**
     * @brief GetNotifyMesg 获取通知消息
     * @param drvid 驱动id
     * @return
     */
    QString GetNotifyMesg(int drvid);
    /**
     * @brief GetOpsMesg    获取操作消息
     * @param drvid 驱动id
     * @return
     */
    QString GetOpsMesg(int drvid);
    /**
     * @brief UpdateStatus  获取更新的设备状态
     * @param drvid 驱动id
     * @return  结果：<int result, int enable, int devNum,
     *                int devStatus, int opsStatus, notifyMessageId, ...>
     */
    StatusReslut UpdateStatus(int drvid);
    int GetUserDevCount(int uid);
    int GetUserDevFeatureCount(int uid, int drvid);
    /**
     * @brief GetUserFeatures  获取用户所有特征
     * @param uid 用户id
     * @return  特征信息列表
     */
    FeatureMap GetUserFeatures(int uid);

Q_SIGNALS:
    /**
     * @brief 设备状态发生变化
     * @param drvid     设备id
     * @param status    设备状态
     */
    void StatusChanged(int drvid, int status);
    /**
     * @brief USB设备热插拔
     * @param drvid     设备id
     * @param action    插拔动作（1：插入，-1：拔出）
     * @param deviceNum 插拔动作后该驱动拥有的设备数量
     */
    void USBDeviceHotPlug(int drvid, int action, int deviceNum);
    void FrameWritten(int drvid);
};

#endif // BIOMETRICHELPER_H
