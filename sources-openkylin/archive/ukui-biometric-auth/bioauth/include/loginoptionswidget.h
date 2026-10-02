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
#include <QMap>
#include "bioauth.h"
#include "biodevices.h"
#include <qgsettings.h>

class QLabel;
class QButtonGroup;
class QHBoxLayout;
class QVBoxLayout;
class QToolButton;
class QTimer;

typedef enum {
    BioT_FingerPrint,           /** 指纹 **/
    BioT_FingerVein,            /** 指静脉 **/
    BioT_Iris,                          /** 虹膜 **/
    BioT_Face,                          /** 人脸 **/
    BioT_VoicePrint,            /** 声纹 **/

    UniT_KCM,                           /**     安全管控 **/
    UniT_General_Ukey,          /** 普通的Ukey **/
    UniT_Advanced_Ukey,         /** 高阶的Ukey **/
    UniT_Remote,            /** 远程账户 **/
}Bio_Type;

class LoginOptionsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LoginOptionsWidget(QWidget *parent = nullptr);
    virtual ~LoginOptionsWidget();

    bool getCurLoginOpt(int& nLoginOptType, int& nDrvId);
    unsigned getLoginOptCount();
    DeviceInfoPtr getFirstDevInfo();
    int convertDeviceType(int nDevType);
    void updateUIStatus(bool update, int nPreOptType = -1);
    void updateUkeyUIStatus(int type);
    void setUser(int uid);
    void setCurrentDevice(int drvid);
    void setCurrentDevice(const QString &deviceName);
    void setCurrentDevice(const DeviceInfoPtr &pDeviceInfo);
    DeviceInfoPtr findDeviceById(int drvid);
    DeviceInfoPtr findDeviceByName(const QString &name);
    void setDeviceDisable(int nDevId, bool bDisable = true);
    bool isDeviceDisable(int nDevId);
    void lockStatusChanged(bool locked);

    /**
     * @brief 获取默认设备
     * @return
     */
    QString GetDefaultDevice(uid_t uid);
    QString GetDefaultDevice(uid_t uid,int bioType);
    int getDefaultAuthType(QString strUserName);

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

    void SetExtraInfo(QString extra_info,QString info_type);
    bool getHasUkeyOptions();
    void setSelectedPassword();

    // 设置所有生物识别的禁用状态
    void setAllDeviceDisable(bool bDisable = true);

    void startBioWaiting(unsigned uCurLoginOptType);
    void stopBioWaiting(bool isDisable = false);

public slots:
    void readDevicesInfo();
    void onIdentifyComplete(int uid, bool ret, int retErrNo);
    void onAuthFinished();
    void onStatusChanged(int drvid, const QString &message);
    void onFrameWritten(int drvid);
    void onUSBDeviceCountChange(int newNum);
    void onOptionSelected(int nIndex);
    void setQRCodeMsg(QString strMsg);
    void setQRCode(QImage& imgQRCode);
    void setFaceImg(QImage& imgFace, int nStatus = 0);

signals:
    void notifyOptionsChange(unsigned uOptionsCount);
    void optionSelected(unsigned uLoginOptType, const DeviceInfoPtr &deviceInfo);
    void updateImage(QImage img);
    // nStatus -1 dbus错误，0 无错误，1 超时，2 其他错误
    void authComplete(uid_t uid, bool bResult, int nStatus);
    void updateAuthMsg(QString strMsg);
    void updateWndSize(unsigned uLoginOptType, unsigned uLoginOptSize);

private:
    void initUI();
    void initConnections();
    void initStyleTheme();
    void addOptionButton(unsigned uLoginOptType, int nDrvId, QString strDrvName);
    void clearOptionButtons();
    void updateOptionButtons();
    void startAuth_();
    QPixmap PixmapToRound(const QPixmap &src, int radius);
    QPixmap scaledPixmap(int width, int height, QString url);
    QPixmap drawSymbolicColoredPixmap(QPixmap &source, QString cgColor);
    void updatePixmap();
    QPixmap scaledSmoothPixmap(const QPixmap &src, int nWidth, int nHeight);

private:
    BioAuth *m_biomericProxy = nullptr;
    BioDevices *m_bioDevices = nullptr;
    DeviceMap m_mapDevices;
    unsigned m_curLoginOptType = LOGINOPT_TYPE_PASSWORD;
    int m_uid = -1;
    QString m_strUserName;
    DeviceInfoPtr m_curDevInfo = nullptr;     // 当前选择的设备信息
    int m_dupFD = -1;               // 透传的图像文件句柄
    bool m_isInAuth;                // 是否正在验证
    bool m_isStopped;               // 是否被强制终止
    QTimer *m_retrytimer = nullptr;           // 重试定时器

    // UI
    QVBoxLayout *m_layoutMain = nullptr;
    QWidget *m_widgetOptContainer = nullptr;  // 包含label和按钮的容器widget
    QVBoxLayout *m_layoutOptContainer = nullptr;  // 包含label和按钮的容器布局
    QHBoxLayout *m_layoutOptBtns = nullptr;
    QHBoxLayout *m_layoutImage = nullptr;

    QLabel *m_labelOptTitle = nullptr;
    QButtonGroup *m_btnGroup = nullptr;
    QList<int> m_listDriveId;
    QMap<int, QToolButton*> m_mapOptBtns;
    QWidget *m_widgetImage = nullptr;
    QLabel          *m_labelFace = nullptr;
    QLabel          *m_labelFaceLoad = nullptr;
    QLabel          *m_labelQRCode = nullptr;   // 二维码图标
    QLabel          *m_labelQRCodeMsg = nullptr; // 二维码状态消息提示
    QLabel          *m_labelQRCodeTip = nullptr;
    QMap<int, QMap<int,bool>> m_mapDisableDev;
    bool            is_Lock = false;
    QPixmap         m_waitingPixmap;
    QTimer          *w_timer;

    bool            isShowUkey = false;

    // 指纹、指静脉、虹膜、声纹识别动图
    QLabel *m_labelBioWaiting = nullptr; // 指纹、指静脉、虹膜、声纹识别动图提示
    QTimer *m_timerBioWaiting = nullptr; // 指纹、指静脉、虹膜、声纹识别动图切换定时器
    int m_nBioWaitingFrame = 0;             // 当前显示的动图帧
    int m_nCurBioWaitingType = -1;          // 当前显示的动图类型
    QGSettings *m_styleSettings = nullptr;
    QString m_strThemeName = "";

    UniAuthService *m_uniAuthService = nullptr;
};

#endif // LOGINOPTIONSWIDGET_H
