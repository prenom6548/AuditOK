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

#include <opencv2/opencv.hpp>
#include "giodbus.h"
#include "biometricdeviceinfo.h"
#include "klabel.h"
#include "common/configuration.h"
#include "imageutil.h"
#include "pluginsloader.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QPushButton>
#include <QTimer>
#include <QPainter>
#include <QtSvg/QSvgRenderer>
#include <QImage>
#include <QApplication>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>

#define ON_TAB_SHEET "QPushButton{background-color: rgba(255,255,255,15%); border-radius: 4px; border: 2px solid #2C73C8;}"
#define ON_NORMAL_SHEET "QPushButton{background-color: rgba(255,255,255,15%); border-radius: 4px; border: none;}\
                         QPushButton::hover{ background-color: rgb(255,255,255,40%);}\
                         QPushButton::pressed {background-color: rgba(255,255,255,40%);}\
                         QPushButton::checked {background-color: rgba(255, 255, 255, 40%);}"

LoginOptionsWidget::LoginOptionsWidget(BiometricProxy* proxy, int uid, UniAuthService* uniauthService, QWidget *parent)
    : QWidget(parent)
    , m_biomericProxy(proxy)
    , m_uid(uid)
    , m_uniauthService(uniauthService)
    , m_configuration(Configuration::instance())
{
    m_listPriority.clear();
    m_listPriority.push_back(BioT_Face);
    m_listPriority.push_back(BioT_FingerPrint);
    m_listPriority.push_back(BioT_Iris);
    m_listPriority.push_back(BioT_VoicePrint);
    m_listPriority.push_back(BioT_FingerVein);
    m_listPriority.push_back(REMOTE_QRCODE_TYPE);
    m_listPriority.push_back(UniT_General_Ukey);
    initUI();
    initConnections();
    m_mapDisableDev.clear();
}

LoginOptionsWidget::~LoginOptionsWidget()
{

}

void LoginOptionsWidget::initUI()
{
    this->setFixedHeight(94);
    // 初始化UI
    m_layoutMain = new QVBoxLayout();
    m_layoutMain->setContentsMargins(0,0,0,0);
    m_layoutMain->setSpacing(16);
    m_layoutOptBtns = new QHBoxLayout();
    m_layoutOptBtns->setContentsMargins(0,0,0,0);
    m_layoutOptBtns->setSpacing(16);

    m_labelOptTitle = new KLabel();
    m_configuration->getCurrentUser(getpwuid(m_uid)->pw_name);
    curFontSize = m_configuration->getUserConfig("fontSize").toDouble();
    m_labelOptTitle->setFontSize(16 + curFontSize);
    m_labelOptTitle->setAlignment(Qt::AlignCenter);
    m_labelOptTitle->setText(tr("Login Options"));
    m_labelOptTitle->setFixedHeight(30);
    m_layoutMain->addWidget(m_labelOptTitle);

    m_btnGroup = new QButtonGroup(this);
    m_btnGroup->setExclusive(true);

    m_layoutOptBtns->setAlignment(Qt::AlignCenter);
    m_layoutMain->addLayout(m_layoutOptBtns);

    m_layoutMain->addStretch();
    this->setLayout(m_layoutMain);
}

void LoginOptionsWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange){
        refreshTranslate();
    }
}

void LoginOptionsWidget::refreshTranslate()
{
    m_labelOptTitle->setText(tr("Login Options"));
}

void LoginOptionsWidget::initConnections()
{
    if(m_biomericProxy && m_biomericProxy->isValid())
    {
        connect(m_biomericProxy, &BiometricProxy::StatusChanged,
                this, &LoginOptionsWidget::onStatusChanged);

        connect(m_biomericProxy, &BiometricProxy::FrameWritten,
                this, &LoginOptionsWidget::onFrameWritten);
        connect(m_biomericProxy, &BiometricProxy::USBDeviceHotPlug,
                this, &LoginOptionsWidget::onUSBDeviceHotPlug);
    }
    connect(m_btnGroup, SIGNAL(buttonClicked(int)), this, SLOT(onOptionSelected(int)));
}

bool LoginOptionsWidget::getCurLoginOpt(int& nLoginOptType, int& nDrvId)
{
    if (m_curDevInfo) {
        nLoginOptType = convertDeviceType(m_curDevInfo->deviceType);
        nDrvId = m_curDevInfo->id;
        return true;
    }
    return false;
}

unsigned LoginOptionsWidget::getLoginOptCount()
{
    return m_mapDevices.size();
}

int LoginOptionsWidget::getVisibleLoginOptCount()
{
    int nCount = 0;
    QMap<int, QPushButton*>::iterator itMapBtn = m_mapOptBtns.begin();
    for ( ; itMapBtn != m_mapOptBtns.end(); itMapBtn++) {
        if (itMapBtn.value() && itMapBtn.value()->isVisible()) {
            nCount ++;
        }
    }
    return nCount;
}

DeviceInfoPtr LoginOptionsWidget::getFirstDevInfo()
{
    DeviceInfoPtr devInfo = nullptr;
    int nDrvId = GetLastDevice(getpwuid(m_uid)->pw_name);
    if (nDrvId >= 0) {
        qDebug()<<"GetLastDevice:"<<nDrvId;
        DeviceMap::iterator itDevInfo = m_mapDevices.begin();
        for (; itDevInfo != m_mapDevices.end(); itDevInfo++) {
            for (auto devinfo : itDevInfo.value()) {
                if (devinfo && devinfo->id == nDrvId && devinfo->deviceType != UniT_Custom) {
                    if (!isDeviceDisable(devinfo->id)) {
                        devInfo =  devinfo;
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
                if (devinfo && !isDeviceDisable(devinfo->id) && devinfo->deviceType != UniT_Custom) {
                    devInfo =  devinfo;
                    break;
                }
            }
            if (devInfo) {
                break;
            }
        }
    }
    return DeviceInfoPtr(devInfo);
}

DeviceInfoPtr LoginOptionsWidget::getWechatDevice()
{
    DeviceInfoPtr devInfo = nullptr;
    DeviceMap::iterator itDevInfo = m_mapDevices.begin();
    for (; itDevInfo != m_mapDevices.end(); itDevInfo++) {
        for (auto devinfo : itDevInfo.value()) {
            if (devinfo && devinfo->deviceType == REMOTE_QRCODE_TYPE) {
                if (!isDeviceDisable(devinfo->id)) {
                    devInfo =  devinfo;
                    break;
                }
            }
        }
        if (devInfo) {
            break;
        }
    }
    return DeviceInfoPtr(devInfo);
}

void LoginOptionsWidget::addOptionButton(unsigned uLoginOptType, int nDrvId, QString strDrvName)
{
    if (m_mapOptBtns.contains(nDrvId)) {
        return ;
    }
    QPushButton *newButton = new QPushButton();
//    QVBoxLayout *layoutBtn = new QVBoxLayout();
//    newButton->setLayout(layoutBtn);
//    QLabel *newLabel = new QLabel();
//    layoutBtn->setAlignment(Qt::AlignCenter);
//    layoutBtn->addWidget(newLabel);
    newButton->setCheckable(true);
    newButton->setChecked(false);
    newButton->setFocusPolicy(Qt::NoFocus);
//    if (nDrvId == -1) {
//        //newButton->setEnabled(false);
//        newButton->setChecked(true);
//        //屏蔽鼠标事件
//        newButton->setAttribute(Qt::WA_TransparentForMouseEvents, true);
//    } else {
       int nLength = m_btnGroup->buttons().length();
       m_btnGroup->addButton(newButton, nLength);
       m_listDriveId.append(nDrvId);
//    }
    QPixmap iconPixmap;
    switch (uLoginOptType) {
    case LOGINOPT_TYPE_PASSWORD:
        iconPixmap = loadSvg(":/images/ukui-loginopt-password.svg", "white", 16);
        break;
    case LOGINOPT_TYPE_GENERAL_UKEY:
        iconPixmap = loadSvg(":/images/ukui-loginopt-ukey.svg", "white", 16);
        break;
    case LOGINOPT_TYPE_FACE:
        iconPixmap = loadSvg(":/images/ukui-loginopt-face.svg", "white", 16);
        break;
    case LOGINOPT_TYPE_FINGERPRINT:
        iconPixmap = loadSvg(":/images/ukui-loginopt-finger.svg", "white", 16);
        break;
    case LOGINOPT_TYPE_IRIS:
        iconPixmap = loadSvg(":/images/ukui-loginopt-iris.svg", "white", 16);
        break;
    case LOGINOPT_TYPE_VOICEPRINT:
        iconPixmap = loadSvg(":/images/ukui-loginopt-voice.svg", "white", 16);
        break;
    case LOGINOPT_TYPE_FINGERVEIN:
        iconPixmap = loadSvg(":/images/ukui-loginopt-fingervein.svg", "white", 16);
        break;
    case LOGINOPT_TYPE_QRCODE:
        iconPixmap = loadSvg(":/images/ukui-loginopt-qrcode.svg", "white", 16);
        break;
    case LOGINOPT_TYPE_CUSTOM:
        {
            LoginAuthInterface *plugin = dynamic_cast<LoginAuthInterface *>(PluginsLoader::instance().findModulesByType(LoginPluginInterface::MODULETYPE_AUTH).values().first());
            QString strIcon = plugin->icon();
            if (strIcon.startsWith("/")) {
                if (strIcon.endsWith(".svg")) {
                    iconPixmap = loadSvg(strIcon, "white", 16);
                } else {
                    iconPixmap.load(strIcon);
                    iconPixmap.scaled(40, 40);
                }
            } else {
                iconPixmap = QIcon::fromTheme(strIcon).pixmap(48,48).scaled(40, 40);
            }
            if (iconPixmap.isNull()) {
                iconPixmap = loadSvg(":/images/ukui-loginopt-custom.svg", "white", 16);
            }
        }
        break;
    }
    newButton->setIcon(iconPixmap);
    //newLabel->setPixmap(iconPixmap);
    newButton->installEventFilter(this);
    newButton->setObjectName(strDrvName);
    newButton->setStyleSheet("QPushButton{text-align:center;background-color: rgba(255,255,255,15%);border: none;border-radius: 4px;outline: none;}"
                             "QPushButton::hover{background-color: rgba(255,255,255,40%);}"
                             "QPushButton::pressed {background-color: rgba(255,255,255,40%);}"
                             "QPushButton::checked {background-color: rgba(255,255,255,40%);}");
    nameList.append(strDrvName);
    newButton->setFixedSize(48, 48);
    if (isDeviceDisable(nDrvId)) {
        newButton->setDisabled(true);
    } else {
        newButton->setDisabled(false);
    }

    m_layoutOptBtns->addWidget(newButton);
    m_mapOptBtns[nDrvId] = newButton;
}

bool LoginOptionsWidget::eventFilter(QObject *obj, QEvent *event)
{
    for(int i = 0; i<nameList.count();i++){
        if(obj->objectName() == nameList.at(i)){
            if(event->type() == 110){
                Q_EMIT showKToolTipClicked(nameList.at(i));
            } else if (event->type() == 11){
                Q_EMIT hideKTooltipClicked();
            }
        }
    }
    return false;
}

bool LoginOptionsWidget::getHasUkeyOptions()
{
    return isShowUkey;
}

void LoginOptionsWidget::clearOptionButtons()
{
    QMap<int, QPushButton*>::iterator itMapBtn = m_mapOptBtns.begin();
    for ( ; itMapBtn != m_mapOptBtns.end(); itMapBtn++) {
        if (itMapBtn.value()) {
            m_btnGroup->removeButton(itMapBtn.value());
            m_layoutOptBtns->removeWidget(itMapBtn.value());
            itMapBtn.value()->deleteLater();
        }
    }
    m_mapOptBtns.clear();
    m_listDriveId.clear();
}

void LoginOptionsWidget::updateOptionButtons()
{
    isShowUkey = false;
    bool hasCustom = false;
    clearOptionButtons();
    addOptionButton(LOGINOPT_TYPE_PASSWORD, -1, tr("Password"));
    DeviceMap::iterator itMapDev = m_mapDevices.begin();
    for ( ; itMapDev != m_mapDevices.end(); itMapDev++) {
        for (DeviceInfoPtr devPtr : itMapDev.value()) {
            if (devPtr) {
                if(devPtr->deviceType == UniT_General_Ukey){
                    //ukey 设备类型排在二维码前，但实际上应该显示在二维码之后，因此暂时不添加
                    isShowUkey = true;
                    continue;
                } else if (devPtr->deviceType == UniT_Custom) {
                    hasCustom = true;
                    continue;
                }
                addOptionButton(itMapDev.key(), devPtr->id, DeviceType::getDeviceType_tr(devPtr->deviceType));
            }
        }
    }

    itMapDev = m_mapDevices.begin();
    if(isShowUkey || hasCustom){
        for ( ; itMapDev != m_mapDevices.end(); itMapDev++) {
            for (DeviceInfoPtr devPtr : itMapDev.value()) {
                if(devPtr && devPtr->deviceType == UniT_General_Ukey){
                    //此处才添加ukey
                    addOptionButton(itMapDev.key(), devPtr->id, DeviceType::getDeviceType_tr(devPtr->deviceType));
                } else if (devPtr && devPtr->deviceType == UniT_Custom) {
                    addOptionButton(itMapDev.key(), devPtr->id, tr("Other"));
                }
            }
        }
    }

    //存在特征但没有插入ukey
    if(!isShowUkey && m_biomericProxy->GetHasUkeyFeature(m_uid)){
        addOptionButton(LOGINOPT_TYPE_GENERAL_UKEY,-2,DeviceType::getDeviceType_tr(LOGINOPT_TYPE_GENERAL_UKEY));
        isShowUkey = true;
    }

    if (m_mapOptBtns.size() <= 2 && (!isShowUkey && !m_mapDevices.contains(LOGINOPT_TYPE_CUSTOM))) {
	//因为默认添加一个密码选项，因此当ukey没有显示出来时，按钮数小于等于2时就隐藏选项界面
        m_labelOptTitle->hide();
        QMap<int, QPushButton*>::iterator itMapBtn = m_mapOptBtns.begin();
        for ( ; itMapBtn != m_mapOptBtns.end(); itMapBtn++) {
            if (itMapBtn.value()) {
                itMapBtn.value()->hide();
            }
        }
    } else {
        m_labelOptTitle->show();
        QMap<int, QPushButton*>::iterator itMapBtn = m_mapOptBtns.begin();
        for ( ; itMapBtn != m_mapOptBtns.end(); itMapBtn++) {
            if (itMapBtn.value()) {
                itMapBtn.value()->show();
            }
        }
    }
    m_mapOptBtns[-1]->hide();
    if(m_mapOptBtns.size() == 2 && (isShowUkey || m_mapDevices.contains(LOGINOPT_TYPE_CUSTOM))) {
        m_mapOptBtns[-1]->show();
    } else if (m_mapOptBtns.size() == 3 && isShowUkey && m_mapDevices.contains(LOGINOPT_TYPE_CUSTOM)) {
        m_mapOptBtns[-1]->show();
    }
}

void LoginOptionsWidget::setUser(int uid)
{
    m_uid = uid;
    readDevicesInfo();
}

void LoginOptionsWidget::readDevicesInfo()
{
    m_mapDevices.clear();
    bool isAuthEnable = getBioAuthEnable(ENABLETYPE_GREETER);
    bool isQRCodeEnable = getQRCodeEnable();
    DeviceList deviceList = m_biomericProxy->GetDevList();
    QStringList listDefDevices = getAllDefDevices();
    qDebug()<<"BeginGetFeature------!";
    FeatureMap mapFeatures = m_biomericProxy->GetUserFeatures(m_uid);
    qDebug() << m_uid <<",count:"<<mapFeatures.size();
    for(auto pDeviceInfo : deviceList)
    {
        if (!isAuthEnable && pDeviceInfo->deviceType <= DeviceType::VoicePrint)
            continue;
        if (!isQRCodeEnable && pDeviceInfo->deviceType == REMOTE_QRCODE_TYPE)
            continue;
        int nFeatureCount = 0;
        if (mapFeatures.contains(pDeviceInfo->shortName)) {
            nFeatureCount = mapFeatures[pDeviceInfo->shortName].size();
        }
        qDebug() << *pDeviceInfo <<",count:"<<nFeatureCount;
        if(nFeatureCount > 0) {
            int nDevType = LOGINOPT_TYPE_OTHERS;
            nDevType = convertDeviceType(pDeviceInfo->deviceType);
            if (listDefDevices.contains(pDeviceInfo->shortName) &&
               !m_mapDevices.contains(nDevType)) {
                m_mapDevices[nDevType].push_back(pDeviceInfo);
            }
        }
    }
    // 检查是否有第三方认证插件
    if (PluginsLoader::instance().findModulesByType(LoginPluginInterface::MODULETYPE_AUTH).size() > 0) {
        LoginAuthInterface *plugin = dynamic_cast<LoginAuthInterface *>(PluginsLoader::instance() \
                                                                        .findModulesByType(LoginPluginInterface::MODULETYPE_AUTH) \
                                                                        .values().first());
        DeviceInfoPtr deviceInfoPtr = std::make_shared<DeviceInfo>();
        deviceInfoPtr->id = LOGINAUTH_CUSTOM_ID;
        deviceInfoPtr->shortName = CUSTOM_PLUGIN_DEV_PREFIX+plugin->name();
        deviceInfoPtr->deviceType = UniT_Custom;
        m_mapDevices[LOGINOPT_TYPE_CUSTOM].push_back(deviceInfoPtr);
    }
    updateOptionButtons();
}

QString LoginOptionsWidget::getCustomDevName()
{
    if (m_mapDevices.contains(LOGINOPT_TYPE_CUSTOM)) {
        return m_mapDevices[LOGINOPT_TYPE_CUSTOM].first()->shortName;
    }
    return "";
}

void LoginOptionsWidget::SetExtraInfo(QString extra_info, QString info_type)
{
    if(!m_biomericProxy)
    {
        qWarning() << "BiometricProxy doesn't exist.";
        return;
    }
    m_biomericProxy->SetExtraInfo(info_type,extra_info);
}

void LoginOptionsWidget::startAuth(DeviceInfoPtr device, int uid)
{
    if(!m_biomericProxy)
    {
        qWarning() << "BiometricProxy doesn't exist.";
        return;
    }
    if (!device) {
        qWarning() << "Biometric Auth device invalid.";
        return ;
    }

    if(m_isInAuth)
    {
        qDebug() << "Identification is currently under way, stop it";
        stopAuth();
    }
    qDebug()<<"deviceInfo:"<<device->id;
    this->m_curDevInfo = device;
    this->m_uid = uid;
    this->m_strUserName = getpwuid(uid)->pw_name;
    this->m_isStopped = false;
    this->m_curLoginOptType = convertDeviceType(this->m_curDevInfo->deviceType);
    updateUIStatus();
    SetLastDevice(this->m_strUserName, this->m_curDevInfo->id);

    if(this->m_curLoginOptType == LOGINOPT_TYPE_GENERAL_UKEY)
        startUkeyAuth();
    else
        startAuth_();
}

void LoginOptionsWidget::startUkeyAuth()
{
    if (!m_curDevInfo)
        return ;
    qDebug().noquote() << QString("Identify:[drvid: %1, uid: %2]").arg(m_curDevInfo->id).arg(m_uid);

    m_isInAuth = true;
    m_dupFD = -1;

    Q_EMIT setLoadingImage();
    m_biomericProxy->StopOps(m_curDevInfo->id);
    QDBusPendingCall call = m_biomericProxy->UkeyIdentify(m_curDevInfo->id, 2,m_uid);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, &LoginOptionsWidget::onIdentifyComplete);
}

void LoginOptionsWidget::startAuth_()
{
    if (!m_curDevInfo)
        return ;
    qDebug().noquote() << QString("Identify:[drvid: %1, uid: %2]").arg(m_curDevInfo->id).arg(m_uid);

    m_isInAuth = true;
    m_dupFD = -1;

    if (m_curDevInfo->deviceType == UniT_Custom) {
        LoginAuthInterface *authPlugin = getCustomLoginAuth();
        if (authPlugin) {
            struct passwd *pwdInfo = getpwuid(m_uid);
            if (pwdInfo) {
                authPlugin->startAuthenticate(m_uid, pwdInfo->pw_name);
            }
        }
    } else {
        Q_EMIT setLoadingImage();
        m_biomericProxy->StopOps(m_curDevInfo->id);
        QDBusPendingCall call = m_biomericProxy->Identify(m_curDevInfo->id, m_uid);
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished,
                this, &LoginOptionsWidget::onIdentifyComplete);
    }
}

void LoginOptionsWidget::stopAuth()
{
    m_isStopped = true;
    if(!m_isInAuth || !m_curDevInfo)
    {
        return;
    }
    if (m_curDevInfo->deviceType == UniT_Custom) {
        LoginAuthInterface *authPlugin = getCustomLoginAuth();
        if (authPlugin) {
            struct passwd *pwdInfo = getpwuid(m_uid);
            if (pwdInfo) {
                authPlugin->stopAuthenticate(m_uid, pwdInfo->pw_name);
            }
        }
    } else {
        m_biomericProxy->StopOps(m_curDevInfo->id);
    }
    if(m_retrytimer&&m_retrytimer->isActive()){
        m_retrytimer->stop();
        delete m_retrytimer;
        m_retrytimer = nullptr;
    }
    m_isInAuth = false;
    Q_EMIT updateImage(QImage());
}

void LoginOptionsWidget::onUkeyIdentifyComplete(QDBusPendingCallWatcher *watcher)
{
    if(m_isStopped == true)
        return ;

    QDBusPendingReply<int, int> reply = *watcher;
    if(reply.isError())
    {
        qWarning() << "Identify error: " << reply.error().message();
        Q_EMIT authComplete(false, -1);
        return;
    }
    int result = reply.argumentAt(0).toInt();
    int authUid = reply.argumentAt(1).toInt();
    qDebug() << result << authUid << m_uid;
    if(result == DBUS_RESULT_SUCCESS) {
        qDebug() << "Identify success";
        Q_EMIT authComplete(true, 0);
    } else {
        qDebug() << "Identify failed";
        Q_EMIT authComplete(false, 2);
    }
}

void LoginOptionsWidget::onIdentifyComplete(QDBusPendingCallWatcher *watcher)
{
    if(m_isStopped == true)
        return ;

    QDBusPendingReply<int, int> reply = *watcher;
    if(reply.isError())
    {
        qWarning() << "Identify error: " << reply.error().message();
        Q_EMIT authComplete(false, -1);
        return;
    }
    int result = reply.argumentAt(0).toInt();
    int authUid = reply.argumentAt(1).toInt();
    qDebug() << result << authUid << m_uid;

    // 特征识别成功，而且用户id匹配
    if(result == DBUS_RESULT_SUCCESS && authUid == m_uid) {
        qDebug() << "Identify success";
        Q_EMIT authComplete(true, 0);
    } else if(result == DBUS_RESULT_NOTMATCH) { // 特征识别不匹配
        qDebug() << "Identify failed";
        Q_EMIT authComplete(false, 2);
    } else if(result == DBUS_RESULT_ERROR) {  //识别发生错误
        StatusReslut ret = m_biomericProxy->UpdateStatus(m_curDevInfo->id);
        qDebug()<<"StatusReslut:"<<ret.result<<","<<ret.enable<<","<<ret.devNum<<","
                  <<ret.devStatus<<","<<ret.opsStatus<<","<<ret.notifyMessageId;
        //识别操作超时 304/404 认证超时；8 网络错误；
        if(ret.result == 0) {
            if (ret.opsStatus == OPS_IDENTIFY_TIMEOUT || ret.opsStatus == OPS_VERIFY_TIMEOUT
                || ret.opsStatus == 8) {    // 304认证超时， 8网络异常
                Q_EMIT authComplete(false, 1);
            } else if (ret.opsStatus == OPS_IDENTIFY_STOP_BY_USER || ret.opsStatus == OPS_VERIFY_STOP_BY_USER) {
                Q_EMIT authComplete(false, -2); // 主动停止，直接重试
            } else if (ret.opsStatus == OPS_OPEN_FAIL || ret.opsStatus == OPS_OPEN_ERROR) {     // 无法打开设备（设备是坏的/被占用），直接停止
                Q_EMIT authComplete(false, 5);
            } else if (ret.opsStatus >= OPS_GET_FLIST_SUCCESS && ret.opsStatus <= OPS_GET_FLIST_MAX) {
                Q_EMIT authComplete(false, -3); // 主动停止，直接重试
            } else {
                Q_EMIT authComplete(false, 2);
            }
        } else {
            Q_EMIT authComplete(false, 2);
        }
    } else {
        Q_EMIT authComplete(false, 2);
    }
}

void LoginOptionsWidget::onFrameWritten(int drvid)
{
    if (!m_curDevInfo || m_curDevInfo->id != drvid || !m_isInAuth) {
        return ;
    }
    if(m_dupFD == -1){
        m_dupFD = get_server_gvariant_stdout(drvid);
    }

    if(m_dupFD <= 0)
        return ;

    cv::Mat img;
    lseek(m_dupFD, 0, SEEK_SET);
    char base64_bufferData[1024*1024];
    int rc = read(m_dupFD, base64_bufferData, 1024*1024);
    // printf("rc = %d\n", rc);

    cv::Mat mat2(1, sizeof(base64_bufferData), CV_8U, base64_bufferData);
    img = cv::imdecode(mat2, cv::IMREAD_COLOR);
    if (!img.data)
        return ;
    cv::cvtColor(img,img,cv::COLOR_BGR2RGB);

    QImage srcQImage = QImage((uchar*)(img.data), img.cols, img.rows, QImage::Format_RGB888);
    Q_EMIT updateImage(srcQImage);
}

void LoginOptionsWidget::onStatusChanged(int drvid, int status)
{
    if(!m_isInAuth || !m_curDevInfo)
    {
        return;
    }
    if(drvid != m_curDevInfo->id)
    {
        return;
    }

//    // 显示来自服务的提示信息
//    if(status == STATUS_NOTIFY)
//    {
//        QString notifyMsg = m_biomericProxy->GetNotifyMesg(drvid);
//        Q_EMIT updateAuthMsg(notifyMsg);
//    }
}

void LoginOptionsWidget::setCurrentDevice(int drvid)
{
    DeviceInfoPtr pDeviceInfo = findDeviceById(drvid);
    if(pDeviceInfo)
    {
        setCurrentDevice(pDeviceInfo);
    }
}

void LoginOptionsWidget::setCurrentDevice(const QString &deviceName)
{
    DeviceInfoPtr pDeviceInfo = findDeviceByName(deviceName);
    if(pDeviceInfo)
    {
        setCurrentDevice(pDeviceInfo);
    }
}

void LoginOptionsWidget::setCurrentDevice(const DeviceInfoPtr &pDeviceInfo)
{
    this->m_curDevInfo = pDeviceInfo;
}

DeviceInfoPtr LoginOptionsWidget::findDeviceById(int drvid)
{
    for(int type : m_mapDevices.keys())
    {
        DeviceList &deviceList = m_mapDevices[type];
        auto iter = std::find_if(deviceList.begin(), deviceList.end(),
                                 [&](DeviceInfoPtr ptr){
            return ptr->id == drvid;
        });
        if(iter != deviceList.end())
        {
            return *iter;
        }
    }
    return DeviceInfoPtr();
}

DeviceInfoPtr LoginOptionsWidget::findDeviceByName(const QString &name)
{
    for(int type : m_mapDevices.keys())
    {
        DeviceList &deviceList = m_mapDevices[type];
        auto iter = std::find_if(deviceList.begin(), deviceList.end(),
                                 [&](DeviceInfoPtr ptr){
            return ptr->shortName == name;
        });
        if(iter != deviceList.end())
        {
            return *iter;
        }
    }
    return DeviceInfoPtr();
}

void LoginOptionsWidget::onUSBDeviceHotPlug(int drvid, int action, int /*devNum*/)
{
    int savedDeviceId = (m_curDevInfo ? m_curDevInfo->id : -1);
    int savedCount = 0;
    for(int type : m_mapDevices.keys())
        savedCount += m_mapDevices.value(type).count();

    switch(action)
    {
    case ACTION_ATTACHED:
    {
        //插入设备后，需要更新设备列表
        readDevicesInfo();
        break;
    }
    case ACTION_DETACHED:
    {
        DeviceInfoPtr pDeviceInfo = findDeviceById(drvid);
        if(pDeviceInfo)
        {
            int nDevType = LOGINOPT_TYPE_PASSWORD;
            nDevType = convertDeviceType(pDeviceInfo->deviceType);
            if (savedDeviceId == drvid) {
                if (m_isInAuth) {
                    Q_EMIT updateAuthMsg(tr("Identify device removed!"));
                }
            }
            m_mapDevices[nDevType].removeOne(pDeviceInfo);
            if(m_mapDevices[nDevType].isEmpty()) {
                m_mapDevices.remove(nDevType);
            }
        }
        break;
    }
    }

    int count = 0;
    for(int type : m_mapDevices.keys())
        count += m_mapDevices.value(type).count();

    //设备数量发生了变化
    if(count != savedCount) {
        updateOptionButtons();
        Q_EMIT notifyOptionsChange(count, at_bioBtn);
        if(at_bioBtn && count >= 1)
            tabOptionSelected(FirstDevice);
        updateUIStatus();
    }
}

bool LoginOptionsWidget::getAuthDouble()
{
    QSettings settings("/etc/biometric-auth/ukui-biometric.conf", QSettings::IniFormat);
    bool distribId = settings.value("DoubleAuth").toBool();
    return distribId;
}

void LoginOptionsWidget::setSelectedPassword()
{
    if (m_mapOptBtns.contains(-1)) {
        QPushButton* btn = m_mapOptBtns[-1];
        if (btn && btn->isVisible()) {
            btn->setChecked(true);
        }
        m_curDevInfo = nullptr;
    }
}

void LoginOptionsWidget::updateUIStatus()
{
    if (!m_curDevInfo && m_mapOptBtns.contains(-1)) {
        QPushButton* btn = m_mapOptBtns[-1];
        if (btn && btn->isVisible()) {
            btn->setChecked(true);
        }
    }
    if (m_curDevInfo) {
        if (m_mapOptBtns.contains(m_curDevInfo->id)) {
            QPushButton* btn = m_mapOptBtns[m_curDevInfo->id];
            if (btn) {
                btn->setChecked(true);
            }
        }
    }
}

void LoginOptionsWidget::onOptionSelected(int nIndex, bool keyNavigation)
{
    if (nIndex < 0)
        return;
    if(!keyNavigation)
        tabOptionSelected(CLEAR);
    if (nIndex < m_listDriveId.size()) {
        DeviceInfoPtr info = findDeviceById(m_listDriveId[nIndex]);
        if (info && !isDeviceDisable(info->id)) {
            Q_EMIT optionSelected(convertDeviceType(info->deviceType), info, keyNavigation);
        }else if(nIndex == 0 && m_listDriveId[nIndex] == -1){
            stopAuth();
            m_curDevInfo = nullptr;
            Q_EMIT optionSelected(LOGINOPT_TYPE_PASSWORD,nullptr,keyNavigation);
        }else if(m_listDriveId[nIndex] == -2){
            //存在ukey特征，但未插入ukey
            stopAuth();
            m_curDevInfo = nullptr;
            Q_EMIT optionSelected(LOGINOPT_TYPE_GENERAL_UKEY,nullptr,keyNavigation);
        }
    }
}

void LoginOptionsWidget::tabOptionSelected(int option)
{
    switch(option) {
    case RIGHT:
    {
        at_bioBtn = true;
        int nCur = nowAt;
        if(nCur >= 0 && nCur < m_listDriveId.count()) {
            if (m_mapOptBtns.contains(m_listDriveId[nCur])) {
                QPushButton* btn = m_mapOptBtns[m_listDriveId[nCur]];
                if (btn) {
                    btn->setStyleSheet(ON_NORMAL_SHEET);
                }
            }
        }
        for (int n = 0; n < m_listDriveId.count(); n ++) {
            nCur = nCur + 1;
            if (nCur >= m_listDriveId.count())
                nCur = 0;
            if (m_mapOptBtns.contains(m_listDriveId[nCur])) {
                QPushButton* btn = m_mapOptBtns[m_listDriveId[nCur]];
                if (btn && btn->isVisible()) {
                    btn->setStyleSheet(ON_TAB_SHEET);
                    nowAt = nCur;
                    break;
                }
            }
        }
    }
        break;
    case LEFT:
    {
        at_bioBtn = true;
        int nCur = nowAt;
        if(nCur >= 0 && nCur < m_listDriveId.count()) {
            if (m_mapOptBtns.contains(m_listDriveId[nCur])) {
                QPushButton* btn = m_mapOptBtns[m_listDriveId[nCur]];
                if (btn) {
                    btn->setStyleSheet(ON_NORMAL_SHEET);
                }
            }
        }
        for (int n = 0; n < m_listDriveId.count(); n ++) {
            nCur = nCur - 1;
            if (nCur < 0)
                nCur = m_listDriveId.count() - 1;
            if (m_mapOptBtns.contains(m_listDriveId[nCur])) {
                QPushButton* btn = m_mapOptBtns[m_listDriveId[nCur]];
                if (btn && btn->isVisible()) {
                    btn->setStyleSheet(ON_TAB_SHEET);
                    nowAt = nCur;
                    break;
                }
            }
        }
    }
        break;
    case CLEAR:
        at_bioBtn = false;
        if(nowAt >= 0) {
            for (int n = 0; n < m_listDriveId.count(); n ++) {
                if (m_mapOptBtns.contains(m_listDriveId[n])) {
                    QPushButton* btn = m_mapOptBtns[m_listDriveId[n]];
                    if (btn) {
                        btn->setStyleSheet(ON_NORMAL_SHEET);
                    }
                }
            }
            nowAt = -1;
        }
        break;
    case FirstDevice:
        at_bioBtn = true;
        if(nowAt >= 0) {
            for (int n = 0; n < m_listDriveId.count(); n ++) {
                if (m_mapOptBtns.contains(m_listDriveId[n])) {
                    QPushButton* btn = m_mapOptBtns[m_listDriveId[n]];
                    if (btn) {
                        btn->setStyleSheet(ON_NORMAL_SHEET);
                    }
                }
            }
            int nCur = 0;
            for (int n = 0; n < m_listDriveId.count(); n ++) {
                nCur = nCur + 1;
                if (nCur >= m_listDriveId.count())
                    nCur = 0;
                if (m_mapOptBtns.contains(m_listDriveId[nCur])) {
                    QPushButton* btn = m_mapOptBtns[m_listDriveId[nCur]];
                    if (btn && btn->isVisible()) {
                        btn->setStyleSheet(ON_TAB_SHEET);
                        nowAt = nCur;
                        break;
                    }
                }
            }
            nowAt = 0;
        }
        break;
    case CLICK:
        at_bioBtn = false;
        if (nowAt >= 0 && nowAt < m_listDriveId.count() && m_mapOptBtns.contains(m_listDriveId[nowAt])) {
            QPushButton* btn = m_mapOptBtns[m_listDriveId[nowAt]];
            if (btn) {
                onOptionSelected(nowAt, true);
            }
        }
        break;
    }
}

int LoginOptionsWidget::convertDeviceType(int nDevType)
{
    int nLoginOptType = LOGINOPT_TYPE_OTHERS;
    switch (nDevType) {
    case BioT_FingerPrint:
        nLoginOptType = LOGINOPT_TYPE_FINGERPRINT;
        break;
    case BioT_FingerVein:
        nLoginOptType = LOGINOPT_TYPE_FINGERVEIN;
        break;
    case BioT_Iris:
        nLoginOptType = LOGINOPT_TYPE_IRIS;
        break;
    case BioT_Face:
        nLoginOptType = LOGINOPT_TYPE_FACE;
        break;
    case BioT_VoicePrint:
        nLoginOptType = LOGINOPT_TYPE_VOICEPRINT;
        break;
    case UniT_General_Ukey:
        nLoginOptType = LOGINOPT_TYPE_GENERAL_UKEY;
        break;
    case UniT_Advanced_Ukey:
        nLoginOptType = LOGINOPT_TYPE_ADVANCED_UKEY;
        break;
    case UniT_Remote:
        nLoginOptType = LOGINOPT_TYPE_QRCODE;
        break;
    case UniT_Custom:
        nLoginOptType = LOGINOPT_TYPE_CUSTOM;
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
        QMap<int, QPushButton*>::iterator itMapBtn = m_mapOptBtns.begin();
        for ( ; itMapBtn != m_mapOptBtns.end(); itMapBtn++) {
            if (itMapBtn.key() == nDevId && itMapBtn.value()) {
                itMapBtn.value()->setDisabled(true);
                break;
            }
        }
    } else {
        m_mapDisableDev[m_uid][nDevId] = false;
        QMap<int, QPushButton*>::iterator itMapBtn = m_mapOptBtns.begin();
        for ( ; itMapBtn != m_mapOptBtns.end(); itMapBtn++) {
            if (itMapBtn.key() == nDevId && itMapBtn.value()) {
                itMapBtn.value()->setDisabled(false);
                break;
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

QPixmap LoginOptionsWidget::loadSvg(QString path, QString color, int size)
{
    int origSize = size;
    const auto ratio = qApp->devicePixelRatio();
    if ( 2 == ratio) {
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
                if ( "white" == cgColor) {
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                } else if( "black" == cgColor) {
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                } else if ("gray"== cgColor) {
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                } else if ("blue" == cgColor){
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

bool LoginOptionsWidget::getBioAuthEnable(int nType)
{
    bool isEnable = false;
    if (m_uniauthService && m_uniauthService->isActivatable()) {
        struct passwd *pwInfo = getpwuid(m_uid);
        if (pwInfo) {
            isEnable = m_uniauthService->getBioAuthStatus(pwInfo->pw_name, ENABLETYPE_BIO);
            if (isEnable) {
                isEnable = m_uniauthService->getBioAuthStatus(pwInfo->pw_name, nType);
            }
            return isEnable;
        } else {
            return false;
        }
    } else {
        return GetAuthEnable();
    }
}

bool LoginOptionsWidget::getQRCodeEnable()
{
    if (m_uniauthService && m_uniauthService->isActivatable()) {
        return m_uniauthService->getQRCodeEnable();
    } else {
        return GetQRCodeEnable();
    }
}

QString LoginOptionsWidget::getDefaultDevice(QString strUserName,int bioType)
{
    if (m_uniauthService && m_uniauthService->isActivatable()) {
        QString defaultDeviceName = "";
            QString strDeviceName = m_uniauthService->getDefaultDevice(strUserName, bioType);
            if(!strDeviceName.isEmpty()) {
                DeviceInfoPtr pDeviceInfo = findDeviceByName(strDeviceName);
                if (pDeviceInfo) {
                    defaultDeviceName = strDeviceName;
                }
            }
        return defaultDeviceName;
    } else {
        return GetDefaultDevice(strUserName);
    }
}

QString LoginOptionsWidget::getDefaultDevice(QString strUserName)
{
    if (m_uniauthService && m_uniauthService->isActivatable()) {
        QString defaultDeviceName = "";
        for (auto bioType : m_listPriority) {
            QString strDeviceName = m_uniauthService->getDefaultDevice(strUserName, bioType);
            if(!strDeviceName.isEmpty()) {
                DeviceInfoPtr pDeviceInfo = findDeviceByName(strDeviceName);
                if (pDeviceInfo) {
                    defaultDeviceName = strDeviceName;
                    break;
                }
            }
        }
        return QString(defaultDeviceName);
    } else {
        return GetDefaultDevice(strUserName);
    }
}

QStringList LoginOptionsWidget::getAllDefDevices()
{
    QStringList listDefDevices;
    if (m_uniauthService && m_uniauthService->isActivatable()) {
        struct passwd *pwdInfo = getpwuid(m_uid);
        if (pwdInfo) {
            listDefDevices = m_uniauthService->getAllDefaultDevice(pwdInfo->pw_name);
        }
    } else {
        QString defaultDeviceName;

        struct passwd *pwd = getpwuid(m_uid);
        QString userConfigFile = QString(pwd->pw_dir) + "/.biometric_auth/ukui_biometric.conf";
        QSettings userConfig(userConfigFile, QSettings::IniFormat);
        qDebug() << userConfig.fileName();
        defaultDeviceName = userConfig.value("DefaultDevice").toString();
        qDebug() << defaultDeviceName;

        if(defaultDeviceName.isEmpty()) {
            QSettings sysConfig("/etc/biometric-auth/ukui-biometric.conf", QSettings::IniFormat);
            defaultDeviceName = sysConfig.value("DefaultDevice").toString();
        }
        qDebug() << "default device: " << defaultDeviceName;
        if(!defaultDeviceName.isEmpty()){
            listDefDevices.push_back(defaultDeviceName);
        }
    }
    return QStringList(listDefDevices);
}

LoginAuthInterface* LoginOptionsWidget::getCustomLoginAuth()
{
    if (PluginsLoader::instance().findModulesByType(LoginPluginInterface::MODULETYPE_AUTH).values().size() <= 0) {
        return nullptr;
    }
    return dynamic_cast<LoginAuthInterface *>(PluginsLoader::instance().findModulesByType(LoginPluginInterface::MODULETYPE_AUTH).values().first());
}
