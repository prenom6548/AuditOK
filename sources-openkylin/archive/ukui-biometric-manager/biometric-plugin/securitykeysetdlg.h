/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2023 KYLINOS Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef SECURITYKEYSETDLG_H
#define SECURITYKEYSETDLG_H

#include <kdialog.h>
#include <QPushButton>
#include <QDBusInterface>
#include <QDBusArgument>
#include <QDBusReply>
#include <QDBusAbstractInterface>
#include <QStringList>
#include "biometricdeviceinfo.h"
#include "pwdlessproxy.h"

class QTimer;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QLineEdit;
class QTextEdit;

using namespace kdk;

/**
 * @brief 安全密钥设置对话框
 *
 */
class SecurityKeySetDlg : public KDialog
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     *
     * @param service 生物特征dbus接口服务指针
     * @param devInfo 绑定的ukey设备信息
     * @param uid 绑定的用户id
     * @param parent 父窗口指针
     */
    explicit SecurityKeySetDlg(QDBusInterface *service, DeviceInfoPtr devInfo, int uid, QWidget *parent = nullptr);
    /**
     * @brief 析构函数
     *
     */
    virtual ~SecurityKeySetDlg();

    void setServiceProxy(PwdlessProxy *proxy);

public Q_SLOTS:
    /**
     * @brief 设备插拔变化通知
     *
     * @param isAttached 是否已插入
     */
    void onUKeyDevStateChanged(bool isAttached, DeviceList devInfoList);

    void onBtnCancel();
    void onBtnSure();

    void onServiceBtnSure();
    void onServiceUkeyRemoved(QString user);
    void onServiceUkeyInserted(QString user);

protected:
    void closeEvent(QCloseEvent *event) override;
    void accept();
    void reject();

private:
    /**
     * @brief 初始化用户界面控件
     *
     */
    void initUI();
    /**
     * @brief 初始化信号槽连接
     *
     */
    void initConnetions();
    void updateLoading();
    void updateLoadingTip();
    void onSwitchUIState(int nState);
    QStringList getFeaturelist(int uid, int indexStart, int indexEnd);
    QStringList getFeatureDevNamelist(int uid);
    /**
     * @brief 获取UKey驱动列表
     * @return
     */
    DeviceList GetUKeyDrvList();
    int enroll(int drvId, int u_id, int i_dx, const QString &idx_Name);
    void handleErrorResult(int error);
    StatusReslut UpdateStatus(int drvid);
    bool chkBound();
    int SetExtraInfo(QString strInfoType, QString strExtraInfo);
    int deleteFeature(int drvid, int uid, int indexStart, int indexEnd);
    void showErrorMessageTip(QString message);
    void showChkMessageTip(QString message);
    bool authoriyUser(bool isEnroll);
    void chkSysState();
    void disableDialog(bool bDisable);
    void refreshConfirmBtnStatus();
    bool chkServiceBound();
    bool isPwdlessKey();

private Q_SLOTS:
    void enrollCallBack(const QDBusMessage &);
    void errorCallBack(const QDBusError &);
    void onPrepareForSleep(bool sleep);
    void onSysLock();
    void onSysUnLock();
    void onBind();
    void onUnBind();

    void onServiceBind();
    void onServiceUnBind();

private:
    bool m_isUKeyDevAttached; /**< 设备是否已接入 */
    bool m_isUKeyDevBound;    /**< 设备是否已绑定 */
    bool m_isUKeyDevBinding;
    bool m_isUKeyDevUnBinding;
    DeviceInfoPtr m_devInfo;            /**< 设备信息 */
    int m_uid;                          /**< 用户id */
    int m_idx;                          /**< 特征下标 */
    QString m_idxName;                  /**< 特征名称 */
    QDBusInterface *m_serviceInterface; /**< 服务接口指针 */
    enum OPS
    {
        IDLE,
        ENROLL,
        VERIFY,
        SEARCH
    } ops,
        m_hisOps;

    /**
     * @brief 休眠状态枚举
     *
     */
    enum SysState_e
    {
        SYSSTATE_LOCK = 0x01,
        SYSSTATE_SLEEP = 0x02,
    };
    unsigned m_uSysState = 0;         /**< 休眠状态 */
    QList<QVariant> m_listEnrollArgs; /**< 休眠前绑定参数 */

    /**
     * @brief UI状态枚举
     *
     */
    enum UIState_e
    {
        UISTATE_NODEV,
        UISTATE_CHKPIN,
        UISTATE_BOUND,
        UISTATE_UNBOUND,
        UISTATE_NODEVTIP
    };
    int m_curUIState; /**< 当前UI状态 */

    QPixmap m_loadingPixmap; /**< 加载图片 */
    QTimer *m_timerLoading;  /**< 加载定时器 */
    QTimer *m_timerLoadingTip;

    // ui controls
    QVBoxLayout *m_layoutMain = nullptr;   /**< TODO: describe */
    QVBoxLayout *m_layoutTop = nullptr;    /**< TODO: describe */
    QHBoxLayout *m_layoutBottom = nullptr; /**< TODO: describe */

    QWidget *m_widgetNoDev = nullptr;     /**< TODO: describe */
    QLabel *m_labelLoading = nullptr;     /**< TODO: describe */
    QLabel *m_labelDetachedTip = nullptr; /**< TODO: describe */

    QWidget *m_widgetNoDevTip = nullptr;    /**< TODO: describe */
    QLabel *m_labelNoDevTipLabel = nullptr; /**< TODO: describe */
    QLabel *m_labelNoDevTipText = nullptr;  /**< TODO: describe */
 
    QWidget *m_widgetChkPin = nullptr;    /**< TODO: describe */
    QLabel *m_labelChkLogo = nullptr;     /**< TODO: describe */
    QLabel *m_labelSecurityKey = nullptr; /**< TODO: describe */
    QPushButton *m_btnPwdEye = nullptr;
    QLineEdit *m_editSecurityKey = nullptr; /**< TODO: describe */
    QLabel *m_labelChkMsg = nullptr;

    QWidget *m_widgetBound = nullptr;     /**< TODO: describe */
    QLabel *m_labelBoundLogo = nullptr;   /**< TODO: describe */
    QTextEdit *m_labelBoundTip = nullptr; /**< TODO: describe */
    QPushButton *m_btnUnbind = nullptr;   /**< TODO: describe */

    QWidget *m_widgetUnbound = nullptr;     /**< TODO: describe */
    QLabel *m_labelUnboundLogo = nullptr;   /**< TODO: describe */
    QLabel *m_labelUnboundTitle = nullptr;  /**< TODO: describe */
    QTextEdit *m_labelUnboundTip = nullptr; /**< TODO: describe */

    QPushButton *m_btnCancel = nullptr; /**< TODO: describe */
    QPushButton *m_btnSure = nullptr;   /**< TODO: describe */

    PwdlessProxy *m_pwdlessProxy = nullptr;
    QString m_name = "";

    // 初始化 -1;统一认证 1;pwdless 2;
    int ukeyType = -1;
};

#endif // SECURITYKEYSETDLG_H
