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
#ifndef BIOCUSTOMTYPE_H
#define BIOCUSTOMTYPE_H

#include <QObject>
#include <QtDBus>

#define BIO_DBUS_SERVICE "org.ukui.Biometric"
#define BIO_DBUS_PATH "/org/ukui/Biometric"
#define BIO_DBUS_INTERFACE "org.ukui.Biometric"


#define DEFAULT_DEVICE "DefaultDevice"


#ifdef LOG
#undef LOG
#endif
#define LOG() qDebug() << "[BIOMETRIC]"

enum LOGINOPT_TYPE {
    LOGINOPT_TYPE_PASSWORD = 0,         // 密码
    LOGINOPT_TYPE_FACE,                 // 人脸
    LOGINOPT_TYPE_FINGERPRINT,          // 指纹
    LOGINOPT_TYPE_IRIS,                 // 虹膜
    LOGINOPT_TYPE_VOICEPRINT,           // 声纹
    LOGINOPT_TYPE_FINGERVEIN,           // 指静脉
    LOGINOPT_TYPE_GENERAL_UKEY,         // 普通的ukey
    LOGINOPT_TYPE_ADVANCED_UKEY,        // 高阶的ukey
    LOGINOPT_TYPE_QRCODE,               // 二维码
    LOGINOPT_TYPE_OTHERS,               // 其他
    LOGINOPT_TYPE_COUNT
};

/* the type of device */
enum BioType {
    BIOTYPE_FINGERPRINT,
    BIOTYPE_FINGERVEIN,
    BIOTYPE_IRIS,
    BIOTYPE_FACE,
    BIOTYPE_VOICEPRINT,
    __MAX_NR_BIOTYPES
};

#define REMOTE_QRCODE_TYPE  (8)

/* StatusChanged D-Bus 信号触发时的状态变化类型 */
enum StatusType {
    STATUS_DEVICE,
    STATUS_OPERATION,
    STATUS_NOTIFY
};

/* 录入/删除/搜索等 D-Bus 调用的最终结果，即返回值里的 result */
enum DBusResult {
    DBUS_RESULT_SUCCESS = 0,
    DBUS_RESULT_ERROR,
    DBUS_RESULT_DEVICEBUSY,
    DBUS_RESULT_NOSUCHDEVICE,
    DBUS_RESULT_PERMISSIONDENIED
};

/**
 * @brief 识别操作（Identify）的ops状态
 */
/* 定义操作类型 */
typedef enum {
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
}BioOpsType;

/*
 * 定义各种操作结果
 */
typedef enum {
    OPS_COMM_SUCCESS = OPS_TYPE_COMM * 100,	/** 空闲状态 **/
    OPS_COMM_FAIL,							/** 操作失败 **/
    OPS_COMM_NO_MATCH = OPS_COMM_FAIL,		/** 不匹配 **/
    OPS_COMM_ERROR,							/** 通用操作错误 **/
    OPS_COMM_STOP_BY_USER,					/** 用户取消 **/
    OPS_COMM_TIMEOUT,						/** 操作超时 **/
    OPS_COMM_OUT_OF_MEM,					/** 无法分配内存 **/
    OPS_COMM_MAX,

    OPS_OPEN_SUCCESS = OPS_TYPE_OPEN * 100,	/** 打开设备完成 **/
    OPS_OPEN_FAIL,							/** 打开设备失败 **/
    OPS_OPEN_ERROR,							/** 打开设备遇到错误 **/
    OPS_OPEN_MAX,

    OPS_ENROLL_SUCCESS = OPS_TYPE_ENROLL * 100,	/** 录入信息成功 **/
    OPS_ENROLL_FAIL,							/** 录入失败 **/
    OPS_ENROLL_ERROR,							/** 录入过程中遇到错误 **/
    OPS_ENROLL_STOP_BY_USER,					/** 录入被用户中断 **/
    OPS_ENROLL_TIMEOUT,							/** 操作超时 **/
    OPS_ENROLL_MAX,

    OPS_VERIFY_MATCH = OPS_TYPE_VERIFY * 100,	/** 认证匹配 **/
    OPS_VERIFY_NO_MATCH,						/** 认证不匹配 **/
    OPS_VERIFY_ERROR,							/** 认证过程中遇到错误 **/
    OPS_VERIFY_STOP_BY_USER,					/** 认证被用户中断 **/
    OPS_VERIFY_TIMEOUT,							/** 操作超时 **/
    OPS_VERIFY_MAX,

    OPS_IDENTIFY_MATCH = OPS_TYPE_IDENTIFY * 100,	/** 识别到指定特征 **/
    OPS_IDENTIFY_NO_MATCH,							/** 未识别出指定特征 **/
    OPS_IDENTIFY_ERROR,								/** 识别过程中遇到错误 **/
    OPS_IDENTIFY_STOP_BY_USER,						/** 识别被用户中断 **/
    OPS_IDENTIFY_TIMEOUT,							/** 操作超时 **/
    OPS_IDENTIFY_MAX,

    OPS_CAPTURE_SUCCESS = OPS_TYPE_CAPTURE * 100,	/** 捕获成功 **/
    OPS_CAPTURE_FAIL,								/** 捕获失败 **/
    OPS_CAPTURE_ERROR,								/** 捕获过程中遇到错误 **/
    OPS_CAPTURE_STOP_BY_USER,						/** 捕获被用户中断 **/
    OPS_CAPTURE_TIMEOUT,							/** 操作超时 **/
    OPS_CAPTURE_MAX,

    OPS_SEARCH_MATCH = OPS_TYPE_SEARCH * 100,	/** 搜索到指定特征 **/
    OPS_SEARCH_NO_MATCH,						/** 未搜索到指定特征 **/
    OPS_SEARCH_ERROR,							/** 搜索过程中遇到错误 **/
    OPS_SEARCH_STOP_BY_USER,					/** 搜索被用户中断 **/
    OPS_SEARCH_TIMEOUT,							/** 操作超时 **/
    OPS_SEARCH_MAX,

    OPS_CLEAN_SUCCESS = OPS_TYPE_CLEAN * 100,	/** 清理特征成功 **/
    OPS_CLEAN_FAIL,								/** 清理失败 **/
    OPS_CLEAN_ERROR,							/** 清理过程中遇到错误 **/
    OPS_CLEAN_STOP_BY_USER,						/** 清理被用户中断 **/
    OPS_CLEAN_TIMEOUT,							/** 操作超时 **/
    OPS_CLEAN_MAX,

    OPS_GET_FLIST_SUCCESS = OPS_TYPE_GET_FLIST * 100,	/** 获取特征列表完成 **/
    OPS_GET_FLIST_FAIL,									/** 获取特征列表失败 **/
    OPS_GET_FLIST_ERROR,								/** 获取特征列表过程中遇到错误 **/
    OPS_GET_FLIST_STOP_BY_USER,							/** 获取特征列表被用户中断 **/
    OPS_GET_FLIST_TIMEOUT,								/** 获取特征列表超时 **/
    OPS_GET_FLIST_MAX,

    OPS_RENAME_SUCCESS = OPS_TYPE_RENAME * 100,	/** 重命名特征完成 **/
    OPS_RENAME_FAIL,							/** 重命名特征失败 **/
    OPS_RENAME_ERROR,							/** 重命名特征过程中遇到错误 **/
    OPS_RENAME_STOP_BY_USER,					/** 重命名特征被用户中断 **/
    OPS_RENAME_TIMEOUT,							/** 重命名特征超时 **/
    OPS_RENAME_MAX,

    OPS_CLOSE_SUCCESS = OPS_TYPE_CLOSE * 100,	/** 关闭设备完成 **/
    OPS_CLOSE_FAIL,								/** 关闭设备失败 **/
    OPS_CLOSE_ERROR,							/** 关闭设备过程中遇到错误 **/
    OPS_CLOSE_MAX,
}OpsResult;

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

struct FeatureInfo {
    int uid;
    int biotype;
    QString device_shortname;
    int index;
    QString index_name;
};

/* the info of device */
struct DeviceInfo {
    int device_id;
    QString device_shortname; /* aka driverName */
    QString device_fullname;
    int driver_enable; /* The corresponding driver is enabled/disabled */
    int device_available; /* The driver is enabled and the device is connected */
    int biotype;
    int stotype;
    int eigtype;
    int vertype;
    int idtype;
    int bustype;
    int dev_status;
    int ops_status;

    bool operator==(const DeviceInfo&) const;

    bool operator !=(const DeviceInfo &deviceInfo) const
    {
        return !(*this == deviceInfo);
    }
    friend QDebug& operator<<(QDebug &stream, const DeviceInfo &deviceInfo);

};
Q_DECLARE_METATYPE(DeviceInfo)

typedef std::shared_ptr<DeviceInfo> DeviceInfoPtr;
typedef QList<DeviceInfoPtr> DeviceList;
typedef QMap<int, DeviceList> DeviceMap;
typedef std::shared_ptr<FeatureInfo> FeatureInfoPtr;
typedef QList<FeatureInfoPtr> FeatureList;
typedef QMap<QString, FeatureList> FeatureMap;

QDBusArgument &operator<<(QDBusArgument &argument, const DeviceInfo &deviceInfo);
const QDBusArgument &operator>>(const QDBusArgument &argument, DeviceInfo &deviceInfo);
QDebug operator <<(QDebug stream, const DeviceInfo &deviceInfo);
QDBusArgument &operator<<(QDBusArgument &argument, const FeatureInfo &featureInfo);
const QDBusArgument &operator>>(const QDBusArgument &argument, FeatureInfo &featureInfo);

QString bioTypeToString(int type);

bool getProductNameN90F3();

#endif // BIOCUSTOMTYPE_H
