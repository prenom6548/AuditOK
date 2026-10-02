/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#ifndef BIOMETRICSWIDGET_H
#define BIOMETRICSWIDGET_H

#include <QWidget>
#include <ukcc/widgets/hoverwidget.h>
#include <ukcc/interface/ukcccommon.h>
#include "biometricdeviceinfo.h"
#include "biometricproxy.h"
#include "biometricenroll.h"
#include "qrcodeenroll.h"
#include "biometricmoreinfo.h"
#include "pwdchangethread.h"
#include "enrollbtn.h"
#include "kswitchbutton.h"
#include "uniauthservice.h"
#include "ksecurityquestiondialog.h"
#include "securityquestionanswer.h"
#include "pwdlessproxy.h"
#include "messagebar.h"
#include <kysdk/applications/accessinfohelper.h>
#include <kysdk/applications/kborderlessbutton.h>
#include <kysdk/applications/klineframe.h>

typedef struct _UserInfomation
{
    QString objpath;
    QString username;
    QString realname;
    QString iconfile;
    QString passwd;
    int accounttype;
    int passwdtype;
    bool current;
    bool logined;
    bool autologin;
    bool noPwdLogin;
    qint64 uid;
} UserInfomation;

enum
{
    STANDARDUSER,
    ADMINISTRATOR,
    ROOT
};

QT_BEGIN_NAMESPACE
namespace Ui
{
class BiometricsWidget;
}
using namespace kdk;
using namespace ukcc;

QT_END_NAMESPACE

class BiometricsWidget : public QWidget
{
    Q_OBJECT

public:
    BiometricsWidget(QWidget *parent = nullptr);
    ~BiometricsWidget();

private:
    Ui::BiometricsWidget *ui;
    // 初始化生物特征组件
    void initBioComonent();
    // 添加生物特征
    void addFeature(FeatureInfoPtr &featureinfo);
    void initSearchText();
    // 更新生物特征设备
    void initDashboardBioAuthSection();
    void updateDevice();
    void updateFeatureList();
    void setCurrentDevice(int drvid);
    void setCurrentDevice(const QString &deviceName);
    void setCurrentDevice(const DeviceInfoPtr &pDeviceInfo);
    DeviceInfoPtr findDeviceById(int drvid);
    DeviceInfoPtr findDeviceByName(const QString &name);
    bool deviceExists(int drvid);
    bool deviceExists(const QString &deviceName);
    void showEnrollDialog();
    void showVerifyDialog(FeatureInfoPtr featureinfo);
    void deleteFeature();
    void deleteFeaturedone(FeatureInfoPtr feature);
    void renameFeaturedone(FeatureInfoPtr feature, QString newname);
    void setBiometricDeviceVisible(QString userName, int authType, bool visible);
    void biometricShowMoreInfoDialog();
    bool isShowBiometric();
    void initUserInfo();
    QString _accountTypeIntToString(int type);
    void showQRCodeScanDialog();
    bool isQRCodeModuleExist();
    bool isPwdlessKey();
    void onMfaStatusChanged(bool status,QString method);

    /**
     * @brief 判断Key驱动设备是否存在
     *
     * @return bool
     */
    bool isUKeyModuleExist();

    QString getCurUserQRCodeAcount(QString &strDrivName, int &drivId);
    void readConfigures();
    QString btnHoverColor(QString styleName, bool hoverFlag);
    bool polkitSecurityQuestions();

    void registerCustomTypes();

    void getDeviceInfo();

    void updateEnableTypeList();

    bool getProductNameN90F3();

    // 增加生物密码
    EnrollBtn *addBioFeatureBtn;
    QHBoxLayout *addUserHorLayout;
    QFrame *addUserFrame;
    QVBoxLayout *otherVerLayout;

    DeviceMap deviceMap;
    DeviceInfoPtr currentDevice;
    BiometricProxy *m_biometricProxy;
    QDBusInterface *serviceInterface;
    QFileSystemWatcher *mBiometricWatcher;
    KSwitchButton *enableBiometricBtn;
    QMap<QString, QListWidgetItem *> biometricFeatureMap;
    UserInfomation m_user;
    QMenu *menu;
    QLabel *textLabel;
    QString m_strBoundWechat;
    bool m_isEnabelQRCode = true;
    bool isShowDialog = false;
    bool isShowEnrollDialog = false;
    UniAuthService *m_uniAuthService = nullptr;
    bool enableAuth = false;
    bool m_isEnableUKey = true;
    bool m_isFeatureChanged = false;
    bool m_isEnableOtp = true;

    PwdlessProxy *m_pwdlessProxy = nullptr;
    QDBusInterface *serviceServiceInterface = nullptr;
    bool hasBindOtp = false;

    SecurityQuestionAnswer *m_pSecurityQuestionAnswer = nullptr;
    KSecurityQuestionDialog *m_pSecQueDig = nullptr;
    QString verifyToken;
    QProcess *m_procSecAnswer = nullptr;
    MessageBar *m_mfaMessage = nullptr;

    int deviceCount;
    QMap<int, QList<DeviceInfo *>> deviceInfosMap;
    QList<int> m_enableTypeList;

protected:
    void resizeEvent(QResizeEvent *event);

signals:
    void updateQRCodeInfo();
    void updateOtpInfo();
    void updateFunctionsWidget();
    void driverAttachedChanged(bool isAttached, DeviceList devInfoList);

private slots:
    void onbiometricTypeBoxCurrentIndexChanged(int index);
    void onbiometricDeviceBoxCurrentIndexChanged(int index);
    void updateFeatureListCallback(QDBusMessage callbackReply);
    void onFeatureChanged(int drvid, int uid, int cType);
    void errorCallback(QDBusError error);
    void showChangePwdDialog();
    void onUpdateFunctionsWidget();
    void showSecurityQuestionDialog();

    /**
     * @brief 设置安全密钥
     *
     */
    void onSetSecurityKey();
    /**
     * @brief 设置otp
     *
     */
    void onSetOtpKey();
    /**
     * @brief USB设备热插拔
     * @param drvid     设备id
     * @param action    插拔动作（1：插入，-1：拔出）
     * @param deviceNum 插拔动作后该驱动拥有的设备数量
     */
    void onBiometricUSBDeviceHotPlug(int drvid, int action, int deviceNum);

    void onUpdateQRCodeInfo();

    void onBiometricServiceStatus(bool isActive);

    void onUpdateOtpInfo();

    void updateEnableTypeListCallback(QDBusMessage callbackReply);

    void onAuthTypeBoxCurrentIndexChanged(int index);

private:
    bool authoriyUserForQRCode(bool isEnroll);
};
#endif // BIOMETRICSWIDGET_H
