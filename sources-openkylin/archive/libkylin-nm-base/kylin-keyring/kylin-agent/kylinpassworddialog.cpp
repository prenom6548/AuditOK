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

#include "kylinpassworddialog.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QApplication>

#define CLOSE_TIME 30*1000

#if 0
static QPalette lightPalette(QWidget *widget)
{
    auto palette = widget->palette();
    //ukui-light palette UKUI3.1
    QColor windowText_at(38, 38, 38),
           windowText_iat(38, 38, 38),
           windowText_dis(166, 166, 166),
           button_at(230, 230, 230),
           button_iat(230, 230, 230),
           button_dis(233, 233, 233),
           light_at(255, 255, 255),
           light_iat(255, 255, 255),
           light_dis(242, 242, 242),
           midlight_at(218, 218, 218),
           midlight_iat(218, 218, 218),
           midlight_dis(230, 230, 230),
           dark_at(77, 77, 77),
           dark_iat(77, 77, 77),
           dark_dis(64, 64, 64),
           mid_at(115, 115, 115),
           mid_iat(115, 115, 115),
           mid_dis(102, 102, 102),
           text_at(38, 38, 38),
           text_iat(38, 38, 38),
           text_dis(140, 140, 140),
           brightText_at(89, 89, 89),
           brightText_iat(89, 89, 89),
           brightText_dis(77, 77, 77),
           buttonText_at(38, 38, 38),
           buttonText_iat(38, 38, 38),
           buttonText_dis(179, 179, 179),
           base_at(255, 255, 255),
           base_iat(245, 245, 245),
           base_dis(237, 237, 237),
           window_at(245, 245, 245),
           window_iat(237, 237, 237),
           window_dis(230, 230, 230),
           shadow_at(0, 0, 0, 16),
           shadow_iat(0, 0, 0, 16),
           shadow_dis(0, 0, 0, 21),
//           shadow_at(214, 214, 214),
//           shadow_iat(214, 214, 214),
//           shadow_dis(201, 201, 201),
           highLight_at(55, 144, 250),
           highLight_iat(55, 144, 250),
           highLight_dis(233, 233, 233),
           highLightText_at(255, 255, 255),
           highLightText_iat(255, 255, 255),
           highLightText_dis(179, 179, 179),
           link_at(55, 144, 250),
           link_iat(55, 144, 250),
           link_dis(55, 144, 250),
           linkVisited_at(114, 46, 209),
           linkVisited_iat(114, 46, 209),
           linkVisited_dis(114, 46, 209),
           alternateBase_at(245, 245, 245),
           alternateBase_iat(245, 245, 245),
           alternateBase_dis(245, 245, 245),
           noRale_at(240, 240, 240),
           noRole_iat(240, 240, 240),
           noRole_dis(217, 217, 217),
           toolTipBase_at(255, 255, 255),
           toolTipBase_iat(255, 255, 255),
           toolTipBase_dis(255, 255, 255),
           toolTipText_at(38, 38, 38),
           toolTipText_iat(38, 38, 38),
           toolTipText_dis(38, 38, 38),
           placeholderText_at(38, 38, 38),
           placeholderText_iat(38, 38, 38),
           placeholderText_dis(38, 38, 38);



    palette.setColor(QPalette::Active, QPalette::WindowText, windowText_at);
    palette.setColor(QPalette::Inactive, QPalette::WindowText, windowText_iat);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, windowText_dis);

    palette.setColor(QPalette::Active, QPalette::Button, button_at);
    palette.setColor(QPalette::Inactive, QPalette::Button, button_iat);
    palette.setColor(QPalette::Disabled, QPalette::Button, button_dis);

    palette.setColor(QPalette::Active, QPalette::Light, light_at);
    palette.setColor(QPalette::Inactive, QPalette::Light, light_iat);
    palette.setColor(QPalette::Disabled, QPalette::Light, light_dis);

    palette.setColor(QPalette::Active, QPalette::Midlight, midlight_at);
    palette.setColor(QPalette::Inactive, QPalette::Midlight, midlight_iat);
    palette.setColor(QPalette::Disabled, QPalette::Midlight, midlight_dis);

    palette.setColor(QPalette::Active, QPalette::Dark, dark_at);
    palette.setColor(QPalette::Inactive, QPalette::Dark, dark_iat);
    palette.setColor(QPalette::Disabled, QPalette::Dark, dark_dis);

    palette.setColor(QPalette::Active, QPalette::Mid, mid_at);
    palette.setColor(QPalette::Inactive, QPalette::Mid, mid_iat);
    palette.setColor(QPalette::Disabled, QPalette::Mid, mid_dis);

    palette.setColor(QPalette::Active, QPalette::Text, text_at);
    palette.setColor(QPalette::Inactive, QPalette::Text, text_iat);
    palette.setColor(QPalette::Disabled, QPalette::Text, text_dis);

    palette.setColor(QPalette::Active, QPalette::BrightText, brightText_at);
    palette.setColor(QPalette::Inactive, QPalette::BrightText, brightText_iat);
    palette.setColor(QPalette::Disabled, QPalette::BrightText, brightText_dis);

    palette.setColor(QPalette::Active, QPalette::ButtonText, buttonText_at);
    palette.setColor(QPalette::Inactive, QPalette::ButtonText, buttonText_iat);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, buttonText_dis);

    palette.setColor(QPalette::Active, QPalette::Base, base_at);
    palette.setColor(QPalette::Inactive, QPalette::Base, base_iat);
    palette.setColor(QPalette::Disabled, QPalette::Base, base_dis);

    palette.setColor(QPalette::Active, QPalette::Window, window_at);
    palette.setColor(QPalette::Inactive, QPalette::Window, window_iat);
    palette.setColor(QPalette::Disabled, QPalette::Window, window_dis);

    palette.setColor(QPalette::Active, QPalette::Shadow, shadow_at);
    palette.setColor(QPalette::Inactive, QPalette::Shadow, shadow_iat);
    palette.setColor(QPalette::Disabled, QPalette::Shadow, shadow_dis);

    palette.setColor(QPalette::Active, QPalette::Highlight, highLight_at);
    palette.setColor(QPalette::Inactive, QPalette::Highlight, highLight_iat);
    palette.setColor(QPalette::Disabled, QPalette::Highlight, highLight_dis);

    palette.setColor(QPalette::Active, QPalette::HighlightedText, highLightText_at);
    palette.setColor(QPalette::Inactive, QPalette::HighlightedText, highLightText_iat);
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, highLightText_dis);

    palette.setColor(QPalette::Active, QPalette::Link, link_at);
    palette.setColor(QPalette::Inactive, QPalette::Link, link_iat);
    palette.setColor(QPalette::Disabled, QPalette::Link, link_dis);

    palette.setColor(QPalette::Active, QPalette::LinkVisited, linkVisited_at);
    palette.setColor(QPalette::Inactive, QPalette::LinkVisited, linkVisited_iat);
    palette.setColor(QPalette::Disabled, QPalette::LinkVisited, linkVisited_dis);

    palette.setColor(QPalette::Active, QPalette::AlternateBase, alternateBase_at);
    palette.setColor(QPalette::Inactive, QPalette::AlternateBase, alternateBase_iat);
    palette.setColor(QPalette::Disabled, QPalette::AlternateBase, alternateBase_dis);

    palette.setColor(QPalette::Active, QPalette::NoRole, noRale_at);
    palette.setColor(QPalette::Inactive, QPalette::NoRole, noRole_iat);
    palette.setColor(QPalette::Disabled, QPalette::NoRole, noRole_dis);

    palette.setColor(QPalette::Active, QPalette::ToolTipBase, toolTipBase_at);
    palette.setColor(QPalette::Inactive, QPalette::ToolTipBase, toolTipBase_iat);
    palette.setColor(QPalette::Disabled, QPalette::ToolTipBase, toolTipBase_dis);

    palette.setColor(QPalette::Active, QPalette::ToolTipText, toolTipText_at);
    palette.setColor(QPalette::Inactive, QPalette::ToolTipText, toolTipText_iat);
    palette.setColor(QPalette::Disabled, QPalette::ToolTipText, toolTipText_dis);

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
    palette.setColor(QPalette::Active, QPalette::PlaceholderText, placeholderText_at);
    palette.setColor(QPalette::Inactive, QPalette::PlaceholderText, placeholderText_iat);
    palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, placeholderText_dis);
#endif

    return palette;

}
#endif

RowWidget::RowWidget(QWidget *widget, QWidget *parent) : valueWidget(widget), QWidget(parent)
{
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    this->setLayout(mainLayout);
    keyLabel = new QLabel(this);
    keyLabel->setMaximumWidth(87);
    valueWidget->setMinimumWidth(280);
    mainLayout->addWidget(keyLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(valueWidget);
}

RowWidget::~RowWidget()
{

}

void RowWidget::setKey(QString text)
{
    this->keyLabel->setText(text);
}

KyPasswordDialog::KyPasswordDialog(QString dialogType, QString connectionID, QWidget *parent)
    :connectionID(connectionID),dialogType(dialogType),QDialog(parent)
{
    this->setAttribute(Qt::WA_QuitOnClose, false);
//    this->setWindowFlags(Qt::Dialog);
//    this->setWindowIcon(QIcon::fromTheme("kylin-network"));
//    this->setWindowTitle(connectionID);

    initUI(dialogType);
    QPalette pal = parent->palette();
    pal.setColor(QPalette::ButtonText, Qt::black);
    m_sdkPasswdEdit->setPalette(pal);
    m_sdkPasswdEdit->setEnabled(true);
    m_sdkPasswdEdit->installEventFilter(this);
    this->setWindowFlags(Qt::MSWindowsFixedSizeDialogHint | Qt::Window | Qt::FramelessWindowHint);

#if 0
    connect(qApp, &QApplication::paletteChanged, this, &KyPasswordDialog::onPaletteChanged);
    onPaletteChanged();
#endif
}

KyPasswordDialog::~KyPasswordDialog(){

}

void KyPasswordDialog::InitConnect()
{
    connect(m_connectBtn, &QPushButton::clicked, this, &KyPasswordDialog::login);
    connect(m_exitBtn, &QPushButton::clicked, this, &KyPasswordDialog::reject);
    connect(m_sdkPasswdEdit, &KPasswordEdit::textChanged, this, &KyPasswordDialog::onTextChanged);
    if (m_usrLineEdit != nullptr) {
        connect(m_usrLineEdit, &QLineEdit::textChanged, this, &KyPasswordDialog::onTextChanged);
    }
    QTimer::singleShot(CLOSE_TIME, this, [=](){
        reject();
    });
}

void KyPasswordDialog::initUI(QString dialogType)
{
#define MAIN_LAYOUT_MARGINS 24,24,24,24
#define DEFAULT_LAYOUT_MARGINS 0,0,0,0

#define DEFAULT_LAYOUT_SPACE 24
#define LESS_LAYOUT_SPACE 16

#define DEFAULT_BTN_FIXSIZE 112,56
#define DEFAULT_FIXHEIGHT 48
#define DEFAULT_PICTURE_FIXSIZE 48,48
#define DEFAULT_LABEL_WIDTH 328

#define DEFAULT_EDIT_WIDTH 278
#define WPA2_EDIT_WIDTH 270

#define DEFAULT_PAGE_SIZE 446,286
#define WPA2_PAGE_SIZE 446,342

#define LABEL_WIDTH 48
#define MIN_EDIT_WIDTH 140

    //设置字体大小
    QFont fontTitle;
    fontTitle.setPointSizeF(13.5);
    fontTitle.setWeight(QFont::Bold);
    QFontMetrics titleFontMetrics(fontTitle);
    QFont fontNormal;
    fontNormal.setPointSizeF(11.5);
    fontNormal.setWeight(QFont::Normal);
    QFontMetrics fontMetrics(fontNormal);

    //主布局
    QVBoxLayout *mainVBoxLayout = new QVBoxLayout(this);
    mainVBoxLayout->setContentsMargins(MAIN_LAYOUT_MARGINS);
    mainVBoxLayout->setSpacing(LESS_LAYOUT_SPACE);
    this->setLayout(mainVBoxLayout);

    //不包含按钮的横向布局
    QHBoxLayout *hBoxLayout =  new QHBoxLayout(this);
    hBoxLayout->setContentsMargins(DEFAULT_LAYOUT_MARGINS);
    hBoxLayout->setSpacing(DEFAULT_LAYOUT_SPACE);

    //图标
    QLabel *picture = new QLabel(this);
    picture->setContentsMargins(DEFAULT_LAYOUT_MARGINS);
    picture->setFixedSize(DEFAULT_PICTURE_FIXSIZE);
    picture->setPixmap(QPixmap(":/authorize.svg"));

    //标题
    QLabel *titleLabel = new QLabel(this);
    QString titleLabelText(tr("Wi-Fi authentication required"));
    titleLabel->setFont(fontTitle);
    titleLabel->setFixedWidth(DEFAULT_LABEL_WIDTH);
    //标题文本省略处理
    titleLabel->setText(titleFontMetrics.elidedText(titleLabelText, Qt::ElideRight, DEFAULT_LABEL_WIDTH));
    if (titleFontMetrics.width(titleLabelText) > DEFAULT_LABEL_WIDTH) {
        titleLabel->setToolTip(titleLabelText);
    }

    //提示信息
    QLabel *tipLabel = new QLabel(this);
    tipLabel->setFixedWidth(DEFAULT_LABEL_WIDTH);
    tipLabel->setAlignment(Qt::AlignTop);
    tipLabel->setWordWrap(true);
    QString text = tr("Access wireless networks");
    text += " " + connectionID + " ";
    text += tr("needs password or encryption key");
    tipLabel->setText(text);
    tipLabel->setFont(fontNormal);

    //密码输入框
    QLabel *pwdLabel = new QLabel(this);
    QString pwdLabelText(tr("password"));
    pwdLabel->setFont(fontNormal);
    m_sdkPasswdEdit = new KPasswordEdit(this);
    m_sdkPasswdEdit->setUseCustomPalette(true);
    if (dialogType != "") {
        this->setFixedSize(WPA2_PAGE_SIZE);
        m_sdkPasswdEdit->setFixedWidth(WPA2_EDIT_WIDTH);
        pwdLabel->setFixedWidth(LABEL_WIDTH);
    } else {
        this->setFixedSize(DEFAULT_PAGE_SIZE);
        m_sdkPasswdEdit->setMaximumWidth(DEFAULT_EDIT_WIDTH);
        m_sdkPasswdEdit->setMinimumWidth(MIN_EDIT_WIDTH);
    }
    m_sdkPasswdEdit->setClearButtonEnabled(false);
    m_sdkPasswdEdit->setState(LoginFailed);
    m_sdkPasswdEdit->setAttribute(Qt::WA_InputMethodEnabled, true);
    QRegExp rx("^[A-Za-z0-9`~!@#$%^&*()_-+=<>,.\\\/]+$");
    QRegExpValidator *latitude = new QRegExpValidator(rx, this);
    m_sdkPasswdEdit->setValidator(latitude);

    //密码输入框布局
    QHBoxLayout *pwdLayout = new QHBoxLayout(this);
    pwdLayout->setContentsMargins(DEFAULT_LAYOUT_MARGINS);
    pwdLayout->setSpacing(8);
    pwdLayout->addWidget(pwdLabel);
    pwdLayout->addWidget(m_sdkPasswdEdit);

    pwdLabel->setText(fontMetrics.elidedText(pwdLabelText, Qt::ElideRight, pwdLabel->width()));
    if (fontMetrics.width(pwdLabelText) > pwdLabel->width()) {
        pwdLabel->setToolTip(pwdLabelText);
    }

    //不包含图标的纵向布局
    QVBoxLayout *contentLayout = new QVBoxLayout(this);
    contentLayout->setContentsMargins(DEFAULT_LAYOUT_MARGINS);
    contentLayout->setSpacing(LESS_LAYOUT_SPACE);
    contentLayout->setAlignment(Qt::AlignTop);
    contentLayout->addWidget(titleLabel);
    contentLayout->addWidget(tipLabel);
    if (dialogType != "") {
        //WPA2企业加密方式用户名输入框
        QLabel *userNameLabel = new QLabel(this);
        QString usernameLabelText(tr("user name"));
        userNameLabel->setFont(fontNormal);
        userNameLabel->setFixedWidth(LABEL_WIDTH);
        userNameLabel->setText(fontMetrics.elidedText(usernameLabelText, Qt::ElideRight, LABEL_WIDTH));
        if (fontMetrics.width(usernameLabelText) > LABEL_WIDTH) {
            userNameLabel->setToolTip(usernameLabelText);
        }
        m_usrLineEdit = new QLineEdit(this);
        m_usrLineEdit->setFixedWidth(WPA2_EDIT_WIDTH);

        //用户名输入框布局
        QHBoxLayout *userNameLayout = new QHBoxLayout(this);
        userNameLayout->setContentsMargins(DEFAULT_LAYOUT_MARGINS);
        userNameLayout->setSpacing(8);
        userNameLayout->addWidget(userNameLabel);
        userNameLayout->addWidget(m_usrLineEdit);
        contentLayout->addLayout(userNameLayout);
    }
    contentLayout->addLayout(pwdLayout);
    contentLayout->addStretch();

    //连接按钮
    m_connectBtn = new QPushButton(this);
    m_connectBtn->setText(tr("Connect"));
    m_connectBtn->setFont(fontNormal);

    //取消按钮
    m_exitBtn = new QPushButton(this);
    m_exitBtn->setText(tr("Cancel"));
    m_exitBtn->setFont(fontNormal);

    //按钮布局
    QHBoxLayout *btnLayout = new QHBoxLayout(this);
    btnLayout->setContentsMargins(DEFAULT_LAYOUT_MARGINS);
    btnLayout->setSpacing(LESS_LAYOUT_SPACE);
    btnLayout->addStretch();
    btnLayout->addWidget(m_exitBtn);
    btnLayout->addWidget(m_connectBtn);

    hBoxLayout->addWidget(picture);
    hBoxLayout->setAlignment(picture, Qt::AlignTop);
    hBoxLayout->addLayout(contentLayout);

    mainVBoxLayout->addLayout(hBoxLayout);
    mainVBoxLayout->addLayout(btnLayout);
}

QString KyPasswordDialog::getPassword()
{
    return m_sdkPasswdEdit->text();
}

void KyPasswordDialog::setPassword(QString password)
{
    m_sdkPasswdEdit->setText(password);
    m_sdkPasswdEdit->setFocus();
}

QString KyPasswordDialog::getUserName()
{
    return m_usrLineEdit->text();
}

void KyPasswordDialog::setUserName(QString name)
{
    m_usrLineEdit->setText(name);
}

void KyPasswordDialog::login()
{
    qDebug() << "KyPasswordDialog login";
    if ((!m_sdkPasswdEdit->text().isEmpty()) && (m_usrLineEdit == nullptr? true : (!m_usrLineEdit->text().isEmpty()))) {
        accept();
    } else {
        m_sdkPasswdEdit->clear();
        if (m_usrLineEdit != nullptr) {
            m_usrLineEdit->clear();
            m_usrLineEdit->setFocus();
        }
    }
}

void KyPasswordDialog::onTextChanged(const QString & text)
{
    m_sdkPasswdEdit->setState(Ordinary);
    if (text == "") {
        //任何输入框为空，则不许按下连接按钮
        m_connectBtn->setEnabled(false);
        return ;
    }

    if (m_usrLineEdit != nullptr) {
        //企业网修改密码页面
        if (!m_usrLineEdit->text().isEmpty() && !m_sdkPasswdEdit->text().isEmpty()) {
            //用户名和密码均不为空，允许按下连接按钮
            m_connectBtn->setEnabled(true);
        } else {
            m_connectBtn->setEnabled(false);
        }
        return ;
    }

    if (m_sdkPasswdEdit->text().isEmpty() || m_sdkPasswdEdit->text().length() < 8) {
        //普通热点密码修改页面，若密码为空，或长度小于8位，不许按下连接按钮
        m_connectBtn->setEnabled(false);
        return ;
    }

    m_connectBtn->setEnabled(true);
}

#if 0
void KyPasswordDialog::onPaletteChanged()
{
    QPalette pal = lightPalette(this);

    this->setPalette(pal);
    m_sdkPasswdEdit->setPalette(pal);
    if (nullptr != m_usrLineEdit)
        m_usrLineEdit->setPalette(pal);

//    setFramePalette(this, pal);
}
#endif

void KyPasswordDialog::paintEvent(QPaintEvent *event)
{
    QPalette pal = this->palette();

    QPainter painter(this);
    painter.setRenderHint(QPainter:: Antialiasing, true);  //设置渲染,启动反锯齿
    painter.setPen(Qt::NoPen);
    painter.setBrush(pal.color(QPalette::Base));

    QRect rect = this->rect();
    painter.drawRoundedRect(rect, 12, 12);


    QDialog::paintEvent(event);
}
