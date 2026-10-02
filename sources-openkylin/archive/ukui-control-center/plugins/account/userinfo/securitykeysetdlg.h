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

#include <QPushButton>
#include <QStringList>
#include "qtdbus/pwdlessservice.h"
#include <kysdk/applications/kdialog.h>
#include <kysdk/applications/klabel.h>
#include <kysdk/applications/kpushbutton.h>
using namespace kdk;

class QTimer;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QLineEdit;
class QTextEdit;

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
     * @param parent 父窗口指针
     */
    explicit SecurityKeySetDlg(QString username, PwdlessService *service, QWidget *parent = nullptr);
    virtual ~SecurityKeySetDlg();

public Q_SLOTS:
    void onBtnSure();
    void onBind();
    void onUnBind();
    void onUKeyInserted(QString bindUser);
    void onUKeyRemoved(QString bindUser);
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
    void onSwitchUIState(int nState);
    void showErrorMessageTip(QString message);
    void showChkMessageTip(QString message);
    void disableDialog(bool bDisable);
    void refreshConfirmBtnStatus();

private:
    bool m_isUKeyDevBinding = false;
    bool m_isInsertUkey = false;
    PwdlessService *m_serviceInterface;
    QString m_username;
    /**
     * @brief UI状态枚举
     *
     */
    enum UIState_e{
        UISTATE_NODEV,
        UISTATE_CHKPIN,
        UISTATE_BOUND,
        UISTATE_UNBOUND,
        UISTATE_TIMEOUT
    };
    int m_curUIState; /**< 当前UI状态 */
    QPixmap m_loadingPixmap; /**< 加载图片 */
    QTimer *m_timerLoading; /**< 加载定时器 */
    // ui controls
    QVBoxLayout *m_layoutMain = nullptr; /**< TODO: describe */
    QVBoxLayout *m_layoutTop = nullptr; /**< TODO: describe */
    QHBoxLayout *m_layoutBottom = nullptr; /**< TODO: describe */

    QWidget *m_widgetNoDev = nullptr; /**< TODO: describe */
    QLabel *m_labelLoading = nullptr; /**< TODO: describe */
    KLabel *m_labelDetachedTip = nullptr; /**< TODO: describe */

    QWidget *m_widgetChkPin = nullptr; /**< TODO: describe */
    QLabel *m_labelChkLogo = nullptr; /**< TODO: describe */
    KLabel *m_labelSecurityKey = nullptr; /**< TODO: describe */
    QLineEdit *m_editSecurityKey = nullptr; /**< TODO: describe */
    KLabel *m_labelChkMsg = nullptr;

    QWidget *m_widgetBound = nullptr; /**< TODO: describe */
    QLabel *m_labelBoundLogo = nullptr; /**< TODO: describe */
    KLabel *m_labelBoundTip = nullptr; /**< TODO: describe */
    QPushButton *m_btnUnbind = nullptr; /**< TODO: describe */

    QWidget *m_widgetUnbound = nullptr; /**< TODO: describe */
    QLabel *m_labelUnboundLogo = nullptr; /**< TODO: describe */
    QLabel *m_labelUnboundTitle = nullptr; /**< TODO: describe */
    KLabel *m_labelUnboundTip = nullptr; /**< TODO: describe */

    KPushButton *m_btnCancel = nullptr; /**< TODO: describe */
    KPushButton *m_btnSure = nullptr; /**< TODO: describe */
};

#endif // SECURITYKEYSETDLG_H
