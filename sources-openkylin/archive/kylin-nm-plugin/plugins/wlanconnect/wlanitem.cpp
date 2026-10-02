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

#include "wlanitem.h"
#include "oobecheckstyle.h"
#include <QPainter>
#include <QRegExpValidator>
#include <QPainterPath>
#define FRAME_SPEED 150
#define LIMIT_TIME 60*1000
#define TOTAL_PAGE 8
#define RADIUS 6.0
#define LOADIMG_SIZE 16,16
#define UNEXPEND_HEIGHT  48
#define EXPEND_HEIGHT  120

#define ENABLE_BUTTON_COLOR qApp->palette().highlight().color()
#define UNABLE_BUTTON_COLOR qApp->palette().button().color()

WlanItem::WlanItem(PluginType type, QWidget *parent)
    : type(type), QFrame(parent)
{
    this->setFixedSize(404, UNEXPEND_HEIGHT);
    QVBoxLayout *m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(16,0,0,0);
    m_mainLayout->setSpacing(0);

    //icon + name
    m_nameFrame = new QFrame(this);
    QHBoxLayout *lanLyt = new QHBoxLayout(m_nameFrame);
    lanLyt->setContentsMargins(0,8,16,4);
    lanLyt->setSpacing(0);
    radioBtn = new RadioItemButton(this);
    // In OOBE mode, force radio icon to render white for better contrast
    if (type == OOBE) {
        radioBtn->setOobe(true);
    }
    titileLabel = new FixLabel(this);
    titileLabel->setMaximumWidth(MAX_WIDTH_DEACTIVATED);
    // m_freq = new FreqLabel(m_nameFrame);
    // m_freq->setEnabled(false);
    m_lbLoadUp = new QLabel(m_nameFrame);
    m_lbLoadDown = new QLabel(m_nameFrame);
    m_lbLoadDownImg = new QLabel(m_nameFrame);
    m_lbLoadUpImg = new QLabel(m_nameFrame);
    m_lbLoadUp->hide();
    m_lbLoadDown->hide();
    m_lbLoadDownImg->hide();
    m_lbLoadUpImg->hide();
    QFont font;
    font.setPointSize(10);
    m_lbLoadUp->setFont(font);
    m_lbLoadDown->setFont(font);
    m_lbLoadUp->setText("0KB/s");
    m_lbLoadDown->setText("0KB/s");
    m_lbLoadDownImg->setPixmap(QPixmap(QLatin1String("://res/icon/load-down.png")));
    m_lbLoadUpImg->setPixmap(QPixmap(QLatin1String("://res/icon/load-up.png")));
    m_lbLoadDownImg->setFixedSize(LOADIMG_SIZE);
    m_lbLoadUpImg->setFixedSize(LOADIMG_SIZE);

    lanLyt->setAlignment(Qt::AlignVCenter);
    lanLyt->addWidget(radioBtn);
    lanLyt->addSpacing(10);
    lanLyt->addWidget(titileLabel,Qt::AlignLeft);
    lanLyt->addSpacing(6);
    // lanLyt->addWidget(m_freq);
    lanLyt->addStretch();
    lanLyt->addSpacing(8);
    lanLyt->addWidget(m_lbLoadUpImg);
    lanLyt->addWidget(m_lbLoadUp);
    lanLyt->addSpacing(2);
    lanLyt->addWidget(m_lbLoadDownImg);
    lanLyt->addWidget(m_lbLoadDown);

    if (type != SIMPLE) {
        infoLabel = new InfoButton(this);
        infoLabel->setIsOobe(type == OOBE);  // 设置是否为OOBE模式
        lanLyt->addSpacing(8);
        lanLyt->addWidget(infoLabel);
        connect(infoLabel, &InfoButton::released, this, &WlanItem::infoButtonClick);
        infoLabel->hide();
    }

    m_expendFrame = new QFrame(this);
    QVBoxLayout *expendnLyt = new QVBoxLayout(m_expendFrame);
    expendnLyt->setContentsMargins(40, 0, 16, 0);
    expendnLyt->setSpacing(0);

    //pwdline + connect Button
    m_pwdFrame = new QFrame(m_expendFrame);
    m_pwdFrameLyt = new QHBoxLayout(m_pwdFrame);
    m_pwdFrameLyt->setContentsMargins(0,0,0,0);
    m_pwdFrameLyt->setSpacing(0);
    m_pwdFrame->setLayout(m_pwdFrameLyt);

    if (type == OOBE) {
        m_pwdLineEdit = new CheckedLineEdit(m_pwdFrame);
    } else {
        m_pwdLineEdit = new KPasswordEdit(m_pwdFrame);
    }
    m_pwdLineEdit->setState(Ordinary);
//    m_pwdLineEdit->setEchoModeBtnVisible(true);
    m_pwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);

    m_pwdLineEdit->setUseCustomPalette(true);
    QPalette pal = this->palette();
    m_pwdLineEdit->setPalette(pal);

    QRegExp rx("^[A-Za-z0-9`~!@#$%^&*()_-+=<>,.\\\/]+$");
    QRegExpValidator *latitude = new QRegExpValidator(rx, this);
    m_pwdLineEdit->setValidator(latitude);

    m_pwdLineEdit->installEventFilter(this);

    connect(m_pwdLineEdit, &KPasswordEdit::textChanged, this, &WlanItem::onPwdEditorTextChanged);
    m_pwdLineEdit->setFixedSize(228, 36);
    m_pwdLineEdit->setEchoMode(QLineEdit::EchoMode::Password);
    m_pwdFrameLyt->addWidget(m_pwdLineEdit);
    m_pwdFrameLyt->addSpacing(8);

    m_connectButton = new QPushButton(m_pwdFrame);
    m_connectButton->setFixedSize(96, 36);
    m_connectButton->setText(tr("Connect"));
    m_connectButton->setEnabled(false);
    if (type == OOBE) {
        // 更高的透明度，更好的可见性
        m_connectButton->setStyleSheet("background-color: rgba(255, 255, 255, 60); color: white; border: none; border-radius: 4px;");
    }
    connect(m_connectButton, &QPushButton::clicked, this, &WlanItem::onConnectButtonClicked);
    m_pwdFrameLyt->addWidget(m_connectButton);

    m_passwordErrorTxt = new QLabel(m_expendFrame);
    m_passwordErrorTxt->setText(tr("Password error"));
    m_passwordErrorTxt->setStyleSheet("color: red;");
    m_passwordErrorTxt->hide();

    //自动连接选择区域UI
    m_autoConnectFrame = new QFrame(m_expendFrame);

    m_autoConnectFrameLyt = new QHBoxLayout(m_autoConnectFrame);
    m_autoConnectFrameLyt->setContentsMargins(0,8,150,8);
    m_autoConnectFrameLyt->setSpacing(0);
    m_autoConnectFrame->setLayout(m_autoConnectFrameLyt);

    m_autoConnectCheckBox = new QCheckBox(m_autoConnectFrame);
    m_autoConnectCheckBox->setText(tr("Auto Connect"));
    m_autoConnectCheckBox->setChecked(true);
    m_autoConnectCheckBox->setFixedSize(17, 17);
    // 标记为 OOBE 以便全局 OobeCheckStyle 绘制 indicator（蓝底 + 白勾）
    if (type == OOBE) {
        m_autoConnectCheckBox->setProperty("oobe", true);
        m_autoConnectCheckBox->setStyle(new OobeCheckStyle(m_autoConnectCheckBox->style()));
    }
    m_autoConnectFrameLyt->addWidget(m_autoConnectCheckBox);
    m_autoConnectFrameLyt->addSpacing(8);

    m_autoConnectLabel = new QLabel(m_autoConnectFrame);
    m_autoConnectLabel->setText(tr("Auto Connect"));
    // 在OOBE模式下确保标签可见
    if (type == OOBE) {
        qDebug() << "====Setting auto connect label palette for OOBE mode==" << OOBE <<"=="<< type;
        QPalette labelPalette = m_autoConnectLabel->palette();
        labelPalette.setColor(QPalette::WindowText, QColor(255, 255, 255)); // 白色文字
        m_autoConnectLabel->setPalette(labelPalette);
    }
    m_autoConnectFrameLyt->addWidget(m_autoConnectLabel);

    expendnLyt->addWidget(m_pwdFrame);
    expendnLyt->addWidget(m_passwordErrorTxt);
    expendnLyt->addWidget(m_autoConnectFrame);

    m_mainLayout->addWidget(m_nameFrame);
    m_mainLayout->addWidget(m_expendFrame);

    m_expendFrame->hide();

    loadIcons.append(QIcon::fromTheme("ukui-loading-1-symbolic"));
    loadIcons.append(QIcon::fromTheme("ukui-loading-2-symbolic"));
    loadIcons.append(QIcon::fromTheme("ukui-loading-3-symbolic"));
    loadIcons.append(QIcon::fromTheme("ukui-loading-4-symbolic"));
    loadIcons.append(QIcon::fromTheme("ukui-loading-5-symbolic"));
    loadIcons.append(QIcon::fromTheme("ukui-loading-6-symbolic"));
    loadIcons.append(QIcon::fromTheme("ukui-loading-7-symbolic"));
    waitTimer = new QTimer(this);
    connect(waitTimer, &QTimer::timeout, this, &WlanItem::updateIcon);

    m_menu = new QMenu(this);//右键菜单
    m_menu->setWindowFlag(Qt::X11BypassWindowManagerHint);
    connect(m_menu, &QMenu::triggered, this, &WlanItem::itemClick);

    m_pwdLineEdit->installEventFilter(this);
    m_autoConnectCheckBox->installEventFilter(this);
}

void WlanItem::updateIcon()
{
    if (currentIconIndex > 6) {
        currentIconIndex = 0;
    }
    radioBtn->setButtonIcon(loadIcons.at(currentIconIndex));
    currentIconIndex ++;
}

void WlanItem::showEnterPricePage(QString devName, QWidget *widget)
{
    if (nullptr != m_enterPirsePage) {
        m_enterPirsePage->show();
    } else {
        QString ssid = getName();
    bool isOobe = (type == OOBE);
    bool isLockScreen = (type == SIMPLE);
    m_enterPirsePage = new EnterpriseWlanPage(ssid, devName, isOobe, isLockScreen, widget);
        connect(m_enterPirsePage, &EnterpriseWlanPage::destroyed, [=](){
            m_enterPirsePage->disconnect(this);
            m_enterPirsePage = nullptr;
        });
        connect(m_enterPirsePage, &EnterpriseWlanPage::connectPeapConnect, this, &WlanItem::connectPeapConnect);
        connect(m_enterPirsePage, &EnterpriseWlanPage::connectTtlsConnect, this, &WlanItem::connectTtlsConnect);
        connect(m_enterPirsePage, &EnterpriseWlanPage::connectLeapConnect, this, &WlanItem::connectLeapConnect);
        connect(m_enterPirsePage, &EnterpriseWlanPage::connectPwdConnect, this, &WlanItem::connectPwdConnect);
        connect(m_enterPirsePage, &EnterpriseWlanPage::connectFastConnect, this, &WlanItem::connectFastConnect);
        m_enterPirsePage->show();
    }
}

void WlanItem::startLoading()
{
    waitTimer->start(FRAME_SPEED);
    m_loading = true;
}

void WlanItem::stopLoading(){
    waitTimer->stop();
    m_loading = false;
}

void WlanItem::mousePressEvent(QMouseEvent *event)
{
    return QFrame::mousePressEvent(event);
}
void WlanItem::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton) {
        if (!m_menu) {
            return QFrame::mouseReleaseEvent(event);
        }

        m_menu->clear();
        if (m_isAcitve || m_loading) {
            m_menu->addAction(new QAction(tr("Disconnect"), this));
        } else if (!m_isAcitve && !m_loading) {
            m_menu->addAction(new QAction(tr("Connect"), this));
        }
        m_menu->move(cursor().pos());
        m_menu->show();

    } else {
        if (!m_isAcitve && !m_loading) {
            Q_EMIT itemClick();
        }
    }
    return QFrame::mouseReleaseEvent(event);
}

void WlanItem::enterEvent(QEvent *event)
{
    m_isIn = true;
    update();
    return QFrame::enterEvent(event);
}

void WlanItem::leaveEvent(QEvent *event)
{
    m_isIn = false;
    update();
    return QFrame::leaveEvent(event);
}

void WlanItem::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        if (m_expendFrame && m_expendFrame->isVisible() && m_pwdLineEdit->text().length() >= 8) {
            onConnectButtonClicked();
        }
    }
    return QFrame::keyPressEvent(event);
}

void WlanItem::paintEvent(QPaintEvent *event)
{
    QPalette pal = this->palette();

    QPainter painter(this);
    painter.setRenderHint(QPainter:: Antialiasing, true);  //设置渲染,启动反锯齿
    painter.setPen(Qt::NoPen);

    // Default background behavior for non-OOBE
    if (type != OOBE) {
        QColor color(240, 240, 240);
        if (m_isFocusIn) {
            color.setAlphaF(1);
            painter.setBrush(color);
        } else if (m_isIn) {
            color.setAlphaF(0.39);
            painter.setBrush(color);
        } else {
            painter.setBrush(pal.color(QPalette::Base));
        }
        QRect rect = this->rect();
        QPainterPath path;
        path.addRoundedRect(rect, RADIUS, RADIUS);
        painter.drawPath(path);
    } else {
        // OOBE: normally transparent background. When expanded (selected), draw
        // a translucent rounded selection box to match design.
        QRect rect = this->rect();
        QPainterPath path;
        path.addRoundedRect(rect, RADIUS, RADIUS);
        // base is transparent
        painter.setBrush(Qt::transparent);
        painter.drawPath(path);

        if (m_isIn && !m_isExpand) {
            QColor hoverFill(255, 255, 255, 60);
            painter.setPen(Qt::NoPen);
            painter.setBrush(hoverFill);
            QRectF hoverRect = rect.adjusted(0.5, 0.5, -1.5, -1.5);
            QPainterPath hoverPath; hoverPath.addRoundedRect(hoverRect, RADIUS, RADIUS);
            painter.drawPath(hoverPath);
        }

        if (m_isExpand) {
            // translucent white fill - increased opacity for better visibility of controls
            QColor fillColor(255,255,255,60); // more opaque white overlay for better control visibility
            painter.setPen(Qt::NoPen);
            painter.setBrush(fillColor);
            QRectF inner = rect.adjusted(0, 0, -1, -1);
            QPainterPath innerPath;
            innerPath.addRoundedRect(inner, RADIUS, RADIUS);
            painter.drawPath(innerPath);

            // soft white border
            QPen pen(QColor(255,255,255,120)); // brighter border
            pen.setWidthF(1.0);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            QRectF borderRect = rect.adjusted(0.5, 0.5, -1.5, -1.5);
            QPainterPath borderPath;
            borderPath.addRoundedRect(borderRect, RADIUS, RADIUS);
            painter.drawPath(borderPath);
        }
    }
    return QFrame::paintEvent(event);
}

bool WlanItem::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_pwdLineEdit && event->type() == QEvent::FocusIn) {
        Q_EMIT passwordFocusIn();
    }
    if (watched == m_pwdLineEdit) {
        if (event->type() == QEvent::FocusIn) {
            m_isFocusIn = true;
        } else if (event->type() == QEvent::FocusOut) {
            m_isFocusIn = false;
        }
    }
    return QFrame::eventFilter(watched,event);
}

void WlanItem::setExpend(bool enable)
{
    if (enable) {
        this->setFixedHeight(EXPEND_HEIGHT);
        if(!m_passwordErrorTxt->isHidden()){
            this->setFixedHeight(EXPEND_HEIGHT + 20);
        }
        m_expendFrame->show();
        m_pwdLineEdit->setFocus();
        m_isExpand = true;
    } else {
        m_expendFrame->hide();
        this->setFixedHeight(UNEXPEND_HEIGHT);
        m_pwdLineEdit->clear();
        m_pwdLineEdit->setState(Ordinary);
        m_isExpand = false;
        m_passwordErrorTxt->hide();
    }
}

bool WlanItem::getExpend()
{
    return !m_expendFrame->isHidden();
}

void WlanItem::onPwdEditorTextChanged()
{
    QPalette btnPal;
    if (m_pwdLineEdit->text().length() < 8) {
        m_connectButton->setEnabled(false);
        // 后续记得还原，应该是主题色板
        btnPal.setColor(QPalette::Button, UNABLE_BUTTON_COLOR);
        m_connectButton->setPalette(btnPal);
        if(type == OOBE){
            m_connectButton->setStyleSheet("background-color: rgba(255, 255, 255, 30);");
        }
    } else {
        m_connectButton->setEnabled(true);
        btnPal.setColor(QPalette::Button, ENABLE_BUTTON_COLOR);
        m_connectButton->setPalette(btnPal);
        if(type == OOBE){
            m_connectButton->setStyleSheet("background-color: #3790FA;");
        }
    }
}

void WlanItem::onConnectButtonClicked()
{
    if (!m_agentRequestId.isEmpty()) {
        // 有agent请求，发射agent专用信号
        Q_EMIT connectButtonClickForAgent(m_agentRequestId, m_pwdLineEdit->text(), m_autoConnectCheckBox->isChecked());
        // 清空requestId，避免重复使用
        m_agentRequestId.clear();
    }else{
    	Q_EMIT connectButtonClick(m_pwdLineEdit->text(), m_autoConnectCheckBox->isChecked());
    }
}
