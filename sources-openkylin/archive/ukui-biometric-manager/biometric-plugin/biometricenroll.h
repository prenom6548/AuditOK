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

#ifndef BIOMETRICENROLL_H
#define BIOMETRICENROLL_H

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
#include <kdialog.h>
#include "biometricdeviceinfo.h"

namespace Ui {
class BiometricEnrollDialog;
}

using namespace kdk;

class BiometricEnrollDialog : public KDialog
{
    Q_OBJECT

public:
    explicit BiometricEnrollDialog(QDBusInterface *service,int bioType,
                                   int deviceId, int uid, QWidget *parent = nullptr);
    ~BiometricEnrollDialog();
    enum Result {SUCESS, ERROR, UNDEFINED};

    void setProcessed(bool val);
    void setTitle(int opsType);
    void setPrompt(QString text);

    int enroll(int drvId, int uid, int idx, const QString &idxName);
    int verify(int drvId, int uid, int idx);
    int search(int drvId, int uid, int idxStart, int idxEnd);
    QString getGif(int type);
    QString getImage(int type);
    Result getResult();
    void resetUI();
    void setIsFace(bool val);
    void chkSysState();


protected:
    void paintEvent(QPaintEvent * event);
    void closeEvent(QCloseEvent *event);

Q_SIGNALS:
    void enrollFinished();

private slots:
    void on_biometricFinishbtn_clicked();
    void on_biometricCancelbtn_clicked();
    void on_biometricConBtn_clicked();
    void onStatusChanged(int, int);
    void onProcessChanged(int, QString,int,QString);
    void enrollCallBack(const QDBusMessage &);
    void verifyCallBack(const QDBusMessage &);
    void searchCallBack(const QDBusMessage &);
    void StopOpsCallBack(const QDBusMessage &);
    void showFinishPrompt();
    void showErrorPrompt(QString tips);
    void errorCallBack(const QDBusError &);
    void onFrameWritten(int);
    void onEnrollFrameWritten(int);
    void onPrepareForSleep(bool sleep);
    void onSysLock();
    void onSysUnLock();
    void switchEnrollImage();

private:
    Ui::BiometricEnrollDialog *ui;
    void initInfoFormat();
    void setupInit();
    void handleErrorResult(int error);
    QString transferBioType(int type);
    QString lowerTransferBioType(int type);
    QStringList getAllFeatureinfolist(int uid, int indexStart, int indexEnd, int driver_type, QList<int>& idxList);
    QString ElideText(QFont font,int width,QString strInfo);
    void updatePixmap();
    QPixmap PixmapToRound(const QPixmap &src, int radius);
    /**
     * @brief UpdateStatus  获取更新的设备状态
     * @param drvid 驱动id
     * @return  结果：<int result, int enable, int devNum,
     *                int devStatus, int opsStatus, notifyMessageId, ...>
     */
    StatusReslut UpdateStatus(int drvid);
    void getEnrollImageList(int type);
    void updateEnrollImage();

    Result opsResult;
    QDBusInterface *serviceInterface;
    enum OPS{IDLE, ENROLL, VERIFY, SEARCH} ops,m_hisOps;

    int type;
    int deviceId;
    int uid;
    bool isProcessed;

    QMovie *movie;
    bool isFace = false;
    int fd = -1;
    int dup_fd = -1;
    bool m_isUseEnrollFd = false;
    enum SysState_e{
        SYSSTATE_LOCK = 0x01,
        SYSSTATE_SLEEP = 0x02,
    };
    unsigned m_uSysState = 0;
    QList<QVariant> m_listEnrollArgs;
    QList<QVariant> m_listVerifyArgs;
    QList<QVariant> m_listSearchArgs;

    QPixmap         m_waitingPixmap;
    QTimer          *w_timer;

    //  指纹录入使用svg图片实现动图效果
    QStringList m_enrollImagePaths;
    int m_currentImage;
    QTimer *m_enrollTimer;
    bool m_enrollSort = true;
    bool m_enrollFinish = false;

private slots:

};

#endif // BIOMETRICENROLL_H
