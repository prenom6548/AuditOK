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
#ifndef LOGINOPTIONSWIDGET_H
#define LOGINOPTIONSWIDGET_H

#include <QWidget>
#include "../dbusifs/biometrichelper.h"
#include "uniauthservice.h"
#include "loginauthinterface.h"
#include "definetypes.h"
#include "mylistwidget.h"
#include "statusbutton.h"
#include <QDBusPendingCallWatcher>

class QLabel;
class QButtonGroup;
class QHBoxLayout;
class QVBoxLayout;
class QPushButton;
class QTimer;
class QPixmap;
class CustomLabel;

#define CUSTOM_PLUGIN_DEV_PREFIX ("Custom:")

typedef QMap<int, DeviceList> DeviceListMap;

class LoginOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LoginOptionsWidget(BiometricHelper *bioHelper, int uid, QWidget *parent = nullptr);
    virtual ~LoginOptionsWidget();

    bool getCurLoginOpt(int &nLoginOptType, int &nDrvId);
    unsigned getLoginOptCount();
    DeviceInfoPtr getWechatDevice();
    int convertDeviceType(int nDevType);

    void setUser(int uid);
    void setCurrentDevice(int drvid);
    void setCurrentDevice(const QString &deviceName);
    void setCurrentDevice(const DeviceInfoPtr &pDeviceInfo);
    DeviceInfoPtr findDeviceById(int drvid);
    DeviceInfoPtr findDeviceByName(const QString &name);
    DeviceList findDeviceByType(int nBioType);
    void setDeviceDisable(int nDevId, bool bDisable = true);
    bool isDeviceDisable(int nDevId);
    void clearDeviceDisable(int uid);
    bool getBioAuthEnable(int nType);
    bool getQRCodeEnable();
    QString getDefaultDevice(QString strUserName, int biotype);
    QString getDefaultDevice(QString strUserName);
    QStringList getAllDefDevices();
    void SetExtraInfo(QString extra_info, QString info_type);
    bool getHasUkeyOptions();
    void setSelectedPassword();
    void updateUIStatus();
    inline DeviceListMap getUserDevices()
    {
        return m_mapDevices;
    }
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
    bool isAuthenticating()
    {
        return m_isInAuth;
    }
    QPixmap loadSvg(QString path, QString color, int size);

    LoginAuthInterface *getCustomLoginAuth();

    void updateLoginOptionFont(QString fontFamily);

    void updateLoginOptionFontSize(double fontSize);

    // 设置所有生物识别的禁用状态
    void setAllDeviceDisable(bool bDisable = true);

    int getDefaultAuthType(QString strUserName);

public slots:
    void readDevicesInfo();
    void onIdentifyComplete(QDBusPendingCallWatcher *watcher);
    void onUkeyIdentifyComplete(QDBusPendingCallWatcher *watcher);
    void onStatusChanged(int drvid, int status);
    void onFrameWritten(int drvid);
    void onUSBDeviceHotPlug(int drvid, int action, int devNum);
    void onOptionSelected(int drvid);
private Q_SLOTS:
    /**
     * @brief item选中状态改变
     * @param current 前一个选中item
     *        previous 当前选中item
     * @return
     */
    void onButtonItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    /**
     * @brief item点击响应
     * @param item
     * @return
     */
    void onButtonItemClicked(QListWidgetItem *item);
protected:
    bool eventFilter(QObject *obj, QEvent *event);

Q_SIGNALS:
    void notifyOptionsChange(unsigned uOptionsCount);
    /**
     * @brief 生物认证硬件拔出（含竞态下列表尚未同步的情况）
     * @param drvid 拔出设备的驱动 id
     * @param detachedBioType 设备 BioType，无法确定时为 -1
     */
    void authHardwareDetached(int drvid, int detachedBioType);
    void optionSelected(unsigned uLoginOptType, const DeviceInfoPtr &deviceInfo);
    void updateImage(QImage img);
    void authComplete(bool bResult, int nStatus);
    void updateAuthMsg(QString strMsg);
    void showKToolTipClicked(const QString text);
    void hideKTooltipClicked();
    void setLoadingImage();
    void customPluginMsg(QString strMsg);

public:
    MyListWidget *getButtonListWidget() const { return m_buttonListWidget; }

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
    QString getDeviceType_tr(int deviceType);

    void changeEvent(QEvent *event);
    void refreshTranslate();
    void updateListPriority();

private:
    BiometricHelper *m_biomericProxy = nullptr;
    QDBusPendingCallWatcher *m_bioIdentifyWatcher = nullptr;
    DeviceListMap m_mapDevices;
    unsigned m_curLoginOptType = LOGINOPT_TYPE_PASSWORD;
    int m_uid;
    QString m_strUserName;
    DeviceInfoPtr m_curDevInfo = nullptr; // 当前选择的设备信息
    int m_dupFD = -1;                     // 透传的图像文件句柄
    bool m_isInAuth = false;              // 是否正在验证
    bool m_isStopped = false;             // 是否被强制终止
    QTimer *m_retrytimer = nullptr;       // 重试定时器
    double curFontSize;

    // UI
    QVBoxLayout *m_layoutMain = nullptr;
    QHBoxLayout *m_layoutOptBtns = nullptr;

    CustomLabel *m_labelOptTitle = nullptr;
    QList<int> m_listDriveId;
    QMap<int, StatusButton *> m_mapOptBtns;
    QMap<int, QListWidgetItem *> m_mapOptItems;
    QMap<int, QMap<int, bool>> m_mapDisableDev;

    MyListWidget *m_buttonListWidget = nullptr;

    QList<int> m_listPriority;

    bool isShowUkey = false;
    int m_nCurDrvid;
};

#endif // LOGINOPTIONSWIDGET_H
