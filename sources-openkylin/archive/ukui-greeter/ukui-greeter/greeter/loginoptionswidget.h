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
#ifndef LOGINOPTIONSWIDGET_H
#define LOGINOPTIONSWIDGET_H

#include <QWidget>
#include "biometricproxy.h"
#include "uniauthservice.h"
#include "loginauthinterface.h"

class QLabel;
class QButtonGroup;
class QHBoxLayout;
class QVBoxLayout;
class QPushButton;
class QTimer;
class QPixmap;
class KLabel;
class Configuration;

typedef enum {
    BioT_FingerPrint,		/** 指纹 **/
    BioT_FingerVein,		/** 指静脉 **/
    BioT_Iris,				/** 虹膜 **/
    BioT_Face,				/** 人脸 **/
    BioT_VoicePrint,		/** 声纹 **/

    UniT_KCM,				/**	安全管控 **/
    UniT_General_Ukey,		/** 普通的Ukey **/
    UniT_Advanced_Ukey,		/** 高阶的Ukey **/
    UniT_Remote,            /** 远程账户 **/
    UniT_Custom = 0xFFFF    /** 第三方 **/
}BioType;

#define CUSTOM_PLUGIN_DEV_PREFIX ("Custom:")

enum OPTION{
    RIGHT = 0,
    LEFT,
    CLEAR,
    CLICK,
    FirstDevice,
};

class LoginOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LoginOptionsWidget(BiometricProxy* proxy, int uid, UniAuthService* uniauthService, QWidget *parent = nullptr);
    virtual ~LoginOptionsWidget();

    bool getCurLoginOpt(int& nLoginOptType, int& nDrvId);
    unsigned getLoginOptCount();
    DeviceInfoPtr getFirstDevInfo();
    DeviceInfoPtr getWechatDevice();
    int convertDeviceType(int nDevType);

    void setUser(int uid);
    void setCurrentDevice(int drvid);
    void setCurrentDevice(const QString &deviceName);
    void setCurrentDevice(const DeviceInfoPtr &pDeviceInfo);
    DeviceInfoPtr findDeviceById(int drvid);
    DeviceInfoPtr findDeviceByName(const QString &name);
    void setDeviceDisable(int nDevId, bool bDisable = true);
    bool isDeviceDisable(int nDevId);
    bool getBioAuthEnable(int nType);
    bool getQRCodeEnable();
    QString getDefaultDevice(QString strUserName,int biotype);
    QString getDefaultDevice(QString strUserName);
    QStringList getAllDefDevices();
    void SetExtraInfo(QString extra_info,QString info_type);
    bool getHasUkeyOptions();
    void setSelectedPassword();
    void updateUIStatus();
    int getVisibleLoginOptCount();
    QString getCustomDevName();

    /**
     * @brief 进行生物识别认证
     * @param deviceInfo    使用的设备
     * @param uid           待认证的用户id
     */
    void startAuth(DeviceInfoPtr device, int uid);

    /**
     * @brief 终止生物识别认证
     */
    void stopAuth();

    /**
     * @brief 是否正在认证
     * @return
     */
    bool isAuthenticating() {
        return m_isInAuth;
    }
    QPixmap loadSvg(QString path, QString color, int size);

    LoginAuthInterface* getCustomLoginAuth();

public slots:
    void readDevicesInfo();
    void onIdentifyComplete(QDBusPendingCallWatcher *watcher);
    void onUkeyIdentifyComplete(QDBusPendingCallWatcher *watcher);
    void onStatusChanged(int drvid, int status);
    void onFrameWritten(int drvid);
    void onUSBDeviceHotPlug(int drvid, int action, int devNum);
    void onOptionSelected(int nIndex, bool keyNavigation = false);
    void tabOptionSelected(int option);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

signals:
    void notifyOptionsChange(unsigned uOptionsCount, bool is_bioBtn);
    void optionSelected(unsigned uLoginOptType, const DeviceInfoPtr &deviceInfo, bool keyNavigation);
    void updateImage(QImage img);
    void authComplete(bool bResult, int nStatus);
    void updateAuthMsg(QString strMsg);
    void showKToolTipClicked(const QString text);
    void hideKTooltipClicked();
    void setLoadingImage();
    void customPluginMsg(QString strMsg);

private:
    void initUI();
    void initConnections();
    void addOptionButton(unsigned uLoginOptType, int nDrvId, QString strDrvName);
    void clearOptionButtons();
    void updateOptionButtons();
    void startAuth_();
    void startUkeyAuth();
    bool getAuthDouble();
    QPixmap drawSymbolicColoredPixmap(QPixmap &source, QString cgColor);

    void changeEvent(QEvent *event);
    void refreshTranslate();

private:
    BiometricProxy* m_biomericProxy = nullptr;
    DeviceMap m_mapDevices;
    unsigned m_curLoginOptType = LOGINOPT_TYPE_PASSWORD;
    int m_uid;
    QString m_strUserName;
    DeviceInfoPtr m_curDevInfo = nullptr;     // 当前选择的设备信息
    int m_dupFD = -1;               // 透传的图像文件句柄
    bool m_isInAuth = false;        // 是否正在验证
    bool m_isStopped = false;               // 是否被强制终止
    QTimer *m_retrytimer = nullptr;           // 重试定时器
    double curFontSize;
    Configuration *m_configuration;

    // UI
    QVBoxLayout *m_layoutMain = nullptr;
    QHBoxLayout *m_layoutOptBtns = nullptr;

    KLabel *m_labelOptTitle = nullptr;
    QButtonGroup *m_btnGroup = nullptr;
    QList<int> m_listDriveId;
    QMap<int, QPushButton*> m_mapOptBtns;
    QMap<int, QMap<int,bool>> m_mapDisableDev;

    //ToolTip
    QStringList nameList;
    int nowAt = -1;
    bool at_bioBtn = false;
    UniAuthService *m_uniauthService = nullptr;
    QList<int>  m_listPriority;

    bool        isShowUkey = false;
};

#endif // LOGINOPTIONSWIDGET_H
