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
#include "loginoptionswidget.h"

#include <pwd.h>
#include <unistd.h>

#include "giodbus.h"
#include "generic.h"
#include "biotypes.h"

#include <QLabel>
#include <QWidget>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QImageReader>
#include <QToolButton>
#include <QSizePolicy>
#include <QTimer>
#include <QPixmap>
#include <QPainter>
#include <QtSvg/QSvgRenderer>
#include <QImage>
#include <QApplication>
#include <QFont>
#include <QDebug>
#include <QPixmapCache>

LoginOptionsWidget::LoginOptionsWidget(QWidget *parent)
    : QWidget(parent), m_biomericProxy(new BioAuth(this)), m_bioDevices(new BioDevices()), w_timer(nullptr), m_uniAuthService(new UniAuthService(this))
{
    m_bioDevices->recordAuthDrive("polkit", 0, false);
    initUI();
    initConnections();
    initStyleTheme();
    m_mapDisableDev.clear();
}

LoginOptionsWidget::~LoginOptionsWidget() {}

void LoginOptionsWidget::initUI()
{
    // 初始化UI
    m_layoutMain = new QVBoxLayout();
    m_layoutMain->setContentsMargins(0, 0, 0, 30);
    m_layoutMain->setSpacing(8);
    m_layoutMain->setAlignment(Qt::AlignTop);

    m_widgetImage = new QWidget();
    m_widgetImage->setFixedSize(154, 154);
    m_layoutImage = new QHBoxLayout();
    m_layoutImage->setContentsMargins(0, 8, 0, 16);
    m_layoutImage->setSpacing(0);
    m_layoutImage->setAlignment(Qt::AlignCenter);
    m_layoutImage->addSpacerItem(new QSpacerItem(109, 0, QSizePolicy::Expanding));
    m_layoutImage->addWidget(m_widgetImage);
    m_layoutImage->addSpacerItem(new QSpacerItem(109, 0, QSizePolicy::Expanding));
    m_widgetImage->hide();

    // 人脸识别
    m_labelFace = new QLabel(m_widgetImage);
    m_labelFace->setObjectName("faceLabel");
    m_labelFace->setAlignment(Qt::AlignCenter);
    m_labelFace->setFixedSize(154, 154);
    QHBoxLayout *layoutFace = new QHBoxLayout();
    layoutFace->setContentsMargins(0, 0, 0, 0);
    layoutFace->setAlignment(Qt::AlignCenter);
    m_labelFace->setLayout(layoutFace);
    m_labelFace->hide();
    m_labelFaceLoad = new QLabel();
    m_labelFaceLoad->setAlignment(Qt::AlignCenter);
    m_labelFaceLoad->setFixedSize(142, 142);
    m_labelFaceLoad->setStyleSheet(
        QString("QLabel{background-color: rgba(230,230,230,0.39); border-radius: %1px; border: 8px solid white;}")
            .arg(71));
    layoutFace->addWidget(m_labelFaceLoad, 0, Qt::AlignVCenter);

    // 二维码窗口
    m_labelQRCode = new QLabel(m_widgetImage);
    m_labelQRCode->setObjectName("qrCodeLabel");
    m_labelQRCode->setAlignment(Qt::AlignCenter);
    m_labelQRCode->setFixedSize(154, 154);
    m_labelQRCode->hide();
    QVBoxLayout *layoutQRCode = new QVBoxLayout();
    layoutQRCode->setAlignment(Qt::AlignCenter);
    layoutQRCode->setSpacing(5);
    m_labelQRCode->setLayout(layoutQRCode);
    m_labelQRCodeTip = new QLabel();
    m_labelQRCodeTip->setFixedSize(22, 22);
    m_labelQRCodeTip->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(22, 22)));
    layoutQRCode->addWidget(m_labelQRCodeTip, 0, Qt::AlignHCenter);
    m_labelQRCodeMsg = new QLabel();
    m_labelQRCodeMsg->setFixedHeight(24);
    QFont font = m_labelQRCodeMsg->font();
    font.setPixelSize(16);
    m_labelQRCodeMsg->setStyleSheet("QLabel{background-color:rgba(255,255,255,0);color:rgb(255,0,0)}");
    m_labelQRCodeMsg->setFont(font);
    layoutQRCode->addWidget(m_labelQRCodeMsg, 0, Qt::AlignHCenter);

    // 生物特征等待验证动画
    m_labelBioWaiting = new QLabel(m_widgetImage);
    m_labelBioWaiting->setAlignment(Qt::AlignCenter);
    m_labelBioWaiting->setFixedSize(154, 154);
    m_labelBioWaiting->hide();

    m_layoutMain->addLayout(m_layoutImage);

    // 创建包含label和按钮的容器widget和布局
    m_widgetOptContainer = new QWidget();
    m_layoutOptContainer = new QVBoxLayout(m_widgetOptContainer);
    m_layoutOptContainer->setContentsMargins(0, 0, 0, 0);
    m_layoutOptContainer->setSpacing(8);
    m_layoutOptContainer->setAlignment(Qt::AlignTop);

    m_labelOptTitle = new QLabel();
    m_labelOptTitle->setAlignment(Qt::AlignLeft);
    m_labelOptTitle->setText(tr("Login Options"));
    m_labelOptTitle->adjustSize();
    m_layoutOptContainer->addWidget(m_labelOptTitle);

    m_btnGroup = new QButtonGroup(this);
    m_btnGroup->setExclusive(true);

    m_layoutOptBtns = new QHBoxLayout();
    m_layoutOptBtns->setContentsMargins(0, 0, 0, 0);
    m_layoutOptBtns->setSpacing(8);
    m_layoutOptBtns->setAlignment(Qt::AlignLeft);
    m_layoutOptContainer->addLayout(m_layoutOptBtns);

    m_layoutMain->addWidget(m_widgetOptContainer);

    this->setLayout(m_layoutMain);

    // 设置水平扩展
    QSizePolicy sizePolicy = this->sizePolicy();
    sizePolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    this->setSizePolicy(sizePolicy);
}

void LoginOptionsWidget::SetExtraInfo(QString extra_info, QString info_type)
{
    if (!m_biomericProxy) {
        qWarning() << "BiometricProxy doesn't exist.";
        return;
    }
    m_biomericProxy->SetExtraInfo(info_type, extra_info);
}

void LoginOptionsWidget::initConnections()
{
    if (m_biomericProxy) {
        connect(m_biomericProxy, &BioAuth::notifyDetail, this, &LoginOptionsWidget::onStatusChanged);

        connect(m_biomericProxy, &BioAuth::frameWritten, this, &LoginOptionsWidget::onFrameWritten);

        connect(m_biomericProxy, &BioAuth::authComplete, this, &LoginOptionsWidget::onIdentifyComplete);

        connect(m_biomericProxy, &BioAuth::authFinished, this, &LoginOptionsWidget::onAuthFinished);
    }
    if (m_bioDevices) {
        connect(m_bioDevices, &BioDevices::deviceCountChanged, this, &LoginOptionsWidget::onUSBDeviceCountChange);
        // readDevicesInfo();
    }
    connect(m_btnGroup, SIGNAL(buttonClicked(int)), this, SLOT(onOptionSelected(int)));
}

void LoginOptionsWidget::initStyleTheme()
{
    const QByteArray idd("org.ukui.style");
    if (QGSettings::isSchemaInstalled(idd)) {
        m_styleSettings = new QGSettings(idd);
    }
    if (m_styleSettings) {
        connect(m_styleSettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "styleName") {
                auto styleNameValue = m_styleSettings->get("styleName");
                if (styleNameValue.isValid()) {
                    m_strThemeName = styleNameValue.toString();
                }
            }
        });
        auto styleNameValue = m_styleSettings->get("styleName");
        if (styleNameValue.isValid()) {
            m_strThemeName = styleNameValue.toString();
        }
    }
}

bool LoginOptionsWidget::getCurLoginOpt(int &nLoginOptType, int &nDrvId)
{
    if (m_curDevInfo) {
        nLoginOptType = convertDeviceType(m_curDevInfo->biotype);
        nDrvId = m_curDevInfo->device_id;
        return true;
    }
    return false;
}

unsigned LoginOptionsWidget::getLoginOptCount()
{
    return m_mapDevices.size();
}

DeviceInfoPtr LoginOptionsWidget::getFirstDevInfo()
{
    DeviceInfoPtr devInfo = nullptr;
    struct passwd *pwdInfo = getpwuid(m_uid);
    if (!pwdInfo)
        return devInfo;
    int nDrvId = m_bioDevices->GetLastDevice(pwdInfo->pw_name);
    if (nDrvId >= 0) {
        qDebug() << "GetLastDevice:" << nDrvId;
        DeviceMap::iterator itDevInfo = m_mapDevices.begin();
        for (; itDevInfo != m_mapDevices.end(); itDevInfo++) {
            for (auto devinfo : itDevInfo.value()) {
                if (devinfo && devinfo->device_id == nDrvId) {
                    if (!isDeviceDisable(devinfo->device_id)) {
                        devInfo = devinfo;
                        break;
                    }
                }
            }
            if (devInfo) {
                break;
            }
        }
    }
    if (!devInfo) {
        DeviceMap::iterator itDevInfo = m_mapDevices.begin();
        for (; itDevInfo != m_mapDevices.end(); itDevInfo++) {
            for (auto devinfo : itDevInfo.value()) {
                if (devinfo && !isDeviceDisable(devinfo->device_id)) {
                    devInfo = devinfo;
                    break;
                }
            }
            if (devInfo) {
                break;
            }
        }
    }
    return devInfo;
}

bool LoginOptionsWidget::getHasUkeyOptions()
{
    return isShowUkey;
}

void LoginOptionsWidget::addOptionButton(unsigned uLoginOptType, int nDrvId, QString strDrvName)
{
    if (m_mapOptBtns.contains(nDrvId)) {
        return;
    }

    QToolButton *newButton = new QToolButton();
    newButton->setCheckable(true);
    newButton->setChecked(false);
    newButton->setFocusPolicy(Qt::NoFocus);
    newButton->setContentsMargins(16, 16, 16, 16);

    int nLength = m_btnGroup->buttons().length();
    m_btnGroup->addButton(newButton, nLength);
    m_listDriveId.append(nDrvId);

    QIcon icon;
    switch (uLoginOptType) {
        case LOGINOPT_TYPE_PASSWORD:
            icon = QIcon(QString("%1/images/ukui-loginopt-password.svg").arg(GET_STR(UKUI_BIOMETRIC)));
            break;
        case LOGINOPT_TYPE_GENERAL_UKEY:
            icon = QIcon(QString("%1/images/ukui-loginopt-ukey.svg").arg(GET_STR(UKUI_BIOMETRIC)));
            break;
        case LOGINOPT_TYPE_FACE:
            icon = QIcon(QString("%1/images/ukui-loginopt-face.svg").arg(GET_STR(UKUI_BIOMETRIC)));
            break;
        case LOGINOPT_TYPE_FINGERPRINT:
            icon = QIcon(QString("%1/images/ukui-loginopt-finger.svg").arg(GET_STR(UKUI_BIOMETRIC)));
            break;
        case LOGINOPT_TYPE_IRIS:
            icon = QIcon(QString("%1/images/ukui-loginopt-iris.svg").arg(GET_STR(UKUI_BIOMETRIC)));
            break;
        case LOGINOPT_TYPE_VOICEPRINT:
            icon = QIcon(QString("%1/images/ukui-loginopt-voice.svg").arg(GET_STR(UKUI_BIOMETRIC)));
            break;
        case LOGINOPT_TYPE_FINGERVEIN:
            icon = QIcon(QString("%1/images/ukui-loginopt-fingervein.svg").arg(GET_STR(UKUI_BIOMETRIC)));
            break;
        case LOGINOPT_TYPE_QRCODE:
            icon = QIcon(QString("%1/images/ukui-loginopt-qrcode.svg").arg(GET_STR(UKUI_BIOMETRIC)));
            break;
    }
    newButton->setIcon(icon);
    newButton->setToolTip(strDrvName);
    newButton->setFixedSize(36, 36);
    if (isDeviceDisable(nDrvId)) {
        newButton->setDisabled(true);
    } else {
        newButton->setDisabled(false);
    }

    m_layoutOptBtns->addWidget(newButton);
    m_mapOptBtns[nDrvId] = newButton;
}

void LoginOptionsWidget::clearOptionButtons()
{
    QMap<int, QToolButton *>::iterator itMapBtn = m_mapOptBtns.begin();
    for (; itMapBtn != m_mapOptBtns.end(); itMapBtn++) {
        if (itMapBtn.value()) {
            m_btnGroup->removeButton(itMapBtn.value());
            m_layoutOptBtns->removeWidget(itMapBtn.value());
            itMapBtn.value()->deleteLater();
        }
    }
    m_listDriveId.clear();
    m_mapOptBtns.clear();
}

void LoginOptionsWidget::updateOptionButtons()
{
    isShowUkey = false;
    clearOptionButtons();
    addOptionButton(LOGINOPT_TYPE_PASSWORD, -1, tr("Password"));
    DeviceMap::iterator itMapDev = m_mapDevices.begin();
    for (; itMapDev != m_mapDevices.end(); itMapDev++) {
        for (DeviceInfoPtr devPtr : itMapDev.value()) {
            if (devPtr) {
                if (devPtr->biotype == LOGINOPT_TYPE_GENERAL_UKEY) {
                    // ukey 设备类型排在二维码前，但实际上应该显示在二维码之后，因此暂时不添加
                    isShowUkey = true;
                    continue;
                }
                addOptionButton(itMapDev.key(), devPtr->device_id, BioDevices::bioTypeToString_tr(devPtr->biotype));
            }
        }
    }

    itMapDev = m_mapDevices.begin();
    if (isShowUkey) {
        for (; itMapDev != m_mapDevices.end(); itMapDev++) {
            for (DeviceInfoPtr devPtr : itMapDev.value()) {
                if (devPtr && devPtr->biotype == LOGINOPT_TYPE_GENERAL_UKEY) {
                    // 此处才添加ukey
                    addOptionButton(itMapDev.key(), devPtr->device_id, BioDevices::bioTypeToString_tr(devPtr->biotype));
                }
            }
        }
    }

    // 存在特征但没有插入ukey
    if (!isShowUkey && m_biomericProxy->GetHasUkeyFeature(m_uid)) {
        addOptionButton(LOGINOPT_TYPE_GENERAL_UKEY, -2, BioDevices::bioTypeToString_tr(LOGINOPT_TYPE_GENERAL_UKEY));
        isShowUkey = true;
    }

    m_labelOptTitle->show();
    QMap<int, QToolButton *>::iterator itMapBtn = m_mapOptBtns.begin();
    for (; itMapBtn != m_mapOptBtns.end(); itMapBtn++) {
        if (itMapBtn.value()) {
            itMapBtn.value()->show();
        }
    }
}

void LoginOptionsWidget::setUser(int uid)
{
    bool bNeedUpdateDevInfo = false;
    if (uid != m_uid) {
        bNeedUpdateDevInfo = true;
    }
    m_uid = uid;
    if (bNeedUpdateDevInfo) {
        if (m_bioDevices) {
            m_bioDevices->setUId(m_uid);
        }
        readDevicesInfo();
    }
}

void LoginOptionsWidget::readDevicesInfo()
{
    m_mapDevices.clear();
    bool isAuthEnable = m_bioDevices->GetBioAuthEnable();
    bool isQRCodeEnable = m_bioDevices->GetQRCodeEnable();
    DeviceList deviceList = m_bioDevices->GetDevList();
    QStringList listDefDevices = m_bioDevices->getAllDefDevices();
    FeatureMap mapFeatures = m_bioDevices->GetUserFeatures(m_uid);
    qDebug() << m_uid << ",count:" << mapFeatures.size();

    for (auto pDeviceInfo : deviceList) {
        int nDevFeatureCount = 0;
        if (m_isInAuth && m_curDevInfo && m_curDevInfo->device_id == pDeviceInfo->device_id) {
            nDevFeatureCount = 1;
        } else {
            if (m_uid == -1) {
                nDevFeatureCount = 0;
            } else if (mapFeatures.contains(pDeviceInfo->device_shortname)) {
                nDevFeatureCount = mapFeatures[pDeviceInfo->device_shortname].size();
            }
        }
        qDebug() << *pDeviceInfo << ",FeatureCount:" << nDevFeatureCount;
        if (nDevFeatureCount > 0) {
            if (!isAuthEnable && pDeviceInfo->biotype <= BIOTYPE_VOICEPRINT)
                continue;
            if (!isQRCodeEnable && pDeviceInfo->biotype == REMOTE_QRCODE_TYPE)
                continue;
            int nDevType = LOGINOPT_TYPE_OTHERS;
            nDevType = convertDeviceType(pDeviceInfo->biotype);
            if ((pDeviceInfo->biotype == REMOTE_QRCODE_TYPE || listDefDevices.contains(pDeviceInfo->device_shortname))
                && !m_mapDevices.contains(nDevType)) {
                m_mapDevices[nDevType].push_back(pDeviceInfo);
            }
        }
    }
    updateOptionButtons();
    updateUIStatus(false);
    // Q_EMIT notifyOptionsChange(m_mapDevices.size());
}

void LoginOptionsWidget::startAuth(DeviceInfoPtr device, int uid)
{
    if (!device || !m_biomericProxy || is_Lock) {
        qWarning() << "BiometricProxy doesn't exist.";
        return;
    }

    if (m_isInAuth) {
        qDebug() << "Identification is currently under way, stop it";
        stopAuth();
    }
    qDebug() << "deviceInfo:" << device->device_id;
    this->m_curDevInfo = device;
    this->m_uid = uid;
    struct passwd *pwdInfo = getpwuid(uid);
    if (pwdInfo) {
        this->m_strUserName = pwdInfo->pw_name;
    }
    this->m_isStopped = false;
    this->m_curLoginOptType = convertDeviceType(this->m_curDevInfo->biotype);
    updateUIStatus(true);
    if (pwdInfo) {
        m_bioDevices->SetLastDevice(pwdInfo->pw_name, this->m_curDevInfo->device_id);
    }
    startAuth_();
}

void LoginOptionsWidget::setSelectedPassword()
{
    if (m_mapOptBtns.contains(-1)) {
        QToolButton *btn = m_mapOptBtns[-1];
        if (btn && btn->isVisible()) {
            btn->setChecked(true);
        }
        m_curDevInfo = nullptr;
    }
}

void LoginOptionsWidget::startAuth_()
{
    if (!m_curDevInfo)
        return;
    qDebug().noquote() << QString("Identify:[drvid: %1, uid: %2]").arg(m_curDevInfo->device_id).arg(m_uid);

    m_isInAuth = true;
    m_dupFD = -1;
    if (!w_timer) {
        w_timer = new QTimer(this);
        w_timer->setInterval(150);
        connect(w_timer, &QTimer::timeout, this, &LoginOptionsWidget::updatePixmap);
    }
    m_waitingPixmap = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(24, 24);
    if (m_curLoginOptType == LOGINOPT_TYPE_QRCODE) {
        m_labelQRCodeTip->setPixmap(m_waitingPixmap);
        m_labelQRCodeTip->show();
        m_labelQRCodeMsg->hide();
    } else if (m_curLoginOptType == LOGINOPT_TYPE_FACE) {
        m_labelFace->setStyleSheet(
            QString("border-radius: %1px;  border:0px solid white;background-color: rgba(230,230,230,0.39);").arg(77));
        m_labelFace->setPixmap(QPixmap(""));
        m_labelFaceLoad->setPixmap(m_waitingPixmap);
        m_labelFaceLoad->show();
    }
    w_timer->start();
    m_bioDevices->recordAuthDrive("polkit", m_curDevInfo->device_id, true);
    m_biomericProxy->startAuth(m_uid, m_curDevInfo);
}

void LoginOptionsWidget::stopAuth()
{
    m_isStopped = true;
    if (!m_isInAuth || !m_curDevInfo) {
        return;
    }
    m_biomericProxy->stopAuth();
    if (m_retrytimer && m_retrytimer->isActive()) {
        m_retrytimer->stop();
        delete m_retrytimer;
        m_retrytimer = nullptr;
    }
    m_isInAuth = false;
    Q_EMIT updateImage(QImage());
    QImage img;
    setFaceImg(img);
}

void LoginOptionsWidget::updatePixmap()
{
    QMatrix matrix;
    matrix.rotate(90.0);
    m_waitingPixmap = m_waitingPixmap.transformed(matrix, Qt::FastTransformation);
    if (m_curLoginOptType == LOGINOPT_TYPE_QRCODE)
        m_labelQRCodeTip->setPixmap(m_waitingPixmap);
    else if (m_curLoginOptType == LOGINOPT_TYPE_FACE) {
        m_labelFaceLoad->setPixmap(m_waitingPixmap);
    }
}

void LoginOptionsWidget::onAuthFinished()
{
    if (m_bioDevices) {
        m_bioDevices->recordAuthDrive("polkit", 0, false);
    }
}

void LoginOptionsWidget::onIdentifyComplete(int uid, bool ret, int retErrNo)
{
    if (m_isStopped == true)
        return;

    if (uid < 0) {
        readDevicesInfo();
        Q_EMIT authComplete(uid, false, -1);
        return;
    }
    qDebug() << retErrNo << uid << m_uid;

    // 特征识别成功，而且用户id匹配
    if (retErrNo == DBUS_RESULT_SUCCESS && uid == m_uid) {
        qDebug() << "Identify success";
        Q_EMIT authComplete(uid, true, 0);
    } else if (retErrNo == -1) { // 特征识别不匹配
        qDebug() << "Identify failed";
        Q_EMIT authComplete(uid, false, 2);
    } else if (retErrNo == -2) { // 识别发生错误
        StatusReslut ret = m_bioDevices->UpdateStatus(m_curDevInfo->device_id);
        qDebug() << "StatusReslut:" << ret.result << "," << ret.enable << "," << ret.devNum << "," << ret.devStatus
                 << "," << ret.opsStatus << "," << ret.notifyMessageId;
        // 识别操作超时
        if (ret.result == 0 && ret.opsStatus == 10) { // 10 当前时间错误导致网络错误
            Q_EMIT authComplete(uid, false, -4);
        } else if (
            ret.result == 0
            && (ret.opsStatus == 404 || ret.opsStatus == 304 || ret.opsStatus == 8)) { // 304认证超时， 8网络异常
            Q_EMIT authComplete(uid, false, 1);
        } else if (ret.opsStatus == OPS_IDENTIFY_STOP_BY_USER || ret.opsStatus == OPS_VERIFY_STOP_BY_USER) {
            Q_EMIT authComplete(uid, false, -2); // 主动停止，直接重试
        } else if (
            ret.opsStatus == OPS_OPEN_FAIL
            || ret.opsStatus == OPS_OPEN_ERROR) { // 无法打开设备（设备是坏的/被占用），直接禁用
            Q_EMIT authComplete(uid, false, 5);
        } else {
            Q_EMIT authComplete(uid, false, 2);
        }
    } else {
        Q_EMIT authComplete(uid, false, 2);
    }
}

QPixmap LoginOptionsWidget::PixmapToRound(const QPixmap &src, int radius)
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

/**
 * @brief scaledPixmap
 * @param width
 * @param height
 * @param url
 * @return
 * 图片缩放
 */
QPixmap LoginOptionsWidget::scaledPixmap(int width, int height, QString url)
{
    QFile imgFile(url);
    if (!imgFile.exists()) {
        qDebug() << "not find the pixmap file";
        return QPixmap();
    }
    QImage image(url);
    QPixmap pixmap = QPixmap::fromImage(image);
    if (pixmap.isNull()) {
        qDebug() << "pixmap is null";
        QProcess exec;
        QString program("file " + url);
        exec.start(program);
        exec.waitForFinished(1000);
        QString output = exec.readAllStandardOutput();
        qDebug() << output;
        if (output.contains("SVG")) {
            qDebug() << "image format is SVG";
            QSvgRenderer render(url);
            QImage image(width, height, QImage::Format_ARGB32);
            image.fill(Qt::transparent);
            QPainter painter(&image);
            render.render(&painter, image.rect());
            pixmap.convertFromImage(image);
        } else if (output.contains("TIFF")) {
            qDebug() << "image format is TIFF";
        }
    }

    return pixmap.scaled(width, height, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
}

void LoginOptionsWidget::onFrameWritten(int drvid)
{
    if (!m_curDevInfo)
        return;

    if (m_curDevInfo && m_curDevInfo->device_id != drvid || !m_isInAuth) {
        return;
    }

    if (m_dupFD == -1) {
        m_dupFD = get_server_gvariant_stdout(drvid);
    }

    if (m_dupFD <= 0)
        return;

    lseek(m_dupFD, 0, SEEK_SET);
    char base64_bufferData[1024 * 1024];
    int rc = read(m_dupFD, base64_bufferData, 1024 * 1024);
    Q_UNUSED(rc);
    // printf("rc = %d\n", rc);

    if (rc <= 0) {
        return;
    }

    QByteArray imageData(base64_bufferData, rc);
    QBuffer imgBuffer(&imageData);
    imgBuffer.open(QIODevice::ReadOnly);

    QImageReader imgReader(&imgBuffer);
    QImage srcQImage = imgReader.read();
    if (srcQImage.isNull()) {
        return;
    }
    srcQImage = srcQImage.convertToFormat(QImage::Format_RGB888);

    if (isAuthenticating()) {
        if (m_curLoginOptType == LOGINOPT_TYPE_FACE) {
            m_labelFace->setStyleSheet(
                QString("border-radius: %1px;  border:0px solid white;background-color: rgba(255,255,255,10%);")
                    .arg(77));
            setFaceImg(srcQImage);
            m_labelFace->show();
            m_labelQRCode->hide();
        } else if (m_curLoginOptType == LOGINOPT_TYPE_QRCODE) {
            m_labelQRCode->setStyleSheet(
                QString("border-radius: %1px;  border:0px solid white;background-color: rgba(255,255,255,100%);")
                    .arg(6));
            setQRCode(srcQImage);
            m_labelQRCode->show();
            m_labelQRCodeTip->hide();
            m_labelFace->hide();
        } else {
            m_labelFace->hide();
            m_labelQRCode->hide();
        }
    } else {
        m_labelFace->hide();
        m_labelQRCode->hide();
    }
    if (w_timer && w_timer->isActive()) {
        w_timer->stop();
    }
    Q_EMIT updateImage(srcQImage);
}

void LoginOptionsWidget::setQRCode(QImage &imgQRCode)
{
    if (imgQRCode.isNull()) {
        imgQRCode.load(QString("%1/images/ukui-qrcode-null.svg").arg(GET_STR(UKUI_BIOMETRIC)));
    }
    imgQRCode = imgQRCode.scaled(QSize(150, 150));
    m_labelQRCode->setAlignment(Qt::AlignCenter);
    m_labelQRCode->setPixmap(QPixmap::fromImage(imgQRCode));
}

void LoginOptionsWidget::setQRCodeMsg(QString strMsg)
{
    if (strMsg.isEmpty()) {
        m_labelQRCodeMsg->hide();
        // m_labelQRCodeTip->hide();
    } else {
        // 一开始认证时就没有网，直接停止加载状态
        if (w_timer && w_timer->isActive()) {
            w_timer->stop();
        }
        m_labelQRCodeMsg->setText(strMsg);
        m_labelQRCodeMsg->show();
        m_labelQRCodeTip->show();
        m_labelQRCodeTip->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(22, 22)));
    }
}

void LoginOptionsWidget::setFaceImg(QImage &imgFace, int nStatus)
{
    QPixmap faceImage;
    m_labelFace->setFixedSize(154, 154);
    // 如果头像文件不存在，则使用默认头像
    if (!imgFace.isNull()) {
        faceImage = PixmapToRound(QPixmap::fromImage(imgFace), 77);
    } else {
        switch (nStatus) {
            case 1:
                faceImage
                    = loadSvg(QString("%1/images/ukui-loginopt-lose.svg").arg(GET_STR(UKUI_BIOMETRIC)), "black", 48);
                m_labelFace->setStyleSheet(
                    QString("border-radius: %1px;  border:0px solid white;background-color: rgba(230,230,230,40%);")
                        .arg(77));
                break;
            case 2:
                faceImage
                    = loadSvg(QString("%1/images/ukui-loginopt-smile.svg").arg(GET_STR(UKUI_BIOMETRIC)), "gray", 48);
                m_labelFace->setStyleSheet(
                    QString("border-radius: %1px;  border:0px solid white;background-color: rgba(230,230,230,100%);")
                        .arg(77));
                break;
            default:
                faceImage
                    = loadSvg(QString("%1/images/ukui-loginopt-smile.svg").arg(GET_STR(UKUI_BIOMETRIC)), "black", 48);
                m_labelFace->setStyleSheet(
                    QString("border-radius: %1px;  border:0px solid white;background-color: rgba(230,230,230,40%);")
                        .arg(77));
                break;
        }
    }

    m_labelFace->setAlignment(Qt::AlignCenter);
    m_labelFace->setPixmap(faceImage);
    m_labelFaceLoad->hide();
}

void LoginOptionsWidget::onStatusChanged(int drvid, const QString &message)
{
    if (!m_isInAuth || !m_curDevInfo) {
        return;
    }
    if (drvid != m_curDevInfo->device_id) {
        return;
    }
    // Q_EMIT updateAuthMsg(message);
}

void LoginOptionsWidget::setCurrentDevice(int drvid)
{
    DeviceInfoPtr pDeviceInfo = findDeviceById(drvid);
    if (pDeviceInfo) {
        setCurrentDevice(pDeviceInfo);
    }
}

void LoginOptionsWidget::setCurrentDevice(const QString &deviceName)
{
    DeviceInfoPtr pDeviceInfo = findDeviceByName(deviceName);
    if (pDeviceInfo) {
        setCurrentDevice(pDeviceInfo);
    }
}

void LoginOptionsWidget::setCurrentDevice(const DeviceInfoPtr &pDeviceInfo)
{
    this->m_curDevInfo = pDeviceInfo;
}

DeviceInfoPtr LoginOptionsWidget::findDeviceById(int drvid)
{
    for (int type : m_mapDevices.keys()) {
        DeviceList &deviceList = m_mapDevices[type];
        auto iter = std::find_if(
            deviceList.begin(), deviceList.end(), [&](DeviceInfoPtr ptr) { return ptr->device_id == drvid; });
        if (iter != deviceList.end()) {
            return *iter;
        }
    }
    return DeviceInfoPtr();
}

DeviceInfoPtr LoginOptionsWidget::findDeviceByName(const QString &name)
{
    for (int type : m_mapDevices.keys()) {
        DeviceList &deviceList = m_mapDevices[type];
        auto iter = std::find_if(
            deviceList.begin(), deviceList.end(), [&](DeviceInfoPtr ptr) { return ptr->device_shortname == name; });
        if (iter != deviceList.end()) {
            return *iter;
        }
    }
    return DeviceInfoPtr();
}

QString LoginOptionsWidget::GetDefaultDevice(uid_t uid, int bioType)
{
    QString defaultDeviceName = "";
    DeviceInfoPtr pDeviceInfo = m_bioDevices->getDefaultDevice(uid, bioType);
    if (pDeviceInfo) {
        defaultDeviceName = pDeviceInfo->device_shortname;
    }
    qDebug() << "default device: " << defaultDeviceName;

    return defaultDeviceName;
}

int LoginOptionsWidget::getDefaultAuthType(QString strUserName)
{
    int defaultAuthType = -1;
    if (m_uniAuthService && m_uniAuthService->isActivatable()) {
        defaultAuthType = m_uniAuthService->getDefaultAuthType(strUserName);
    }
    return defaultAuthType;
}

QString LoginOptionsWidget::GetDefaultDevice(uid_t uid)
{
    QString defaultDeviceName = "";
    DeviceInfoPtr pDeviceInfo = m_bioDevices->getDefaultDevice(uid);
    if (pDeviceInfo) {
        defaultDeviceName = pDeviceInfo->device_shortname;
    }
    qDebug() << "default device: " << defaultDeviceName;

    return defaultDeviceName;
}

void LoginOptionsWidget::onUSBDeviceCountChange(int newNum)
{
    qDebug() << "deviceNewNum:" << newNum;
    int savedCount = 0;
    for (int type : m_mapDevices.keys())
        savedCount += m_mapDevices.value(type).count();

    readDevicesInfo();

    int count = 0;
    for (int type : m_mapDevices.keys())
        count += m_mapDevices.value(type).count();

    // 设备数量发生了变化
    if (count != savedCount) {
        updateOptionButtons();
        Q_EMIT notifyOptionsChange(count);
    }
    updateUIStatus(false);
}

void LoginOptionsWidget::updateUkeyUIStatus(int type)
{
    m_labelFace->hide();
    m_labelQRCode->hide();
    m_widgetImage->hide();
    m_labelOptTitle->adjustSize();
    int containerMinHeight = m_labelOptTitle->sizeHint().height() + 8 + 36;
    m_widgetOptContainer->setMinimumHeight(containerMinHeight);
    setFixedHeight(containerMinHeight + 30);

    Q_EMIT this->updateWndSize(this->m_curLoginOptType, this->m_mapOptBtns.size());
}

void LoginOptionsWidget::updateUIStatus(bool update, int nPreOptType /* = -1*/)
{
    if (m_mapOptBtns.contains(-1)) {
        QToolButton *btn = m_mapOptBtns[-1];
        if (btn && btn->isVisible()) {
            btn->setChecked(true);
        }
    }
    if (m_curDevInfo) {
        if (m_mapOptBtns.contains(m_curDevInfo->device_id)) {
            QToolButton *btn = m_mapOptBtns[m_curDevInfo->device_id];
            if (btn) {
                btn->setChecked(true);
            }
        }
    } else if (nPreOptType == LOGINOPT_TYPE_GENERAL_UKEY) {
	//ukey已绑定但是没有接入设备时,因为当前设备为空，为了避免选中密码而加此处理
        if (m_mapOptBtns.contains(-2)) {
            QToolButton *btn = m_mapOptBtns[-2];
            if (btn) {
                btn->setChecked(true);
            }
        }
    }

    if (update) {
        bool isSameType = false;
        if (nPreOptType != -1) {
            isSameType = (m_curLoginOptType == nPreOptType);
            m_curLoginOptType = nPreOptType;
        }
        // 计算容器的最小高度（label高度 + 间距8 + 按钮高度36），确保间距始终为8
        m_labelOptTitle->adjustSize();
        int containerMinHeight = m_labelOptTitle->sizeHint().height() + 8 + 36;
        m_widgetOptContainer->setMinimumHeight(containerMinHeight);
        
        if (m_curLoginOptType == LOGINOPT_TYPE_FACE) {
            m_labelFace->setStyleSheet(
                QString("border-radius: %1px;  border:0px solid white;background-color: rgba(255,255,255,10%);")
                    .arg(77));
            m_labelFace->show();
            m_labelQRCode->hide();
            m_widgetImage->show();
            setFixedHeight(containerMinHeight + 12 + 154 + 48);
        } else if (m_curLoginOptType == LOGINOPT_TYPE_QRCODE) {
            if (!isSameType) {
                m_labelQRCode->setStyleSheet(
                    QString("border-radius: %1px;  border:0px solid white;background-color: rgba(255,255,255,100%);")
                        .arg(6));
                QImage img;
                setQRCode(img);
                m_labelQRCode->show();
                m_labelFace->hide();
                m_widgetImage->show();
            }
            setFixedHeight(containerMinHeight + 12 + 154 + 48);
        } else {
            m_labelFace->hide();
            m_labelQRCode->hide();
            if (m_curLoginOptType == LOGINOPT_TYPE_FINGERPRINT || m_curLoginOptType == LOGINOPT_TYPE_FINGERVEIN
                || m_curLoginOptType == LOGINOPT_TYPE_IRIS || m_curLoginOptType == LOGINOPT_TYPE_VOICEPRINT) {
                m_widgetImage->show();
                setFixedHeight(containerMinHeight + 12 + 154 + 48);
            } else {
                // 密码选项和其他选项：容器最小高度 + 布局底部边距30
                m_widgetImage->hide();
                setFixedHeight(containerMinHeight + 30);
            }
        }
        Q_EMIT this->updateWndSize(this->m_curLoginOptType, this->m_mapOptBtns.size());
    }
}

void LoginOptionsWidget::onOptionSelected(int nIndex)
{
    if (nIndex < 0)
        return;
    if (nIndex < m_listDriveId.size()) {
        DeviceInfoPtr info = findDeviceById(m_listDriveId[nIndex]);
        if (info && !isDeviceDisable(info->device_id)) {
            Q_EMIT optionSelected(convertDeviceType(info->biotype), info);
            this->m_curLoginOptType = convertDeviceType(info->biotype);
        } else if (nIndex == 0 && m_listDriveId[nIndex] == -1) {
            stopAuth();
            m_curDevInfo = nullptr;
            Q_EMIT optionSelected(LOGINOPT_TYPE_PASSWORD, nullptr);
            this->m_curLoginOptType = LOGINOPT_TYPE_PASSWORD;

        } else if (m_listDriveId[nIndex] == -2) {
            // 存在ukey特征，但未插入ukey
            stopAuth();
            m_curDevInfo = nullptr;
            Q_EMIT optionSelected(LOGINOPT_TYPE_GENERAL_UKEY, nullptr);
            this->m_curLoginOptType = LOGINOPT_TYPE_GENERAL_UKEY;
        }
    }
}

int LoginOptionsWidget::convertDeviceType(int nDevType)
{
    int nLoginOptType = LOGINOPT_TYPE_OTHERS;
    switch (nDevType) {
        case BIOTYPE_FINGERPRINT:
            nLoginOptType = LOGINOPT_TYPE_FINGERPRINT;
            break;
        case BIOTYPE_FINGERVEIN:
            nLoginOptType = LOGINOPT_TYPE_FINGERVEIN;
            break;
        case BIOTYPE_IRIS:
            nLoginOptType = LOGINOPT_TYPE_IRIS;
            break;
        case BIOTYPE_FACE:
            nLoginOptType = LOGINOPT_TYPE_FACE;
            break;
        case BIOTYPE_VOICEPRINT:
            nLoginOptType = LOGINOPT_TYPE_VOICEPRINT;
            break;
        case UniT_General_Ukey:
            nLoginOptType = LOGINOPT_TYPE_GENERAL_UKEY;
            break;
        case UniT_Advanced_Ukey:
            nLoginOptType = LOGINOPT_TYPE_ADVANCED_UKEY;
            break;
        case REMOTE_QRCODE_TYPE:
            nLoginOptType = LOGINOPT_TYPE_QRCODE;
            break;
        default:
            nLoginOptType = LOGINOPT_TYPE_OTHERS;
            break;
    }
    return nLoginOptType;
}

void LoginOptionsWidget::setDeviceDisable(int nDevId, bool bDisable)
{
    if (bDisable) {
        m_mapDisableDev[m_uid][nDevId] = true;
        QMap<int, QToolButton *>::iterator itMapBtn = m_mapOptBtns.begin();
        for (; itMapBtn != m_mapOptBtns.end(); itMapBtn++) {
            if (itMapBtn.key() == nDevId && itMapBtn.value()) {
                itMapBtn.value()->setDisabled(true);
                break;
            }
        }
    } else {
        m_mapDisableDev[m_uid][nDevId] = false;
        QMap<int, QToolButton *>::iterator itMapBtn = m_mapOptBtns.begin();
        for (; itMapBtn != m_mapOptBtns.end(); itMapBtn++) {
            if (itMapBtn.key() == nDevId && itMapBtn.value()) {
                itMapBtn.value()->setDisabled(false);
                break;
            }
        }
    }
}

void LoginOptionsWidget::setAllDeviceDisable(bool bDisable)
{
    if (bDisable) {
        QMap<int, QToolButton *>::iterator itMapBtn = m_mapOptBtns.begin();
        for (; itMapBtn != m_mapOptBtns.end(); itMapBtn++) {

            m_mapDisableDev[m_uid][itMapBtn.key()] = true;
            if (itMapBtn.value()) {
                itMapBtn.value()->setDisabled(true);
            }
        }
    } else {
        QMap<int, QToolButton *>::iterator itMapBtn = m_mapOptBtns.begin();
        for (; itMapBtn != m_mapOptBtns.end(); itMapBtn++) {
            m_mapDisableDev[m_uid][itMapBtn.key()] = false;
            if (itMapBtn.value()) {
                itMapBtn.value()->setDisabled(false);
            }
        }
    }
}

bool LoginOptionsWidget::isDeviceDisable(int nDevId)
{
    if (m_mapDisableDev[m_uid].contains(nDevId)) {
        return m_mapDisableDev[m_uid][nDevId];
    }
    return false;
}

void LoginOptionsWidget::lockStatusChanged(bool locked)
{
    if (locked) {
        is_Lock = true;
        stopAuth();
    } else {
        is_Lock = false;
        readDevicesInfo();
    }
}

QPixmap LoginOptionsWidget::loadSvg(QString path, QString color, int size)
{
    int origSize = size;
    const auto ratio = qApp->devicePixelRatio();
    if (2 == ratio) {
        size += origSize;
    } else if (3 == ratio) {
        size += origSize;
    }
    QPixmap pixmap(size, size);
    QSvgRenderer renderer(path);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);
    return drawSymbolicColoredPixmap(pixmap, color);
}

QPixmap LoginOptionsWidget::drawSymbolicColoredPixmap(QPixmap &source, QString cgColor)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if ("white" == cgColor) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                } else if ("black" == cgColor) {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                } else if ("gray" == cgColor) {
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                } else if ("blue" == cgColor) {
                    color.setRed(61);
                    color.setGreen(107);
                    color.setBlue(229);
                    img.setPixelColor(x, y, color);
                } else {
                    return source;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

QPixmap LoginOptionsWidget::scaledSmoothPixmap(const QPixmap &src, int nWidth, int nHeight)
{
    if (src.isNull()) {
        return QPixmap();
    }

    QPixmap pixmapa(src);
    QPixmap pixmap(nWidth, nHeight);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.drawPixmap(0, 0, nWidth, nHeight, pixmapa);
    return pixmap;
}

void LoginOptionsWidget::startBioWaiting(unsigned uCurLoginOptType)
{
    if (uCurLoginOptType == m_nCurBioWaitingType
        || (uCurLoginOptType != LOGINOPT_TYPE_FINGERPRINT && uCurLoginOptType != LOGINOPT_TYPE_FINGERVEIN
            && uCurLoginOptType != LOGINOPT_TYPE_IRIS && uCurLoginOptType != LOGINOPT_TYPE_VOICEPRINT)) {
        return;
    }
    m_nCurBioWaitingType = uCurLoginOptType;
    if (!m_timerBioWaiting) {
        m_timerBioWaiting = new QTimer(this);
        m_timerBioWaiting->setInterval(100);
    }
    if (m_timerBioWaiting) {
        if (m_timerBioWaiting->isActive()) {
            m_timerBioWaiting->stop();
        }
        // 重新连接信号槽
        disconnect(m_timerBioWaiting, nullptr, this, nullptr);
        connect(m_timerBioWaiting, &QTimer::timeout, this, [&, this, uCurLoginOptType]() {
            m_nBioWaitingFrame += 1;
            QString strPicName = "";
            QString strBasePath = ":/image/assets/biowaiting";
            if (m_strThemeName == "ukui-dark" || m_strThemeName == "ukui-black") {
                strBasePath = ":/image/assets/biowaiting/dark";
            }
            switch (uCurLoginOptType) {
                case LOGINOPT_TYPE_FINGERPRINT: {
                    if (m_nBioWaitingFrame >= 15) {
                        m_nBioWaitingFrame = 0;
                    }
                    strPicName = QString("%1/fingerprint%2.svg").arg(strBasePath).arg(m_nBioWaitingFrame);
                } break;
                case LOGINOPT_TYPE_FINGERVEIN: {
                    if (m_nBioWaitingFrame >= 12) {
                        m_nBioWaitingFrame = 0;
                    }
                    strPicName = QString("%1/fingervein%2.svg").arg(strBasePath).arg(m_nBioWaitingFrame);
                } break;
                case LOGINOPT_TYPE_IRIS: {
                    if (m_nBioWaitingFrame >= 12) {
                        m_nBioWaitingFrame = 0;
                    }
                    strPicName = QString("%1/iris%2.svg").arg(strBasePath).arg(m_nBioWaitingFrame);
                } break;
                case LOGINOPT_TYPE_VOICEPRINT: {
                    if (m_nBioWaitingFrame >= 8) {
                        m_nBioWaitingFrame = 0;
                    }
                    strPicName = QString("%1/voiceprint%2.svg").arg(strBasePath).arg(m_nBioWaitingFrame);
                } break;
                default:
                    break;
            }
            if (!strPicName.isEmpty()) {
                QPixmap pixmap;
                if (QPixmapCache::find(strPicName, &pixmap)) {
                    m_labelBioWaiting->setPixmap(pixmap);
                } else {
                    pixmap = scaledSmoothPixmap(QPixmap(strPicName), 154, 154);
                    if (!pixmap.isNull()) {
                        QPixmapCache::insert(strPicName, pixmap);
                        m_labelBioWaiting->setPixmap(pixmap);
                    }
                }
                m_labelBioWaiting->show();
            }
        });
        m_timerBioWaiting->start();
        m_nBioWaitingFrame = 0;
        QString strPicName = "";
        QString strBasePath = ":/image/assets/biowaiting";
        if (m_strThemeName == "ukui-dark" || m_strThemeName == "ukui-black") {
            strBasePath = ":/image/assets/biowaiting/dark";
        }
        switch (uCurLoginOptType) {
            case LOGINOPT_TYPE_FINGERPRINT: {
                strPicName = QString("%1/fingerprint%2.svg").arg(strBasePath).arg(m_nBioWaitingFrame);
            } break;
            case LOGINOPT_TYPE_FINGERVEIN: {
                strPicName = QString("%1/fingervein%2.svg").arg(strBasePath).arg(m_nBioWaitingFrame);
            } break;
            case LOGINOPT_TYPE_IRIS: {
                strPicName = QString("%1/iris%2.svg").arg(strBasePath).arg(m_nBioWaitingFrame);
            } break;
            case LOGINOPT_TYPE_VOICEPRINT: {
                strPicName = QString("%1/voiceprint%2.svg").arg(strBasePath).arg(m_nBioWaitingFrame);
            } break;
            default:
                break;
        }
        if (!strPicName.isEmpty()) {
            QPixmap pixmap;
            if (QPixmapCache::find(strPicName, &pixmap)) {
                m_labelBioWaiting->setPixmap(pixmap);
            } else {
                pixmap = scaledSmoothPixmap(QPixmap(strPicName), 154, 154);
                if (!pixmap.isNull()) {
                    QPixmapCache::insert(strPicName, pixmap);
                    m_labelBioWaiting->setPixmap(pixmap);
                }
            }
            m_labelBioWaiting->show();
        }
    }
}

void LoginOptionsWidget::stopBioWaiting(bool isDisable /* = false*/)
{
    if (m_timerBioWaiting) {
        if (m_timerBioWaiting->isActive()) {
            m_timerBioWaiting->stop();
        }
    }
    m_nBioWaitingFrame = 0;
    if (isDisable) {
        QString strPicName = "";
        QString strBasePath = ":/image/assets/biowaiting";
        if (m_strThemeName == "ukui-dark" || m_strThemeName == "ukui-black") {
            strBasePath = ":/image/assets/biowaiting/dark";
        }
        switch (m_nCurBioWaitingType) {
            case LOGINOPT_TYPE_FINGERPRINT: {
                strPicName = QString("%1/fingerprint%2.svg").arg(strBasePath).arg(m_nBioWaitingFrame);
            } break;
            case LOGINOPT_TYPE_FINGERVEIN: {
                strPicName = QString("%1/fingervein%2.svg").arg(strBasePath).arg(m_nBioWaitingFrame);
            } break;
            case LOGINOPT_TYPE_IRIS: {
                strPicName = QString("%1/iris%2.svg").arg(strBasePath).arg(m_nBioWaitingFrame);
            } break;
            case LOGINOPT_TYPE_VOICEPRINT: {
                strPicName = QString("%1/voiceprint%2.svg").arg(strBasePath).arg(m_nBioWaitingFrame);
            } break;
            default:
                break;
        }
        if (!strPicName.isEmpty()) {
            QPixmap pixmap;
            if (QPixmapCache::find(strPicName, &pixmap)) {
                m_labelBioWaiting->setPixmap(pixmap);
            } else {
                pixmap = scaledSmoothPixmap(QPixmap(strPicName), 154, 154);
                if (!pixmap.isNull()) {
                    QPixmapCache::insert(strPicName, pixmap);
                    m_labelBioWaiting->setPixmap(pixmap);
                }
            }
        }
    } else {
        m_labelBioWaiting->hide();
    }
    m_nCurBioWaitingType = -1;
}
