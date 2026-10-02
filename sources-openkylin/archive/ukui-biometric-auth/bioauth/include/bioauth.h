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
#ifndef BIOAUTH_H
#define BIOAUTH_H

#include <QObject>
#include "biotypes.h"

class QDBusInterface;

/*!
 * \brief The BioAuth class
 * 负责真正的生物识别操作，通过startAuth开始认证，
 * 认证完成后会发出携带结果的authComplete信号
 */
class BioAuth : public QObject
{
    Q_OBJECT
public:
    explicit BioAuth(qint32 uid, const DeviceInfoPtr deviceInfo, QObject *parent = nullptr);
    explicit BioAuth(QObject *parent = nullptr);
    ~BioAuth();
    void setDevice(const DeviceInfoPtr deviceInfo);
    void setUid(qint32 uid);
    DeviceInfoPtr getDevice();
    void startAuth();
    void startUkeyAuth();
    void startAuth(qint32 uid, const DeviceInfoPtr deviceInfo);
    void stopAuth();
    bool isAuthenticating();
    void init();
    /**
     * @brief 设置一些认证时所需的额外的信息
     * @param info_type         额外的信息类型，ukey pincode认证时传 "pincode"
     * @param extra_info        额外的信息内容，ukey pincode认证时传PIN码内容
     * @return  结果：<int result> （设置额外信息的结果)
     */
    int SetExtraInfo(QString info_type, QString extra_info);
    /**
         * @brief 获取当前用户已连接设备对应特征数目
         * @param uid              用户id
         * @param indexStart       用于认证的特征索引范围
         * @param indexEnd
         * @return  返回是否存在ukey特征
         */
    bool GetHasUkeyFeature(int uid, int indexStart = 0, int indexEnd = -1);

signals:
    void authComplete(int uid, bool result, int retErrNo);
    void authFinished();
    void notify(const QString &message);
    void notifyDetail(int deviceId, const QString &message);
    void frameWritten(int deviceId);

private slots:
    void onIdentityComplete(QDBusPendingCallWatcher *watcher);
    void onStatusChanged(int deviceId, int statusType);
    void onFrameWritten(int drvid);

private:
    QDBusInterface      *serviceInterface;

    qint32              uid;
    DeviceInfoPtr       deviceInfo = nullptr;
    bool isInAuthentication;
};

#endif // BIOAUTH_H
