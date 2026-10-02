/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDebug>
#include <QScreen>
#include <QPalette>
#include <QPainter>
#include <QDBusReply>
#include <QApplication>
#include <QPainterPath>

#include "clib-syslog.h"
#include "privacy-osd.h"
#include "windowmanager/windowmanager.h"
#include "ukuistylehelper/ukuistylehelper.h"

/*
 * 设计原则：
 * 分为三部分，
 *      主要提示：隐私开关状态
 *      次要提示：此窗口的显示隐藏方法
 *      图层：横排图层。
 * 三个状态，w240，高161，
 * 48*3+24*4=240
 *
 * 上下边界示例：左右均为居中布局 10-15
 * |-------------------------------|
 * |         24                    |上边距
 * |   primaryPrompt               |字体Noto Sans CJK SC大小18(默认)-24
 * |         4 pix                 |
 * |m_secondaryPrompt              |字体Noto Sans CJK SC大小12（默认）-18
 * |         16                    |
 * |-24- osd1-24-osd2-24-osd3  -24-|
 * |          24                   |
 * |------------------------------ |
 *
*/
PrivacyOSD::PrivacyOSD(QWidget *parent):
    QWidget(parent)
{
    initWindowHelper();
    initWindowsAttribute();
    initAllControl();
    geometryChangedHandle();
}

PrivacyOSD::~PrivacyOSD()
{
    qDeleteAll(m_iconsMap.begin(), m_iconsMap.end()); // 更简洁的删除方式
    m_iconsMap.clear();

    delete m_styleSettings;
    delete m_prvacyBus;
}

void PrivacyOSD::test( QString icons)
{
//    QString icons = "WiFi:0,Camera:0,Mic:0";
    QStringList deviceList = icons.split(",");
    // 创建一个QMap来存储设备名称和状态
    QMap<QString, bool> deviceStatus;

    for (const QString &device : deviceList) {
        // 使用 ':' 分割每个设备字符串
        QStringList parts = device.split(':');
        if (parts.size() == 2) {
            QString deviceName = parts[0];
            int status = parts[1].toInt();
//            if (status == 0x01) {
//                iconsState |= true;
//            }
            deviceStatus[deviceName] = status;
        }
    }

    showSwitchWithImage(deviceStatus,  true);
    show();
    m_hideTimer->start(3000);
}

void PrivacyOSD::initIcons(QMap<QString, bool> &deviceStatus)
{
    // 使用静态变量缓存上次的状态，避免按下fn+s时重复更新
    static QMap<QString, bool> sDeviceStatus;
    if (sDeviceStatus == deviceStatus) {
        return;//修改字体大小直接隐藏此窗口，再次唤醒时根据字体大小重新计算。故不需要处理。
    }
    sDeviceStatus = deviceStatus;

    // 计算总长度和起始位置
    const int iconWidth = 48;
    const int spacing = 24;
    const int totalLength = deviceStatus.count() * iconWidth + (deviceStatus.count() - 1) * spacing;
    const int startX = this->width() / 2 - totalLength / 2;
    const int startY = 89 + m_secondaryPromptIncreament + m_primaryPromptIncrement; // 假定起始 Y 坐标固定
    const int xOffset = iconWidth + spacing;
    USD_LOG_SHOW_PARAM2(startX, startY);
    // Lambda 函数：更新或创建 QLabel
    auto updateOrCreateLabel = [&](const QString& key, int& currentX) {
        QString svgName = getSvgIcon(key, deviceStatus[key]);
        if (svgName.isEmpty()) {
            USD_LOG(LOG_ERR, "Can't find the svg: %s %d", key.toStdString().c_str(), deviceStatus[key]);
            return;
        }

        QPixmap pixmap = QIcon::fromTheme(svgName).pixmap(iconWidth, iconWidth); // 确保 pixmap 大小正确
        QLabel* label = m_iconsMap.value(key);

        if (!label) {
            label = new QLabel(this);
            m_iconsMap.insert(key, label);
            label->setFixedSize(QSize(iconWidth, iconWidth));
            label->move(currentX, startY);
        }

        label->move(currentX, startY);
        label->setPixmap(drawLightColoredPixmap(pixmap, m_styleSettings->get("style-name").toString()));
        currentX += xOffset;
    };

    // 遍历图标顺序并更新
    int currentX = startX;
    const QStringList iconOrder = {"Mic", "Camera", "WiFi", "Bt"};
    for (const QString& currentIcon : iconOrder) {
        if (deviceStatus.contains(currentIcon)) {
            updateOrCreateLabel(currentIcon, currentX);
        }
    }
}

double PrivacyOSD::getGlobalOpacity() const
{
    const char* schemaName = "org.ukui.control-center.personalise";
    const char* key = "transparency";

    // 检查 schema 是否已安装
    if (QGSettings::isSchemaInstalled(schemaName)) {
        QGSettings gsetting(schemaName);
        // 检查 key 是否存在并返回其值
        if (gsetting.keys().contains(key)) {
            bool ok = false;
            double opacity = gsetting.get(key).toDouble(&ok);
            if (ok) {
                return opacity;
            }
        }
    }

    // 返回默认值
    return 0.0;
}

void PrivacyOSD::showSwitchWithImage(QMap<QString, bool> &deviceStatus, bool switchState)
{
    if (switchState) {
        m_primaryPrompt->setText(tr("Privacy protection: ON"));//Privacy protection: ON"
    } else {
        m_primaryPrompt->setText(tr("Privacy protection: OFF"));
    }

    // 根据设备数量设置固定大小
    setFixedSizeWithImageCount(deviceStatus.count());
    initPromptFont();
    initIcons(deviceStatus);
    if (deviceStatus.isEmpty()) {
        initEmptyFont();
    } else {
        showDeviceMsg(deviceStatus, switchState);
    }

    repaintWidget();
    geometryChangedHandle();
}

void PrivacyOSD::initWindowsAttribute()
{
    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::Tool |
                   Qt::WindowStaysOnTopHint |
                   Qt::X11BypassWindowManagerHint |
                   Qt::Popup);
    setAttribute(Qt::WA_TranslucentBackground, true);

    setAutoFillBackground(true);
}

void PrivacyOSD::initAllControl()
{
    m_primaryPrompt = new QLabel(this);
    m_secondaryPrompt = new QLabel(tr("show or hide press Fn + S"), this); // 直接在构造函数中设置文本

    if (qApp->property(SECOND_COLOR).isValid() && qApp->property(SECOND_COLOR).canConvert<QColor>()) {
        QPalette secondaryPa = QApplication::palette();
        QColor secondarycolor = qApp->property(SECOND_COLOR).value<QColor>();
        secondaryPa.setColor(QPalette::Text, secondarycolor);
        m_secondaryPrompt->setPalette(secondaryPa);
    }

    // 初始化 QTimer 并连接信号槽
    m_hideTimer = new QTimer(this);
    connect(m_hideTimer, &QTimer::timeout, this, &PrivacyOSD::doHideTimer);

    // 初始化 QGSettings 并连接信号槽
    m_styleSettings = new QGSettings(QT_THEME_SCHEMA);
    connect(m_styleSettings, &QGSettings::changed, this, &PrivacyOSD::onStyleChanged);

    // 连接 QScreen 的 geometryChanged 信号
    connect(QApplication::primaryScreen(), &QScreen::geometryChanged, this, &PrivacyOSD::geometryChangedHandle);

    // 初始化 QDBusInterface 并检查有效性
    m_prvacyBus = new QDBusInterface(QStringLiteral("org.freedesktop.OEMPrivacyKey"),
                                     QStringLiteral("/org/freedesktop/OEMPrivacyKey"),
                                     QStringLiteral("org.freedesktop.OEMPrivacyKey.Manager"), QDBusConnection::systemBus(), this);

    if (m_prvacyBus && m_prvacyBus->isValid()) {//睡眠登录、
        connect(m_prvacyBus, SIGNAL(privacykey(bool)), this, SLOT(doPrivacyBusSignal(bool)));

        QDBusConnection::systemBus().connect(QStringLiteral("org.freedesktop.login1"),
                                             QStringLiteral("/org/freedesktop/login1"),
                                             QStringLiteral("org.freedesktop.login1.Manager"),
                                             QStringLiteral("PrepareForSleep"),
                                             this, SLOT(doPrepareForSleep(bool)));

        QDBusConnection::sessionBus().connect(QString(),\
                                              QStringLiteral("/GlobalSignal"),\
                                              QStringLiteral("org.ukui.SettingsDaemon.GlobalSignal"),\
                                              "Active", this, SLOT(onUserActive(bool)));

        QDBusConnection::sessionBus().connect(QStringLiteral("org.ukui.ScreenSaver"),\
                                              QStringLiteral("/"),\
                                              QStringLiteral("org.ukui.ScreenSaver"),\
                                              "unlock", this, SLOT(onUnlock()));
        // 初始化提示字体
        initPromptFont();
        doPrivacyBusSignal(false);
    } else {
        SYS_LOG(LOG_WARNING, "connect OEMPrivacyKey error");
    }
}

void PrivacyOSD::setFixedSizeWithImageCount(int imagesCount)
{
    int fixedWidth = OSD_MINWIDTH;//最小尺寸
    if (imagesCount > 2) {
        fixedWidth = imagesCount*48+24*(1+imagesCount);//48 svg图像宽度，24：svg图像间隔。
        fixedWidth = fixedWidth < OSD_MINWIDTH ? OSD_MINWIDTH : fixedWidth;
    }
    setFixedWidth(fixedWidth);
}

//理论上不存在运行中切换语言的方式，不需要重新计算文案位置
void PrivacyOSD::initPromptFont()
{
    // 定义字体
    QFont primaryFont("Noto Sans CJK SC");
    QFont secondaryFont("Noto Sans CJK SC");

    m_secondaryPrompt->setWordWrap(true);
    m_secondaryPrompt->setAlignment(Qt::AlignCenter);

    m_primaryPrompt->setWordWrap(true);
    m_primaryPrompt->setAlignment(Qt::AlignCenter);
    // 获取基础字体大小
    double baseSize = m_styleSettings->get(SYSTEM_FONT_SIZE).toDouble();
    double primaryFontSize = 16;
    double secondaryFontSize = 12;

    primaryFontSize = 16 * (1 + (baseSize - 10.0) / 16.0);
    secondaryFontSize = 12 * (1 + (baseSize - 10.0) / 12.0);

    // 设置字体大小
    primaryFont.setPixelSize(primaryFontSize);
    secondaryFont.setPixelSize(secondaryFontSize);

    // 应用字体到标签
    m_primaryPrompt->setFont(primaryFont);
    m_secondaryPrompt->setFont(secondaryFont);
    USD_LOG_SHOW_PARAM2(getPromptMaxWidth(), getSecondaryMaxWidth());

    // 调整控件位置与换行
    adjustPromptPositions();
}

void PrivacyOSD::initEmptyFont()
{
    if (m_emptyPrompt == nullptr) {
        m_emptyPrompt = new QLabel(tr("Please configure the privacy switch in the BIOS to control the devices that need to be controled"), this);

        if (qApp->property(SECOND_COLOR).isValid() && qApp->property(SECOND_COLOR).canConvert<QColor>()) {
            QPalette secondaryPa = QApplication::palette();
            QColor secondarycolor = qApp->property(SECOND_COLOR).value<QColor>();
            secondaryPa.setColor(QPalette::Text, secondarycolor);
            m_emptyPrompt->setPalette(secondaryPa);
        }
    }

    QFont emptyFont("Noto Sans CJK SC");

    m_emptyPrompt->setWordWrap(true);
    m_emptyPrompt->setAlignment(Qt::AlignCenter);

    double baseSize = m_styleSettings->get(SYSTEM_FONT_SIZE).toDouble();
    double emptyFontSize = 14;

    emptyFontSize = 14 * (1 + (baseSize - 10.0) / 14.0);

    emptyFont.setPixelSize(emptyFontSize);
    m_emptyPrompt->setFont(emptyFont);


    QFontMetrics emptyMetrics(m_emptyPrompt->font());

    QRect emptyRect = emptyMetrics.boundingRect(QRect(0,0,gerEmptyMaxWidth(),0),
                                                    Qt::TextWordWrap, m_emptyPrompt->text());


    // 计算并设置位置
    int emptyX = (this->size().width() / 2) - (std::min(gerEmptyMaxWidth(), emptyMetrics.width(m_emptyPrompt->text())) / 2);
    int emptyY = 89 + m_secondaryPromptIncreament + m_primaryPromptIncrement;;

    m_emptyPrompt->setFixedWidth(gerEmptyMaxWidth());
    m_emptyPrompt->setFixedHeight(emptyRect.height());
    m_emptyPrompt->move(emptyX, emptyY);
}

void PrivacyOSD::showDeviceMsg(QMap<QString, bool> &deviceStatus, bool switchState)
{
    if (m_deviceMsgLabel == nullptr) {
        QStringList devList;
        for (auto dev : deviceStatus.keys()) {
            devList.append(tr(dev.toUtf8().data()));
        }
        QString str = QString(tr("Device %1 disabled, toggle the keyboard side switch to exit privay mode.")).arg(devList.join(tr(",")));
        m_deviceMsgLabel = new QLabel(str, this);
        if (qApp->property(SECOND_COLOR).isValid() && qApp->property(SECOND_COLOR).canConvert<QColor>()) {
            QPalette secondaryPa = QApplication::palette();
            QColor secondarycolor = qApp->property(SECOND_COLOR).value<QColor>();
            secondaryPa.setColor(QPalette::Text, secondarycolor);
            m_deviceMsgLabel->setPalette(secondaryPa);
        }
    }

    if (switchState) {
        m_deviceMsgLabel->show();
        QFont devFont("Noto Sans CJK SC");

        m_deviceMsgLabel->setWordWrap(true);
        m_deviceMsgLabel->setAlignment(Qt::AlignCenter);

        double baseSize = m_styleSettings->get(SYSTEM_FONT_SIZE).toDouble();
        double devFontSize = 14 * (1 + (baseSize - 10.0) / 14.0);

        devFont.setPixelSize(devFontSize);
        m_deviceMsgLabel->setFont(devFont);

        QFontMetrics devMetrics(m_deviceMsgLabel->font());
        QRect devRect = devMetrics.boundingRect(QRect(0,0, getDevMaxWidth(),0),
                                                        Qt::TextWordWrap, m_deviceMsgLabel->text());

        // 计算并设置位置
        int devX = (this->size().width() / 2) - (std::min(getDevMaxWidth(), devMetrics.width(m_deviceMsgLabel->text())) / 2);
        int devY = 89 + m_secondaryPromptIncreament + m_primaryPromptIncrement + 48 + 12;

        m_deviceMsgLabel->setFixedWidth(getDevMaxWidth());
        m_deviceMsgLabel->setFixedHeight(devRect.height());
        m_deviceMsgLabel->move(devX, devY);

        setFixedHeight(devY + m_deviceMsgLabel->height() + 24);
    } else {
        m_deviceMsgLabel->hide();
        setFixedHeight(165 + m_primaryPromptIncrement + m_secondaryPromptIncreament);
    }
}

void PrivacyOSD::adjustPromptPositions()
{
    QFontMetrics primaryMetrics(m_primaryPrompt->font());
    QFontMetrics secondaryMetrics(m_secondaryPrompt->font());

    QRect primaryRect = primaryMetrics.boundingRect(QRect(0,0,getPromptMaxWidth(),0),
                                                    Qt::TextWordWrap, m_primaryPrompt->text());

    QRect secondaryRect =  secondaryMetrics.boundingRect(QRect(0,0,getSecondaryMaxWidth(),0),
                                                         Qt::TextWordWrap, m_secondaryPrompt->text());

    // 计算并设置位置
    int primaryX = (this->size().width() / 2) - (std::min(getPromptMaxWidth(), primaryMetrics.width(m_primaryPrompt->text())) / 2);
    int primaryY = 24;

    m_primaryPrompt->setFixedWidth(getPromptMaxWidth());
    m_primaryPrompt->setFixedHeight(primaryRect.height());
    m_primaryPrompt->move(primaryX, primaryY);
    m_primaryPromptIncrement =  primaryRect.height() - primaryMetrics.height();

    int secondaryX = (this->size().width() / 2) - (std::min(getSecondaryMaxWidth(), secondaryMetrics.width(m_secondaryPrompt->text())) / 2);
    int secondaryY = primaryY + 31 + m_primaryPromptIncrement; // 24 (primary Y) + 7 (spacing between prompts, adjusted from 55-24)

    m_secondaryPrompt->setFixedWidth(getSecondaryMaxWidth() + 2);
    m_secondaryPrompt->setFixedHeight(secondaryRect.height());
    m_secondaryPrompt->move(secondaryX, secondaryY);
    m_secondaryPromptIncreament = secondaryRect.height() - secondaryMetrics.height();

    setFixedHeight(165 + m_primaryPromptIncrement +
                  m_secondaryPromptIncreament);
}

void PrivacyOSD::priScreenChanged(int x, int y, int width, int height)
{
    const QByteArray id(PANEL_SCHEMA);
    int pSize = 48;

    if (QGSettings::isSchemaInstalled(id)){
        QGSettings settings(PANEL_SCHEMA);
        if (settings.keys().contains(PANEL_SIZE_KEY)) {
            settings.get(PANEL_SIZE_KEY).toInt();
        }
        pSize = pSize < 48 ? 48 : pSize;
    }

    int ax,ay;
    ax = x + width / 2 - this->width()/2;
    ay = y + height - this->height() - pSize -8 ;
    setGeometry(QRect(ax, ay, this->width(), this->height()));
}

void PrivacyOSD::repaintWidget()
{
    if(m_styleSettings->get(THEME_STYLE_NAME).toString() == THEME_DARK_MODE){
        setPalette(QPalette(QColor("#232426")));//设置窗口背景色
    } else {
        setPalette(QPalette(QColor("#F5F5F5")));//设置窗口背景
    }

    for (auto it = m_iconsMap.begin(); it != m_iconsMap.end(); ++it) {
        QPixmap m_pixmap = *(it.value()->pixmap());
        it.value()->setPixmap(drawLightColoredPixmap(m_pixmap,m_styleSettings->get(THEME_STYLE_NAME).toString()));
    }
}

void PrivacyOSD::onStyleChanged(const QString &key)
{
    if(!this->isHidden()) {
        hide();
    }
}

void PrivacyOSD::geometryChangedHandle()
{
    int x=QApplication::primaryScreen()->geometry().x();
    int y=QApplication::primaryScreen()->geometry().y();
    int width = QApplication::primaryScreen()->size().width();
    int height = QApplication::primaryScreen()->size().height();

    priScreenChanged(x, y, width, height);
}

void PrivacyOSD::doHideTimer()
{
    this->hide();
    m_hideTimer->stop();
    m_isCallByDbus = false;
}

void PrivacyOSD::doPrivacyBusSignal(bool state)
{
    m_isCallByDbus = true;
    USD_LOG(LOG_DEBUG,".");
    callPrivacyBusShowOSD(state);
}

QString PrivacyOSD::getSvgIcon(QString device, bool state)
{
    if (!device.compare("mic",Qt::CaseInsensitive)) {
        return state ? "microphone-hardware-disabled-symbolic" : "audio-input-microphone-symbolic";
    }

    if (!device.compare("camera",Qt::CaseInsensitive)) {
        return state ? "camera-hardware-disabled-symbolic" : "camera-app-symbolic";
    }

    if (!device.compare("WiFi",Qt::CaseInsensitive)) {
        return state ? "network-wireless-hardware-disabled-symbolic" : "network-wireless-symbolic";
    }

    if (!device.compare("Bt",Qt::CaseInsensitive)) {
        return state ? "bluetooth-hardware-disabled-symbolic" : "bluetooth-active-symbolic";
    }

    return "";
}

void PrivacyOSD::doPrepareForSleep(bool isSleep)
{
    USD_LOG(LOG_DEBUG,".");
    if (false == isSleep && m_inPrivacy) {
        callPrivacyBusShowOSD(false);
    }
}

void PrivacyOSD::onUserActive(bool isActive)
{
    USD_LOG(LOG_DEBUG,".");
    if (true == isActive && m_inPrivacy) {
        callPrivacyBusShowOSD(false);
    }
}

void PrivacyOSD::onUnlock()
{
    USD_LOG(LOG_DEBUG,".");
    if (m_inPrivacy) {
        callPrivacyBusShowOSD(false);
    } else {
        if (false == isHidden()) {
            hide();
        }
    }
}

bool PrivacyOSD::initPrivacyState()
{
    QString msg;
    bool privacttate = false;

    QDBusReply<QString> replay = m_prvacyBus->call("GetPrivacyKeyState");
    if (!replay.isValid()) {
        SYS_LOG(LOG_DEBUG, "call GetPrivacyKeyState error.");
        return privacttate;
    }

    msg = replay.value();
    QStringList deviceList = msg.split(",");


    for (const QString &device : deviceList) {
        // 使用 ':' 分割每个设备字符串
        QStringList parts = device.split(':');
        if (parts.size() == 2) {
            int status = parts[1].toInt();
            if (status == 0x01) {
                privacttate |= true;
            }
        }
    }

    return privacttate;
}

QPixmap PrivacyOSD::drawLightColoredPixmap(const QPixmap &source, const QString &style)
{
    int value = 0;
    if (style == THEME_DARK_MODE) {
        value = 255;
    }

    QColor gray(255,255,255);
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                if (qAbs(color.red()-gray.red())<20 && qAbs(color.green()-gray.green())<20 && qAbs(color.blue()-gray.blue())<20) {
                    color.setRed(value);
                    color.setGreen(value);
                    color.setBlue(value);
                    img.setPixelColor(x, y, color);
                }
                else {
                    color.setRed(value);
                    color.setGreen(value);
                    color.setBlue(value);
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}

void PrivacyOSD::callPrivacyBusShowOSD(bool state)
{
    static bool switchState = false;
    if (!m_prvacyBus->isValid()) {
        SYS_LOG(LOG_DEBUG, "connect dbus error.");
        return;
    }

    QDBusReply<bool> switchStateReply = m_prvacyBus->call("GetSwitchState");
    if (!switchStateReply.isValid()) {
        SYS_LOG(LOG_WARNING, "call GetSwitchState error!");
        return;
    }

    QDBusReply<QString> replay = m_prvacyBus->call("GetPrivacyKeyState");
    if (!replay.isValid()) {
        SYS_LOG(LOG_DEBUG, "call GetPrivacyKeyState error.");
        return;
    }

    //根据新需求调整历史功能，开关状态无变化且不为隐藏状态，dbus信号隐藏OSD。实际操作为fn+s隐藏OSD
    if (switchState == switchStateReply.value() && false == isHidden() && m_isCallByDbus) {
        USD_LOG(LOG_DEBUG,"hide it");
        hide();
        m_isCallByDbus = false;
        return;
    }
    switchState = switchStateReply.value();

    QStringList deviceList = replay.value().split(",");
    QMap<QString, bool> deviceStatus;

    for (const QString &device : deviceList) {
        // 使用 ':' 分割每个设备字符串
        QStringList parts = device.split(':');
        if (parts.size() == 2) {
            QString deviceName = parts.first();
            deviceStatus[deviceName] = switchState;
        }
    }

    if (m_inPrivacy > -1 || (m_inPrivacy == -1 && switchState & (!state))) {//首次启动时开启管空或者非首次启动时显示OSD
        showSwitchWithImage(deviceStatus, switchState & (!state));
        show();
        m_hideTimer->start(3000);
    }
    m_inPrivacy = switchState & (!state);
}

void PrivacyOSD::initWindowHelper()
{
    m_windowHelper = new UkuiWindowHelper(this);
    m_windowHelper->setWindowState(UkuiWindowHelper::State::KeepAbove);
    m_windowHelper->removeTitleBar();
    m_windowHelper->setSkipTaskBar(true);
    m_windowHelper->setSkipSwitcher(true);
    m_windowHelper->setWindowRole(UkuiWindowHelper::WindowRole::OnScreenDisplay);
}

QT_BEGIN_NAMESPACE
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);
QT_END_NAMESPACE
void PrivacyOSD::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 12, 12);
    // 画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(this->palette().base());
    pixmapPainter.setCompositionMode(QPainter::CompositionMode_Difference);
    pixmapPainter.setOpacity(0.16);
    //    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();
    // 模糊这个黑底
    QImage img = pixmap.toImage();
    qt_blurImage(img, 8, false, false);

    // 挖掉中心
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);
    // 绘制阴影
    painter.drawPixmap(this->rect(), pixmap, pixmap.rect());

    //绘制描边
    QPainterPath linePath;
    linePath.addRoundedRect(QRect(9,9,this->width()-12,this->height()-12),12,12);
    painter.setCompositionMode(QPainter::CompositionMode_Difference);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(this->palette().color(QPalette::ColorRole::BrightText));
    painter.setBrush(Qt::transparent);
    painter.setOpacity(0.15);
    painter.drawPath(linePath);

    //毛玻璃
    qreal opacity = getGlobalOpacity();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::transparent);
    painter.setBrush(QApplication::palette().base());
    painter.setPen(Qt::transparent);
    painter.setOpacity(opacity);
    painter.drawPath(linePath);

    if (m_windowHelper != nullptr) {
        m_windowHelper->setBlurEffect(QRegion(linePath.toFillPolygon().toPolygon()));
    }

    QWidget::paintEvent(e);
}
/*
 * 如果本身字体小于最值则用本身字体的长度
*/
int PrivacyOSD::getPromptMaxWidth()
{
    QFontMetrics primaryMetrics(m_primaryPrompt->font());
    return primaryMetrics.width(m_primaryPrompt->text()) >
            (this->width() - 64) ?
                (this->width() - 64) :
                primaryMetrics.width(m_primaryPrompt->text());
}
/*
 * 如果本身字体小于最值则用本身字体的长度
*/
int PrivacyOSD::getSecondaryMaxWidth()
{
    QFontMetrics secondaryMetrics(m_secondaryPrompt->font());
    return secondaryMetrics.width(m_secondaryPrompt->text()) >
            (this->width() - 40) ?
                (this->width() - 40) :
                (secondaryMetrics.width(m_secondaryPrompt->text()));
}

int PrivacyOSD::gerEmptyMaxWidth()
{
    QFontMetrics emptyMetrics(m_emptyPrompt->font());
    return emptyMetrics.width(m_emptyPrompt->text()) >
            (this->width() - 40) ?
                (this->width() - 40) :
                (emptyMetrics.width(m_emptyPrompt->text()));
}

int PrivacyOSD::getDevMaxWidth()
{
    QFontMetrics devMetrics(m_deviceMsgLabel->font());
    return devMetrics.width(m_deviceMsgLabel->text()) >
            (this->width() - 40) ?
                (this->width() - 40) :
                (devMetrics.width(m_deviceMsgLabel->text()));
}

