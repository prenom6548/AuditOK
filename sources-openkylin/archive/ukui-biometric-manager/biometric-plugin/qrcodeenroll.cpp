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

#include "qrcodeenroll.h"
#include "ui_qrcodeenroll.h"
#include "fontsizeutil.h"
#include <QPixmap>
#include <QDBusUnixFileDescriptor>
#include <unistd.h>
#include <QImageReader>
#include <QBuffer>
#include <QByteArray>
#include <QDBusAbstractInterface>
#include "biometricdeviceinfo.h"
#include "biometricproxy.h"
#include "servicemanager.h"
#include "giodbus.h"
#include <ukuiwindowhelper.h>

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

#define SS_DBUS_SERVICE "org.ukui.ScreenSaver"
#define SS_DBUS_PATH "/"
#define SS_DBUS_INTERFACE "org.ukui.ScreenSaver"

QRCodeEnrollDialog::QRCodeEnrollDialog(QDBusInterface *service, int uid, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QRCodeEnrollDialog)
    , serviceInterface(service)
    , m_uid(uid)
    , ops(IDLE)
    , m_hisOps(IDLE)
    , opsResult(UNDEFINED)
    , w_timer(nullptr)
{
    // 去除窗管标题栏
    UkuiWindowHelper *pUkuiWindowHelper = new UkuiWindowHelper(this);
    pUkuiWindowHelper->removeTitleBar();
    ui->setupUi(this);
    setupInit();
    connect(serviceInterface, SIGNAL(StatusChanged(int, int)), this, SLOT(onStatusChanged(int, int)));

    connect(
        serviceInterface,
        SIGNAL(ProcessChanged(int, QString, int, QString)),
        this,
        SLOT(onProcessChanged(int, QString, int, QString)));

    connect(serviceInterface, SIGNAL(FrameWritten(int)), this, SLOT(onFrameWritten(int)));

    ServiceManager *sm = ServiceManager::instance();
    connect(sm, &ServiceManager::serviceStatusChanged, this, [&](bool activate) {
        if (!activate) {
            close();
        }
    });
    QDBusInterface *iface = new QDBusInterface(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager",
        QDBusConnection::systemBus(),
        this);
    connect(iface, SIGNAL(PrepareForSleep(bool)), this, SLOT(onPrepareForSleep(bool)));

    QDBusInterface *interfaceScreensaver
        = new QDBusInterface(SS_DBUS_SERVICE, SS_DBUS_PATH, SS_DBUS_INTERFACE, QDBusConnection::sessionBus(), this);

    connect(interfaceScreensaver, SIGNAL(lock()), this, SLOT(onSysLock()));
    connect(interfaceScreensaver, SIGNAL(unlock()), this, SLOT(onSysUnLock()));
}

QRCodeEnrollDialog::~QRCodeEnrollDialog()
{
    delete ui;
}

void QRCodeEnrollDialog::setupInit()
{
    setWindowTitle(tr(""));
    setAttribute(Qt::WA_DeleteOnClose);

    ui->closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    ui->closeBtn->setAttribute(Qt::WA_NoMousePropagation);
    ui->closeBtn->setProperty("isWindowButton", 0x2);
    ui->closeBtn->setProperty("useIconHighlightEffect", 0x8);
    ui->closeBtn->setFixedSize(30, 30);
    ui->closeBtn->setToolTip(tr("Close"));

    ui->confirmPushBtn->setProperty("useButtonPalette", true);

    ui->qrCodeImageLabel->setPixmap(QPixmap(":/imgs/resources/ukui-qrcode-null.svg").scaled(QSize(200, 200)));
    QVBoxLayout *layoutImg = new QVBoxLayout(ui->qrCodeImageLabel);
    layoutImg->setContentsMargins(0, 0, 0, 0);
    layoutImg->setSpacing(8);
    layoutImg->setAlignment(Qt::AlignCenter);
    m_labelTipMsg = new QLabel();
    m_labelTipMsg->setAlignment(Qt::AlignCenter);
    m_labelTipMsg->setWordWrap(true);
    layoutImg->addWidget(m_labelTipMsg);
    m_labelTipMsgS = new KALabel();
    layoutImg->addWidget(m_labelTipMsgS);
    m_labelTipMsgE = new QLabel();
    QPalette pe;
    pe.setColor(QPalette::WindowText, Qt::red);
    m_labelTipMsgE->setPalette(pe);
    layoutImg->addWidget(m_labelTipMsgE);
    m_labelTipMsgS->hide();
    m_labelTipMsgE->hide();

    ui->btnNext->setProperty("isImportant", true);

    if (!w_timer) {
        w_timer = new QTimer(this);
        w_timer->setInterval(150);
        connect(w_timer, &QTimer::timeout, this, &QRCodeEnrollDialog::updatePixmap);
    }
    m_waitingPixmap = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(24, 24);
    m_labelTipMsg->setPixmap(m_waitingPixmap);
    m_labelTipMsg->show();
    w_timer->start();

    ui->comboboxSelect->view()->setTextElideMode(Qt::ElideRight);
    ui->hlayoutEnrollBtn->setAlignment(Qt::AlignRight);
    ui->hlayoutSelectBtn->setAlignment(Qt::AlignRight);
    
    // 设置 bindQRCodeTitle 字体大小为 24
    FontSizeUtil::setFontSize(ui->bindQRCodeTitle, 24.0);
}

void QRCodeEnrollDialog::setTitle(int biotype, QString strDrivName)
{
    QString title = "";
    switch (biotype) {
        case ENROLL:
            title += tr("Bind %1 Account").arg(strDrivName.isEmpty() ? tr("Wechat") : strDrivName);
            break;
        case VERIFY:
            title += tr("Verify %1 Account").arg(strDrivName.isEmpty() ? tr("Wechat") : strDrivName);
            break;
        case SEARCH:
            title += tr("Search %1 Account").arg(strDrivName.isEmpty() ? tr("Wechat") : strDrivName);
            break;
    }

    ui->bindQRCodeTitle->setText(title);
}

void QRCodeEnrollDialog::setPrompt(QString text)
{
    if (text.isEmpty()) {
        ui->bindQRCodeTip->setText("");
        ui->bindQRCodeTip->hide();
    } else {

        ui->bindQRCodeTip->setText(text);
        ui->bindQRCodeTip->show();
    }
}

void QRCodeEnrollDialog::on_closeBtn_clicked()
{
    close();
}

QRCodeEnrollDialog::Result QRCodeEnrollDialog::getResult()
{
    return opsResult;
}

int QRCodeEnrollDialog::enrollBegin(DeviceList deviceList)
{
    m_devList = deviceList;
    if (!m_devList.isEmpty()) {
        if (m_devList.count() == 1) {
            SwitchStatus(PROGRESS_ENROLL);
            m_curDevice = m_devList.first();
            int num = 1;
            QStringList list = getFeaturelist(m_curDevice->id, getuid(), 0, -1);
            QString featurename;
            while (1) {
                featurename = "qrcode" + QString::number(num);
                if (!list.contains(featurename))
                    break;
                num++;
            }
            enroll(m_curDevice, getuid(), -1, featurename);
        } else {
            SwitchStatus(PROGRESS_SELECT);
        }
    }
    return 0;
}

int QRCodeEnrollDialog::enroll(DeviceInfoPtr deviceInfo, int u_id, int i_dx, const QString &idx_Name)
{
    if (ops == ENROLL || !deviceInfo) {
        return 1;
    }
    m_labelTipMsgS->hide();
    m_labelTipMsgE->hide();
    ui->confirmPushBtn->hide();
    ui->btnPrevious->hide();
    adjustSize();
    QList<QVariant> args;
    args << deviceInfo->id << m_uid << i_dx << idx_Name;
    m_uid = u_id;
    m_idx = i_dx;
    m_idxName = idx_Name;
    QString strDevName;
    if (deviceInfo->shortName == "wechat_driver") {
        this->setTitle(ENROLL);
        strDevName = tr("wechat");
    } else {
        this->setTitle(ENROLL, deviceInfo->shortName);
        strDevName = deviceInfo->shortName;
    }
    this->setPrompt(tr("Please use %1 scanning code for binding.").arg(strDevName));
    qDebug() << "args:" << args;
    serviceInterface->callWithCallback(
        "Enroll", args, this, SLOT(enrollCallBack(const QDBusMessage &)), SLOT(errorCallBack(const QDBusError &)));
    ops = ENROLL;
    m_hisOps = ENROLL;
    m_listEnrollArgs = args;
    if (w_timer && !w_timer->isActive()) {
        w_timer->start();
    }
    return 0;
}

void QRCodeEnrollDialog::enrollCallBack(const QDBusMessage &reply)
{
    int result;
    dup_fd = -1;
    result = reply.arguments()[0].value<int>();
    qDebug() << "Enroll result: " << result;

    switch (result) {
        case DBUS_RESULT_SUCCESS: { /* 录入成功 */
            opsResult = SUCESS;
            setPrompt(tr("Bind Successfully"));
            showFinishPrompt();
            m_hisOps = IDLE;
            break;
        }
        default:
            opsResult = ERROR;
            handleErrorResult(result);
            break;
    }
    ops = IDLE;
}

int QRCodeEnrollDialog::verify(int drvId, int uid, int idx)
{
    QList<QVariant> args;
    args << drvId << uid << idx;

    this->setTitle(VERIFY);
    serviceInterface->callWithCallback(
        "Verify", args, this, SLOT(verifyCallBack(const QDBusMessage &)), SLOT(errorCallBack(const QDBusError &)));
    ops = VERIFY;

    return exec();
}

void QRCodeEnrollDialog::verifyCallBack(const QDBusMessage &reply)
{
    int result;
    dup_fd = -1;
    result = reply.arguments()[0].value<int>();
    qDebug() << "Verify result: " << result;

    if (result >= 0) {
        opsResult = SUCESS;
        setPrompt(tr("Verify successfully"));
        showFinishPrompt();
    } else if (result == DBUS_RESULT_NOTMATCH) {
        ui->qrCodeImageLabel->setStyleSheet("background-color:#FFFFFF");
        m_labelTipMsg->setPixmap(QIcon::fromTheme("dialog-error").pixmap(QSize(64, 64)));
        ui->confirmPushBtn->setText(tr("Finish"));
        ui->confirmPushBtn->show();
        ui->btnPrevious->hide();
        adjustSize();
        m_labelTipMsgE->setText(tr("Not Match"));
        m_labelTipMsgE->show();
    } else {
        handleErrorResult(result);
    }

    ops = IDLE;
}

QStringList QRCodeEnrollDialog::getFeaturelist(int drvid, int uid, int indexStart, int indexEnd)
{
    QStringList list;
    QList<QDBusVariant> qlist;
    FeatureInfo *featureInfo;
    int listsize;
    QDBusMessage result = serviceInterface->call(QStringLiteral("GetFeatureList"), drvid, uid, indexStart, indexEnd);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetDevList error:" << result.errorMessage();
        return list;
    }
    QList<QVariant> variantList = result.arguments();
    listsize = variantList[0].value<int>();
    variantList[1].value<QDBusArgument>() >> qlist;
    for (int i = 0; i < listsize; i++) {
        featureInfo = new FeatureInfo;
        qlist[i].variant().value<QDBusArgument>() >> *featureInfo;
        list.append(featureInfo->index_name);
        delete featureInfo;
    }
    return list;
}

int QRCodeEnrollDialog::search(int drvId, int uid, int idxStart, int idxEnd)
{
    QList<QVariant> args;
    args << drvId << uid << idxStart << idxEnd;

    this->setTitle(SEARCH);

    serviceInterface->callWithCallback(
        "Search", args, this, SLOT(searchCallBack(const QDBusMessage &)), SLOT(errorCallBack(const QDBusError &)));

    ops = SEARCH;

    return exec();
}

void QRCodeEnrollDialog::searchCallBack(const QDBusMessage &reply)
{
    return;
}

void QRCodeEnrollDialog::showFinishPrompt()
{
    if (w_timer && w_timer->isActive()) {
        w_timer->stop();
    }
    m_labelTipMsg->setPixmap(QIcon::fromTheme("ukui-dialog-success").pixmap(QSize(64, 64)));
    ui->confirmPushBtn->setText(tr("Finish"));
    ui->confirmPushBtn->show();
    ui->btnPrevious->hide();
    ui->bindQRCodeTip->setText("");
    ui->bindQRCodeTitle->setText("");
    ui->qrCodeImageLabel->clear();
    ui->qrCodeImageLabel->setStyleSheet("");
    m_labelTipMsgE->hide();
    m_labelTipMsg->show();
    m_labelTipMsgS->show();
    adjustSize();
    if (ops == ENROLL) {
        QString strDirvName = tr("wechat");
        if (m_curDevice && m_curDevice->shortName != "wechat_driver") {
            strDirvName = m_curDevice->shortName;
        }
        m_labelTipMsgS->setText(tr("The %1 account is bound successfully!").arg(strDirvName));
    } else if (ops == VERIFY) {
        m_labelTipMsgS->setText(tr("Verify successfully"));
    }
}

void QRCodeEnrollDialog::StopOpsCallBack(const QDBusMessage &reply)
{
    dup_fd = -1;
    int ret = reply.arguments().at(0).toInt();
    accept();
}

void QRCodeEnrollDialog::errorCallBack(const QDBusError &error)
{
    m_hisOps = IDLE;
    dup_fd = -1;
    qDebug() << "DBus Error: " << error.message();
    accept();
}

void QRCodeEnrollDialog::closeEvent(QCloseEvent *event)
{
    if (m_curDevice) {
        serviceInterface->call("StopOps", m_curDevice->id, 1);
        m_curDevice = nullptr;
    }
}

void QRCodeEnrollDialog::onFrameWritten(int drvId)
{
    if (ops == IDLE) {
        qDebug() << "Not in enroll or verify!";
        return;
    }
    if (!m_curDevice || drvId != m_curDevice->id) {
        qDebug() << "Device not pair!";
        return;
    }
    if (dup_fd == -1) {
        dup_fd = get_server_gvariant_stdout(drvId);
    }

    if (dup_fd < 0)
        return;

    if (w_timer && w_timer->isActive()) {
        w_timer->stop();
        m_labelTipMsg->hide();
        ui->confirmPushBtn->setText(tr("Cancel"));
        ui->confirmPushBtn->show();
        if (m_devList.count() > 1) {
            ui->btnPrevious->show();
        } else {
            ui->btnPrevious->hide();
        }
    }

    lseek(dup_fd, 0, SEEK_SET);
    char base64_bufferData[1024 * 1024];
    int rc = read(dup_fd, base64_bufferData, 1024 * 1024);
    QImage image;
    if (rc > 0) {
        QByteArray imageData(base64_bufferData, rc);
        QBuffer imgBuffer(&imageData);
        imgBuffer.open(QIODevice::ReadOnly);

        QImageReader imgReader(&imgBuffer);
        image = imgReader.read();
        if (!image.isNull()) {
            image = image.convertToFormat(QImage::Format_RGB888);
        }
    }

    if (image.isNull()) {
        QFileInfo fileInfoImg("/var/lib/biometric-auth/qr_image.jpg");
        if (fileInfoImg.exists()) {
            QImage srcQImage("/var/lib/biometric-auth/qr_image.jpg");
            ui->qrCodeImageLabel->setPixmap(QPixmap::fromImage(srcQImage).scaled(QSize(200, 200)));
        }
    } else {
        ui->qrCodeImageLabel->setPixmap(QPixmap::fromImage(image).scaled(QSize(200, 200)));
    }

    qDebug() << "onFrameWritten end!!";
}

void QRCodeEnrollDialog::onProcessChanged(int drvId, QString aa, int statusType, QString bb)
{
    int count = statusType * 15 / 100;
    QString filename = QString("/usr/share/ukui-biometric/images/huawei/") + (count < 10 ? "0" : "")
                       + QString::number(count) + ".svg";

    qDebug() << "filename:" << filename;
}

void QRCodeEnrollDialog::onStatusChanged(int drvId, int statusType)
{
    if (!(m_curDevice && drvId == m_curDevice->id && statusType == STATUS_NOTIFY))
        return;
    qDebug() << "onStatusChanged:" << drvId << "," << statusType;

    // 过滤掉当录入时使用生物识别授权接收到的认证的提示信息
    if (ops == ENROLL) {
        QDBusMessage reply = serviceInterface->call("UpdateStatus", drvId);
        if (reply.type() == QDBusMessage::ErrorMessage) {
            qDebug() << "DBUS: " << reply.errorMessage();
            return;
        }
        int devStatus = reply.arguments().at(3).toInt();
        qDebug() << "statusType=" << statusType << "devStatus" << devStatus;
        if (!(devStatus >= 201 && devStatus < 203)) {
            return;
        }
        // 认证结束后，重新获取fd，刷新录入界面
        if (devStatus == 201 && statusType == STATUS_NOTIFY) {
            dup_fd = -1;
        }
    } else if (ops == IDLE) {
        return;
    }

    QDBusMessage notifyReply = serviceInterface->call("GetNotifyMesg", drvId);
    if (notifyReply.type() == QDBusMessage::ErrorMessage) {
        qDebug() << "DBUS: " << notifyReply.errorMessage();
        return;
    }
    QString prompt = notifyReply.arguments().at(0).toString();
    qDebug() << prompt;
}

void QRCodeEnrollDialog::on_confirmPushBtn_clicked()
{
    close();
}

void QRCodeEnrollDialog::handleErrorResult(int error)
{
    qDebug() << "error = " << error;
    ui->qrCodeImageLabel->setStyleSheet("background-color:#FFFFFF");
    ui->qrCodeImageLabel->setPixmap(QPixmap(":/imgs/resources/ukui-qrcode-null.svg").scaled(QSize(200, 200)));
    m_labelTipMsgS->hide();
    dup_fd = -1;
    if (w_timer && w_timer->isActive()) {
        w_timer->stop();
    }
    m_labelTipMsg->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(22, 22)));
    m_labelTipMsgE->show();
    switch (error) {
        case DBUS_RESULT_ERROR: {
            // 操作失败，需要进一步获取失败原因
            if (!m_curDevice) {
                return;
            }
            QDBusMessage msg = serviceInterface->call("GetOpsMesg", m_curDevice->id);
            if (msg.type() == QDBusMessage::ErrorMessage) {
                qDebug() << "UpdateStatus error: " << msg.errorMessage();
                m_hisOps = IDLE;
                return;
            }

            statusReslut ret = UpdateStatus(m_curDevice->id);
            qDebug() << "StatusReslut:" << ret.result << "," << ret.enable << "," << ret.devNum << "," << ret.devStatus
                     << "," << ret.opsStatus << "," << ret.notifyMessageId;

            if (ret.opsStatus == 8) {
                ui->bindQRCodeTip->show();
                ui->bindQRCodeTitle->show();
                ui->confirmPushBtn->setText(tr("Confirm"));
                ui->confirmPushBtn->show();
                ui->btnPrevious->hide();
                m_labelTipMsg->show();
                m_labelTipMsgE->show();
                m_labelTipMsgS->hide();
                m_labelTipMsgE->setText(tr("Abnormal network"));
                adjustSize();
                if (!m_uSysState) {
                    QTimer::singleShot(10000, this, [&, this]() {
                        if (m_curDevice) {
                            enroll(m_curDevice, m_uid, m_idx, m_idxName);
                        }
                    });
                }
            } else if (ret.opsStatus == 10) {
                ui->bindQRCodeTip->show();
                ui->bindQRCodeTitle->show();
                ui->confirmPushBtn->setText(tr("Confirm"));
                ui->confirmPushBtn->show();
                ui->btnPrevious->hide();
                m_labelTipMsg->show();
                m_labelTipMsgE->show();
                m_labelTipMsgS->hide();
                m_labelTipMsgE->setText(tr("Acquisition failure"));
                adjustSize();
                if (!m_uSysState) {
                    QTimer::singleShot(10000, this, [&, this]() {
                        if (m_curDevice) {
                            enroll(m_curDevice, m_uid, m_idx, m_idxName);
                        }
                    });
                }
            } else {
                if (!m_uSysState) {
                    QTimer::singleShot(5000, this, [&, this]() {
                        if (m_curDevice) {
                            enroll(m_curDevice, m_uid, m_idx, m_idxName);
                        }
                    });
                }
            }
            break;
        }
        case DBUS_RESULT_DEVICEBUSY:
            // 设备忙
            m_labelTipMsgE->setText(tr("Binding failure"));
            m_hisOps = IDLE;
            break;
        case DBUS_RESULT_NOSUCHDEVICE:
            // 设备不存在
            m_labelTipMsgE->setText(tr("Binding failure"));
            m_hisOps = IDLE;
            break;
        case DBUS_RESULT_PERMISSIONDENIED:
            // 没有权限
            on_closeBtn_clicked();
            m_hisOps = IDLE;
            break;
    }
}

statusReslut QRCodeEnrollDialog::UpdateStatus(int drvid)
{
    statusReslut status;
    QDBusMessage result = serviceInterface->call(QStringLiteral("UpdateStatus"), drvid);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "UpdateStatus error:" << result.errorMessage();
        status.result = -1;
        return status;
    }

    status.result = result.arguments().at(0).toInt();
    status.enable = result.arguments().at(1).toInt();
    status.devNum = result.arguments().at(2).toInt();
    status.devStatus = result.arguments().at(3).toInt();
    status.opsStatus = result.arguments().at(4).toInt();
    status.notifyMessageId = result.arguments().at(5).toInt();

    return status;
}

void QRCodeEnrollDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
}

void QRCodeEnrollDialog::onPrepareForSleep(bool sleep)
{
    /// 系统休眠时，会关闭总线，导致设备不可用，发生错误
    /// 在系统休眠之前停止认证，在系统唤醒后重新开始认证
    bool isChanged = false;
    if (sleep) {
        if (!(m_uSysState & SYSSTATE_SLEEP)) {
            m_uSysState |= SYSSTATE_SLEEP;
            isChanged = true;
        }
    } else {
        if (m_uSysState & SYSSTATE_SLEEP) {
            m_uSysState &= ~SYSSTATE_SLEEP;
            isChanged = true;
        }
    }
    if (isChanged) {
        chkSysState();
    }
}

void QRCodeEnrollDialog::onSysLock()
{
    if (!(m_uSysState & SYSSTATE_LOCK)) {
        m_uSysState |= SYSSTATE_LOCK;
        chkSysState();
    }
}

void QRCodeEnrollDialog::onSysUnLock()
{
    if (m_uSysState & SYSSTATE_LOCK) {
        m_uSysState &= ~SYSSTATE_LOCK;
        chkSysState();
    }
}

void QRCodeEnrollDialog::chkSysState()
{
    qInfo() << "chkSysState:" << m_uSysState << "," << m_hisOps;
    if (m_uSysState) { // 停止
        if (m_curDevice) {
            serviceInterface->call("StopOps", m_curDevice->id, 1);
        }
    } else { // 恢复
        if (m_hisOps == IDLE)
            return;
        QString strDirvName = tr("wechat");
        if (m_curDevice && m_curDevice->shortName != "wechat_driver") {
            strDirvName = m_curDevice->shortName;
        }
        setPrompt(tr("Please use %1 scanning code for binding.").arg(strDirvName));
        switch (m_hisOps) {
            case ENROLL: {
                this->serviceInterface->callWithCallback(
                    "Enroll",
                    this->m_listEnrollArgs,
                    this,
                    SLOT(enrollCallBack(const QDBusMessage &)),
                    SLOT(errorCallBack(const QDBusError &)));
                this->ops = this->m_hisOps;
            } break;
            default:
                break;
        }
    }
}

void QRCodeEnrollDialog::updatePixmap()
{
    QMatrix matrix;
    matrix.rotate(90.0);
    m_waitingPixmap = m_waitingPixmap.transformed(matrix, Qt::FastTransformation);
    m_labelTipMsg->setPixmap(m_waitingPixmap);
    m_labelTipMsg->show();
}

void QRCodeEnrollDialog::SwitchStatus(EnrollProgress progress)
{
    switch (progress) {
        case PROGRESS_SELECT: {
            if (m_curDevice) {
                serviceInterface->call("StopOps", m_curDevice->id, 1);
                m_curDevice = nullptr;
            }
            // 隐藏控件
            ui->bindQRCodeTip->hide();
            ui->bindQRCodeTitle->hide();
            ui->qrCodeImageLabel->hide();
            ui->confirmPushBtn->hide();
            ui->btnPrevious->hide();
            
            // 通过设置布局的拉伸因子为 0 和 margins 为 0 来隐藏布局占据的空间
            ui->verticalLayout_2->setStretchFactor(ui->verticalLayout_3, 0);
            ui->verticalLayout_2->setStretchFactor(ui->horizontalLayout_2, 0);
            ui->verticalLayout_3->setContentsMargins(0, 0, 0, 0);
            ui->horizontalLayout_2->setContentsMargins(0, 0, 0, 0);

            ui->labelTitleIcon->setPixmap(QIcon::fromTheme("ukui-control-center").pixmap(QSize(24, 24)));
            ui->labelTitleName->setText(tr("Scan code login"));
            ui->labelSelectTitle->setText(tr("Scan the code to log in using the following app"));
            ui->btnCancel->setText(tr("Cancel"));
            ui->btnNext->setText(tr("Next"));
            // init drive list combobox
            qDebug() << "--" << m_devList.count() << ui->comboboxSelect->actions().count();
            if (!m_devList.isEmpty() && ui->comboboxSelect->count() == 0) {
                for (auto device : m_devList) {
                    QString strDrivName = "";
                    if (device->shortName == "wechat_driver") {
                        strDrivName = tr("Wechat");
                    } else {
                        strDrivName = device->shortName;
                    }
                    ui->comboboxSelect->addItem(strDrivName, device->id);
                }
            }

            ui->labelTitleIcon->show();
            ui->labelTitleName->show();
            ui->labelSelectTitle->show();
            ui->comboboxSelect->show();
            ui->btnCancel->show();
            ui->btnNext->show();
            setMinimumHeight(220);
            setMaximumHeight(220);
        } break;
        case PROGRESS_ENROLL: {
            ui->labelTitleIcon->hide();
            ui->labelTitleName->hide();
            ui->labelSelectTitle->hide();
            ui->comboboxSelect->hide();
            ui->btnCancel->hide();
            ui->btnNext->hide();

            // 显示控件
            ui->bindQRCodeTip->show();
            ui->bindQRCodeTitle->show();
            ui->qrCodeImageLabel->show();
            ui->confirmPushBtn->show();
            
            // 恢复布局的拉伸因子和 margins
            ui->verticalLayout_2->setStretchFactor(ui->verticalLayout_3, 1);
            ui->verticalLayout_2->setStretchFactor(ui->horizontalLayout_2, 1);
            ui->verticalLayout_3->setContentsMargins(0, 0, 0, 0);
            ui->horizontalLayout_2->setContentsMargins(31, 10, 31, 8);
            if (m_devList.count() > 1) {
                ui->btnPrevious->show();
            } else {
                ui->btnPrevious->hide();
            }
            setMinimumHeight(440);
            setMaximumHeight(440);
        } break;
        default:
            break;
    }
}

void QRCodeEnrollDialog::on_btnCancel_clicked()
{
    close();
}

void QRCodeEnrollDialog::on_btnPrevious_clicked()
{
    SwitchStatus(PROGRESS_SELECT);
}

void QRCodeEnrollDialog::on_btnNext_clicked()
{
    QVariant varSelect = ui->comboboxSelect->currentData();
    if (varSelect.isValid()) {
        int nDrivId = varSelect.toInt();
        qDebug() << "Current Select:" << nDrivId;
        for (auto device : m_devList) {
            if (device->id == nDrivId) {
                m_curDevice = device;
                break;
            }
        }
    }
    if (m_curDevice) {
        SwitchStatus(PROGRESS_ENROLL);
        int num = 1;
        QStringList list = getFeaturelist(m_curDevice->id, getuid(), 0, -1);
        QString featurename;
        while (1) {
            featurename = "qrcode" + QString::number(num);
            if (!list.contains(featurename))
                break;
            num++;
        }
        enroll(m_curDevice, getuid(), -1, featurename);
    }
}
