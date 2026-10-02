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

#include "biometricenroll.h"
#include "ui_biometricenroll.h"
#include <QPixmap>
#include <QDBusUnixFileDescriptor>
#include <unistd.h>
#include <QImageReader>
#include <QBuffer>
#include <QByteArray>
#include "biometricdeviceinfo.h"
#include "biometricproxy.h"
#include "servicemanager.h"
#include "giodbus.h"
#include <ukuiwindowhelper.h>
#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

#define SS_DBUS_SERVICE "org.ukui.ScreenSaver"
#define SS_DBUS_PATH "/"
#define SS_DBUS_INTERFACE "org.ukui.ScreenSaver"

BiometricEnrollDialog::BiometricEnrollDialog(
    QDBusInterface *service, int bioType, int deviceId, int uid, QWidget *parent)
    : KDialog(parent)
    , ui(new Ui::BiometricEnrollDialog)
    , serviceInterface(service)
    , type(bioType)
    , deviceId(deviceId)
    , uid(uid)
    , movie(nullptr)
    , ops(IDLE)
    , m_hisOps(IDLE)
    , isProcessed(false)
    , opsResult(UNDEFINED)
    , w_timer(nullptr)
    , m_enrollTimer(nullptr)
{
    // 去除窗管标题栏
    UkuiWindowHelper *pUkuiWindowHelper = new UkuiWindowHelper(this);
    pUkuiWindowHelper->removeTitleBar();
    ui->setupUi(this);
    initInfoFormat();
    setupInit();

    connect(serviceInterface, SIGNAL(StatusChanged(int, int)), this, SLOT(onStatusChanged(int, int)));

    connect(
        serviceInterface,
        SIGNAL(ProcessChanged(int, QString, int, QString)),
        this,
        SLOT(onProcessChanged(int, QString, int, QString)));

    connect(serviceInterface, SIGNAL(FrameWritten(int)), this, SLOT(onFrameWritten(int)));

    if (connect(serviceInterface, SIGNAL(EnrollFrameWritten(int)), this, SLOT(onEnrollFrameWritten(int)))) {
        m_isUseEnrollFd = true;
        if (!w_timer) {
            w_timer = new QTimer(this);
            w_timer->setInterval(150);
            connect(w_timer, &QTimer::timeout, this, &BiometricEnrollDialog::updatePixmap);
        }
        m_waitingPixmap = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(24, 24);
        ui->biometricEnrollLable->setPixmap(m_waitingPixmap);
        ui->biometricEnrollLable->setFixedSize(154, 154);
        w_timer->start();
        ui->biometricFinishbtn->hide();
        ui->biometricCancelbtn->show();
        qDebug() << "Connect onEnrollFrameWritten Succeed!!";
    }

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

BiometricEnrollDialog::~BiometricEnrollDialog()
{
    delete ui;
}

void BiometricEnrollDialog::setIsFace(bool val)
{
    isFace = val;
}

void BiometricEnrollDialog::setProcessed(bool val)
{
    isProcessed = val;
    if (isProcessed) {
        if (w_timer) {
            w_timer->stop();
        }
        ui->biometricEnrollLable->setPixmap(QPixmap("/usr/share/ukui-biometric/images/huawei/00.svg"));
        ui->biometricEnrollLable->setFixedSize(154, 154);
    } else {
        ui->biometricEnrollLable->setPixmap(getImage(type));
        ui->biometricEnrollLable->setFixedSize(154, 154);
        if (!movie) {
            movie = new QMovie(getGif(type));
            movie->setScaledSize(QSize(156, 156));
        }
    }
}

void BiometricEnrollDialog::initInfoFormat()
{
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->biometricOpsLbl, "Login Options", "BiometricEnroll", "biometric title label of BiometricEnroll dailog");
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->biometricPromptLbl, "Login Options", "BiometricEnroll", "biometric prompt label of BiometricEnroll dailog");
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->biometricConBtn, "Login Options", "BiometricEnroll", "biometric Continue button of BiometricEnroll dailog");
    ui->biometricConBtn->setProperty("useButtonPalette", true);
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->biometricFinishbtn,
        "Login Options",
        "BiometricEnroll",
        "biometric finish button of BiometricEnroll dailog");
    ui->biometricFinishbtn->setProperty("isImportant", true);
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->biometricEnrollLable,
        "Login Options",
        "BiometricEnroll",
        "biometric enroll label of BiometricEnroll dailog");
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->biometricFinishLabel,
        "Login Options",
        "BiometricEnroll",
        "biometric finish label of BiometricEnroll dailog");
    KDK_EXTEND_ALL_INFO_FORMAT(
        ui->biometricCancelbtn,
        "Login Options",
        "BiometricEnroll",
        "biometric cancel button of BiometricEnroll dailog");
    ui->biometricCancelbtn->setProperty("useButtonPalette", true);
}

void BiometricEnrollDialog::setupInit()
{
    if (isProcessed) {
        ui->biometricEnrollLable->setPixmap(QPixmap("/usr/share/ukui-biometric/images/huawei/00.svg"));
        ui->biometricEnrollLable->setFixedSize(154, 154);
    } else if (type == BIOTYPE_FINGERPRINT || type == BIOTYPE_IRIS) {
        getEnrollImageList(type);
        if (!m_enrollTimer) {
            m_enrollTimer = new QTimer(this);
        }
        connect(m_enrollTimer, &QTimer::timeout, this, &BiometricEnrollDialog::switchEnrollImage);
        // 设置初始图片
        m_currentImage = 0;
        updateEnrollImage();
    } else {
        ui->biometricEnrollLable->setPixmap(getImage(type));
        ui->biometricEnrollLable->setFixedSize(154, 154);
        if (!movie) {
            movie = new QMovie(getGif(type));
            movie->setScaledSize(QSize(156, 156));
        }
    }
    QFont font1;
    QString str1 = ElideText(font1, 110, ui->biometricConBtn->text());
    if (str1 != ui->biometricConBtn->text()) {
        ui->biometricConBtn->setToolTip(ui->biometricConBtn->text());
        ui->biometricConBtn->setText(str1);
    }

    QFont font2;
    QString str2 = ElideText(font1, 95, ui->biometricFinishbtn->text());
    if (str2 != ui->biometricFinishbtn->text()) {
        ui->biometricFinishbtn->setToolTip(ui->biometricFinishbtn->text());
        ui->biometricFinishbtn->setText(str2);
    }
    QFont cancelFont;
    QString cancelStr = ElideText(cancelFont, 95, ui->biometricCancelbtn->text());
    if (cancelStr != ui->biometricCancelbtn->text()) {
        ui->biometricCancelbtn->setToolTip(ui->biometricCancelbtn->text());
        ui->biometricCancelbtn->setText(cancelStr);
    }
    ui->biometricFinishbtn->hide();
    ui->biometricConBtn->hide();
    ui->biometricFinishLabel->hide();
    this->mainWidget()->setLayout(ui->mainLayout);
}

QString BiometricEnrollDialog::ElideText(QFont font, int width, QString strInfo)
{
    QFontMetrics fontMetrics(font);
    // 如果当前字体下，字符串长度大于指定宽度
    if (fontMetrics.width(strInfo) > width) {
        strInfo = QFontMetrics(font).elidedText(strInfo, Qt::ElideRight, width);
    }
    return strInfo;
}

QString BiometricEnrollDialog::transferBioType(int type)
{
    switch (type) {
        case BIOTYPE_FINGERPRINT:
            return tr("FingerPrint");
        case BIOTYPE_FINGERVEIN:
            return tr("Fingervein");
        case BIOTYPE_IRIS:
            return tr("Iris");
        case BIOTYPE_FACE:
            return tr("Face");
        case BIOTYPE_VOICEPRINT:
            return tr("VoicePrint");
    }
    return QString();
}

QString BiometricEnrollDialog::lowerTransferBioType(int type)
{
    switch (type) {
        case BIOTYPE_FINGERPRINT:
            return tr("fingerprint");
        case BIOTYPE_FINGERVEIN:
            return tr("fingervein");
        case BIOTYPE_IRIS:
            return tr("iris");
        case BIOTYPE_FACE:
            return tr("face");
        case BIOTYPE_VOICEPRINT:
            return tr("voiceprint");
    }
    return QString();
}

void BiometricEnrollDialog::setTitle(int biotype)
{
    QString title = lowerTransferBioType(type);
    switch (biotype) {
        case ENROLL:
            title = tr("Enroll ") + title + tr(" feature");
            break;
        case VERIFY:
            title += tr("Verify ");
            break;
        case SEARCH:
            title += tr("Search ");
            break;
    }

    ui->biometricOpsLbl->setText(title);
}

void BiometricEnrollDialog::setPrompt(QString text)
{
    ui->biometricPromptLbl->setText(text);
    ui->biometricPromptLbl->setAlignment(Qt::AlignCenter);
}

BiometricEnrollDialog::Result BiometricEnrollDialog::getResult()
{
    return opsResult;
}

int BiometricEnrollDialog::enroll(int drvId, int uid, int idx, const QString &idxName)
{
    QList<QVariant> args;
    args << drvId << uid << idx << idxName;

    this->setTitle(ENROLL);
    this->closeButton()->setEnabled(false);

    serviceInterface->callWithCallback(
        "Enroll", args, this, SLOT(enrollCallBack(const QDBusMessage &)), SLOT(errorCallBack(const QDBusError &)));
    ops = ENROLL;
    m_hisOps = ENROLL;
    m_listEnrollArgs = args;

    return exec();
}

void BiometricEnrollDialog::enrollCallBack(const QDBusMessage &reply)
{
    int result;
    dup_fd = -1;
    result = reply.arguments()[0].value<int>();
    qDebug() << "Enroll result: " << result;

    if (w_timer && w_timer->isActive()) {
        w_timer->stop();
        ui->biometricFinishbtn->show();
    }
    this->closeButton()->setEnabled(true);

    QString bioType = lowerTransferBioType(type);
    switch (result) {
        case DBUS_RESULT_SUCCESS: { /* 录入成功 */
            opsResult = SUCESS;
            setPrompt(bioType + tr("add successfully"));
            Q_EMIT enrollFinished();
            showFinishPrompt();
            m_hisOps = IDLE;
            break;
        }
        default:
            opsResult = ERROR;
            ui->biometricEnrollLable->setPixmap(QIcon::fromTheme("dialog-error").pixmap(QSize(88, 88)));
            ui->biometricEnrollLable->setFixedSize(88, 88);
            handleErrorResult(result);
            break;
    }
    ops = IDLE;
}

int BiometricEnrollDialog::verify(int drvId, int uid, int idx)
{
    QList<QVariant> args;
    args << drvId << uid << idx;

    this->setTitle(VERIFY);

    serviceInterface->callWithCallback(
        "Verify", args, this, SLOT(verifyCallBack(const QDBusMessage &)), SLOT(errorCallBack(const QDBusError &)));
    ops = VERIFY;
    m_hisOps = VERIFY;
    m_listVerifyArgs = args;

    return exec();
}

QString BiometricEnrollDialog::getGif(int type)
{
    switch (type) {
        case BIOTYPE_FINGERPRINT:
            return "/usr/share/ukui-biometric/images/FingerPrint.gif";
        case BIOTYPE_FINGERVEIN:
            return "/usr/share/ukui-biometric/images/FingerVein.gif";
        case BIOTYPE_IRIS:
            return "/usr/share/ukui-biometric/images/Iris.gif";
        case BIOTYPE_VOICEPRINT:
            return "/usr/share/ukui-biometric/images/VoicePrint.gif";
    }
    return QString();
}

QString BiometricEnrollDialog::getImage(int type)
{
    switch (type) {
        case BIOTYPE_FINGERPRINT:
            if (isProcessed) {
                return "/usr/share/ukui-biometric/images/huawei/00.svg";
            } else {
                return "/usr/share/ukui-biometric/images/FingerPrint.png";
            }
        case BIOTYPE_FINGERVEIN:
            return "/usr/share/ukui-biometric/images/FingerVein.png";
        case BIOTYPE_IRIS:
            return "/usr/share/ukui-biometric/images/Iris.png";
        case BIOTYPE_VOICEPRINT:
            return "/usr/share/ukui-biometric/images/VoicePrint.png";
    }
    return QString();
}

void BiometricEnrollDialog::getEnrollImageList(int type)
{
    QString path;
    QDir dir;
    switch (type) {
        case BIOTYPE_FINGERPRINT:
            path = ":/imgs/resources/fingerprint";
            dir.setPath(path);
            m_enrollImagePaths = dir.entryList(QStringList("*.svg"), QDir::Files);
            // 构造完整的文件路径
            for (int i = 0; i < m_enrollImagePaths.size(); ++i) {
                m_enrollImagePaths[i] = path + "/" + m_enrollImagePaths[i];
            }
            break;
        case BIOTYPE_IRIS:
            path = ":/imgs/resources/iris";
            dir.setPath(path);
            m_enrollImagePaths = dir.entryList(QStringList("*.svg"), QDir::Files);
            // 构造完整的文件路径
            for (int i = 0; i < m_enrollImagePaths.size(); ++i) {
                m_enrollImagePaths[i] = path + "/" + m_enrollImagePaths[i];
            }
            break;
    }
}

void BiometricEnrollDialog::verifyCallBack(const QDBusMessage &reply)
{
    int result;
    dup_fd = -1;
    result = reply.arguments()[0].value<int>();
    qDebug() << "Verify result: " << result;

    if (result >= 0) {
        opsResult = SUCESS;
        setPrompt(tr("Verify successfully"));
        showFinishPrompt();
        m_hisOps = IDLE;
    } else if (result == DBUS_RESULT_NOTMATCH) {
        setPrompt(tr("Not Match"));
        ui->biometricEnrollLable->setPixmap(QIcon::fromTheme("dialog-error").pixmap(QSize(88, 88)));
        ui->biometricEnrollLable->setFixedSize(88, 88);
        m_hisOps = IDLE;
    } else {
        handleErrorResult(result);
    }

    ops = IDLE;
}

void BiometricEnrollDialog::resetUI()
{
    w_timer->start();
    if (isProcessed) {
        if (w_timer) {
            w_timer->stop();
        }

        ui->biometricEnrollLable->setPixmap(QPixmap("/usr/share/ukui-biometric/images/huawei/00.svg"));
        ui->biometricEnrollLable->setFixedSize(154, 154);
    } else {
        ui->biometricEnrollLable->setPixmap(getImage(type));
        ui->biometricEnrollLable->setFixedSize(154, 154);
        if (!movie) {
            movie = new QMovie(getGif(type));
            movie->setScaledSize(QSize(156, 156));
        }
    }
    ui->biometricFinishbtn->hide();
    ui->biometricCancelbtn->show();
    ui->biometricConBtn->hide();
    ui->biometricFinishLabel->hide();
    ui->biometricPromptLbl->show();
    ui->biometricPromptLbl->clear();

    ui->biometricOpsLbl->show();

    if ((type == BIOTYPE_FINGERPRINT || type == BIOTYPE_IRIS) && !isProcessed) {
        m_enrollTimer->start();
        m_enrollFinish = false;
    }
}

void BiometricEnrollDialog::on_biometricFinishbtn_clicked()
{
    close();
}

void BiometricEnrollDialog::on_biometricCancelbtn_clicked()
{
    close();
}

QStringList BiometricEnrollDialog::getAllFeatureinfolist(
    int uid, int indexStart, int indexEnd, int driver_type, QList<int> &idxList)
{
    QStringList list;
    QList<QDBusVariant> qlist;
    FeatureInfo *featureInfo;
    int listsize;
    QDBusMessage result = serviceInterface->call(QStringLiteral("GetAllFeatureList"), uid, indexStart, indexEnd);
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
        if (driver_type == featureInfo->biotype)
            idxList.append(featureInfo->index);
        delete featureInfo;
    }
    return list;
}

void BiometricEnrollDialog::on_biometricConBtn_clicked()
{
    resetUI();

    int num = 1;
    int freeIndex;
    QList<int> idxList;
    QStringList list = getAllFeatureinfolist(getuid(), 0, -1, type, idxList);
    QString featurename;
    while (1) {
        featurename = DeviceType::getDeviceType_tr(type) + QString::number(num);
        if (!list.contains(featurename))
            break;
        num++;
    }
    num = 0;
    while (1) {
        if (!idxList.contains(num)) {
            freeIndex = num;
            break;
        }
        num++;
    }
    enroll(deviceId, getuid(), freeIndex, featurename);
}

int BiometricEnrollDialog::search(int drvId, int uid, int idxStart, int idxEnd)
{
    QList<QVariant> args;
    args << drvId << uid << idxStart << idxEnd;

    this->setTitle(SEARCH);

    serviceInterface->callWithCallback(
        "Search", args, this, SLOT(searchCallBack(const QDBusMessage &)), SLOT(errorCallBack(const QDBusError &)));

    ops = SEARCH;
    m_hisOps = SEARCH;
    m_listSearchArgs = args;

    return exec();
}

void BiometricEnrollDialog::searchCallBack(const QDBusMessage &reply)
{
    m_hisOps = IDLE;
    return;
}

void BiometricEnrollDialog::showFinishPrompt()
{
    ui->biometricEnrollLable->setPixmap(QIcon::fromTheme("ukui-dialog-success").pixmap(QSize(88, 88)));
    ui->biometricEnrollLable->setFixedSize(88, 88);
    QString typeText = transferBioType(type);
    if (ops == ENROLL)
        ui->biometricFinishLabel->setText(typeText + tr("add successfully"));
    else if (ops == VERIFY)
        ui->biometricFinishLabel->setText(tr("Verify successfully"));

    ui->biometricFinishLabel->show();
    ui->biometricFinishbtn->show();
    ui->biometricFinishbtn->setText(tr("Finish"));
    ui->biometricCancelbtn->hide();
    ui->biometricPromptLbl->hide();
    ui->biometricOpsLbl->hide();
    ui->biometricButtonWidget->show();

    if (ops == ENROLL)
        ui->biometricConBtn->show();
    else
        ui->biometricConBtn->hide();

    if (m_enrollTimer) {
        m_enrollTimer->stop();
        m_enrollFinish = true;
    }
}

void BiometricEnrollDialog::showErrorPrompt(QString tips)
{
    ui->biometricEnrollLable->setPixmap(QIcon::fromTheme("dialog-error").pixmap(QSize(88, 88)));
    ui->biometricEnrollLable->setFixedSize(88, 88);
    ui->biometricFinishLabel->setText(tips);

    ui->biometricFinishLabel->show();
    ui->biometricFinishbtn->show();
    ui->biometricFinishbtn->setText(tr("Finish"));
    ui->biometricCancelbtn->hide();

    ui->biometricPromptLbl->hide();
    ui->biometricOpsLbl->hide();
    ui->biometricButtonWidget->show();
    ui->biometricConBtn->hide();

    if (m_enrollTimer) {
        m_enrollTimer->stop();
        m_enrollFinish = true;
    }
}

void BiometricEnrollDialog::StopOpsCallBack(const QDBusMessage &reply)
{
    dup_fd = -1;
    int ret = reply.arguments().at(0).toInt();
    accept();
}

void BiometricEnrollDialog::errorCallBack(const QDBusError &error)
{
    m_hisOps = IDLE;
    dup_fd = -1;
    qDebug() << "DBus Error: " << error.message();
    accept();
}

void BiometricEnrollDialog::closeEvent(QCloseEvent *event)
{
    if (ops != IDLE)
        serviceInterface->call("StopOps", deviceId, 5);
}

void BiometricEnrollDialog::onFrameWritten(int drvId)
{
    if (m_isUseEnrollFd && ops == ENROLL) {
        return;
    }
    if (ops == IDLE) {
        qDebug() << "Not in enroll or verify!";
        return;
    }
    if (drvId != deviceId) {
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
        if (image.isNull()) {
            return;
        }
        image = image.convertToFormat(QImage::Format_RGB888);
    }

    QPixmap faceQImage = PixmapToRound(QPixmap::fromImage(image), 77);
    ui->biometricEnrollLable->setPixmap(faceQImage);
    ui->biometricEnrollLable->setFixedSize(154, 154);
}

void BiometricEnrollDialog::onEnrollFrameWritten(int drvId)
{
    if (!m_isUseEnrollFd || ops != ENROLL) {
        return;
    }
    if (ops == IDLE) {
        qDebug() << "Not in enroll or verify!";
        return;
    }
    if (drvId != deviceId) {
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
    }
    lseek(dup_fd, 0, SEEK_SET);
    char base64_bufferData[1024 * 1024];
    int rc = read(dup_fd, base64_bufferData, 1024 * 1024);
    printf("rc = %d\n", rc);

    QImage image;
    if (rc > 0) {
        QByteArray imageData(base64_bufferData, rc);
        QBuffer imgBuffer(&imageData);
        imgBuffer.open(QIODevice::ReadOnly);

        QImageReader imgReader(&imgBuffer);
        image = imgReader.read();
        if (image.isNull()) {
            return;
        }
        image = image.convertToFormat(QImage::Format_RGB888);
    }
    QPixmap faceQImage = PixmapToRound(QPixmap::fromImage(image), 77);
    ui->biometricEnrollLable->setPixmap(faceQImage);
    ui->biometricEnrollLable->setFixedSize(154, 154);
}

void BiometricEnrollDialog::onProcessChanged(int drvId, QString aa, int statusType, QString bb)
{
    int count = statusType * 15 / 100;
    QString filename = QString("/usr/share/ukui-biometric/images/huawei/") + (count < 10 ? "0" : "")
                       + QString::number(count) + ".svg";

    ui->biometricEnrollLable->setPixmap(QPixmap(filename));
    ui->biometricEnrollLable->setFixedSize(154, 154);
}

void BiometricEnrollDialog::onStatusChanged(int drvId, int statusType)
{
    if (!(drvId == deviceId && statusType == STATUS_NOTIFY))
        return;

    this->closeButton()->setEnabled(true);
    ui->biometricFinishbtn->hide();
    ui->biometricCancelbtn->show();

    // 过滤掉当录入时使用生物识别授权接收到的认证的提示信息
    if (ops == ENROLL) {
        QDBusMessage reply = serviceInterface->call("UpdateStatus", drvId);
        if (reply.type() == QDBusMessage::ErrorMessage) {
            qDebug() << "DBUS: " << reply.errorMessage();
            return;
        }
        int devStatus = reply.arguments().at(3).toInt();
        qDebug() << devStatus;

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

    if (!isProcessed && !isFace && type != BIOTYPE_FINGERPRINT && type != BIOTYPE_IRIS) {
        if (w_timer && w_timer->isActive()) {
            w_timer->stop();
        }
        if (movie) {
            if (movie->state() != QMovie::Running) {
                ui->biometricEnrollLable->setMovie(movie);
                movie->start();
            } else {
                movie->stop();
                ui->biometricEnrollLable->setMovie(movie);
                movie->start();
            }
        }
    } else if (!isProcessed && (type == BIOTYPE_FINGERPRINT || type == BIOTYPE_IRIS)) {
        if (w_timer && w_timer->isActive()) {
            w_timer->stop();
        }
        if (m_enrollTimer) {
            m_enrollTimer->start(50);
        }
    }

    QDBusMessage notifyReply = serviceInterface->call("GetNotifyMesg", drvId);
    if (notifyReply.type() == QDBusMessage::ErrorMessage) {
        qDebug() << "DBUS: " << notifyReply.errorMessage();
        return;
    }
    QString prompt = notifyReply.arguments().at(0).toString();
    qDebug() << prompt;

    if (type == 3) {
        setPrompt(tr("Keep looking straight at the camera."));
    } else {
        setPrompt(prompt);
    }
}

void BiometricEnrollDialog::handleErrorResult(int error)
{
    dup_fd = -1;
    switch (error) {
        case DBUS_RESULT_ERROR: {
            // 操作失败，需要进一步获取失败原因
            QDBusMessage msg = serviceInterface->call("GetOpsMesg", deviceId);
            if (msg.type() == QDBusMessage::ErrorMessage) {
                qDebug() << "UpdateStatus error: " << msg.errorMessage();
                setPrompt(tr("D-Bus calling error"));
                m_hisOps = IDLE;
                return;
            }
            showErrorPrompt(msg.arguments().at(0).toString());
            qDebug() << "GetOpsMesg: deviceId--" << deviceId;
            StatusReslut ret = UpdateStatus(deviceId);
            qDebug() << "StatusReslut:" << ret.result << "," << ret.enable << "," << ret.devNum << "," << ret.devStatus
                     << "," << ret.opsStatus << "," << ret.notifyMessageId;
            // 识别操作超时 304/404 认证超时；8 网络错误；
            if (ret.result == 0) {
                if (ret.opsStatus != OPS_ENROLL_STOP_BY_USER && ret.opsStatus != OPS_IDENTIFY_STOP_BY_USER
                    && ret.opsStatus != OPS_VERIFY_STOP_BY_USER && ret.opsStatus != OPS_SEARCH_STOP_BY_USER
                    && ret.opsStatus != OPS_GET_FLIST_SUCCESS && ret.opsStatus != OPS_OPEN_SUCCESS
                    && ret.opsStatus != OPS_COMM_SUCCESS
                    && ret.opsStatus != OPS_COMM_STOP_BY_USER // 添加一个"打开设备完成"或者"操作成功"的操作结果，在hwe上录入特征时锁屏，解锁后后获取的信息返回是“打开设备完成”或者"操作成功"
                    && ret.opsStatus != OPS_IDENTIFY_MATCH && ret.opsStatus != OPS_TYPE_IDENTIFY && ret.opsStatus != OPS_IDENTIFY_NO_MATCH ) { // v11人脸锁屏解锁后返回的消息是“识别特征匹配”
                    m_hisOps = IDLE; // 主动停止
                }
            } else {
                m_hisOps = IDLE;
            }
            break;
        }
        case DBUS_RESULT_DEVICEBUSY:
            // 设备忙
            showErrorPrompt(tr("Device is busy"));
            m_hisOps = IDLE;
            break;
        case DBUS_RESULT_NOSUCHDEVICE:
            // 设备不存在
            showErrorPrompt(tr("No such device"));
            m_hisOps = IDLE;
            break;
        case DBUS_RESULT_PERMISSIONDENIED:
            // 没有权限
            // showErrorPrompt(tr("Permission denied"));
            close();
            m_hisOps = IDLE;
            break;
    }
}

void BiometricEnrollDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
}

void BiometricEnrollDialog::onPrepareForSleep(bool sleep)
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

void BiometricEnrollDialog::onSysLock()
{
    if (!(m_uSysState & SYSSTATE_LOCK)) {
        m_uSysState |= SYSSTATE_LOCK;
        chkSysState();
    }
}

void BiometricEnrollDialog::onSysUnLock()
{
    if (m_uSysState & SYSSTATE_LOCK) {
        m_uSysState &= ~SYSSTATE_LOCK;
        chkSysState();
    }
}

void BiometricEnrollDialog::chkSysState()
{
    qInfo() << "chkSysState:" << m_uSysState << "," << m_hisOps;
    if (m_uSysState) { // 停止
        serviceInterface->call(QStringLiteral("StopOps"), deviceId, 5);
    } else { // 恢复
        if (m_hisOps == IDLE)
            return;
        resetUI();
        ui->biometricEnrollLable->setPixmap(getImage(type));
        ui->biometricEnrollLable->setFixedSize(154, 154);
        setPrompt("");
        if (movie && movie->state() == QMovie::Running) {
            movie->stop();
        }
        if (m_enrollTimer && m_enrollTimer->isActive()) {
            m_enrollTimer->stop();
        }
        switch (m_hisOps) {
            case ENROLL: {
                this->closeButton()->setEnabled(false);
                serviceInterface->callWithCallback(
                    "Enroll",
                    m_listEnrollArgs,
                    this,
                    SLOT(enrollCallBack(const QDBusMessage &)),
                    SLOT(errorCallBack(const QDBusError &)));
                ops = m_hisOps;
            } break;
            case VERIFY: {
                serviceInterface->callWithCallback(
                    "Verify",
                    m_listVerifyArgs,
                    this,
                    SLOT(verifyCallBack(const QDBusMessage &)),
                    SLOT(errorCallBack(const QDBusError &)));
                ops = m_hisOps;
            } break;
            case SEARCH: {
                serviceInterface->callWithCallback(
                    "Search",
                    m_listSearchArgs,
                    this,
                    SLOT(searchCallBack(const QDBusMessage &)),
                    SLOT(errorCallBack(const QDBusError &)));
                ops = m_hisOps;
            } break;
            default:
                break;
        }
    }
}

QPixmap BiometricEnrollDialog::PixmapToRound(const QPixmap &src, int radius)
{
    if (src.isNull()) {
        return QPixmap();
    }

    QPixmap pixmapa(src);
    QPixmap pixmap(radius * 2, radius * 2);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addEllipse(0, 0, radius * 2, radius * 2);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, radius * 2, radius * 2, pixmapa);
    return pixmap;
}

void BiometricEnrollDialog::updatePixmap()
{
    QMatrix matrix;
    matrix.rotate(90.0);
    m_waitingPixmap = m_waitingPixmap.transformed(matrix, Qt::FastTransformation);
    ui->biometricEnrollLable->setPixmap(m_waitingPixmap);
}

StatusReslut BiometricEnrollDialog::UpdateStatus(int drvid)
{
    StatusReslut status;
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

void BiometricEnrollDialog::updateEnrollImage()
{
    if (m_currentImage >= 0 && m_currentImage < m_enrollImagePaths.size()) {
        QPixmap pixmap;
        pixmap.load(m_enrollImagePaths.at(m_currentImage));
        ui->biometricEnrollLable->setPixmap(pixmap);
        ui->biometricEnrollLable->setFixedSize(156, 156);
    }
}

void BiometricEnrollDialog::switchEnrollImage()
{
    if (m_enrollSort) {
        m_currentImage += 1;
        if (m_currentImage == m_enrollImagePaths.size()) {
            m_enrollSort = false;
            //  等待一段时间后重新启动定时器
            m_enrollTimer->stop();
            QTimer::singleShot(100, this, [&, this]() {
                if (!m_enrollFinish) {
                    m_enrollTimer->start();
                }
            });
        }
    } else {
        m_currentImage -= 1;
        if (m_currentImage < 0) {
            m_enrollSort = true;
            //  等待一段时间后重新启动定时器
            m_enrollTimer->stop();
            QTimer::singleShot(100, this, [&, this]() {
                if (!m_enrollFinish) {
                    m_enrollTimer->start();
                }
            });
        }
    }
    updateEnrollImage();
}
