/* loginwindow.cpp
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#include "loginwindow.h"
#include <QDebug>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QtConcurrent/QtConcurrentRun>
#include <QKeyEvent>
#include <QGridLayout>
#include <QAbstractItemModel>
#include <QPainter>
#include <QPainterPath>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include "imageutil.h"
#include "greeterwrapper.h"
#include "iconedit.h"
#include "globalv.h"
#include "configuration.h"
#include "biometricproxy.h"
#include "servicemanager.h"
#include "loginauthinterface.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <libintl.h>
#include <locale.h>
#include "pam-tally.h"
#include "greeterutils.h"
#include "common/configuration.h"
#include "klabel.h"

#ifdef WITHKYSEC
#include <kysec/libkysec.h>
#include <kysec/status.h>
#endif

#define _(string) gettext(string)

LoginWindow::LoginWindow(GreeterWrapper *greeter, QWidget *parent)
    : QWidget(parent),
      m_timer(nullptr),
      isLockingFlg(false),
      m_nCurLockMin(0),
      w_timer(nullptr),
      m_greeter(greeter),
      m_uniauthService(new UniAuthService(this)),
      m_configuration(Configuration::instance())
{
    QFile qss(":/ukui-greeter.qss");
    qss.open(QFile::ReadOnly);
    setStyleSheet(qss.readAll());

    m_strDefaultFace = UKUIGreeter::Utils::getDefaultFace();
    initUI();
    m_failMap.clear();
    pam_tally_init();  //
    connect(m_greeter, SIGNAL(showMessage(QString,QLightDM::Greeter::MessageType)),
            this, SLOT(onShowMessage(QString,QLightDM::Greeter::MessageType)));
    connect(m_greeter, SIGNAL(showPrompt(QString,QLightDM::Greeter::PromptType)),
            this, SLOT(onShowPrompt(QString,QLightDM::Greeter::PromptType)));
    connect(m_greeter, SIGNAL(authenticationComplete()),
            this, SLOT(onAuthenticationComplete()));

    Configuration::instance()->saveLastLoginUser1("");

    QString tempPath = QDir::tempPath();
    QString filePath = tempPath + "/ukui-greeter";
    QFile file(filePath);
    if(!file.exists()) {
        isPowerup = true;
        file.open(QFile::ReadOnly|QFile::Text|QIODevice::Append);
    }

}

void LoginWindow::initUI()
{
    setFixedWidth(500);

    m_userWidget = new QWidget(this);
    m_userWidget->setObjectName(QStringLiteral("userWidget"));

    /* 头像 */
    const QString SheetStyle = QString("border-radius: %1px;  border:0px   solid white;").arg((int)(77*scale));
    m_labelHeadImg = new QLabel(m_userWidget);
    m_labelHeadImg->setObjectName(QStringLiteral("faceLabel"));
    m_labelHeadImg->setFocusPolicy(Qt::NoFocus);
    m_labelHeadImg->setStyleSheet(SheetStyle);
    m_labelHeadImg->setAlignment(Qt::AlignCenter);
    m_labelHeadImg->hide();

    // 人脸识别
    m_labelFace = new QLabel(m_userWidget);
    m_labelFace->setObjectName("faceLabel");
    m_labelFace->setAlignment(Qt::AlignCenter);
    m_labelFace->hide();

    // 二维码窗口
    m_labelLoginTypeTip = new KLabel(m_userWidget);
    m_labelLoginTypeTip->installEventFilter(this);
    m_labelLoginTypeTip->setObjectName("loginTypeTipLabel");
    m_labelLoginTypeTip->setAlignment(Qt::AlignCenter);
    m_labelQRCode = new QLabel(m_userWidget);
    m_labelQRCode->setObjectName("qrCodeLabel");
    m_labelQRCode->setAlignment(Qt::AlignCenter);
    m_labelQRCode->hide();
    QVBoxLayout *layoutQRCode = new QVBoxLayout();
    layoutQRCode->setAlignment(Qt::AlignCenter);
    layoutQRCode->setSpacing(10);
    m_labelQRCode->setLayout(layoutQRCode);
    m_labelQRCodeTip = new KLabel();
    m_labelQRCodeTip->setFixedSize(22,22);
    m_labelQRCodeTip->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(22,22)));
    layoutQRCode->addWidget(m_labelQRCodeTip, 0, Qt::AlignHCenter);
    m_labelQRCodeMsg = new KLabel();
    m_labelQRCodeMsg->installEventFilter(this);
    m_labelQRCodeMsg->setFixedHeight(30);
    m_labelQRCodeMsg->setStyleSheet("QLabel{background-color:rgba(255,255,255,0);color:rgb(255,0,0)}");
    layoutQRCode->addWidget(m_labelQRCodeMsg, 0, Qt::AlignHCenter);

    /* 用户名 */
    m_nameLabel = new KLabel(m_userWidget);
    m_nameLabel->setObjectName(QStringLiteral("login_nameLabel"));
    m_nameLabel->setFocusPolicy(Qt::NoFocus);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->installEventFilter(this);

    /* 密码框所在窗口 */
    m_passwdWidget = new QWidget(this);

    /* 密码框 */
    m_passwordEdit = new IconEdit(m_passwdWidget);
    m_passwordEdit->installEventFilter(this);
    connect(m_passwordEdit,&IconEdit::clickedPassword,
            this,&LoginWindow::onM_passwordEditClicked);
    connect(m_passwordEdit, &IconEdit::setKToolTip,
            this, &LoginWindow::showTooltip);
    connect(m_passwordEdit, &IconEdit::hideKTooltip,
            this, &LoginWindow::hideTooltip);
    /* 密码认证信息显示列表 */
    m_messageLabel = new KLabel(m_passwdWidget);
    m_messageLabel->installEventFilter(this);
    m_messageButton = new QPushButton(m_passwdWidget);

    
    //QtConcurrent::run([=](){
        m_passwdWidget->setObjectName(QStringLiteral("passwordWidget"));
        m_passwordEdit->setObjectName(QStringLiteral("passwordEdit"));
        m_messageLabel->setObjectName(QStringLiteral("messageLabel"));
        m_messageLabel->setAlignment(Qt::AlignLeft);
        m_messageButton->setObjectName(QStringLiteral("messageButton"));
        m_messageButton->hide();

        m_passwordEdit->setIcon(QIcon(":/images/login-button.svg"));
        //m_passwordEdit->setFocusPolicy(Qt::StrongFocus);
        m_passwordEdit->installEventFilter(this);
        m_passwordEdit->hide(); //收到请求密码的prompt才显示出来
        connect(m_passwordEdit, SIGNAL(clicked(const QString&)),
                this, SLOT(onLogin(const QString&)));

        //setFocusProxy(m_passwordEdit);

    //});
    /* ukey密码框所在窗口 */
    m_ukeyPasswdWidget = new QWidget(this);
    m_ukeyPasswdWidget->setObjectName(QStringLiteral("passwordWidget"));

    /* 密码框 */
    m_ukeyPasswordEdit = new IconEdit(m_ukeyPasswdWidget);
    m_ukeyPasswordEdit->installEventFilter(this);
    connect(m_ukeyPasswordEdit,&IconEdit::clickedPassword,
            this,&LoginWindow::onM_passwordEditClicked);

    m_ukeyPasswdWidget->setInputMethodHints(Qt::ImhNone);
    m_ukeyPasswordEdit->setObjectName(QStringLiteral("passwordEdit"));
    m_ukeyPasswordEdit->setIcon(QIcon(":/images/login-button.svg"));
    m_ukeyPasswordEdit->installEventFilter(this);
    m_ukeyPasswordEdit->setType(QLineEdit::Password);
    m_ukeyPasswordEdit->setPrompt(tr("Enter the ukey password"));
    m_ukeyPasswordEdit->setModeBtnVisible(false);
    connect(m_ukeyPasswordEdit, SIGNAL(clicked(const QString&)),
            this, SLOT(onRespondUkey(const QString&)));

    /* 密码认证信息显示列表 */
    m_ukeyMessageLabel = new KLabel(m_ukeyPasswdWidget);
    m_ukeyMessageLabel->setObjectName(QStringLiteral("messageLabel"));
    m_ukeyMessageLabel->setAlignment(Qt::AlignLeft);
    
    //font = m_ukeyMessageLabel->font();
    //font.setPixelSize(14);
    m_ukeyMessageLabel->setFontSize(14);
    m_ukeyMessageLabel->setStyleSheet("QToolTip{border-radius:4px;background-color:#FFFFFF;color:black;font-size:16px}");

    m_ukeyPasswdWidget->hide();

    m_loadingWidget = new QWidget(m_ukeyPasswdWidget);
    m_loadingText = new KLabel(m_loadingWidget);
    m_loadingText->setText(tr("Insert the ukey into the USB port"));
    QFont loadTextFont = m_loadingText->font();
    loadTextFont.setPixelSize(18);
    m_loadingText->setFont(loadTextFont);
    m_loadingButton = new QPushButton(m_loadingWidget);
    m_loadingButton->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_loadingButton->setFlat(true);
    m_loadingButton->installEventFilter(this);
    m_loadingButton->setStyleSheet("QPushButton{text-align:center;color: rgba(255, 255, 255, 255);border: none;border-radius: 4px;outline: none;}");

    m_loadingWidget->adjustSize();

    QVBoxLayout *loadingLayout= new QVBoxLayout(m_loadingWidget);
    loadingLayout->addWidget(m_loadingButton);
    loadingLayout->addWidget(m_loadingText);
    startLoadingUkey();

    m_customWidget = new QWidget(this);
    m_layoutCustom = new QHBoxLayout();
    m_layoutCustom->setSpacing(0);
    m_layoutCustom->setContentsMargins(0,0,0,0);
    m_customWidget->setLayout(m_layoutCustom);
    m_customWidget->hide();

    isinput_passwd = false;
}

void LoginWindow::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    if(m_passwordEdit->isVisible()) {
        m_passwordEdit->setFocus();
    } else if(m_ukeyPasswordEdit->isVisible()) {
        m_ukeyPasswordEdit->setFocus();
    }
}

void LoginWindow::resizeEvent(QResizeEvent *)
{
    setChildrenGeometry();
}

void LoginWindow::keyReleaseEvent(QKeyEvent *event)
{
    //    qDebug() << event->key();
    QWidget::keyReleaseEvent(event);
}

bool LoginWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == m_nameLabel ){
        if(event->type() == QEvent::ToolTip){
            if(toolStr != m_nameLabel->text()){
                Q_EMIT setKToolTip(toolStr);
            }
        } else if (event->type() == QEvent::Leave){
            Q_EMIT hideKTooltip();
        }
    }

    if(obj == m_messageLabel){
        if(event->type() == 110){
            Q_EMIT setKToolTip(m_messageLabelTip);
        } else if (event->type() == 11){
            Q_EMIT hideKTooltip();
        }
    }

    if(obj == m_labelLoginTypeTip){
        if(event->type() == 110){
            Q_EMIT setKToolTip(m_strLoginTypeTip);
        } else if (event->type() == 11){
            Q_EMIT hideKTooltip();
        }
    }

    if (obj == m_labelQRCodeMsg ) {
        if(event->type() == QEvent::ToolTip) {
            if(msgToolStr != m_labelQRCodeMsg->text()) {
                Q_EMIT setKToolTip(msgToolStr);
            }
        } else if (event->type() == QEvent::Leave) {
            Q_EMIT hideKTooltip();
        }
    }
    return false;
}

void LoginWindow::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange){
        refreshTranslate();
    }
}

void LoginWindow::refreshTranslate()
{
    if(m_nameLabel){
        if(m_nameLabel->text() == "Login" || m_nameLabel->text() == "登录")
            m_nameLabel->setText(tr("Login"));
    }

    if(m_ukeyPasswordEdit){
        m_ukeyPasswordEdit->setPrompt(tr("Enter the ukey password"));
    }

    if(m_loadingText){
        m_loadingText->setText(tr("Insert the ukey into the USB port"));
    }

    if(m_loadingWidget){
        m_loadingWidget->adjustSize();
	m_loadingWidget->setGeometry((m_ukeyPasswdWidget->width() - m_loadingWidget->width())/2, 0, m_loadingWidget->width(), m_loadingWidget->height());
    }
}

void LoginWindow::setChildrenGeometry()
{
    if(scale < 0.5)
        setFixedWidth(360);
    else
        setFixedWidth(600);

    // 用户信息显示位置
    m_userWidget->setGeometry(0, 0,
                              width(), 376*scale);
    m_labelLoginTypeTip->setGeometry(0,0,m_userWidget->width(), 30);
    m_labelLoginTypeTip->setAlignment(Qt::AlignCenter);
    m_labelHeadImg->setStyleSheet(QString("border-radius: %1px;  border:0px   solid white;").arg((int)(77*scale)));
    m_labelHeadImg->setGeometry((width() - 154*scale) / 2 , m_labelLoginTypeTip->geometry().bottom()+24*scale, 154*scale, 154*scale);
    setFace(m_face);

    m_labelQRCode->setStyleSheet(QString("border-radius: %1px;  border:0px solid white;background-color: rgba(255,255,255,100%);").arg((int)(6*scale)));
    m_labelQRCode->setGeometry((width() - 154*scale) / 2 , m_labelLoginTypeTip->geometry().bottom()+24*scale, 154*scale, 154*scale);
    setQRCode(m_imgQRCode);

    m_labelFace->setStyleSheet(QString("border-radius: %1px;  border:0px solid white;background-color: rgba(255,255,255,10%);").arg((int)(77*scale)));
    m_labelFace->setGeometry((width() - 154*scale) / 2 , m_labelLoginTypeTip->geometry().bottom()+24*scale, 154*scale, 154*scale);
    QImage faceImg;
    setFaceImg(faceImg);

    m_nameLabel->setGeometry(0, m_labelHeadImg->geometry().bottom() + 16*scale,
                                 width(), 42);

    m_passwdWidget->setGeometry(0, m_nameLabel->geometry().bottom() + 24*scale, width(), 100);
    m_passwordEdit->setGeometry((m_passwdWidget->width() - 240)/2, 0, 240, 40);
    m_messageLabel->setGeometry((m_passwdWidget->width() - 240)/2,
                                m_passwordEdit->geometry().bottom() + 4,
                                width()-(m_passwdWidget->width() - 240)/2, 20);
    
    m_ukeyPasswdWidget->setGeometry(0, m_nameLabel->geometry().bottom() + 24*scale, width(), 100);
    m_ukeyPasswordEdit->setGeometry((m_ukeyPasswdWidget->width() - 240)/2, 0, 240, 40);
    m_ukeyMessageLabel->setGeometry((m_ukeyPasswdWidget->width() - 240)/2,
                                m_ukeyPasswordEdit->geometry().bottom() + 4,
                                width()-(m_ukeyPasswdWidget->width() - 240)/2, 36);
    m_loadingWidget->adjustSize();
    m_loadingWidget->setGeometry((m_ukeyPasswdWidget->width() - m_loadingWidget->width())/2, 0, m_loadingWidget->width(), m_loadingWidget->height());
    m_messageLabel->setMinimumHeight(36);

    m_messageButton->setGeometry((m_passwdWidget->width() - 200)/2, 0, 200, 40);
    m_messageButton->setStyleSheet("QPushButton{text-align:center;background-color: rgba(255,255,255,15%);border: none;border-radius: 4px;outline: none;} \
                                   QPushButton::hover{background-color: rgba(255,255,255,40%);} \
                                   QPushButton::pressed {background-color: rgba(255,255,255,30%);}");
 
    m_customWidget->setGeometry(0, 0,
                              width(), 376*scale+100);

    setBiometricWidgetGeometry();
}

void LoginWindow::startLoadingUkey()
{
    //isLoadingUkey = true;
    m_loadingWidget->show();
    m_ukeyPasswordEdit->hide();
    m_ukeyMessageLabel->hide();
    if(!m_loadingTimer)
    {
        m_loadingTimer = new QTimer(this);
        m_loadingTimer->setInterval(150);
        connect(m_loadingTimer, &QTimer::timeout, this, &LoginWindow::updateLoadingPixmap);
    }

    QPixmap icon = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(27,27);
    m_loadingPixmap = ImageUtil::drawSymbolicColoredPixmap(icon, "white");
    m_loadingButton->setIcon(m_loadingPixmap);
    m_loadingButton->setIconSize(QSize(27, 27));
    m_loadingTimer->start();
}

void LoginWindow::stopLoadingUkey()
{
    isLoadingUkey = false;
    m_loadingWidget->hide();
    m_ukeyPasswordEdit->show();
    m_ukeyMessageLabel->show();
    if(m_loadingTimer){
        m_loadingTimer->stop();
    }
}

void LoginWindow::updateLoadingPixmap()
{
    QMatrix matrix;
    matrix.rotate(90.0);
    m_loadingPixmap = m_loadingPixmap.transformed(matrix, Qt::FastTransformation);
    m_loadingButton->setIcon(QIcon(m_loadingPixmap));
}

void LoginWindow::switchLoginOptType(unsigned uLoginOptType)
{
    switch(uLoginOptType) {
    case LOGINOPT_TYPE_PASSWORD:
    {
        m_userWidget->show();
        m_labelHeadImg->show();
        m_labelQRCode->hide();
        m_labelFace->hide();
        m_passwdWidget->show();
        m_ukeyPasswdWidget->hide();
        m_customWidget->hide();
    }
        break;
    case LOGINOPT_TYPE_FACE:
    {
        m_userWidget->show();
        m_labelHeadImg->hide();
        m_labelQRCode->hide();
        m_labelFace->show();
        m_passwdWidget->show();
        m_ukeyPasswdWidget->hide();
        m_customWidget->hide();
    }
        break;
    case LOGINOPT_TYPE_FINGERPRINT:
    case LOGINOPT_TYPE_VOICEPRINT:
    case LOGINOPT_TYPE_FINGERVEIN:
    case LOGINOPT_TYPE_IRIS:
    {
        m_userWidget->show();
        m_labelHeadImg->show();
        m_labelQRCode->hide();
        m_labelFace->hide();
        m_passwdWidget->show();
        m_ukeyPasswdWidget->hide();
        m_customWidget->hide();
    }
        break;
    case LOGINOPT_TYPE_QRCODE:
    {
        m_userWidget->show();
        m_labelHeadImg->hide();
        setQRCodeMsg("");
        m_labelQRCode->show();
        m_labelFace->hide();
        m_passwdWidget->show();
        m_ukeyPasswdWidget->hide();
        m_customWidget->hide();
    }
        break;
    case LOGINOPT_TYPE_GENERAL_UKEY:
    {
        m_userWidget->show();
        m_labelHeadImg->show();
        m_labelQRCode->hide();
        m_labelFace->hide();
        m_passwdWidget->hide();
        m_customWidget->hide();
        m_ukeyPasswdWidget->show();
//        setFocusProxy(m_ukeyPasswordEdit);
//        m_ukeyPasswordEdit->setFocusPolicy(Qt::StrongFocus);
//        m_ukeyPasswordEdit->setFocus();
    }
	break;
    case LOGINOPT_TYPE_CUSTOM:
    {
        m_userWidget->hide();
        m_passwdWidget->hide();
        m_labelHeadImg->hide();
        setQRCodeMsg("");
        m_labelQRCode->hide();
        m_labelFace->hide();
        m_ukeyPasswdWidget->hide();
        LoginAuthInterface* plugin = m_widgetLoginOpts->getCustomLoginAuth();
        if (plugin) {
            QWidget *authWidget = plugin->getPluginMainWnd();
            if (authWidget) {
                if (m_customWidget != authWidget->parent()) {
                    m_layoutCustom->addWidget(authWidget);
                    qDebug()<<"CustomWidget Size:"<<authWidget->size();
                    QObject *obj = dynamic_cast<QObject*>(plugin);
                    connect(obj, SIGNAL(requestAuthAccount(QString)), this, SLOT(onCustomRequestAccount(QString)));
                    connect(obj, SIGNAL(authenticateResult(int, QString)), this, SLOT(onCustomAuthResult(int, QString)));
                    connect(obj, SIGNAL(onRequest(QString)), this, SLOT(onCustomRequest(QString)));
                    connect(obj, SIGNAL(getLoginPluginEnv()), this, SLOT(onCustomPlugEnv()));
                }
                setFocusProxy(authWidget);
                authWidget->show();
            }
        }
        m_customWidget->show();
    }
        break;
    default:
        return;
    }

    if (uLoginOptType != m_uCurLoginOptType || (m_deviceInfo && m_deviceInfo->id != m_nLastDeviceId) || m_name != m_lastUserName) {
        clearMessage();
        switch(uLoginOptType) {
        case LOGINOPT_TYPE_PASSWORD:
        {
            setLoginTypeTip("");
        }
            break;
        case LOGINOPT_TYPE_FACE:
        {
            setLoginTypeTip(tr("Verify face recognition or enter password to unlock"));
        }
            break;
        case LOGINOPT_TYPE_FINGERPRINT:
        {
            setLoginTypeTip(tr("Press fingerprint or enter password to unlock"));
        }
            break;
        case LOGINOPT_TYPE_VOICEPRINT:
        {
            setLoginTypeTip(tr("Verify voiceprint or enter password to unlock"));
        }
            break;
        case LOGINOPT_TYPE_FINGERVEIN:
        {
            setLoginTypeTip(tr("Verify finger vein or enter password to unlock"));
        }
            break;
        case LOGINOPT_TYPE_IRIS:
        {
            setLoginTypeTip(tr("Verify iris or enter password to unlock"));
        }
            break;
        case LOGINOPT_TYPE_QRCODE:
        {
            setLoginTypeTip(tr("Use the bound wechat scanning code or enter the password to unlock"));
        }
            break;
	case LOGINOPT_TYPE_GENERAL_UKEY:
        {
            setLoginTypeTip("");
        }
	    break;
        case LOGINOPT_TYPE_CUSTOM:
        {
            setLoginTypeTip("");
        }
            break;
        default:
            return;
        }
    }
    if (m_deviceInfo) {
        m_nLastDeviceId = m_deviceInfo->id;
    } else {
        m_nLastDeviceId = -1;
    }

    m_lastUserName = m_name;
    m_uCurLoginOptType = uLoginOptType;
}

/**
 * @brief LoginWindow::reset
 * 复原UI
 */
void LoginWindow::reset()
{
    m_passwordEdit->clear();
    m_passwordEdit->setType(QLineEdit::Password);
    m_messageButton->hide();
    clearMessage();
    authMode = UNKNOWN;
    m_deviceInfo = DeviceInfoPtr();
    m_deviceName = "";
    isLockingFlg = false;
    if(m_widgetLoginOpts){
        m_widgetLoginOpts->hide();
    }
}

void LoginWindow::setFocusin(int target)
{
    if(m_passwordEdit && m_passwordEdit->isVisible()) {
        if (m_ukeyPasswordEdit) {
            m_ukeyPasswordEdit->setFocusin(2);
        }
        switch (target) {
        case REMOVE: //焦点清除
            if(m_widgetLoginOpts)
                m_widgetLoginOpts->tabOptionSelected(2);
            m_passwordEdit->setFocusin(2);
            if(is_showMessageBtn) {
                m_messageButton->clearFocus();
                m_messageButton->setStyleSheet("QPushButton:!checked:!pressed:!hover{background-color: rgba(255,255,255,40)}"
                                               "QPushButton:!checked:!pressed:hover{background-color: rgba(255,255,255,100)}"
                                               "QPushButton:pressed{background-color: rgba(255,255,255,40)}");
            }
            m_nameLabel->setFocus();
            break;
        case IN_LIGIN: //焦点在登录按钮
            if(m_widgetLoginOpts)
                m_widgetLoginOpts->tabOptionSelected(2);
            m_passwordEdit->setFocusin(1);
            break;
        case BIO_RIGHT: //登录选项焦点右移
            if(m_widgetLoginOpts)
                m_widgetLoginOpts->tabOptionSelected(0);
            m_passwordEdit->setFocusin(2);
            m_nameLabel->setFocus();
            break;
        case BIO_LEFT: //登录选项焦点左移
            if(m_widgetLoginOpts)
                m_widgetLoginOpts->tabOptionSelected(1);
            m_passwordEdit->setFocusin(target);
            m_nameLabel->setFocus();
            break;
        case IN_LINEEDIT: //焦点在密码输入框
            if(m_widgetLoginOpts)
                m_widgetLoginOpts->tabOptionSelected(2);
            m_passwordEdit->setFocusin(0);
            break;
        case ON_MESSAGEBTN: //免密登录按钮
            if(m_widgetLoginOpts)
                m_widgetLoginOpts->tabOptionSelected(2);
            m_passwordEdit->setFocusin(2);
            m_messageButton->setFocus();
            m_messageButton->setStyleSheet("QPushButton{background-color: rgba(255,255,255,15%); border-radius: 4px; border: 2px solid #2C73C8;}");
            break;
        default:
            if(m_passwordEdit)
                m_passwordEdit->setFocusin(target);
            m_widgetLoginOpts->tabOptionSelected(2);
            break;
        }
    } else {
        if (m_passwordEdit) {
            m_passwordEdit->setFocusin(2);
        }
        switch (target) {
        case REMOVE: //焦点清除
            if(m_widgetLoginOpts)
                m_widgetLoginOpts->tabOptionSelected(2);
            m_ukeyPasswordEdit->setFocusin(2);
            if(is_showMessageBtn) {
                m_messageButton->clearFocus();
                m_messageButton->setStyleSheet("QPushButton:!checked:!pressed:!hover{background-color: rgba(255,255,255,40)}"
                                               "QPushButton:!checked:!pressed:hover{background-color: rgba(255,255,255,100)}"
                                               "QPushButton:pressed{background-color: rgba(255,255,255,40)}");
            }
            m_nameLabel->setFocus();
            break;
        case IN_LIGIN: //焦点在登录按钮
            if(m_widgetLoginOpts)
                m_widgetLoginOpts->tabOptionSelected(2);
            m_ukeyPasswordEdit->setFocusin(1);
            break;
        case BIO_RIGHT: //登录选项焦点右移
            if(m_widgetLoginOpts)
                m_widgetLoginOpts->tabOptionSelected(0);
            m_ukeyPasswordEdit->setFocusin(2);
            m_nameLabel->setFocus();
            break;
        case BIO_LEFT: //登录选项焦点左移
            if(m_widgetLoginOpts)
                m_widgetLoginOpts->tabOptionSelected(1);
            m_ukeyPasswordEdit->setFocusin(target);
            m_nameLabel->setFocus();
            break;
        case IN_LINEEDIT: //焦点在密码输入框
            if(m_widgetLoginOpts)
                m_widgetLoginOpts->tabOptionSelected(2);
            m_ukeyPasswordEdit->setFocusin(0);
            break;
        case ON_MESSAGEBTN: //免密登录按钮
            if(m_widgetLoginOpts)
                m_widgetLoginOpts->tabOptionSelected(2);
            m_ukeyPasswordEdit->setFocusin(2);
            m_messageButton->setFocus();
            m_messageButton->setStyleSheet("QPushButton{background-color: rgba(255,255,255,15%); border-radius: 4px; border: 2px solid #2C73C8;}");
            break;
        default:
            if(m_ukeyPasswordEdit)
                m_ukeyPasswordEdit->setFocusin(target);
            m_widgetLoginOpts->tabOptionSelected(2);
            break;
        }
    }
}

void LoginWindow::setClick()
{
    m_widgetLoginOpts->tabOptionSelected(3);
}

void LoginWindow::checkPassword()
{
    if (m_passwordEdit && m_passwordEdit->isVisible()) {
        m_passwordEdit->setFocus();
        m_passwordEdit->clicked_cb();
    } else if (m_ukeyPasswordEdit && m_ukeyPasswordEdit->isVisible()) {
        m_ukeyPasswordEdit->setFocus();
        m_ukeyPasswordEdit->clicked_cb();
    }
    setFocusin(REMOVE);
}

void LoginWindow::clearMessage()
{
    m_messageLabel->clear();
    m_preStrMessage = "";
    m_preStrMessageType = -1;
}

/**
 * @brief LoginWindow::setUserName
 * @param userName 用户名
 *
 * 设置用户名
 */
void LoginWindow::setUserName(const QString& name)
{
    //m_nameLabel->setText(userName);
    curFontSize = m_configuration->getUserConfig("fontSize").toDouble();
    m_configuration->setUsersFont();
    if(m_nameLabel->text() == name)
        return ;
    QFont font;
    font.setPixelSize(24 + curFontSize);
    double m_ptToPx = m_configuration->getPtToPx();
    QString str = ElideText(font,400,name);
    toolStr = name;

    m_ukeyMessageLabel->setFontSize(14 + curFontSize);
    m_passwordEdit->setFontSize(curFontSize);
    m_nameLabel->setFontSize(24 + curFontSize);
    m_messageLabel->setFontSize(14 + curFontSize);
    font.setPointSize((14 + curFontSize) *m_ptToPx);
    m_messageButton->setFont(font);
    m_labelLoginTypeTip->setFontSize(14 + curFontSize);
    m_nameLabel->setText(str);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    m_nameLabel->adjustSize();
    m_nameLabel->move((width() - m_nameLabel->width())/2,m_labelHeadImg->geometry().bottom() + 16*scale);
    m_labelQRCodeMsg->setFontSize(14 + curFontSize);
}

/**
 * @brief LoginWindow::userName
 * @return 当前的用户名
 *
 * 获取当前用户名
 */
QString LoginWindow::getUserName()
{
    if(m_nameLabel->text() == tr("Login"))
        return "Login";
    return m_nameLabel->text();
}

/**
 * @brief LoginWindow::getRealName
 * @return
 * 获取用户真名
 */
QString LoginWindow::getRealName()
{
    return m_name;
}

/**
 * @brief LoginWindow::setFace
 * @param faceFile 用户头像文件
 *
 * 设置用户头像
 */
void LoginWindow::setFace(const QString& facePath)
{
    QFile faceFile(facePath);
    QPixmap faceImage;
    m_labelHeadImg->setFixedSize(154*scale,154*scale);
    //如果头像文件不存在，则使用默认头像
    if(faceFile.exists()){
        faceImage.load(facePath);
    }
    else{
        faceImage.load(m_strDefaultFace);
    }
    faceImage = cutPixmap(faceImage);
    faceImage = faceImage.scaled(154*scale,154*scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    faceImage =  PixmapToRound(faceImage,77*scale);
    m_labelHeadImg->setAlignment(Qt::AlignCenter);
    m_labelHeadImg->setPixmap(faceImage);

    m_face = facePath;
}

void LoginWindow::setQRCode(QImage& imgQRCode)
{
    if (imgQRCode.isNull()) {
        m_imgQRCode.load(":/images/ukui-qrcode-null.svg");
    } else {
        m_imgQRCode = imgQRCode;
        m_labelQRCodeTip->hide();
    }
    m_imgQRCode = m_imgQRCode.scaled(QSize(150*scale, 150*scale));
    m_labelQRCode->setAlignment(Qt::AlignCenter);
    m_labelQRCode->setPixmap(QPixmap::fromImage(m_imgQRCode));
}

void LoginWindow::setQRCodeMsg(QString strMsg)
{
    if (strMsg.isEmpty()) {
        m_labelQRCodeMsg->hide();
        //m_labelQRCodeTip->hide();
    } else {
        //一开始认证时就没有网，直接停止加载状态
        if(w_timer && w_timer->isActive())
        {
            w_timer->stop();
        }
        QFont font;
        font.setPixelSize(14 + curFontSize);
        qDebug() << m_labelQRCode->width() ;
        QString str = ElideText(font, m_labelQRCode->width() - 12* scale, strMsg);
        m_labelQRCodeMsg->setText(str);
        msgToolStr = strMsg;
        m_labelQRCodeMsg->show();
        m_labelQRCodeTip->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(22,22)));
        m_labelQRCodeTip->show();
    }
}

void LoginWindow::setFaceImg(QImage& imgFace, int nStatus)
{
    QPixmap faceImage;
    m_labelFace->setFixedSize(154*scale,154*scale);
    //如果头像文件不存在，则使用默认头像
    if(!imgFace.isNull()) {
        faceImage = PixmapToRound(QPixmap::fromImage(imgFace),77*scale);
    } else {
        switch(nStatus){
        case 1:
            faceImage = QPixmap(":/images/ukui-loginopt-lose.svg");
            break;
        case 2:
            faceImage = m_widgetLoginOpts->loadSvg(":/images/ukui-loginopt-smile.svg", "gray", 48);
            m_labelFace->setStyleSheet(QString("border-radius: %1px;  border:0px solid white;background-color: rgba(255,255,255,20%);").arg((int)(77*scale)));
            break;
        default:
            faceImage = QPixmap(":/images/ukui-loginopt-smile.svg");
            break;
        }
    }

    m_labelFace->setAlignment(Qt::AlignCenter);
    m_labelFace->setPixmap(faceImage);
}

void LoginWindow::updatePixmap()
{
    QMatrix matrix;
    matrix.rotate(90.0);
    m_waitingPixmap = m_waitingPixmap.transformed(matrix, Qt::FastTransformation);
    if(m_uCurLoginOptType == LOGINOPT_TYPE_QRCODE)
        m_labelQRCodeTip->setPixmap(m_waitingPixmap);
    else if(m_uCurLoginOptType == LOGINOPT_TYPE_FACE) {
        m_labelFace->setPixmap(m_waitingPixmap);
    }
}

/**
 * @brief LoginWindow::setLoggedIn
 * @param isLoggedIn
 *
 * 设置当前用户是否已已经登录
 */
void LoginWindow::setLoggedIn(bool isLoggedIn)
{
    //m_isLoginLabel->setText(isLoggedIn ? tr("logged in") : "");
}

/**
 * @brief LoginWindow::setPrompt
 * @param text
 *
 * 设置密码框的提示信息
 */
void LoginWindow::setPrompt(const QString& text)
{
    m_passwordEdit->setPrompt(text);
}

/**
 * @brief LoginWindow::password
 * @return
 *
 * 获取密码
 */
QString LoginWindow::getPassword()
{
    return m_passwordEdit->text();
}

int LoginWindow::getBioNum()
{
    return m_widgetLoginOpts->getVisibleLoginOptCount();
}

bool LoginWindow::setUserIndex(const QModelIndex& index)
{
    isinput_passwd = false;

    if(!index.isValid()){
        return false;
    }
    // 重复选择时，不重新认证
    QString strNewName = index.data(QLightDM::UsersModel::NameRole).toString();
    if (!strNewName.isEmpty() && strNewName != "*login"
            && strNewName != "*guest" && strNewName == m_name) {
        return false;
    }

    if(m_widgetLoginOpts){
        m_widgetLoginOpts->stopAuth();
        if(m_bioTimer && m_bioTimer->isActive())
            m_bioTimer->stop();
    }

    //设置用户名
    m_strRealName = index.data(QLightDM::UsersModel::RealNameRole).toString();
    m_name = strNewName;
    setUserName(m_strRealName.isEmpty() ? m_name : m_strRealName);


    //设置头像
    QString facePath = index.data(QLightDM::UsersModel::ImagePathRole).toString();
    setFace(facePath);

    //显示是否已经登录
    bool isLoggedIn = index.data(QLightDM::UsersModel::LoggedInRole).toBool();
    setLoggedIn(isLoggedIn);

    //设置生物识别设备窗口的uid
    m_uid = index.data(QLightDM::UsersModel::UidRole).toInt();
    setChildrenGeometry();

    startAuthentication();
    reset();

    //设置用户名

    return true;
}

/**
 * @brief 手动输入的用户名不在用户列表中（uid < 1000或者用户不存在）
 * @param userName
 */
void LoginWindow::setUserNotInView(const QString &userName)
{
    isloginauth = false;
    m_strRealName = "";
    m_name = userName;
    QString realName = "";
//    setUserName(userName);
    qDebug()<<"userName = "<<userName;
    struct passwd * pw = getpwnam(userName.toLatin1().data());
    if(pw){
        m_uid = pw->pw_uid;
        if(pw->pw_gecos && pw->pw_gecos[0] != '\0'){
             QString geoces = QString(pw->pw_gecos);
             QStringList list = geoces.split(QLatin1Char(','));
             realName = list.at(0);
        }
        qDebug()<<"m_uid = "<<m_uid << "userName = "<<userName<<" realName = "<<realName;
    }

    if(userName ==  "*login") {
	realName = tr("Login");
        m_strRealName = tr("Login");
    }
    if(realName.isEmpty()){
        setUserName(userName);
        m_strRealName = userName;
    }else{
        setUserName(realName);
        m_strRealName = realName;
    }

}

void LoginWindow::startAuthentication()
{
    prompted = false;
    unacknowledged_messages = false;
    isloginauth = false;
    //用户认证
    if(m_name == "*guest")
    {                       //游客登录
        qDebug() << "guest login";
        m_passwordEdit->show();
        setPrompt(tr("login"));
    }
    else if(m_name == "*login")
    { //手动输入用户名
        m_greeter->authenticate("");
    }
    else
    {
        qDebug() << "login: " << m_name;
        m_greeter->authenticate(m_name);
    }
}

/**
 * @brief LoginWindow::startWaiting
 *
 * 等待认证结果
 */
void LoginWindow::startWaiting()
{
    m_passwordEdit->startWaiting();   //等待认证结果期间不能再输入密码
}

void LoginWindow::stopWaiting()
{
    m_passwordEdit->stopWaiting();
}

void LoginWindow::onRespondUkey(const QString &text)
{
     if (m_widgetLoginOpts && m_deviceInfo){
         m_widgetLoginOpts->SetExtraInfo(text,"pincode");
         m_widgetLoginOpts->startAuth(m_deviceInfo, m_uid);
     }
}

void LoginWindow::onLogin(const QString &str)
{
    clearMessage();
    unacknowledged_messages=false;
    isloginauth = false;
    qDebug()<<m_name;
    if(m_name == "*guest")
    {
        m_greeter->authenticateAsGuest();
    }
    else if(m_name == "*login")
    {   //用户输入用户名
        isloginauth = true;
        isManual = true;
        m_name_is_login = true;

        m_greeter->respond(str);

        manualLoginName = str;

        if (!isinput_passwd){
            Q_EMIT userChangedByManual(str);
            isinput_passwd = true;
        }
    }
    else
    {  //当密码长度超过1000多时，lightm会出异常，这里暂时先截断
        if(str.length()>1000)
            m_greeter->respond(str.mid(0,1000));
        else
            m_greeter->respond(str);
        startWaiting();
        m_passwordEdit->readOnly(true);
    }
}

void LoginWindow::startFirstLogin()
{
    bioService = new QDBusInterface("org.ukui.Biometric",
                                                      "/org/ukui/Biometric",
                                                      "org.ukui.Biometric",
                                                      QDBusConnection::systemBus(),this);

    if(!bioService->isValid())
        return ;

    QDBusMessage result = bioService->call(QStringLiteral("GetDevList"));
    if(result.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "GetDevList error:" << result.errorMessage();
        return ;
    }
    auto dbusArg =  result.arguments().at(1).value<QDBusArgument>();
    QList<QVariant> variantList;
    dbusArg >> variantList;

    if(variantList.count() == 0)
        return ;


    DeviceInfoPtr pDeviceInfo = std::make_shared<DeviceInfo>();
    auto arg = variantList.at(0).value<QDBusArgument>();
    arg >> *pDeviceInfo;

    qDebug()<<"pDeviceInfo->id = "<<pDeviceInfo->id;

    if(pDeviceInfo->id <= 0)
        return ;

    QList<QVariant> argList;
    argList << pDeviceInfo->id << -1 << 0 << -1;

    drvid = pDeviceInfo->id;
    QDBusPendingCall call = bioService->asyncCallWithArgumentList(QStringLiteral("Identify"), argList);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, &LoginWindow::onFirstLoginIdentifyComplete);

    isFirstAuth = true;

    QTimer::singleShot(3000, [&]{
        if(isFirstAuth){
            bioService->call(QStringLiteral("StopOps"), drvid, 3000);
        }
    });

}

void LoginWindow::onFirstLoginIdentifyComplete(QDBusPendingCallWatcher *watcher)
{
    isFirstAuth = false;
    QDBusPendingReply<int, int> reply = *watcher;

    if(reply.isError())
    {
        qWarning() << "Identify error: " << reply.error().message();
        return;
    }
    int result = reply.argumentAt(0).toInt();
    int authUid = reply.argumentAt(1).toInt();
    qDebug() << result << authUid <<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

    if(result == 0 && authUid > 0){
        struct passwd *pwd;
        pwd = getpwuid(authUid);
        if(pwd && pwd->pw_name){
            if(m_uniauthService && m_uniauthService->isActivatable()){
	        bool isEnable = m_uniauthService->getBioAuthStatus(pwd->pw_name, ENABLETYPE_BIO);
		if(isEnable == false)
	            return;
	    }
            setDirLogin();
            isBioSuccess = true;
            m_greeter->authenticate(pwd->pw_name);
        }
    }
}

void LoginWindow::onShowPrompt(QString text, QLightDM::Greeter::PromptType type)
{
    qDebug()<< "prompt: "<< text;
    if(isPowerup && m_configuration->getIs990()){
        waitBiometricServiceStatus();
        startFirstLogin();
    }

    if(text == BIOMETRIC_PAM || text == BIOMETRIC_PAM_DOUBLE || text == BIOMETRIC_PAM_QRCODE) {
        if(isBioSuccess){
            m_greeter->respond(BIOMETRIC_SUCCESS);
            return ;
        }
        useDoubleAuth = true;
        if(authMode == PASSWORD) {
            skipBiometricAuth();
        } else {
            skipBiometricAuth();
            if(isPowerup  && m_configuration->getIs990()){
                QTimer::singleShot(1500, [&]{
                    performBiometricAuth();
                });
                isPowerup = false;
            }else{
                performBiometricAuth();
            }
	}
    } else {
        m_name_is_login = false;
        qDebug()<<"m_name_is_login = false";
        stopWaiting();
        if(!text.isEmpty()){
            m_passwordEdit->readOnly(false);
            m_passwordEdit->show();
        }
        if (m_passwordEdit->isVisible()) {
            m_passwordEdit->setFocus();
        }
        if(type != QLightDM::Greeter::PromptTypeSecret){
            isauto_switch = true;
            m_passwordEdit->setType(QLineEdit::Normal);
        }
        else if(isauto_switch)
        {
            m_passwordEdit->setType(QLineEdit::Password);
            isauto_switch = false;
        }

        prompted = true;
        unacknowledged_messages = false;
        m_preStrMessage = "";
        qDebug()<<"unacknowledged_messages = false";
        if (!m_biometricProxy || !m_biometricProxy->isValid() || !m_deviceInfo) {
            switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
            if (m_widgetLoginOpts)
                m_widgetLoginOpts->setSelectedPassword();
        }
        if(text == "Password: "||text == "密码："){
            text = tr("Password: ");
            m_isPassWdInput = true;
            setWillLoginUser("", false);
        } else if (text == "Input password" || text == "Input Password" || text == "输入密码") {
            text = tr("Input Password");
            m_isPassWdInput = true;
            setWillLoginUser("", false);
        }
        if(text == "login:") {
            text = tr("Username");
            m_name = "*login";
            //m_nameLabel->setText(tr("login"));
            m_name_is_login = true;
            m_isPassWdInput = false;
            switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
            qDebug()<<"m_name_is_login = true";
            if (!m_strServiceUser.isEmpty()) {
                qDebug()<<"User on AutoLogin:"<<m_strServiceUser;
                QTimer::singleShot(50, this, [=](){
                    QString tmpServiceUser = m_strServiceUser;
                    onLogin(tmpServiceUser);
                    m_strServiceUser = tmpServiceUser;
                });
            }
        }
        m_passwordEdit->clear();
        setPrompt(text);
        if (unlock_countdown()) {
            if(!m_timer){
                m_timer = new QTimer(this);
                m_timer->setInterval(500);
                connect(m_timer, &QTimer::timeout, this, &LoginWindow::unlock_countdown);
            } else {
                if(m_timer->isActive()) {
                    m_timer->stop();
                }
            }
            m_timer->start();
        }
    }
}

void LoginWindow::onShowMessage(QString text, QLightDM::Greeter::MessageType type)
{
    Q_UNUSED(type);
    QString strDisplay = "";//显示的pam返回字符串
    qDebug()<< "message: "<< text<<"type = "<<type;
    //设置认证错误提示label最小高度
    m_messageLabel->setMinimumHeight(36);
    unacknowledged_messages = true;
    //账户锁定倒计时
    if (text.indexOf("account locked") != -1 || text.indexOf("账户已锁定") != -1 
        || text.indexOf("Account locked") != -1 || text.indexOf("永久锁定") != -1)
    {
        if (unlock_countdown()) {
            if(!m_timer){
                m_timer = new QTimer(this);
                m_timer->setInterval(500);
                connect(m_timer, &QTimer::timeout, this, &LoginWindow::unlock_countdown);
            } else {
                if(m_timer->isActive()) {
                    m_timer->stop();
                }
            }
            m_timer->start();
        }
        return;
    }
    else if (text.indexOf("No password received, please input password") != -1){//空密码
        m_messageLabel->setText(tr("Password cannot be empty"));
        m_messageLabelTip = "Password cannot be empty";
        return;
    }

    if(text == "密码为空，请输入密码" && qgetenv("LANGUAGE") == "en_US")
        text = "No password received, please input password";

    if (!m_preStrMessage.isEmpty() && m_preStrMessageType != type && m_preStrMessageType != -1)
    {
        text = m_preStrMessage + "," + text;
    }

    qDebug()<<"unacknowledged_messages = true";

    strDisplay = text;
    //设置完整字符串的tooltip
    m_messageLabelTip = text;
    QFontMetrics font(m_messageLabel->font());
    //返回字符串末尾带省略号的字符串
    strDisplay = font.elidedText(text, Qt::ElideRight, m_messageLabel->width()-8);
    m_messageLabel->setText(strDisplay);

    m_preStrMessage = text;
    m_preStrMessageType = type;
    //stopWaiting();
}

void LoginWindow::onAuthenticationComplete()
{
    stopWaiting();

    if(m_widgetLoginOpts){
        m_widgetLoginOpts->stopAuth();
    }

    if(m_greeter->isAuthenticated()) {
        // 认证成功，启动session
        qDebug()<< "authentication success:"<<m_strServiceUser<<m_name<<m_isOneKeyLogin;
        if((prompted && !unacknowledged_messages )||direct_login
            || (!m_strServiceUser.isEmpty() && m_strServiceUser == m_name && m_isOneKeyLogin)){
            direct_login = false;
            Configuration::instance()->saveLastLoginUser(m_name);
            // 三权分立不保存秒级登录用户信息
            #ifdef WITHKYSEC
            if (kysec_is_disabled() || !kysec_get_3adm_status()) {
            #endif
                Configuration::instance()->saveLastLoginUser1(m_name);
            #ifdef WITHKYSEC
            }
            #endif
            m_greeter->startSession();
        }
        else
        {
            qDebug()<<"prompted  = "<<prompted<<"  unacknowledged_messages = "<<unacknowledged_messages;
            prompted = true;
            show_authenticated();
        }
    } else {
        // 认证失败，重新开始用户的认证
        qDebug() << "authentication failed";
        if (prompted)
        {
            m_passwordEdit->clear();

            //如果用户输入了不存在的用户名导致的认证失败，让用户重新输入用户名
            if(isManual){
                if(m_name_is_login){
                    m_name_is_login = false;
                    m_name = "*login";
                }
                if (!m_isPassWdInput) {
                    if(!unacknowledged_messages && !isLockingFlg)
                        onShowMessage(tr("User name input error!"), QLightDM::Greeter::MessageTypeError);
                } else {
                    if(!unacknowledged_messages && !isLockingFlg)
                        onShowMessage(tr("Authentication failure, Please try again"), QLightDM::Greeter::MessageTypeError);
                }
            }
            else{
                if(!unacknowledged_messages && !isLockingFlg){
                    onShowMessage(tr("Authentication failure, Please try again"), QLightDM::Greeter::MessageTypeError);
                }
                authMode = PASSWORD;
            }
            isinput_passwd = false;
            isManual = false;
            if (unlock_countdown()) {
                if(!m_timer){
                    m_timer = new QTimer(this);
                    m_timer->setInterval(500);
                    connect(m_timer, &QTimer::timeout, this, &LoginWindow::unlock_countdown);
                } else {
                    if(m_timer->isActive()) {
                        m_timer->stop();
                    }
                }
                m_timer->start();
            }
            startAuthentication();
        }
        else
        {
            show_authenticated(false);
        }
    }
    setWillLoginUser("", false);
}

void LoginWindow::show_authenticated(bool successful)
{
    m_passwordEdit->hide();
    m_messageButton->show();
    is_showMessageBtn = true;
    Q_EMIT showMessageBtn(is_showMessageBtn);
    m_messageButton->setFocus();
    m_messageButton->setDefault(true);

    connect(m_messageButton, &QPushButton::clicked,
            this, &LoginWindow::onMessageButtonClicked);
    if(successful) {
        isretry = false;
        m_messageButton->setText(tr("Login"));
        QString strMsgTip = m_messageLabel->text();
        switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
        if (!strMsgTip.isEmpty()) {
            m_messageLabel->setText(strMsgTip);
            m_messageLabel->move(m_messageButton->x(), m_messageLabel->y());
        }
    } else {
        isretry = true;
        m_messageButton->setText(tr("Retry"));
    }
}

bool LoginWindow::checkAuthOK(QString strUserName)
{
    bool isAuthenticated = m_greeter->isAuthenticated();
    qDebug()<<"checkAuthOK:"<<isAuthenticated<<","<<m_name<<","<<strUserName;
    if (isAuthenticated && m_name == strUserName) {
        Configuration::instance()->saveLastLoginUser(m_name);
        // 三权分立不保存秒级登录用户信息
        #ifdef WITHKYSEC
        if (kysec_is_disabled() || !kysec_get_3adm_status()) {
        #endif
            Configuration::instance()->saveLastLoginUser1(m_name);
        #ifdef WITHKYSEC
        }
        #endif
        m_greeter->startSession();
        return true;
    }
    return false;
}

void LoginWindow::onMessageButtonClicked()
{
    m_messageButton->setDefault(false);
    if(!isretry)
    {
        m_messageButton->hide();
        Configuration::instance()->saveLastLoginUser(m_name);
        // 三权分立不保存秒级登录用户信息
        #ifdef WITHKYSEC
        if (kysec_is_disabled() || !kysec_get_3adm_status()) {
        #endif
            Configuration::instance()->saveLastLoginUser1(m_name);
        #ifdef WITHKYSEC
        }
        #endif
        m_greeter->startSession();
    }
    else
    {
        m_messageButton->hide();
        is_showMessageBtn = false;
        authMode = PASSWORD;

        m_messageLabel->setText("");
        startAuthentication();
    }

}

bool LoginWindow::getLineeditStatus()
{
    return is_showMessageBtn;
}

void LoginWindow::showTooltip(QString text)
{
    Q_EMIT setKToolTip(text);
}

void LoginWindow::hideTooltip()
{
    Q_EMIT hideKTooltip();
}

void LoginWindow::onM_passwordEditClicked()
{
    Q_EMIT clickPassword(true);
}

void LoginWindow::restartBioAuth()
{
    startBioAuth();
}

void LoginWindow::onPrepareForSleep(bool sleep)
{
    ///系统休眠时，会关闭总线，导致设备不可用，发生错误
    ///在系统休眠之前停止认证，在系统唤醒后重新开始认证
    if(sleep)
    {
        if(useDoubleAuth){
            if(m_widgetLoginOpts){
                manualStopBio = true;
                m_widgetLoginOpts->stopAuth();
                if(m_bioTimer && m_bioTimer->isActive())
                    m_bioTimer->stop();
            }
        }
    }
    else
    {
        if(useDoubleAuth){
            manualStopBio = false;
            restartBioAuth();
        }
    }
}

void LoginWindow::setDirLogin()
{
    direct_login = true;
}

void LoginWindow::pamBioSuccess()
{
    m_bioTimer->stop();
    if (!m_widgetLoginOpts) {
        switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
        return ;
    }

    if(m_deviceInfo && m_deviceInfo->deviceType == LOGINOPT_TYPE_GENERAL_UKEY){
       //ukey时不调用ukey认证 
    }else{
        m_widgetLoginOpts->startAuth(m_deviceInfo, m_uid);
    }

    if (m_deviceInfo) {
        switchLoginOptType(m_widgetLoginOpts->convertDeviceType(m_deviceInfo->deviceType));
    } else {
        switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
    }
}

void LoginWindow::startBioAuth(unsigned uTimeout)
{
    if(m_widgetLoginOpts)
        m_widgetLoginOpts->stopAuth();

    if(m_deviceInfo && m_deviceInfo->deviceType == LOGINOPT_TYPE_GENERAL_UKEY){
        //ukey时不调用ukey认证
       switchLoginOptType(m_widgetLoginOpts->convertDeviceType(m_deviceInfo->deviceType));
       return;
    }

    if(!m_bioTimer){
        m_bioTimer = new QTimer(this);
        connect(m_bioTimer, SIGNAL(timeout()), this, SLOT(pamBioSuccess()));
    }
    m_bioTimer->start(uTimeout);
}

void LoginWindow::waitBiometricServiceStatus()
{
    qDebug()<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~` get biometric status";
    QDBusInterface iface("org.freedesktop.systemd1", "/org/freedesktop/systemd1",
                         "org.freedesktop.systemd1.Manager",QDBusConnection::systemBus());

    QDBusReply<QDBusObjectPath> bioPath = iface.call("GetUnit","biometric-authentication.service");
    if(!bioPath.isValid()){
        return ;
    }

    QDBusInterface bioface("org.freedesktop.systemd1", bioPath.value().path(),
                           "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());
    QDBusReply<QDBusVariant> sessionReply = bioface.call("Get", "org.freedesktop.systemd1.Unit", "UnitFileState");
    if(!sessionReply.isValid())
        qWarning() << sessionReply.error();
    else {
        QString res = sessionReply.value().variant().toString();
        if(res == "disable")
            return;
    }

    qDebug()<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~   get activeState";
    int times = 0;
    while(times<20){
        QDBusReply<QDBusVariant> sessionReply = bioface.call("Get", "org.freedesktop.systemd1.Unit", "ActiveState");
        if(!sessionReply.isValid()){
            qWarning() << sessionReply.error();
            return ;
        }
        else {
            QString res = sessionReply.value().variant().toString();
            if(res == "activating"){
                times ++;
                usleep(100000);
            }else{
                break;
            }
        }
    }
    qDebug()<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ times = "<<times;
}

void LoginWindow::performBiometricAuth()
{
    if(!m_biometricProxy)
    {
        waitBiometricServiceStatus();
        ServiceManager *sm = ServiceManager::instance();
        connect(sm, &ServiceManager::serviceStatusChanged,
                this, &LoginWindow::onBiometricDbusChanged);
        m_biometricProxy = new BiometricProxy(this);
        if (m_uniauthService && m_uniauthService->isActivatable()) {
            isHiddenSwitchButton = m_uniauthService->getHiddenSwitchButton();
            maxFailedTimes = m_uniauthService->getMaxFailedTimes();
        } else {
            isHiddenSwitchButton = GetHiddenSwitchButton();
            maxFailedTimes = GetFailedTimes();
        }
    }

    //服务没启动，或者打开DBus连接出错
    if(!m_biometricProxy->isValid())
    {
        qWarning() << "An error occurs when connect to the biometric DBus";
        useDoubleAuth = false;
        //skipBiometricAuth();
        if (m_deviceInfo) {
            if (!m_widgetLoginOpts || !m_widgetLoginOpts->findDeviceById(m_deviceInfo->id)
                || m_widgetLoginOpts->isDeviceDisable(m_deviceInfo->id)) {
                m_deviceInfo = DeviceInfoPtr();
            }
        }
        switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
        return;
    }

    //初始化生物识别认证UI
    initBiometricWidget();

    //没有可用设备，不启用生物识别认证
    if(m_widgetLoginOpts->getLoginOptCount() < 1) {
        useDoubleAuth = false;
        qWarning() << "No available devices";
        //skipBiometricAuth();
        if (m_deviceInfo) {
            if (!m_widgetLoginOpts || !m_widgetLoginOpts->findDeviceById(m_deviceInfo->id)
                || m_widgetLoginOpts->isDeviceDisable(m_deviceInfo->id)) {
                m_deviceInfo = DeviceInfoPtr();
            }
        }
        return;
    }

    if(!useDoubleAuth)
        clearMessage();

    //获取默认设备
    if (m_widgetLoginOpts) {
        if(m_name == "*login")
            m_deviceName = m_widgetLoginOpts->getDefaultDevice(manualLoginName);
        else
            m_deviceName = m_widgetLoginOpts->getDefaultDevice(m_name);

        if (m_isCustomDefault) {    // 认证插件配置默认插件认证
            if (m_widgetLoginOpts) {
                m_deviceName = m_widgetLoginOpts->getCustomDevName();
            }
        }
    }
    qDebug() << m_deviceName;
    if (m_deviceInfo) {
        if (!m_widgetLoginOpts || !m_widgetLoginOpts->findDeviceById(m_deviceInfo->id)
            || m_widgetLoginOpts->isDeviceDisable(m_deviceInfo->id)) {
            m_deviceInfo = DeviceInfoPtr();
        }
    }
    //如果默认设备为空的话，第一次不启动生物识别认证
    if(m_deviceName.isEmpty() && !m_deviceInfo)
    {
        qDebug() << "No default device";
        useDoubleAuth = false;
        //skipBiometricAuth();
        return;
    }
    //第一次，获取默认设备的设备信息，之后使用的则是从设备选择窗口传出的设备信息
    if(!m_deviceInfo)
    {
        m_deviceInfo = m_widgetLoginOpts->findDeviceByName(m_deviceName);
        if(!m_deviceInfo)
        {
            qDebug() << "Device Not found: " << m_deviceName;
            useDoubleAuth = false;
            //skipBiometricAuth();
            return;
        }
    }

    //已经错误超过3次
    if(m_failMap.contains(m_uid) && m_failMap[m_uid][m_deviceInfo->id] >= maxFailedTimes)
    {
        if (m_deviceInfo->deviceType == REMOTE_QRCODE_TYPE) {
            setLoginTypeTip(tr("Failed to verify %1, please enter password to unlock").arg(DeviceType::getDeviceType_tr(m_deviceInfo->deviceType)));
        } else {
            setLoginTypeTip(tr("Unable to verify %1, please enter password to unlock").arg(DeviceType::getDeviceType_tr(m_deviceInfo->deviceType)));
            if (m_deviceInfo->deviceType == DeviceType::Face) {
                QImage imgFailed;
                setFaceImg(imgFailed, 1);
            }
        }
        m_widgetLoginOpts->setDeviceDisable(m_deviceInfo->id, true);
        useDoubleAuth = false;
        //skipBiometricAuth();
        return;
    }

    if(m_deviceInfo->deviceType == LOGINOPT_TYPE_GENERAL_UKEY){
        switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
        m_widgetLoginOpts->setSelectedPassword();
        //skipBiometricAuth();
        return ;
    }

    switchLoginOptType(m_widgetLoginOpts->convertDeviceType(m_deviceInfo->deviceType));
    // 自行校验生物设备，pam greeter校验密码
    startBioAuth();
    //skipBiometricAuth();
}

void LoginWindow::skipBiometricAuth()
{
    m_greeter->respond(BIOMETRIC_IGNORE);
}

void LoginWindow::initBiometricWidget()
{
    if(m_widgetLoginOpts) {
        m_widgetLoginOpts->setUser(m_uid);
    } else {
        m_widgetLoginOpts = new LoginOptionsWidget(m_biometricProxy, m_uid, m_uniauthService, this);
        connect(m_widgetLoginOpts, &LoginOptionsWidget::authComplete,
                this, &LoginWindow::onBiometricAuthComplete);
        connect(m_widgetLoginOpts, &LoginOptionsWidget::optionSelected,
                this, &LoginWindow::onDeviceChanged);
        connect(m_widgetLoginOpts, &LoginOptionsWidget::updateImage,
                this, &LoginWindow::onLoginOptImage);
        connect(m_widgetLoginOpts, &LoginOptionsWidget::setLoadingImage,
                this, &LoginWindow::onLoadingImage);
        connect(m_widgetLoginOpts, &LoginOptionsWidget::notifyOptionsChange,
                this, &LoginWindow::onLoginOptsCount);
        connect(m_widgetLoginOpts, &LoginOptionsWidget::updateAuthMsg,
                this, &LoginWindow::setLoginMsg);
        connect(m_widgetLoginOpts, &LoginOptionsWidget::showKToolTipClicked,
                this, &LoginWindow::showTooltip);
        connect(m_widgetLoginOpts, &LoginOptionsWidget::hideKTooltipClicked,
                this, &LoginWindow::hideTooltip);
        m_widgetLoginOpts->setUser(m_uid);
    }

    qDebug()<<"OptsCount:"<<m_widgetLoginOpts->getLoginOptCount();
    if (m_widgetLoginOpts->getLoginOptCount() < 1){
	switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
        m_widgetLoginOpts->hide();
    }else{
        m_widgetLoginOpts->show();
    }

    if(m_widgetLoginOpts->getHasUkeyOptions() && m_widgetLoginOpts->getLoginOptCount() < 1){
        m_widgetLoginOpts->show();
        m_widgetLoginOpts->setSelectedPassword();
    }

    m_widgetLoginOpts->setEnabled(true);

    setBiometricWidgetGeometry();
}

void LoginWindow::setBiometricWidgetGeometry()
{
    //生物识别
    if(m_widgetLoginOpts)
    {
        m_widgetLoginOpts->setGeometry(0, m_passwdWidget->geometry().bottom(),
                                           width(), 92*scale);
        qDebug()<<"LoginOptGeometry:"<<m_widgetLoginOpts->geometry()<<","<<m_passwdWidget->geometry()
               <<","<<geometry()<<",isHidden:"<<m_widgetLoginOpts->isHidden();
    }
}

void LoginWindow::onLoginOptsCount(unsigned uCount, bool is_bioBtn)
{
    qDebug()<<"----------------------onLoginOptsCount Count:"<<uCount;
    if (!m_widgetLoginOpts)
        return ;
    if (uCount > 0) {
        setBiometricWidgetGeometry();
        m_widgetLoginOpts->show();
        qDebug()<<"LoginOptsShow!!";
    } else {
        switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
        m_widgetLoginOpts->hide();
    }
    if (!m_deviceInfo || !m_widgetLoginOpts->findDeviceById(m_deviceInfo->id)
        || m_widgetLoginOpts->isDeviceDisable(m_deviceInfo->id)) {
        m_widgetLoginOpts->stopAuth();
        authMode = BIOMETRIC;
        //初始化生物识别认证UI
        initBiometricWidget();

        //没有可用设备，不启用生物识别认证
        if(m_widgetLoginOpts->getLoginOptCount() < 1) {
            useDoubleAuth = false;
            qWarning() << "No available devices";
            if (m_deviceInfo) {
                m_deviceInfo = DeviceInfoPtr();
            }
            return;
        }

        if(!useDoubleAuth)
            clearMessage();

        //获取默认设备
        if(m_name == "*login"){
            if(isLoadingUkey){
                m_deviceName = m_widgetLoginOpts->getDefaultDevice(manualLoginName,UniT_General_Ukey);
            }else{
                m_deviceName = m_widgetLoginOpts->getDefaultDevice(manualLoginName);
            }
        } else{
            if(isLoadingUkey){
                m_deviceName = m_widgetLoginOpts->getDefaultDevice(m_name,UniT_General_Ukey);
            }else{
                m_deviceName = m_widgetLoginOpts->getDefaultDevice(m_name);
            }
        }

        qDebug() << m_deviceName;
        if (m_deviceInfo) {
            if (!m_widgetLoginOpts || !m_widgetLoginOpts->findDeviceById(m_deviceInfo->id)
                || m_widgetLoginOpts->isDeviceDisable(m_deviceInfo->id)) {
                m_deviceInfo = DeviceInfoPtr();
            }
        }
        //如果默认设备为空的话，第一次不启动生物识别认证
        if(m_deviceName.isEmpty() && !m_deviceInfo) {
            qDebug() << "No default device";
            useDoubleAuth = false;
            return;
        }
        //第一次，获取默认设备的设备信息，之后使用的则是从设备选择窗口传出的设备信息
        if(!m_deviceInfo) {
            m_deviceInfo = m_widgetLoginOpts->findDeviceByName(m_deviceName);
            if(!m_deviceInfo) {
                qDebug() << "Device Not found: " << m_deviceName;
                useDoubleAuth = false;
                return;
            }
        }

        //已经错误超过3次
        if(m_failMap.contains(m_uid) && m_failMap[m_uid][m_deviceInfo->id] >= maxFailedTimes)
        {
            if (m_deviceInfo->deviceType == REMOTE_QRCODE_TYPE) {
                setLoginTypeTip(tr("Failed to verify %1, please enter password to unlock").arg(DeviceType::getDeviceType_tr(m_deviceInfo->deviceType)));
            } else {
                setLoginTypeTip(tr("Unable to verify %1, please enter password to unlock").arg(DeviceType::getDeviceType_tr(m_deviceInfo->deviceType)));
                if (m_deviceInfo->deviceType == DeviceType::Face) {
                    QImage imgFailed;
                    setFaceImg(imgFailed, 1);
                }
            }
            m_widgetLoginOpts->setDeviceDisable(m_deviceInfo->id, true);
            useDoubleAuth = false;
            return;
        }

        if (m_deviceInfo->deviceType == LOGINOPT_TYPE_GENERAL_UKEY){
            stopLoadingUkey();
        }

        if(m_deviceInfo){
            m_widgetLoginOpts->setCurrentDevice(m_deviceInfo);
            m_widgetLoginOpts->updateUIStatus();
        }

        switchLoginOptType(m_widgetLoginOpts->convertDeviceType(m_deviceInfo->deviceType));
        // 自行校验生物设备，pam greeter校验密码
        startBioAuth();
    }
    if(is_bioBtn && uCount <= 1)
        Q_EMIT loginOptionClicked();
}

void LoginWindow::onLoginOptImage(QImage img)
{
    if (m_uCurLoginOptType == LOGINOPT_TYPE_FACE) {
        setFaceImg(img);
    } else if (m_uCurLoginOptType == LOGINOPT_TYPE_QRCODE) {
        setQRCode(img);
    }
    if(w_timer && w_timer->isActive())
    {
        w_timer->stop();
    }
}

void LoginWindow::onLoadingImage()
{
    if(!w_timer)
    {
        w_timer = new QTimer(this);
        w_timer->setInterval(150);
        connect(w_timer, &QTimer::timeout, this, &LoginWindow::updatePixmap);
    }
    m_waitingPixmap = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(24, 24);
    m_labelFace->setAlignment(Qt::AlignCenter);
    if (m_uCurLoginOptType == LOGINOPT_TYPE_FACE) {
        m_labelFace->setPixmap(m_waitingPixmap);
        m_labelQRCodeTip->show();
    } else if (m_uCurLoginOptType == LOGINOPT_TYPE_QRCODE) {
        m_labelQRCodeTip->setPixmap(m_waitingPixmap);
        m_labelQRCodeTip->show();
    }
    w_timer->start();
}

void LoginWindow::onDeviceChanged(unsigned uCurLoginOptType, const DeviceInfoPtr &deviceInfo, bool keyNavigation)
{
    if(!keyNavigation)
        Q_EMIT loginOptionClicked();
    isLoadingUkey = false;
    if(uCurLoginOptType == LOGINOPT_TYPE_PASSWORD){
        switchLoginOptType(uCurLoginOptType);
        if(m_widgetLoginOpts){
            m_widgetLoginOpts->stopAuth();
        }
        m_deviceInfo = nullptr;
        authMode = PASSWORD;
        startAuthentication();
        return;
    }
    if (uCurLoginOptType != LOGINOPT_TYPE_GENERAL_UKEY && !deviceInfo)
        return;

    if(deviceInfo)
        qDebug() << "device changed: " << *deviceInfo;
    if(deviceInfo && m_failMap[m_uid][deviceInfo->id] >= maxFailedTimes){
        qDebug() << "Failed MAX!!";
        return ;
    }

    if (uCurLoginOptType == LOGINOPT_TYPE_GENERAL_UKEY && !deviceInfo){
        isLoadingUkey = true;
        startLoadingUkey();
    }else if(uCurLoginOptType == LOGINOPT_TYPE_GENERAL_UKEY && deviceInfo){
        stopLoadingUkey();
    }

    if (uCurLoginOptType != LOGINOPT_TYPE_GENERAL_UKEY && deviceInfo == m_deviceInfo) {
        return ;
    }

    if(m_bioTimer && m_bioTimer->isActive())
        m_bioTimer->stop();

    authMode = BIOMETRIC;
    m_deviceInfo = deviceInfo;
    switchLoginOptType(uCurLoginOptType);
    if(!isBioSuccess && deviceInfo)
        startBioAuth();
}

void LoginWindow::onBiometricAuthComplete(bool result, int nStatus)
{
    if(!result) {
    if(m_deviceInfo && m_deviceInfo->deviceType == LOGINOPT_TYPE_GENERAL_UKEY && m_ukeyPasswdWidget->isVisible()){
            m_ukeyPasswordEdit->stopWaiting();
            m_ukeyPasswordEdit->clearText();
        }

        if(manualStopBio){
            manualStopBio = false;
            return;
        }

        if (nStatus == 5 && m_deviceInfo) {
            if(w_timer && w_timer->isActive())
                w_timer->stop();
            QImage imgFailed;
            setFaceImg(imgFailed, 2);
            return;
        } else if (nStatus >= 2 && nStatus != 5)if (nStatus >= 2) {
            if (m_deviceInfo) {
                if(m_failMap.contains(m_uid) && m_failMap[m_uid].contains(m_deviceInfo->id)){
                    m_failMap[m_uid][m_deviceInfo->id] = m_failMap[m_uid][m_deviceInfo->id] + 1;
                }else{
                    m_failMap[m_uid][m_deviceInfo->id] = 1;
                }
                qDebug()<<"Failed count:"<<m_failMap[m_uid][m_deviceInfo->id]<<",Max:"<<maxFailedTimes;
                if (m_deviceInfo->deviceType == DeviceType::Face) {
                    QImage imgFailed;
                    setFaceImg(imgFailed, 1);
                }
                if(m_failMap[m_uid][m_deviceInfo->id] >= maxFailedTimes){
                    if (m_deviceInfo->deviceType == REMOTE_QRCODE_TYPE) {
                        setLoginTypeTip(tr("Failed to verify %1, please enter password to unlock").arg(DeviceType::getDeviceType_tr(m_deviceInfo->deviceType)));
                        QImage nullImage;
                        setQRCode(nullImage);
                    }else if(m_deviceInfo->deviceType == LOGINOPT_TYPE_GENERAL_UKEY){
                        setUkeyTypeTip(tr("Unable to verify %1, please enter password to unlock").arg(DeviceType::getDeviceType_tr(m_deviceInfo->deviceType)));
                    }else {
                        setLoginTypeTip(tr("Unable to verify %1, please enter password to unlock").arg(DeviceType::getDeviceType_tr(m_deviceInfo->deviceType)));
                    }
                    useDoubleAuth = false;
		    if(m_widgetLoginOpts)
                        m_widgetLoginOpts->setDeviceDisable(m_deviceInfo->id, true);
                    
		    if(m_ukeyPasswdWidget && m_ukeyPasswdWidget->isVisible()){
                        m_ukeyPasswordEdit->readOnly(true);
                    }

		    return ;
                }

		if(m_deviceInfo->deviceType == LOGINOPT_TYPE_GENERAL_UKEY){
                    setUkeyTypeTip(tr("Failed to verify %1, you still have %2 verification opportunities")
                                   .arg(DeviceType::getDeviceType_tr(m_deviceInfo->deviceType))
                                   .arg(maxFailedTimes-m_failMap[m_uid][m_deviceInfo->id]));
                }else {
                    setLoginTypeTip(tr("Failed to verify %1, you still have %2 verification opportunities")
                        .arg(DeviceType::getDeviceType_tr(m_deviceInfo->deviceType))
                        .arg(maxFailedTimes-m_failMap[m_uid][m_deviceInfo->id]));
                }
	    }
        }
        if(!isBioSuccess) {
            if (nStatus <= 0) {
                qDebug()<<"Biometric dbus error:"<<nStatus;
            }
            if (m_uCurLoginOptType == LOGINOPT_TYPE_QRCODE && nStatus == 1) {
                setQRCodeMsg(tr("Abnormal network"));
                startBioAuth(10000);
            } else {
                startBioAuth();
            }
            if (nStatus >= 2 && m_deviceInfo) {
                if (m_deviceInfo->deviceType == DeviceType::Face) {
                    QImage imgFailed;
                    setFaceImg(imgFailed, 1);
                }
            }
        }
    } else {
        //onShowMessage("验证成功!", QLightDM::Greeter::MessageTypeInfo);
        setDirLogin();
        isBioSuccess = true;
        onBiometricButtonClicked();
    }
}

void LoginWindow::onBiometricButtonClicked()
{
    //当前没有设备
    if (!m_deviceInfo) {
        qWarning()<<"未选择生物设备--";
        return ;
    }
    authMode = BIOMETRIC;
    startAuthentication();
}

void LoginWindow::onPasswordButtonClicked()
{
    skipBiometricAuth();
}

void LoginWindow::onOtherDevicesButtonClicked()
{
    if(m_widgetLoginOpts)
        m_widgetLoginOpts->stopAuth();
}

void LoginWindow::onRetryButtonClicked()
{
    if (m_widgetLoginOpts)
        m_widgetLoginOpts->startAuth(m_deviceInfo, m_uid);
    //m_retryButton->setVisible(false);
}

bool LoginWindow::unlock_countdown()
{
    int failed_count = 0;
    int time_left = 0;
    int deny = 0;
    int fail_time =0;
    int unlock_time = 0;
    
    
    if (NULL == m_name.toLatin1().data())
    {
        qDebug() << "get user info failed." ;
        return false;
    }
    
    struct passwd* user = NULL;
    user = getpwnam(m_name.toLatin1().data());
    if(user)
        pam_tally_unlock_time_left(user->pw_uid, &failed_count, &time_left, &deny,&fail_time,&unlock_time);
    else
        return false;

    // qDebug() << "user->pw_uid = " << user->pw_uid << "failed_count:" << failed_count << "time_left:" <<time_left <<"deny:"<<deny<<"fail_time:"<< fail_time<<"unlock_time:" << unlock_time;
    if(time_left >= 60)//请多少分钟后重试
    {
        int nMinuteleft = time_left/60;
        if (isLockingFlg == false){
            m_nCurLockMin = unlock_time/60;//获取当前需要锁定的分钟数
        }

        //如果当前设置的不是1min钟锁定，那么1min显示成2min，由2min直接跳到59s || 剩余分钟数小于当前设置的锁定时间，并且大于1min，自增+1
        if ((nMinuteleft == 1 && m_nCurLockMin != 1)  || (nMinuteleft > 1 && nMinuteleft < m_nCurLockMin))
        {
            nMinuteleft = nMinuteleft + 1;
        }
        m_messageLabel->setText(tr("Please try again in %1 minutes.").arg(nMinuteleft));
        m_messageLabelTip = (tr("Please try again in %1 minutes.").arg(nMinuteleft));
        m_passwordEdit->clearText();
        m_passwordEdit->readOnly(true);
        m_passwordEdit->setLocked(true);
        isLockingFlg = true;
        return true;
    }
    else if(time_left > 0 && time_left < 60)//请多少秒后重试
    {
        m_messageLabel->setText(tr("Please try again in %1 seconds.").arg(time_left%60));
        m_messageLabelTip = (tr("Please try again in %1 seconds.").arg(time_left%60));
        m_passwordEdit->clearText();
        m_passwordEdit->readOnly(true);
        m_passwordEdit->setLocked(true);
        isLockingFlg = true;
        return true;
    }
    else if (failed_count == 0xFFFF)//账号被永久锁定
    {
        m_messageLabel->setText(tr("Account locked permanently."));
        m_messageLabelTip = (tr("Account locked permanently."));
        m_passwordEdit->clearText();
        m_passwordEdit->readOnly(true);
        m_passwordEdit->setLocked(true);
        isLockingFlg = true;
        return true;
    }
    else
    {
        if(m_passwordEdit){
            m_passwordEdit->readOnly(false);
            m_passwordEdit->setLocked(false);
            if (m_passwordEdit->isVisible())
                m_passwordEdit->setFocus();
        }
        //    qDebug() << "清理 1 clearMessage";
        if (isLockingFlg)
        {
            onShowMessage("", QLightDM::Greeter::MessageTypeError);
            //onShowMessage(tr("Authentication failure, Please try again"), QLightDM::Greeter::MessageTypeInfo);
            isLockingFlg = false;
        }
        if (m_timer)
            m_timer->stop();
    }
    return false;
}

void LoginWindow::root_unlock_countdown()
{
    int failed_count = 0;
    int time_left = 0;
    int deny = 0;
    int fail_time =0;
    int unlock_time = 0;
    if (NULL == m_name.toLatin1().data())
    {
        qDebug() << "get user info failed." ;
        return;
    }
    
    struct passwd* user = NULL;
    user = getpwnam(m_name.toLatin1().data());
    pam_tally_root_unlock_time_left(user->pw_uid, &failed_count, &time_left, &deny,&fail_time,&unlock_time);
    if(time_left >= 60)//请多少分钟后重试
    {
        int nMinuteleft = time_left/60;
        if (isLockingFlg == false){
            m_nCurLockMin = unlock_time/60;//获取当前需要锁定的分钟数
        }

        //如果当前设置的不是1min钟锁定，那么1min显示成2min，由2min直接跳到59s || 剩余分钟数小于当前设置的锁定时间，并且大于1min，自增+1
        if ((nMinuteleft == 1 && m_nCurLockMin != 1)  || (nMinuteleft > 1 && nMinuteleft < m_nCurLockMin))
        {
            nMinuteleft = nMinuteleft + 1;
        }

        m_messageLabel->setText(tr("Please try again in %1 minutes.").arg(nMinuteleft));
        m_messageLabelTip = (tr("Please try again in %1 minutes.").arg(nMinuteleft));
        m_passwordEdit->clearText();
        m_passwordEdit->readOnly(true);
        m_passwordEdit->setLocked(true);
        isLockingFlg = true;
        return ;
    }
    else if(time_left > 0 && time_left < 60)//请多少秒后重试
    {
        char ch[100]={0};
        m_messageLabel->setText(tr("Please try again in %1 seconds.").arg(time_left%60));
        m_messageLabelTip = (tr("Please try again in %1 seconds.").arg(time_left%60));
        m_passwordEdit->clearText();
        m_passwordEdit->readOnly(true);
        m_passwordEdit->setLocked(true);
        isLockingFlg = true;
        return ;
    }
    else if (failed_count == 0xFFFF)//账号被永久锁定
    {
        m_messageLabel->setText(tr("Account locked permanently."));
        m_messageLabelTip = (tr("Account locked permanently."));
        m_passwordEdit->clearText();
        m_passwordEdit->readOnly(true);
        m_passwordEdit->setLocked(true);
        isLockingFlg = true;
        return ;
    }
    else
    {
        if(m_passwordEdit){
            m_passwordEdit->readOnly(false);
            m_passwordEdit->setLocked(false);
            if (m_passwordEdit->isVisible())
                m_passwordEdit->setFocus();
        }
        if (isLockingFlg)
        {
            onShowMessage("", QLightDM::Greeter::MessageTypeError);
            //onShowMessage(tr("Authentication failure, Please try again"), QLightDM::Greeter::MessageTypeInfo);
            isLockingFlg = false;
        }
        if (m_timer)
            m_timer->stop();
    }
    return ;
}

void LoginWindow::setUkeyTypeTip(QString text)
{
    QString textTip = text;
    if (!textTip.isEmpty()) {
//        QFontMetrics font(m_ukeyMessageLabel->font());
//        QString textTip = font.elidedText(textTip, Qt::ElideRight, m_messageLabel->width()-8);
        m_ukeyMessageLabel->setText(textTip);
        m_ukeyMessageLabel->setToolTip(text);
        m_ukeyMessageLabel->show();
    } else {
        m_ukeyMessageLabel->hide();
    }
}

void LoginWindow::setLoginTypeTip(QString strLoginTypeTip)
{
    m_strLoginTypeTip = strLoginTypeTip;
    if (!m_strLoginTypeTip.isEmpty()) {
        QFontMetrics font(m_labelLoginTypeTip->font());
        QString strDisplay = font.elidedText(m_strLoginTypeTip, Qt::ElideRight, m_messageLabel->width()-8);
        m_labelLoginTypeTip->setText(strDisplay);
        m_labelLoginTypeTip->show();
    } else {
        m_labelLoginTypeTip->hide();
    }
}

void LoginWindow::setLoginMsg(QString strLoginMsg)
{
    setLoginTypeTip(strLoginMsg);
}

void LoginWindow::onBiometricDbusChanged(bool bActive)
{
    qDebug()<<"BiometricDbus:"<<bActive;
    if (bActive) {
        QTimer::singleShot(1000, this, [=](){
            qDebug()<<"OnDelay init biometric!!";
            if(!m_biometricProxy) {
                m_biometricProxy = new BiometricProxy(this);
                if (m_uniauthService && m_uniauthService->isActivatable()) {
                    isHiddenSwitchButton = m_uniauthService->getHiddenSwitchButton();
                    maxFailedTimes = m_uniauthService->getMaxFailedTimes();
                } else {
                    isHiddenSwitchButton = GetHiddenSwitchButton();
                    maxFailedTimes = GetFailedTimes();
                }
            }

            //服务没启动，或者打开DBus连接出错
            if(!m_biometricProxy->isValid()) {
                qWarning() << "An error occurs when connect to the biometric DBus";
                useDoubleAuth = false;
                if (m_deviceInfo) {
                    if (!m_widgetLoginOpts || !m_widgetLoginOpts->findDeviceById(m_deviceInfo->id)
                        || m_widgetLoginOpts->isDeviceDisable(m_deviceInfo->id)) {
                        m_deviceInfo = DeviceInfoPtr();
                    }
                }
                switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
                return;
            }

            //初始化生物识别认证UI
            initBiometricWidget();

            //没有可用设备，不启用生物识别认证
            if(m_widgetLoginOpts->getLoginOptCount() < 1) {
                useDoubleAuth = false;
                qWarning() << "No available devices";
                if (m_deviceInfo) {
                    if (!m_widgetLoginOpts || !m_widgetLoginOpts->findDeviceById(m_deviceInfo->id)
                        || m_widgetLoginOpts->isDeviceDisable(m_deviceInfo->id)) {
                        m_deviceInfo = DeviceInfoPtr();
                    }
                }
                return;
            }

            if(!useDoubleAuth)
                clearMessage();

            //获取默认设备
            if (m_widgetLoginOpts) {
                if(m_name == "*login")
                    m_deviceName = m_widgetLoginOpts->getDefaultDevice(manualLoginName);
                else
                    m_deviceName = m_widgetLoginOpts->getDefaultDevice(m_name);
            }
            qDebug() << m_deviceName;
            if (m_deviceInfo) {
                if (!m_widgetLoginOpts || !m_widgetLoginOpts->findDeviceById(m_deviceInfo->id)
                    || m_widgetLoginOpts->isDeviceDisable(m_deviceInfo->id)) {
                    m_deviceInfo = DeviceInfoPtr();
                }
            }
            //如果默认设备为空的话，第一次不启动生物识别认证
            if(m_deviceName.isEmpty() && !m_deviceInfo) {
                qDebug() << "No default device";
                useDoubleAuth = false;
                return;
            }
            //第一次，获取默认设备的设备信息，之后使用的则是从设备选择窗口传出的设备信息
            if(!m_deviceInfo) {
                m_deviceInfo = m_widgetLoginOpts->findDeviceByName(m_deviceName);
                if(!m_deviceInfo) {
                    qDebug() << "Device Not found: " << m_deviceName;
                    useDoubleAuth = false;
                    return;
                }
            }

            //已经错误超过3次
            if(m_failMap.contains(m_uid) && m_failMap[m_uid][m_deviceInfo->id] >= maxFailedTimes) {
                if (m_deviceInfo->deviceType == REMOTE_QRCODE_TYPE) {
                    setLoginTypeTip(tr("Failed to verify %1, please enter password to unlock").arg(DeviceType::getDeviceType_tr(m_deviceInfo->deviceType)));
                } else {
                    setLoginTypeTip(tr("Unable to verify %1, please enter password to unlock").arg(DeviceType::getDeviceType_tr(m_deviceInfo->deviceType)));
                    if (m_deviceInfo->deviceType == DeviceType::Face) {
                        QImage imgFailed;
                        setFaceImg(imgFailed, 1);
                    }
                }
                m_widgetLoginOpts->setDeviceDisable(m_deviceInfo->id, true);
                useDoubleAuth = false;
                return;
            }

            if(m_deviceInfo->deviceType == LOGINOPT_TYPE_GENERAL_UKEY){
                switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
                m_widgetLoginOpts->setSelectedPassword();
                return ;
            }

            switchLoginOptType(m_widgetLoginOpts->convertDeviceType(m_deviceInfo->deviceType));
            // 自行校验生物设备，pam greeter校验密码
            startBioAuth();
        });
    }
}

void LoginWindow::setWillLoginUser(QString strUserName, bool isOneKeyLogin)
{
    m_strServiceUser = strUserName;
    m_isOneKeyLogin = isOneKeyLogin;
}

void LoginWindow::onCustomRequestAccount(QString strName)
{
    qDebug()<<"onCustomRequestAccount:"<<strName;
    Q_EMIT customRequestAccount(strName);
}

void LoginWindow::onCustomAuthResult(int nResult, QString strMsg)
{
    if (m_uCurLoginOptType != LOGINOPT_TYPE_CUSTOM) {
        qDebug()<<"onCustomAuthResult error Current Auth type is not costom!!";
        return ;
    }
    if(nResult) {
        if(!isBioSuccess) {
            qDebug()<<"onCustomAuthResult:"<<nResult<<strMsg;
            startBioAuth();
        }
    } else {
        qDebug()<<"onCustomAuthResult success!!";
        setDirLogin();
        isBioSuccess = true;
        onBiometricButtonClicked();
    }
}

QString LoginWindow::onCustomRequest(QString strReqJson)
{
    qDebug()<<"onCustomRequest:"<<strReqJson;
    QJsonParseError jsonParseError;
    QJsonObject retObj;
    retObj["Ret"] = 0;
    retObj["Message"] = "Success";
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(strReqJson.toUtf8(), &jsonParseError);
    if (jsonParseError.error != QJsonParseError::NoError) {
        retObj["Ret"] = -1;
        retObj["Message"] = "Parse request json failed!!";
        qWarning()<<"Parse request json failed!!";
        return QString(QJsonDocument(retObj).toJson());
    } else {
        QJsonObject rootObj = jsonDoc.object();
        if (rootObj.isEmpty()) {
             qWarning()<<"JSON串为空";
             retObj["Ret"] = -1;
             retObj["Message"] = "Request Json is null!!";
             qWarning()<<"Request Json is null!!";
             return QString(QJsonDocument(retObj).toJson());
        } else {
            QJsonObject contentObj;
            QString reqType = rootObj.value("ReqType").toString();
            qInfo() << "Req type: " << reqType;
            if (reqType == "GetInformations") { // 请求信息
                QString strType = rootObj.value("Type").toString();
                if (strType.contains("CurrentUser")) {  // 当前用户信息
                    if (m_uid >= 0 && !m_name.isEmpty() && !m_name.startsWith("*")) {
                        QJsonObject user;
                        user["Name"] = m_name;
                        user["Id"] = m_uid;
                        QFile faceFile(m_face);
                        if (!faceFile.exists()) {
                            user["HeadImg"] = m_strDefaultFace;
                        } else {
                            user["HeadImg"] = m_face;
                        }
                        user["RealName"] = m_strRealName;
                        contentObj["CurrentUser"] = user;
                    } else {
                        retObj["Ret"] = -1;
                        retObj["Message"] = "Current user is invalid!";
                    }
                }
            }
            retObj["Content"] = contentObj;
            return QString(QJsonDocument(retObj).toJson());
        }
    }
}

int LoginWindow::onCustomPlugEnv()
{
    return LoginPluginInterface::LOGINPLUGINENV_LOGIN;
}

void LoginWindow::setCustomAuthDefault(bool isDefault)
{
    m_isCustomDefault = isDefault;
}
