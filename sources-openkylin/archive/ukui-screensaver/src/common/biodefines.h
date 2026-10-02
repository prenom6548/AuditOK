/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 **/

#ifndef BIODEFINES_H
#define BIODEFINES_H

#include <QString>
#include <QtDBus>

#define BIOMETRIC_DBUS_SERVICE "org.ukui.Biometric"
#define BIOMETRIC_DBUS_PATH "/org/ukui/Biometric"
#define BIOMETRIC_DBUS_INTERFACE "org.ukui.Biometric"

#define UKUI_BIOMETRIC_IMAGES_PATH "/usr/share/ukui-biometric/images/"
#define UKUI_BIOMETRIC_CONFIG_PATH ".biometric_auth/ukui_biometric.conf"
#define UKUI_BIOMETRIC_SYS_CONFIG_PATH "/etc/biometric-auth/ukui-biometric.conf"
#define SHARE_BIOMETRIC_CONFIG_PATH                                                                                    \
    "/var/lib/lightdm-data/%1/ukui-biometric.conf" // greeter、screensaver、polkit share conf

/**
 * @brief StatusChanged D-Bus 信号触发时的状态变化类型
 */
enum StatusType
{
    STATUS_DEVICE,
    STATUS_OPERATION,
    STATUS_NOTIFY
};

/**
 * @brief 识别、终止操作等DBus调用的结果，即返回值里的 result
 */
enum DBusResult
{
    DBUS_RESULT_SUCCESS = 0,
    DBUS_RESULT_NOTMATCH = -1,
    DBUS_RESULT_ERROR = -2,
    DBUS_RESULT_DEVICEBUSY = -3,
    DBUS_RESULT_NOSUCHDEVICE = -4,
    DBUS_RESULT_PERMISSIONDENIED = -5
};

/**
 * @brief 识别操作（Identify）的ops状态
 */
/* 定义操作类型 */
typedef enum
{
    OPS_TYPE_COMM = 0,
    OPS_TYPE_OPEN,
    OPS_TYPE_ENROLL,
    OPS_TYPE_VERIFY,
    OPS_TYPE_IDENTIFY,
    OPS_TYPE_CAPTURE,
    OPS_TYPE_SEARCH,
    OPS_TYPE_CLEAN,
    OPS_TYPE_GET_FLIST,
    OPS_TYPE_RENAME,
    OPS_TYPE_CLOSE,
} BioOpsType;

/*
 * 定义各种操作结果
 */
typedef enum
{
    OPS_COMM_SUCCESS = OPS_TYPE_COMM * 100, /** 空闲状态 **/
    OPS_COMM_FAIL,                          /** 操作失败 **/
    OPS_COMM_NO_MATCH = OPS_COMM_FAIL,      /** 不匹配 **/
    OPS_COMM_ERROR,                         /** 通用操作错误 **/
    OPS_COMM_STOP_BY_USER,                  /** 用户取消 **/
    OPS_COMM_TIMEOUT,                       /** 操作超时 **/
    OPS_COMM_OUT_OF_MEM,                    /** 无法分配内存 **/
    OPS_COMM_MAX,

    OPS_OPEN_SUCCESS = OPS_TYPE_OPEN * 100, /** 打开设备完成 **/
    OPS_OPEN_FAIL,                          /** 打开设备失败 **/
    OPS_OPEN_ERROR,                         /** 打开设备遇到错误 **/
    OPS_OPEN_MAX,

    OPS_ENROLL_SUCCESS = OPS_TYPE_ENROLL * 100, /** 录入信息成功 **/
    OPS_ENROLL_FAIL,                            /** 录入失败 **/
    OPS_ENROLL_ERROR,                           /** 录入过程中遇到错误 **/
    OPS_ENROLL_STOP_BY_USER,                    /** 录入被用户中断 **/
    OPS_ENROLL_TIMEOUT,                         /** 操作超时 **/
    OPS_ENROLL_MAX,

    OPS_VERIFY_MATCH = OPS_TYPE_VERIFY * 100, /** 认证匹配 **/
    OPS_VERIFY_NO_MATCH,                      /** 认证不匹配 **/
    OPS_VERIFY_ERROR,                         /** 认证过程中遇到错误 **/
    OPS_VERIFY_STOP_BY_USER,                  /** 认证被用户中断 **/
    OPS_VERIFY_TIMEOUT,                       /** 操作超时 **/
    OPS_VERIFY_MAX,

    OPS_IDENTIFY_MATCH = OPS_TYPE_IDENTIFY * 100, /** 识别到指定特征 **/
    OPS_IDENTIFY_NO_MATCH,                        /** 未识别出指定特征 **/
    OPS_IDENTIFY_ERROR,                           /** 识别过程中遇到错误 **/
    OPS_IDENTIFY_STOP_BY_USER,                    /** 识别被用户中断 **/
    OPS_IDENTIFY_TIMEOUT,                         /** 操作超时 **/
    OPS_IDENTIFY_MAX,

    OPS_CAPTURE_SUCCESS = OPS_TYPE_CAPTURE * 100, /** 捕获成功 **/
    OPS_CAPTURE_FAIL,                             /** 捕获失败 **/
    OPS_CAPTURE_ERROR,                            /** 捕获过程中遇到错误 **/
    OPS_CAPTURE_STOP_BY_USER,                     /** 捕获被用户中断 **/
    OPS_CAPTURE_TIMEOUT,                          /** 操作超时 **/
    OPS_CAPTURE_MAX,

    OPS_SEARCH_MATCH = OPS_TYPE_SEARCH * 100, /** 搜索到指定特征 **/
    OPS_SEARCH_NO_MATCH,                      /** 未搜索到指定特征 **/
    OPS_SEARCH_ERROR,                         /** 搜索过程中遇到错误 **/
    OPS_SEARCH_STOP_BY_USER,                  /** 搜索被用户中断 **/
    OPS_SEARCH_TIMEOUT,                       /** 操作超时 **/
    OPS_SEARCH_MAX,

    OPS_CLEAN_SUCCESS = OPS_TYPE_CLEAN * 100, /** 清理特征成功 **/
    OPS_CLEAN_FAIL,                           /** 清理失败 **/
    OPS_CLEAN_ERROR,                          /** 清理过程中遇到错误 **/
    OPS_CLEAN_STOP_BY_USER,                   /** 清理被用户中断 **/
    OPS_CLEAN_TIMEOUT,                        /** 操作超时 **/
    OPS_CLEAN_MAX,

    OPS_GET_FLIST_SUCCESS = OPS_TYPE_GET_FLIST * 100, /** 获取特征列表完成 **/
    OPS_GET_FLIST_FAIL,                               /** 获取特征列表失败 **/
    OPS_GET_FLIST_ERROR,                              /** 获取特征列表过程中遇到错误 **/
    OPS_GET_FLIST_STOP_BY_USER,                       /** 获取特征列表被用户中断 **/
    OPS_GET_FLIST_TIMEOUT,                            /** 获取特征列表超时 **/
    OPS_GET_FLIST_MAX,

    OPS_RENAME_SUCCESS = OPS_TYPE_RENAME * 100, /** 重命名特征完成 **/
    OPS_RENAME_FAIL,                            /** 重命名特征失败 **/
    OPS_RENAME_ERROR,                           /** 重命名特征过程中遇到错误 **/
    OPS_RENAME_STOP_BY_USER,                    /** 重命名特征被用户中断 **/
    OPS_RENAME_TIMEOUT,                         /** 重命名特征超时 **/
    OPS_RENAME_MAX,

    OPS_CLOSE_SUCCESS = OPS_TYPE_CLOSE * 100, /** 关闭设备完成 **/
    OPS_CLOSE_FAIL,                           /** 关闭设备失败 **/
    OPS_CLOSE_ERROR,                          /** 关闭设备过程中遇到错误 **/
    OPS_CLOSE_MAX,
} OpsResult;

/**
 * @brief UpdateStauts调用返回的结果
 */
struct StatusReslut
{
    int result;
    int enable;
    int devNum;
    int devStatus;
    int opsStatus;
    int notifyMessageId;
};

/**
 * @brief USB设备插拔动作
 */
enum USBDeviceAction
{
    ACTION_ATTACHED = 1,
    ACTION_DETACHED = -1
};

/**
 * @brief 特征的信息
 */
struct FeatureInfo
{
    int uid;
    int biotype;
    QString device_shortname;
    int index;
    QString index_name;
};

/**
 * @brief 设备的信息
 */
struct DeviceInfo
{
    int id = -1;
    QString shortName;
    QString fullName;
    int driverEnable;
    int deviceNum;
    int deviceType;
    int storageType;
    int eigType;
    int verifyType;
    int identifyType;
    int busType;
    int deviceStatus;
    int OpsStatus;
};

QDBusArgument &operator<<(QDBusArgument &arg, const DeviceInfo &deviceInfo);
const QDBusArgument &operator>>(const QDBusArgument &arg, DeviceInfo &deviceInfo);
QDBusArgument &operator<<(QDBusArgument &argument, const FeatureInfo &featureInfo);
const QDBusArgument &operator>>(const QDBusArgument &argument, FeatureInfo &featureInfo);

typedef std::shared_ptr<DeviceInfo> DeviceInfoPtr;
typedef QList<DeviceInfoPtr> DeviceList;
typedef QMap<int, DeviceInfoPtr> DeviceMap;
typedef std::shared_ptr<FeatureInfo> FeatureInfoPtr;
typedef QList<FeatureInfoPtr> FeatureList;
typedef QMap<QString, FeatureList> FeatureMap;

QDebug operator<<(QDebug stream, const FeatureInfo &featureInfo);
QDebug operator<<(QDebug stream, const DeviceInfo &deviceInfo);

Q_DECLARE_METATYPE(DeviceInfo)
Q_DECLARE_METATYPE(FeatureInfo)

QString getDeviceTypeTr(int deviceType);

typedef enum
{
    BioT_FingerPrint, /** 指纹 **/
    BioT_FingerVein,  /** 指静脉 **/
    BioT_Iris,        /** 虹膜 **/
    BioT_Face,        /** 人脸 **/
    BioT_VoicePrint,  /** 声纹 **/

    UniT_KCM,            /** 安全管控 **/
    UniT_General_Ukey,   /** 普通的Ukey **/
    UniT_Advanced_Ukey,  /** 高阶的Ukey **/
    UniT_Remote,         /** 远程账户 **/
    UniT_Custom = 0xFFFF /** 第三方 **/
} BioType;

enum BIOAUTH_STATE
{
    BIOAUTH_IDLE = 0,
    BIOAUTH_START,
    BIOAUTH_TIMEOUT,
    BIOAUTH_DEVICEREMOVED,
    BIOAUTH_COMPLETE
};

#endif // BIODEFINES_H
