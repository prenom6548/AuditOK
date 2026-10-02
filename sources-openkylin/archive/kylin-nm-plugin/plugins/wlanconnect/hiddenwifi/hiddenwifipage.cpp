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
#include "hiddenwifipage.h"
#include "../checkedlineedit.h"
#include "../oobecheckstyle.h"
#include <QPainter>
#include <QListView>
#include <QScrollBar>

#define  WINDOW_WIDTH  480
#define  MIN_WINDOW_HEIGHT  328
#define  PEAP_WINDOW_HEIGHT  494
#define  MAX_WINDOW_HEIGHT  540
#define  PAGE_LAYOUT_MARGINS  0,0,0,0
#define  TOP_LAYOUT_MARGINS  24,14,24,24
#define  BOTTOM_LAYOUT_MARGINS  24,24,24,24
#define  LAYOUT_SPACING  16
#define  LABEL_MIN_WIDTH  150
#define  MAX_NAME_LENGTH 32
#define  EDIT_MIN_WIDTH  274

HiddenWiFiPage::HiddenWiFiPage(QString interface, bool isLockScreen, QWidget *parent)
        : m_deviceName(interface),
            QWidget(parent),
            m_isLockScreen(isLockScreen)
{
    initUI();
    initComponent();

    setFixedWidth(WINDOW_WIDTH);
    setAttribute(Qt::WA_DeleteOnClose);

    if (isLockScreen) {
        setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明
        setWindowFlags(Qt::FramelessWindowHint);   //设置无边框窗口
        setWindowFlags(Qt::Popup);
    }

    setJoinBtnEnable();
    centerToScreen();
}

void HiddenWiFiPage::setOobe(bool on)
{
    m_isOobe = on;
    if (m_isOobe) {

        this->setStyleSheet(
            "QWidget { background-color: transparent; color: #FFFFFF; }"
            "QLabel { color: #FFFFFF; }"
            "QLineEdit, QPlainTextEdit, QSpinBox { background-color: #3a3a3a; color: #FFFFFF; border: 1px solid rgba(255,255,255,20); border-radius:4px; }"
        );

        m_centerWidget->setStyleSheet("background-color: transparent;");
        QPalette centerPal = m_centerWidget->palette();
        centerPal.setColor(QPalette::Window, Qt::transparent);
        m_centerWidget->setPalette(centerPal);

        QPalette editPal;
        editPal.setColor(QPalette::Base, QColor("#3a3a3a"));
        editPal.setColor(QPalette::Text, Qt::white);
        editPal.setColor(QPalette::PlaceholderText, QColor(200,200,200));

        m_nameEdit->setPalette(editPal);

        // Ensure labels are visible (white) in OOBE mode
        QList<QWidget*> labelList = { m_descriptionLabel, m_pwdLabel, m_emptyLabel, m_checkLabel };
        for (QWidget *w : labelList) {
            if (!w) continue;
            QPalette p = w->palette();
            p.setColor(QPalette::WindowText, Qt::white);
            p.setColor(QPalette::Text, Qt::white);
            w->setPalette(p);
            // also apply stylesheet as an extra safeguard
            w->setStyleSheet("color: #FFFFFF;");
        }
        // FixLabel (m_nameLabel, m_secuTypeLabel) may be custom; set stylesheet too
        if (m_nameLabel) m_nameLabel->setStyleSheet("color: #FFFFFF;");
        if (m_secuTypeLabel) m_secuTypeLabel->setStyleSheet("color: #FFFFFF;");

        // 对所有下拉框设置popup view，确保展开时有深色不透明背景
        QList<QComboBox*> comboList = {
            m_secuTypeCombox
        };
        for (QComboBox *cb : comboList) {
            QListView *listView = new QListView();
            listView->setAutoFillBackground(true);
            QPalette cbPal = listView->palette();
            cbPal.setColor(QPalette::Base, QColor("#3a3a3a"));
            cbPal.setColor(QPalette::Text, Qt::white);
            cbPal.setColor(QPalette::Highlight, QColor("#3790FA"));
            cbPal.setColor(QPalette::HighlightedText, Qt::white);
            listView->setPalette(cbPal);
            cb->setView(listView);
            cb->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
            cb->view()->verticalScrollBar()->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
        }

        m_nameEdit->setStyleSheet("height: 36px;background-color: #3a3a3a;");

        m_secuTypeCombox->setStyleSheet(
            "QComboBox { background-color: #3a3a3a; color: #FFFFFF; border: 1px solid rgba(255,255,255,20); border-radius:4px; height:34px}"
        );

        // If password edit isn't CheckedLineEdit, replace it so the eye icon and palette are white
        if (m_pwdEdit) {
            kdk::CheckedLineEdit *checked = qobject_cast<kdk::CheckedLineEdit *>(m_pwdEdit);
            if (!checked) {
                // preserve properties
                QString curText = m_pwdEdit->text();
                QString ph = m_pwdEdit->placeholderText();
                const QValidator *val = m_pwdEdit->validator();
                bool clearBtn = m_pwdEdit->isClearButtonEnabled();
                QWidget *parent = m_pwdEdit->parentWidget();
                QLayout *lay = parent ? parent->layout() : nullptr;

                if (lay) {
                    lay->removeWidget(m_pwdEdit);
                }
                m_pwdEdit->hide();
                delete m_pwdEdit;

                // create new CheckedLineEdit with same parent
                kdk::CheckedLineEdit *newPwd = new kdk::CheckedLineEdit(parent ? parent : this);
                newPwd->setPlaceholderText(ph);
                newPwd->setText(curText);
                if (val){
                    newPwd->setValidator(val);
                } 
                newPwd->setClearButtonEnabled(clearBtn);
                newPwd->setInputBackgroundTransparent(false);
                // ensure palette is applied
                newPwd->setPalette(editPal);

                m_pwdEdit = newPwd;
                if (lay) {
                    QGridLayout *g = qobject_cast<QGridLayout *>(lay);
                    if (g) {
                        g->addWidget(m_pwdEdit, 2, 1);
                    } else {
                        lay->addWidget(m_pwdEdit);
                    }
                }
                connect(m_pwdEdit, &LineEdit::textChanged, this, &HiddenWiFiPage::setJoinBtnEnable);
            }
        }

        // Style join button for OOBE
        QString btnStyle = QString(
            "QPushButton {"
            "    background-color: #3790FA;"
            "    color: #ffffff;"
            "    border-radius: 4px;"
            "}"
            "QPushButton:disabled {"
            "    background-color: rgba(55, 144, 250, 0.5);"
            "    color: rgba(255, 255, 255, 0.7);"
            "}"
        );
        m_joinBtn->setStyleSheet(btnStyle);
        m_cancelBtn->setStyleSheet({"background-color: #3a3a3a; color: #FFFFFF; "});
        m_cancelBtn->setFixedSize(80, 30);
        m_joinBtn->setFixedSize(80, 30);

        // 在 OOBE 模式下，默认显示为最简视图（安全类型为 None），隐藏企业/密码区域
        if (m_secuTypeCombox) {
            int noneIndex = -1;
            for (int i = 0; i < m_secuTypeCombox->count(); ++i) {
                if (m_secuTypeCombox->itemData(i).toInt() == NONE) {
                    noneIndex = i;
                    break;
                }
            }
            if (noneIndex >= 0) {
                m_secuTypeCombox->setCurrentIndex(noneIndex);
            }
        }

        // Recreate enterprise security widget with OOBE flag so internal widgets use correct palette/icons
        if (m_secuWidget) {
            // find index in center vbox and replace
            int idx = -1;
            for (int i = 0; i < m_centerVBoxLayout->count(); ++i) {
                if (m_centerVBoxLayout->itemAt(i)->widget() == m_secuWidget) {
                    idx = i;
                    break;
                }
            }
            QWidget *old = m_secuWidget;
            // disconnect old
            old->disconnect(this);
            // delete later
            m_secuWidget = new EntSecurityWidget(m_isOobe, m_isLockScreen, this);
            if (idx >= 0) {
                QWidget *placeholder = m_centerVBoxLayout->itemAt(idx)->widget();
                m_centerVBoxLayout->removeWidget(placeholder);
                // insert new at same index
                m_centerVBoxLayout->insertWidget(idx, m_secuWidget);
            }
            // reconnect signals similar to initComponent
            connect(m_secuWidget, &EntSecurityWidget::setSecuPageState, this, [=](bool status) {
               m_isSecuOk = status;
               setJoinBtnEnable();
            });
            connect(m_secuWidget, &EntSecurityWidget::eapTypeChanged, this, [=](KyEapMethodType type) {
               setWindowWidth(type);
            });
            // delete old widget
            old->deleteLater();
        }

        // Ensure the remember checkbox uses OOBE style (so its indicator/text are visible)
        if (m_rememberCheckBox) {
            m_rememberCheckBox->setProperty("oobe", true);
            m_rememberCheckBox->setStyle(new OobeCheckStyle(m_rememberCheckBox->style()));
            QPalette cbPal = m_rememberCheckBox->palette();
            cbPal.setColor(QPalette::WindowText, Qt::white);
            cbPal.setColor(QPalette::ButtonText, Qt::white);
            m_rememberCheckBox->setPalette(cbPal);
        }

        // hide enterprise/password widgets until user selects another security type
        showNone();
    }
}

HiddenWiFiPage::~HiddenWiFiPage()
{
    delete m_bottomDivider;
}

void HiddenWiFiPage::getSecuType(KySecuType &secuType)
{
    secuType = (KySecuType)m_secuTypeCombox->currentData().toInt();
}

void HiddenWiFiPage::paintEvent(QPaintEvent *event)
{
    QPalette pal = qApp->palette();
    QColor colorPal = pal.color(QPalette::Background);

    //设置窗体为圆角
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    QColor useColor = colorPal;
    if (m_isOobe) {
        useColor = QColor(0,0,0);
    }
    painter.setBrush(useColor);
    painter.setPen(Qt::transparent);
    auto rect = this->rect();
    painter.drawRoundedRect(rect, 12, 12);      //窗口圆角

    return QWidget::paintEvent(event);
}

bool HiddenWiFiPage::eventFilter(QObject *w, QEvent *event)
{
    return QWidget::eventFilter(w, event);
}

void HiddenWiFiPage::setBtnEnable(bool on)
{
    m_cancelBtn->setEnabled(on);
    m_joinBtn->setEnabled(on);
}

void HiddenWiFiPage::initUI()
{
    m_topWidget = new QWidget(this);
    m_centerWidget = new QWidget(this);
    m_bottomWidget = new QWidget(this);
    m_secuWidget = new EntSecurityWidget(false, m_isLockScreen, this);

    m_descriptionLabel = new QLabel(this);
    m_nameLabel = new FixLabel(this);
    m_secuTypeLabel = new FixLabel(this);
    m_pwdLabel = new QLabel(this);
    m_emptyLabel = new QLabel(this);
    m_checkLabel = new QLabel(this);

    m_nameEdit =new LineEdit(this);
    m_secuTypeCombox = new QComboBox(this);
    if(m_isOobe){
        m_pwdEdit = new CheckedLineEdit(this);
        static_cast<CheckedLineEdit *>(m_pwdEdit)->setInputBackgroundTransparent(false);
    }else{
        m_pwdEdit = new KPasswordEdit(this);     
    }

    m_rememberCheckBox = new QCheckBox(this);
    m_bottomDivider = new Divider(this);
//    m_showListBtn = new KBorderlessButton(this);
    m_cancelBtn =new QPushButton(this);
    m_joinBtn =new QPushButton(this);

    m_centerWidget->setFixedWidth(WINDOW_WIDTH);
    m_nameLabel->setFixedWidth(LABEL_MIN_WIDTH);
    m_secuTypeLabel->setFixedWidth(LABEL_MIN_WIDTH);
    m_pwdLabel->setFixedWidth(LABEL_MIN_WIDTH);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setWidget(m_centerWidget);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setFixedWidth(WINDOW_WIDTH);
    m_scrollArea->setWidgetResizable(true);

    m_pageLayout = new QVBoxLayout(this);
    m_pageLayout->setContentsMargins(PAGE_LAYOUT_MARGINS);
    m_pageLayout->setSpacing(0);
    m_pageLayout->addWidget(m_topWidget);
    m_pageLayout->addWidget(m_scrollArea);
    m_pageLayout->addWidget(m_bottomDivider);
    m_pageLayout->addWidget(m_bottomWidget);

    m_topLayout = new QHBoxLayout(m_topWidget);
    m_topLayout->setContentsMargins(TOP_LAYOUT_MARGINS);
    m_topLayout->setSpacing(0);
    m_topLayout->addWidget(m_descriptionLabel);
    m_topLayout->addStretch();

    m_centerVBoxLayout = new QVBoxLayout(m_centerWidget);
    m_centerVBoxLayout->setContentsMargins(24, 0, 24, 8);
    m_centerVBoxLayout->setSpacing(0);

    QWidget *centerWidget = new QWidget(this);
    QGridLayout *centerLayout = new QGridLayout(centerWidget);
    centerLayout->setContentsMargins(PAGE_LAYOUT_MARGINS);
    centerLayout->setVerticalSpacing(LAYOUT_SPACING);
    //SSID Label和输入框 第0行，第0列，第1列
    centerLayout->addWidget(m_nameLabel, 0, 0);
    centerLayout->addWidget(m_nameEdit, 0, 1);
    //安全性 Label和选项框 第1行，第0列，第1列
    centerLayout->addWidget(m_secuTypeLabel, 1, 0);
    centerLayout->addWidget(m_secuTypeCombox, 1, 1);
    //密码 Label和密码框 第2行，第0列，第1列
    centerLayout->addWidget(m_pwdLabel, 2, 0);
    centerLayout->addWidget(m_pwdEdit, 2, 1);

    //记住该网络复选框
    QWidget *checkWidget = new QWidget(this);
    QHBoxLayout *checkLayout = new QHBoxLayout(checkWidget);
    checkLayout->setContentsMargins(PAGE_LAYOUT_MARGINS);
    m_emptyLabel->setMinimumWidth(LABEL_MIN_WIDTH);
    m_rememberCheckBox->setChecked(true);
    checkLayout->addWidget(m_emptyLabel);
    checkLayout->addWidget(m_rememberCheckBox);
    checkLayout->addWidget(m_checkLabel);
    checkLayout->addStretch();

    m_centerVBoxLayout->addWidget(centerWidget);
    m_centerVBoxLayout->addWidget(m_secuWidget);
    m_centerVBoxLayout->addWidget(checkWidget);
    m_centerVBoxLayout->addStretch();

    //底部按钮
    m_bottomLayout = new QHBoxLayout(m_bottomWidget);
    m_bottomLayout->setContentsMargins(BOTTOM_LAYOUT_MARGINS);
    m_bottomLayout->setSpacing(LAYOUT_SPACING);
//    m_bottomLayout->addWidget(m_showListBtn);
    m_bottomLayout->addStretch();
    m_bottomLayout->addWidget(m_cancelBtn);
    m_bottomLayout->addWidget(m_joinBtn);

    m_secuTypeCombox->addItem(tr("None"),NONE);
    m_secuTypeCombox->addItem(tr("WPA&WPA2 Personal"),WPA_AND_WPA2_PERSONAL);
    m_secuTypeCombox->addItem(tr("WPA&WPA2 Enterprise"), WPA_AND_WPA2_ENTERPRISE);
    m_secuTypeCombox->addItem(tr("WPA3 Personal"), WPA3_PERSONAL);

     //请输入您想要加入的网络信息
    m_descriptionLabel->setText(tr("Please enter the network information"));
    QFont font = m_descriptionLabel->font();
    font.setWeight(57); //medium weight
    m_descriptionLabel->setFont(font);
    m_nameLabel->setLabelText(tr("Network name(SSID)")); //网络名(SSID)
    m_secuTypeLabel->setLabelText(tr("Security type")); //安全性
    m_pwdLabel->setText(tr("Password")); //密码
    m_checkLabel->setText(tr("Remember the Network")); //记住该网络
//    m_showListBtn->setText(tr("Show Network List")); //显示网络列表
    m_cancelBtn->setText(tr("Cancel"));
    m_joinBtn->setText(tr("Join"));

    m_nameEdit->setMaxLength(MAX_NAME_LENGTH);
    m_nameEdit->setPlaceholderText(tr("Required")); //必填
    QString hint = tr("Required");
    m_pwdEdit->setPlaceholderText(hint);

    QRegExp rx("^[A-Za-z0-9`~!@#$%^&*()_-+=<>,.\\\/]+$");
    QRegExpValidator *latitude = new QRegExpValidator(rx, this);
    m_pwdEdit->setValidator(latitude);
    m_pwdEdit->setClearButtonEnabled(false); //禁用ClearBtn按钮

    this->setWindowTitle(tr("Find and Join WLAN"));
    this->setWindowIcon(QIcon::fromTheme("kylin-network"));

    QPalette pa = m_scrollArea->palette();
    pa.setBrush(QPalette::Window, Qt::transparent);
    m_scrollArea->setPalette(pa);
    showNone();
}

void HiddenWiFiPage::centerToScreen()
{
    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    int desk_x = desk_rect.width();
    int desk_y = desk_rect.height();
    int x = this->width();
    int y = this->height();
    this->move((desk_x - x)/ 2 , (desk_y - y)/ 2);
}

void HiddenWiFiPage::initComponent()
{
    connect(m_cancelBtn, &QPushButton::clicked, this, [=] {
        close();
    });

    connect(m_joinBtn, SIGNAL(clicked()), this, SLOT(on_btnJoin_clicked()));

    //安全类型变化
    connect(m_secuTypeCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HiddenWiFiPage::onSecuTypeComboxIndexChanged);

    //按钮状态变化
    connect(m_nameEdit, &LineEdit::textChanged, this, &HiddenWiFiPage::setJoinBtnEnable);
    connect(m_pwdEdit, &LineEdit::textChanged, this, &HiddenWiFiPage::setJoinBtnEnable);
    connect(m_secuTypeCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HiddenWiFiPage::setJoinBtnEnable);
    connect(m_secuWidget, &EntSecurityWidget::setSecuPageState, this, [=](bool status) {
       m_isSecuOk = status;
       setJoinBtnEnable();
    });
    connect(m_secuWidget, &EntSecurityWidget::eapTypeChanged, this, [=](KyEapMethodType type) {
       setWindowWidth(type);
    });
}

void HiddenWiFiPage::showNone()
{
    this->setFixedHeight(MIN_WINDOW_HEIGHT);
    m_secuWidget->hide();
    m_pwdLabel->hide();
    m_pwdEdit->hide();
    m_rememberCheckBox->hide();
    m_checkLabel->hide();
}

void HiddenWiFiPage::showPsk()
{
    this->setFixedHeight(MIN_WINDOW_HEIGHT);

    m_pwdLabel->show();
    m_pwdEdit->show();
    m_rememberCheckBox->show();
    m_checkLabel->show();

    m_secuWidget->hide();
}

void HiddenWiFiPage::showEnt()
{
    KyEapMethodType eapType;
    m_secuWidget->getEnterpriseType(eapType);
    setWindowWidth(eapType);

    m_secuWidget->show();
    m_rememberCheckBox->show();
    m_checkLabel->show();

    m_pwdLabel->hide();
    m_pwdEdit->hide();
}

void HiddenWiFiPage::setJoinBtnEnable()
{
    if (m_nameEdit->text().isEmpty()) {
        qDebug() << "network name is empty";
        m_isJoinBtnEnable = false;
    } else {
        int index = m_secuTypeCombox->currentData().toInt();
        if (index == NONE) {
            if (!m_nameEdit->text().isEmpty()) {
                m_isJoinBtnEnable = true;
            }
        } else if (index == WPA_AND_WPA2_PERSONAL || index == WPA3_PERSONAL) {
            if (m_pwdEdit->text().isEmpty() || m_pwdEdit->text().length() < 8 ) {
                qDebug() << "password is empty or length < 8";
                m_isJoinBtnEnable = false;
            } else {
                m_isJoinBtnEnable = true;
            }
        } else if (index == WPA_AND_WPA2_ENTERPRISE) {
            m_isJoinBtnEnable = m_isSecuOk;
        }
    }
    qDebug() << "setJoinBtnEnable "<< m_isJoinBtnEnable;
    m_joinBtn->setEnabled(m_isJoinBtnEnable);
}

void HiddenWiFiPage::setWindowWidth(KyEapMethodType eapType)
{
    int type = eapType;
    if (type == TLS || type == FAST) {
        this->setFixedHeight(MAX_WINDOW_HEIGHT);
    } else if (type == PEAP || type == TTLS || type == LEAP || type == PWD) {
        this->setFixedHeight(PEAP_WINDOW_HEIGHT);
    }
}


void HiddenWiFiPage::on_btnJoin_clicked()
{
    qDebug() << "on_btnJoin_clicked";

    KyWirelessConnectSetting connSettingInfo;
    //基本信息
    connSettingInfo.m_ssid = m_nameEdit->text();
    connSettingInfo.setConnectName(connSettingInfo.m_ssid);
    connSettingInfo.setIfaceName(m_deviceName);
    connSettingInfo.isHidden = true;
    connSettingInfo.m_isAutoConnect = m_rememberCheckBox->isChecked();
    connSettingInfo.m_secretFlag = 0;

    //ipv4 ipv6
    connSettingInfo.setIpConfigType(IPADDRESS_V4, CONFIG_IP_DHCP);
    connSettingInfo.setIpConfigType(IPADDRESS_V6, CONFIG_IP_DHCP);

    int index = m_secuTypeCombox->currentData().toInt();
    if (index == NONE) {
        Q_EMIT connectHideNormalConnect(connSettingInfo, NONE);
    } else if (index == WPA_AND_WPA2_PERSONAL || index == WPA3_PERSONAL) {
        connSettingInfo.m_psk = m_pwdEdit->text();
        Q_EMIT connectHideNormalConnect(connSettingInfo, (KySecuType)index);
    } else if (index == WPA_AND_WPA2_ENTERPRISE) {
        KyEapMethodType eapType;
        m_secuWidget->getEnterpriseType(eapType);
        if (eapType == PEAP) {
            KyEapMethodPeapInfo info = m_secuWidget->assemblePeapInfo();
            Q_EMIT connectHidePeapConnect(info, connSettingInfo);
        } else if (eapType == TTLS) {
            KyEapMethodTtlsInfo info = m_secuWidget->assembleTtlsInfo();
            Q_EMIT connectHideTtlsConnect(info, connSettingInfo);
        }  else if (eapType == LEAP) {
            KyEapMethodLeapInfo info = m_secuWidget->assembleLeapInfo();
            Q_EMIT connectHideLeapConnect(info, connSettingInfo);
        } else if (eapType == PWD) {
            KyEapMethodPwdInfo info = m_secuWidget->assemblePwdInfo();
            Q_EMIT connectHidePwdConnect(info, connSettingInfo);
        } else if (eapType == FAST) {
            KyEapMethodFastInfo info = m_secuWidget->assembleFastInfo();
            Q_EMIT connectHideFastConnect(info, connSettingInfo);
        } else {
            qWarning() << "unsupport now!!!";
        }
    }

    close();
}

void HiddenWiFiPage::onSecuTypeComboxIndexChanged()
{
    int index = m_secuTypeCombox->currentData().toInt();
    if (index == WPA_AND_WPA2_PERSONAL || index == WPA3_PERSONAL) {
        showPsk();
    } else if (index == WPA_AND_WPA2_ENTERPRISE) {
        showEnt();
    } else if (index == NONE) {
        showNone();
    }
    centerToScreen();
}

