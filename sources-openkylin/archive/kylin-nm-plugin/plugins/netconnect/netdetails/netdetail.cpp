/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "netdetail.h"
#include "../colorpalette.h"
#include "oobetabbar.h"

#define THEME_SCHAME "org.ukui.style"
#define COLOR_THEME "styleName"

#include <QEvent>
#include <QTimer>
#include <QPainterPath>
#include <QRegion>

#define  WINDOW_WIDTH  480
#define  WINDOW_HEIGHT 430
#define  BUTTON_SIZE 30
#define  ICON_SIZE 22,22
#define  TITLE_LAYOUT_MARGINS 9,9,0,0
#define  LAYOUT_MARGINS 24,0,24,0
#define  BOTTOM_LAYOUT_SPACING 16
#define  PAGE_LAYOUT_SPACING 1
#define  DETAIL_PAGE_NUM 0
#define  IPV4_PAGE_NUM 1
#define  IPV6_PAGE_NUM 2
#define  SECURITY_PAGE_NUM 3
#define  CREATE_NET_PAGE_NUM 4
#define  PAGE_MIN_HEIGHT 40
#define  LAN_TAB_WIDTH 300
#define  WLAN_TAB_WIDTH 400

void NetDetail::setNetdetailSomeEnable(bool on)
{
    detailPage->setEnabled(on);
    ipv4Page->setEnabled(on);
    if (!m_isPppoe) {
        ipv6Page->setEnabled(on);
    }
    cancelBtn->setEnabled(on);
    forgetBtn->setEnabled(on);
    confimBtn->setEnabled(on);
}

NetDetail::NetDetail(PluginType type, QString interface, QString name, QString uuid, bool isPppoe, ConInfo info, QWidget *parent)
    :m_pluginType(type),
     m_deviceName(interface),
     m_name(name),
     m_uuid(uuid),
     m_isPppoe(isPppoe),
     m_info(info),
     QWidget(parent)
{
//    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(WINDOW_WIDTH,WINDOW_HEIGHT);
    centerToScreen();

    // 如果是OOBE模式，使用标准色板设置背景色样式
    if (m_pluginType == OOBE) {
        QPalette standardPalette1 = getDefaultPalette(true);
        QPalette pal = this->palette();
        pal.setColor(QPalette::Window, standardPalette1.color(QPalette::Window)); // 使用标准色板中的窗口色
        pal.setColor(QPalette::WindowText, standardPalette1.color(QPalette::WindowText)); // 使用标准色板中的窗口文本色
        this->setPalette(pal);
        //设置圆角
        this->setAttribute(Qt::WA_TranslucentBackground, true);

        // 设置圆角mask，让子控件绘制区域也被约束在圆角范围内，消除直角白边
        QPainterPath path;
        path.addRoundedRect(this->rect(), 12, 12);
        QRegion region(path.toFillPolygon().toPolygon());
        this->setMask(region);
    }

    initUI();
    loadPage();
    initComponent();
    pagePadding();

    connect(qApp, &QApplication::paletteChanged, this, &NetDetail::onPaletteChanged);

    isIpv4Ok = true;
    isIpv6Ok = true;

    qDebug() << interface << name << uuid;

    setConfirmEnable();
}

void NetDetail::onPaletteChanged()
{
    QPalette pal = qApp->palette();

    QGSettings * styleGsettings = nullptr;
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id)) {
       styleGsettings = new QGSettings(style_id);
       QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
       if(currentTheme == "ukui-default"){
           pal = lightPalette(this);
       }
    }

    this->setPalette(pal);

    setFramePalette(detailPage, pal);
    if (m_pluginType != OOBE) {
        setFramePalette(ipv4Page, pal);
        if (!m_isPppoe) {
            setFramePalette(ipv6Page, pal);
        }
    }

    if (m_pluginType != OOBE) {
        QPalette listwidget_pal(detailPage->m_listWidget->palette());
        listwidget_pal.setColor(QPalette::Base, this->palette().base().color());
        listwidget_pal.setColor(QPalette::AlternateBase, this->palette().alternateBase().color());
        detailPage->m_listWidget->setAlternatingRowColors(true);
        detailPage->m_listWidget->setPalette(listwidget_pal);
    }

    if (styleGsettings != nullptr) {
        delete styleGsettings;
        styleGsettings = nullptr;
    }
}

void NetDetail::currentRowChangeSlot(int row)
{
    stackWidget->setCurrentIndex(row);
}

void NetDetail::paintEvent(QPaintEvent *event)
{
    QPalette pal = qApp->palette();
    QColor colorPal;

    // 检测是否在OOBE模式下运行
    bool isOobeMode = (m_pluginType == OOBE);
    if (isOobeMode) {
        // 在OOBE模式下强制使用白色背景以避免黑色显示问题
        colorPal = QColor("#1D1D1D"); // 使用纯白色背景
    } else {
        colorPal = pal.color(QPalette::Background);
    }

    //设置窗体为圆角
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    painter.setBrush(colorPal);
    painter.setPen(Qt::transparent);
    auto rect = this->rect();
    painter.drawRoundedRect(rect, 12, 12);      //窗口圆角

    return QWidget::paintEvent(event);
}

void NetDetail::closeEvent(QCloseEvent *event)
{
    Q_EMIT this->detailPageClose(m_deviceName, m_uuid);
    return QWidget::closeEvent(event);
}

void NetDetail::centerToScreen()
{
    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    int desk_x = desk_rect.width();
    int desk_y = desk_rect.height();
    int x = this->width();
    int y = this->height();
    if (m_pluginType == OOBE){
        //减去插件的宽度
        this->move(desk_x / 2 - x / 2 + desk_rect.left() - 225, desk_y / 2 - y / 2 + desk_rect.top() - 50);
    }else{
        this->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());
    }
}

void NetDetail::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(9,9,14,24);

    detailPage = new DetailPage(m_isPppoe, m_pluginType == OOBE, this);
    ipv4Page = new Ipv4Page(this);
    if (m_pluginType == OOBE) {
        ipv4Page->setOobe(true);
    }
    if (!m_isPppoe) {
        ipv6Page = new Ipv6Page(this);
        if (m_pluginType == OOBE) {
            ipv6Page->setOobe(true);
        }
    }

    this->installEventFilter(this);

    centerWidget = new QWidget(this);
    bottomWidget = new QWidget(this);

    stackWidget = new QStackedWidget(centerWidget);
    stackWidget->addWidget(detailPage);
    stackWidget->addWidget(ipv4Page);
    if (!m_isPppoe) {
        stackWidget->addWidget(ipv6Page);
    }

    mainLayout->addWidget(centerWidget);
    mainLayout->addWidget(bottomWidget);

    bottomWidget->setMinimumHeight(PAGE_MIN_HEIGHT);

    pageFrame = new QFrame(this);
    QHBoxLayout *pageLayout = new QHBoxLayout(pageFrame);
    pageLayout->setSpacing(PAGE_LAYOUT_SPACING);

    onPaletteChanged();

    // TabBar
    if (m_pluginType == OOBE) {
        qDebug() << "Using OobeTabBar for OOBE mode";
        m_oobeTabBar = new OobeTabBar(this);
        m_oobeTabBar->setFixedHeight(36);
        m_oobeTabBar->addTab(tr("Detail")); //详情
        m_oobeTabBar->addTab(tr("Ipv4"));//Ipv4
        if (!m_isPppoe) {
            m_oobeTabBar->addTab(tr("Ipv6"));//Ipv6
        }
        m_oobeTabBar->setFixedWidth(LAN_TAB_WIDTH);

    } else {
        m_netTabBar = new KTabBar(KTabBarStyle::SegmentDark, this);
        m_netTabBar->addTab(tr("Detail")); //详情
        m_netTabBar->addTab(tr("Ipv4"));//Ipv4
        if (!m_isPppoe) {
            m_netTabBar->addTab(tr("Ipv6"));//Ipv6
        }
        m_netTabBar->setFixedWidth(LAN_TAB_WIDTH);
    }

    pageLayout->addStretch();
    if(m_pluginType == OOBE){
        pageLayout->addWidget(m_oobeTabBar);
        qDebug() << "oobe stylesheet:" << m_oobeTabBar->styleSheet();
            // TabBar关联选项卡页面
        connect(m_oobeTabBar, SIGNAL(currentChanged(int)), this, SLOT(currentRowChangeSlot(int)));

    } else {    
        pageLayout->addWidget(m_netTabBar);
        // TabBar关联选项卡页面
        connect(m_netTabBar, SIGNAL(currentChanged(int)), this, SLOT(currentRowChangeSlot(int)));
    }
    pageLayout->addStretch();



    confimBtn = new QPushButton(this);
    confimBtn->setFixedHeight(BUTTON_SIZE);
    confimBtn->setText(tr("Confirm"));

    cancelBtn = new QPushButton(this);
    cancelBtn->setFixedHeight(BUTTON_SIZE);
    cancelBtn->setText(tr("Cancel"));

    forgetBtn = new QPushButton(this);
    forgetBtn->setFixedHeight(BUTTON_SIZE);
    forgetBtn->setText(tr("Forget this network"));

    if (m_pluginType == OOBE){
        // Use palette colors instead of hard-coded stylesheet so the
        // buttons follow the current theme / standard palette for OOBE.
        QPalette standardPalette = getDefaultPalette(true);
        QColor btnColor = standardPalette.color(QPalette::Highlight);

        // Some styles ignore widget palettes for QPushButton backgrounds.
        // Use an explicit stylesheet built from the palette color for reliable results.
        QString btnStyle = QString("background-color:%1; color:%2; border-radius:4px;")
            .arg(btnColor.name()).arg(QString("#ffffff"));
        confimBtn->setStyleSheet(btnStyle);
        forgetBtn->setStyleSheet(btnStyle);
        cancelBtn->setStyleSheet(btnStyle);
    }

    this->setWindowIcon(QIcon::fromTheme("kylin-network"));

    QVBoxLayout *centerlayout = new QVBoxLayout(centerWidget);
    centerlayout->setContentsMargins(LAYOUT_MARGINS);
    centerlayout->addWidget(pageFrame);
    centerlayout->addWidget(stackWidget);

    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(LAYOUT_MARGINS);
    bottomLayout->setSpacing(BOTTOM_LAYOUT_SPACING);
    bottomLayout->addWidget(forgetBtn);
    bottomLayout->addStretch();
    bottomLayout->addWidget(cancelBtn);
    bottomLayout->addWidget(confimBtn);

    this->setAutoFillBackground(true);
    onPaletteChanged();
}

void NetDetail::loadPage()
{
    stackWidget->setCurrentIndex(DETAIL_PAGE_NUM);
    this->setWindowTitle(m_name);
}

void NetDetail::initComponent()
{
    connect(cancelBtn, &QPushButton::clicked, this, [=] {
        close();
    });
    connect(confimBtn, SIGNAL(clicked()), this, SLOT(on_btnConfirm_clicked()));
    connect(forgetBtn, SIGNAL(clicked()), this, SLOT(on_btnForget_clicked()));

    connect(ipv4Page, &Ipv4Page::setIpv4PageState, this, [=](bool status) {
       isIpv4Ok = status;
       setConfirmEnable();
    });

    if (!m_isPppoe) {
        connect(ipv6Page, &Ipv6Page::setIpv6PageState, this, [=](bool status) {
           isIpv6Ok = status;
           setConfirmEnable();
        });
    }
}

void NetDetail::pagePadding()
{
    //网络详情页填充
    detailPage->setSSID(m_name);
    detailPage->setProtocol(m_info.strConType);
    detailPage->setIpv4(m_info.strDynamicIpv4);
    detailPage->setIpv4Dns(m_info.strDynamicIpv4Dns);
    if (!m_isPppoe) {
        detailPage->setIpv6(m_info.strDynamicIpv6);
    }
    detailPage->setMac(m_info.strMac);
    detailPage->setBandWidth(m_info.strBandWidth);

    //ipv4页面填充
    if (m_info.ipv4ConfigType == CONFIG_IP_MANUAL) {
        ipv4Page->setIpv4Config(m_info.ipv4ConfigType);
        ipv4Page->setIpv4(m_info.strIPV4Address);
        ipv4Page->setNetMask(m_info.strIPV4NetMask);
        ipv4Page->setIpv4FirDns(m_info.strIPV4FirDns);
        ipv4Page->setIpv4SecDns(m_info.strIPV4SecDns);
        ipv4Page->setGateWay(m_info.strIPV4GateWay);
    } else {
        ipv4Page->setIpv4Config(m_info.ipv4ConfigType);
    }

    if (!m_isPppoe) {
    //ipv6页面填充
        if (m_info.ipv6ConfigType == CONFIG_IP_MANUAL) {
            ipv6Page->setIpv6Config(m_info.ipv6ConfigType);
            ipv6Page->setIpv6(m_info.strIPV6Address);
            ipv6Page->setIpv6Perfix(m_info.iIPV6Prefix);
            ipv6Page->setIpv6FirDns(m_info.strIPV6FirDns);
            ipv6Page->setIpv6SecDns(m_info.strIPV6SecDns);
            ipv6Page->setGateWay(m_info.strIPV6GateWay);
        } else {
            ipv6Page->setIpv6Config(m_info.ipv6ConfigType);
        }
    }
}

//点击了保存更改网络设置的按钮
void NetDetail::on_btnConfirm_clicked()
{
    setNetdetailSomeEnable(false);
    checkNeedUpdate();
}

//点击忘记网络
void NetDetail::on_btnForget_clicked()
{
    qDebug() << "user choose forget connection uuid = " << m_uuid;
    Q_EMIT deleteConnect(m_uuid);
    close();
}

void NetDetail::setConfirmEnable()
{
    if (isIpv4Ok && isIpv6Ok) {
        isConfirmBtnEnable = true;
    } else {
        isConfirmBtnEnable = false;
    }

    qDebug() << "setConfirmEnable "<< isConfirmBtnEnable;
    confimBtn->setEnabled(isConfirmBtnEnable);
}

void NetDetail::checkNeedUpdate()
{
    bool ipv4Change = ipv4Page->checkIsChanged(m_info);
    bool ipv6Change = false;
    if (!m_isPppoe) {
        ipv6Change = ipv6Page->checkIsChanged(m_info);
    }

    if (ipv4Change || ipv6Change) {
        Q_EMIT needUpdate(m_deviceName, m_uuid);
    } else {
        close();
    }
}

void NetDetail::updateConnect(KyConnectSetting &connetSetting)
{
    ipv4Page->updateSetting(connetSetting);
    ipv6Page->updateSetting(connetSetting);
}

bool NetDetail::eventFilter(QObject *w, QEvent *event)
{
   // 回车键触发确定按钮点击事件
   if (event->type() == QEvent::KeyPress) {
       QKeyEvent *mEvent = static_cast<QKeyEvent *>(event);
       if (mEvent->key() == Qt::Key_Enter || mEvent->key() == Qt::Key_Return) {
           if (confimBtn->isEnabled()) {
               Q_EMIT confimBtn->clicked();
           }
           return true;
       } else if (mEvent->key() == Qt::Key_Escape) {
            close();
            return true;
       }
   }
   return QWidget::eventFilter(w, event);
}
