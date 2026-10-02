/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2020 KYLINOS Information Technology Co., Ltd.
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

#ifndef QRCODEENROLL_H
#define QRCODEENROLL_H

#include <QDialog>
#include <QPainter>
#include <QPainterPath>
#include <QListWidget>
#include <QDebug>
#include <QPushButton>
#include <QMovie>
#include <QDBusInterface>
#include <QDBusArgument>
#include <QDBusReply>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDBusAbstractInterface>
#include <QStringList>
#include <QList>
#include "kalabel.h"
#include "biometricdeviceinfo.h"

/**
 * @brief UpdateStauts调用返回的结果
 */
struct statusReslut
{
    int result;
    int enable;
    int devNum;
    int devStatus;
    int opsStatus;
    int notifyMessageId;
};

namespace Ui
{
class QRCodeEnrollDialog;
}

class QRCodeEnrollDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QRCodeEnrollDialog(QDBusInterface *service, int uid, QWidget *parent = nullptr);
    ~QRCodeEnrollDialog();
    enum Result
    {
        SUCESS,
        ERROR,
        UNDEFINED
    };

    void setTitle(int opsType, QString strDrivName = "");
    void setPrompt(QString text);

    int enrollBegin(DeviceList deviceList);
    int verify(int drvId, int uid, int idx);
    int search(int drvId, int uid, int idxStart, int idxEnd);
    Result getResult();
    statusReslut UpdateStatus(int drvid);
    void chkSysState();

protected:
    void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *event);

private slots:
    void on_closeBtn_clicked();
    void onStatusChanged(int, int);
    void onProcessChanged(int, QString, int, QString);
    void enrollCallBack(const QDBusMessage &);
    void verifyCallBack(const QDBusMessage &);
    void searchCallBack(const QDBusMessage &);
    void StopOpsCallBack(const QDBusMessage &);
    void showFinishPrompt();
    void errorCallBack(const QDBusError &);
    void onFrameWritten(int);
    void onPrepareForSleep(bool sleep);
    void onSysLock();
    void onSysUnLock();
    void updatePixmap();

private:
    Ui::QRCodeEnrollDialog *ui;
    void setupInit();
    int enroll(DeviceInfoPtr deviceInfo, int u_id, int i_dx, const QString &idx_Name);
    void handleErrorResult(int error);
    QStringList getFeaturelist(int drvid, int uid, int indexStart, int indexEnd);

    enum EnrollProgress
    {
        PROGRESS_SELECT,
        PROGRESS_ENROLL
    } m_enrollProgress;
    void SwitchStatus(EnrollProgress progress);

    Result opsResult;
    QDBusInterface *serviceInterface;
    enum OPS
    {
        IDLE,
        ENROLL,
        VERIFY,
        SEARCH
    } ops,
        m_hisOps;

    int m_uid;
    int m_idx;
    QString m_idxName;

    bool isFace = false;
    int fd = -1;
    int dup_fd = -1;
    QLabel *m_labelTipMsg = nullptr;
    KALabel *m_labelTipMsgS = nullptr;
    QLabel *m_labelTipMsgE = nullptr;
    enum SysState_e
    {
        SYSSTATE_LOCK = 0x01,
        SYSSTATE_SLEEP = 0x02,
    };
    unsigned m_uSysState = 0;
    QList<QVariant> m_listEnrollArgs;

    QPixmap m_waitingPixmap;
    QTimer *w_timer;

    DeviceList m_devList;
    DeviceInfoPtr m_curDevice;

private slots:
    void on_confirmPushBtn_clicked();
    void on_btnCancel_clicked();
    void on_btnPrevious_clicked();
    void on_btnNext_clicked();
};

#endif // QRCODEENROLL_H
