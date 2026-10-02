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
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMovie>
#include <QFile>
#include <QSpacerItem>
#include <QKeyEvent>
#include <QPainterPath>
#include <QDir>
#include <QFont>
#include <QLineEdit>
#include <QPainter>
#include <QPixmap>
#include <QFontMetrics>
#include <QtMath>
#include <QAbstractItemView>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <libintl.h>
#include <locale.h>
#include <QMatrix>
#include "bioauthwidget.h"
#include <QHBoxLayout>
#include <QAction>
#include <pwd.h>
#include "generic.h"
#include "biotypes.h"
#include "modeButton.h"
#include <QDebug>
#define _(string) gettext(string)
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

MainWindow::MainWindow(QWidget *parent)
    : kdk::KDialog(parent)
    , ui(new Ui::MainWindow)
    , users(new Users(this))
    , enableBioAuth(false)
    , receiveBioPAM(false)
    , authMode(UNDEFINED)
    , useDoubleAuth(false)
    , m_timer(nullptr)
    , w_timer(nullptr)
    , isLockingFlg(false)
    , m_nCurLockMin(0)
    , isbioSuccess(false)
{
    ui->setupUi(this);

    mainLayout()->addLayout(ui->verticalLayout_2);

    pam_tally_init(); //

    // 登录选项
    m_labelTip = new KALabel();
    m_labelTip->setText("");
    m_labelTip->setFixedHeight(30);
    m_labelTip->hide();
    m_loginOptsWidget = new LoginOptionsWidget();
    m_loginOptsWidget->hide();
    ui->titleTipLayout->addWidget(m_labelTip);
    ui->loginOptionsLayout->setAlignment(Qt::AlignTop | Qt::AlignCenter);
    ui->loginOptionsLayout->addWidget(m_loginOptsWidget);
    ui->loadingUkeyWidget->adjustSize();
    ui->loadingUkeyWidget->hide();
    ui->widgetUkeyAuth->hide();
    ui->loadingUkeyBtn->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->ukeyPassword->setEchoMode(QLineEdit::Password);
    ui->loadingUkeyLbl->setText(tr("Insert the ukey into the USB port"));
    ui->loadingUkeyLbl->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    ui->ukeyTipLbl->setText(tr("Enter the ukey password"));
    ui->ukeyPassword->setPlaceholderText(tr("Input Password"));
    ui->btnCancel->setAutoDefault(false);
    ui->btnCancel->setDefault(false);
    maxFailedTimes = bioDevices.getFailedTimes();
    isHiddenSwitchButton = bioDevices.GetHiddenSwitchButton();
    closeButton()->setToolTip(tr("Close"));

    // 连接登录选项信号响应槽
    connect(m_loginOptsWidget, &LoginOptionsWidget::optionSelected, this, &MainWindow::onOptionSelected);
    connect(m_loginOptsWidget, &LoginOptionsWidget::notifyOptionsChange, this, &MainWindow::onNotifyOptionsChanged);
    connect(m_loginOptsWidget, &LoginOptionsWidget::authComplete, this, &MainWindow::onLoginOptionsAuthCompleted);
    connect(m_loginOptsWidget, &LoginOptionsWidget::updateAuthMsg, this, &MainWindow::onUpdateBioAuthMsg);
    connect(m_loginOptsWidget, &LoginOptionsWidget::updateWndSize, this, &MainWindow::onUpdateWndSize);

    QFile qssFile(":/qss/src/main.qss");
    qssFile.open(QIODevice::ReadOnly);
    setStyleSheet(qssFile.readAll());
    qssFile.close();

    ui->lePassword->installEventFilter(this);
    // 初始化UI状态
    switchWidget(UNDEFINED);
    editIcon();
    ui->lblContent->adjustSize();
    ui->lblContent->hide();
    ui->lblMessage->setFixedSize(372, 30);
    ui->widgetPasswdAuth->adjustSize();
    ui->widgetUkeyAuth->adjustSize();
    ui->cmbUsers->view()->setTextElideMode(Qt::ElideRight);
    ui->cmbUsers->setFixedHeight(36);
    ui->lePassword->setFixedHeight(36);
    ui->btnBioAuth->hide();
    ui->returnButton->hide();
    ui->btnAuth->setProperty("isImportant", true);
    ui->btnCancel->setProperty("useButtonPalette", true);

    settings = new QGSettings("org.ukui.style", "", this);
    fontSize = settings->get("system-font-size").toInt();
    connect(settings, &QGSettings::changed, this, &MainWindow::onConfigurationChanged);

    QDBusInterface *interfaceScreensaver
        = new QDBusInterface("org.ukui.ScreenSaver", "/", "org.ukui.ScreenSaver", QDBusConnection::sessionBus());
    connect(interfaceScreensaver, SIGNAL(lock()), this, SLOT(onLockStatus()));
    connect(interfaceScreensaver, SIGNAL(unlock()), this, SLOT(onUnlockStatus()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onConfigurationChanged(QString key)
{
    if (key == "systemFontSize") {
        m_loginOptsWidget->updateUIStatus(false);
        fontSize = settings->get("system-font-size").toInt();
    }
    if (key == "iconThemeName") {
        setIcon(app_IconName);
    }
    if (key == "styleName") {
        QPalette pe;
        if (no_changes) {
            pe.setColor(QPalette::Text, Qt::red);
            m_labelTip->setPalette(pe);
        } else {
            QColor color = palette().color(QPalette::Text);
            QPalette pal(this->palette());
            pal.setColor(QPalette::Text, QColor(color));
            m_labelTip->setPalette(pal);
        }
    }
}

void MainWindow::restart_bio_identify()
{
    DeviceInfoPtr device = bioDevices.getDefaultDevice(getUid(userName));
    if (device) {
        m_loginOptsWidget->startAuth(device, getUid(userName));
        setMessage(tr("Please enter your password or enroll your fingerprint "));
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_loginOptsWidget) {
        m_loginOptsWidget->stopAuth();
    }
    m_failMap.clear();
    emit canceled();
    return kdk::KDialog::closeEvent(event);
}

void MainWindow::setUkeyTypeTip(QString text)
{
    QString textTip = text;
    if (!textTip.isEmpty()) {
        QPalette pe;
        pe.setColor(QPalette::Text, Qt::red);
        ui->ukeyMessage->setPalette(pe);
        ui->ukeyMessage->setText(textTip);
        ui->ukeyMessage->setToolTip(text);
        ui->ukeyMessage->show();
    } else {
        ui->ukeyMessage->hide();
    }
}

void MainWindow::startLoadingUkey()
{
    m_loadingPixmap = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(27, 27);
    ui->loadingUkeyBtn->setIcon(m_loadingPixmap);
    ui->loadingUkeyBtn->setIconSize(QSize(27, 27));

    ui->loadingUkeyWidget->adjustSize();
    ui->cmbUsers->hide();
    ui->widgetUkeyAuth->hide();
    ui->lePassword->hide();

    ui->widgetPasswdAuth->hide();

    isLoadingUkey = true;
    ui->loadingUkeyWidget->show();

    if (!m_loadingTimer) {
        m_loadingTimer = new QTimer(this);
        m_loadingTimer->setInterval(150);
        connect(m_loadingTimer, &QTimer::timeout, this, &MainWindow::updateLoadingPixmap);
    }
    m_loadingTimer->start();
}

void MainWindow::stopLoadingUkey()
{
    isLoadingUkey = false;
    if (m_loadingTimer) {
        m_loadingTimer->stop();
    }

    ui->loadingUkeyWidget->hide();
    // ui->cmbUsers->show();
    // ui->widgetUkeyAuth->show();
}

void MainWindow::updateLoadingPixmap()
{
    QMatrix matrix;
    matrix.rotate(90.0);
    m_loadingPixmap = m_loadingPixmap.transformed(matrix, Qt::FastTransformation);
    ui->loadingUkeyBtn->setIcon(QIcon(m_loadingPixmap));
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption *option = new QStyleOption();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(0, 0, 0, 0), 0, 0);
    // 画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(QColor(0, 0, 0, 100));
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();

    // 模糊这个黑底
    QImage img = pixmap.toImage();
    qt_blurImage(img, 10, false, false);

    // 挖掉中心
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);

    // 绘制阴影
    p.drawPixmap(this->rect(), pixmap, pixmap.rect());

    // 绘制一个背景
    p.save();
    p.fillPath(rectPath, option->palette.color(QPalette::Base));
    p.restore();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->lePassword) {
        if (event->type() == QEvent::KeyPress) { // 禁止复制粘贴功能。
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->matches(QKeySequence::Copy) || keyEvent->matches(QKeySequence::Cut)
                || keyEvent->matches(QKeySequence::Paste)) {
                event->ignore();
                return true;
            }
        }
        if (event->type() == QEvent::MouseButtonRelease) { // 禁用鼠标中键
            QMouseEvent *mouseevent = static_cast<QMouseEvent *>(event);
            if (mouseevent->button() == Qt::MidButton) {
                event->ignore();
                return true;
            }
        }
    }

    return kdk::KDialog::eventFilter(obj, event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Escape: {
            break;
        }
        default: {
            kdk::KDialog::keyPressEvent(event);
            break;
        }
    }
}

/*** main ***/
void MainWindow::on_cmbUsers_currentTextChanged(const QString &userName)
{
    qDebug() << "user changed to " << userName;
    m_loginOptsWidget->stopAuth();
    if (m_bioTimer && m_bioTimer->isActive())
        m_bioTimer->stop();
    authMode = UNDEFINED;

    QList<UserItem> accountUsers = users->getUsers();
    this->userName = userName;
    for (UserItem user : accountUsers) {
        if (user.realName == userName) {
            this->userName = user.name;
        }
    }

    m_deviceInfo = DeviceInfoPtr();
    ui->lblMessage->clear();
    ui->lblMessage->setToolTip("");
    setMessage("");
    isLockingFlg = false;
    emit userChanged(this->userName);
}

int MainWindow::enable_biometric_authentication()
{
    char conf_file[] = GET_STR(CONFIG_FILE);
    FILE *file;
    char line[1024], is_enable[16];
    int i;

    if ((file = fopen(conf_file, "r")) == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), file)) {
        i = sscanf(line, "EnableAuth=%s\n", is_enable);
        if (i > 0) {
            break;
        }
    }

    fclose(file);
    if (!strcmp(is_enable, "true"))
        return 1;
    return 0;
}

void MainWindow::on_btnDetails_clicked()
{ /*
     if(ui->widgetDetails->isHidden()) {
         ui->widgetDetails->show();
         ui->btnDetails->setIcon(QIcon(":/image/assets/arrow_down.svg"));
 //        resize(width(), height() + ui->widgetDetails->height());
     }
     else {
         ui->widgetDetails->hide();
         ui->btnDetails->setIcon(QIcon(":/image/assets/arrow_right.svg"));
 //        resize(width(), height() - ui->widgetDetails->height());
     }
     adjustSize();*/
}

void MainWindow::on_btnCancel_clicked()
{
    close();
}

void MainWindow::on_btnAuth_clicked()
{
    if (doNotNeedAuth) {
        emit yesPrompted();
        return;
    }

    if (ui->widgetUkeyAuth->isVisible()) {
        on_ukeyPassword_returnPressed();
    } else {
        on_lePassword_returnPressed();
    }
}

void MainWindow::onShowYesPrompt()
{
    doNotNeedAuth = true;
    ui->widgetPasswdAuth->hide();
    ui->cmbUsers->hide();
    ui->btnBioAuth->hide();
    ui->returnButton->hide();
    ui->btnAuth->show();
}

void MainWindow::onLockStatus()
{
    // m_loginOptsWidget->stopAuth();
    m_loginOptsWidget->lockStatusChanged(true);
}

void MainWindow::onUnlockStatus()
{
    // m_loginOptsWidget->readDevicesInfo();
    m_loginOptsWidget->lockStatusChanged(false);
    startBioAuth();
}

/*** pagePassword ***/
void MainWindow::editIcon()
{
    m_modeButton = new ModeButton(ui->lePassword);
    m_modeButton->setFocusPolicy(Qt::NoFocus);
    connect(m_modeButton, &QPushButton::clicked, this, [=]() {
        if (ui->lePassword->echoMode() == QLineEdit::Password) {
            ui->lePassword->setEchoMode(QLineEdit::Normal);
        } else {
            ui->lePassword->setEchoMode(QLineEdit::Password);
        }
    });
    QHBoxLayout *layout = new QHBoxLayout(ui->lePassword);
    layout->addStretch();
    layout->addWidget(m_modeButton);
    layout->setContentsMargins(0, 0, 8, 0);

    ui->lePassword->setTextMargins(1, 1, 4 + m_modeButton->width(), 1);
}

void MainWindow::on_ukeyPassword_returnPressed()
{
    if (m_loginOptsWidget && m_deviceInfo) {
        m_loginOptsWidget->SetExtraInfo(ui->ukeyPassword->text(), "pincode");
        m_loginOptsWidget->startAuth(m_deviceInfo, getUid(userName));
    }
}

void MainWindow::on_lePassword_returnPressed()
{
    emit accept(ui->lePassword->text());
    ui->btnAuth->hide();
    ui->btnLoading->show();
    setMessage("");
    if (!w_timer) {
        w_timer = new QTimer(this);
        w_timer->setInterval(150);
        connect(w_timer, &QTimer::timeout, this, &MainWindow::updatePixmap);
    }
    m_waitingPixmap = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(24, 24);
    ui->btnLoading->setIcon(QIcon(m_waitingPixmap));

    w_timer->start();
    // switchWidget(UNDEFINED);
    // setMessage(tr("in authentication, please wait..."));
    hasSendPassword = true;
}

void MainWindow::on_btnBioAuth_clicked()
{
    emit switchToBiometric();
    authMode = BIOMETRIC;
}

void MainWindow::on_returnButton_clicked()
{
    m_loginOptsWidget->stopAuth();
    accept(BIOMETRIC_IGNORE);
}
/*** end of control's slot ***/

/*** public member ***/

void MainWindow::setIcon(const QString &iconName)
{
    //    QIcon::setThemeName("ukui-icon-theme");
    app_IconName = iconName;
    if (!QIcon::hasThemeIcon("ukui-polkit")) {
        QDir iconsDir("/usr/share/icons");
        auto themesList = iconsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        qDebug() << themesList;
        for (auto theme : themesList) {
            QIcon::setThemeName(theme);
            if (QIcon::hasThemeIcon("ukui-polkit")) {
                qDebug() << theme << "has ukui-polkit icon";
                break;
            }
        }
    }
    QPixmap icon = QIcon::fromTheme("ukui-polkit").pixmap(64, 64);
    QPixmap actionIcon = QIcon::fromTheme(app_IconName).pixmap(32, 32);
    QPainter painter;

    painter.begin(&icon);
    QRect rect(32, 32, 32, 32);
    painter.drawPixmap(rect, actionIcon);
    painter.end();

    setWindowIcon(icon);
    // ui->lblIcon->setPixmap(icon);
}

void MainWindow::setHeader(const QString &text)
{
    if (is_Mavis)
        ui->lblHeader->setText(tr("This operation requires the administrator's authorization. "
                                  "Please enter your password to allow this operation."));
    else
        ui->lblHeader->setText(text);
    ui->lblHeader->adjustSize();
    ui->lblContent->setText(tr("A program is attempting to perform an action that requires privileges."
                               "It requires authorization to perform the action."));
    ui->lblContent->adjustSize();
}

void MainWindow::setUsers(const QStringList &usersList)
{
    if (usersList.isEmpty())
        return;

    if (usersList.size() == 1) {
        UserItem user = users->getUserByName(usersList.at(0));
        if (user.realName != "") {
            userName = user.realName;
        } else {
            userName = usersList.at(0);
        }
    }

    QList<UserItem> accountUsers = users->getUsers();
    for (QString identifyUser : usersList) {
        bool isFoundAccount = false;
        for (UserItem user : accountUsers) {
            if (identifyUser == user.name) {
                if (user.realName != "")
                    ui->cmbUsers->addItem(user.realName);
                else
                    ui->cmbUsers->addItem(user.name);
                isFoundAccount = true;
                break;
            }
        }
        if (!isFoundAccount) {
            ui->cmbUsers->addItem(identifyUser);
        }
    }

    ui->cmbUsers->show();
    // ui->cmbUsers->adjustSize();
}
/*
void MainWindow::setDetails(const QString &subPid, const QString &callerPid, const QString &actionId,
                            const QString &actionDesc, const QString vendorName,
                            const QString vendorUrl)
{
    ui->lblSubjectPid->setText(subPid);
    ui->lblCallerPid->setText(callerPid);
    ui->lblActionId->setText(actionId);
    ui->lblActionDesc->setText(actionDesc);
    QString vendor = QString("<a href=\"%1\">%2").arg(vendorUrl).arg(vendorName);
    ui->lblVendor->setText(vendor);
}
*/
void MainWindow::setPrompt(const QString &text, bool echo)
{
    QString prompt = text;

    if (text == "Input Password") {
        prompt = tr("Input Password");
    } else if (text == "Password: " || text == "Password:") {
        prompt = tr("Password: ");
    } else if (text == "_Password: ") {
        prompt = tr("_Password: ");
    } else if (text == "_Password:") {
        prompt = tr("_Password:");
    }
    if (!m_timer) {
        m_timer = new QTimer(this);
        m_timer->setInterval(200);
        connect(m_timer, &QTimer::timeout, this, &MainWindow::unlock_countdown);
    }
    // qDebug() << "6666";
    m_timer->start();
    unlock_countdown();

    ui->lePassword->clear();
    ui->lePassword->setPlaceholderText(prompt);
    //  switchWidget(PASSWORD);
}

/*
    转换pam英文提示，目前转换的就3个翻译
    Authenticated failed, account locked!
    Authenticated failed, 1 login attemps left
    Account locked, 4 minutes left
*/
QString MainWindow::check_is_pam_message(QString text)
{
    if (text.isEmpty()) {
        return text;
    }
    setlocale(LC_ALL, "");
    bindtextdomain("Linux-PAM", "/usr/share/locale");
    bind_textdomain_codeset("Linux-PAM", "UTF-8");
    textdomain("Linux-PAM");
    char *str;
    QString strTrans = "";
    QByteArray ba = text.toLocal8Bit(); // must
    str = ba.data();
    char l_str[1024];
    int a, b;
    // 兼容旧版本翻译，以及适配新版本翻译
    // 因为不知道什么原因，pam发过来的始终为英文，因此这里主动加载pam的翻译文件，使用gettext获取翻译
    if (text.contains("attemps", Qt::CaseSensitive) && sscanf(str, "Authenticated failed, %d login attemps left", &a))
        snprintf(l_str, 1024, _("Authenticated failed, %d login attemps left"), a);
    else if (
        text.contains("attempts", Qt::CaseSensitive) && sscanf(str, "Authenticated failed, %d login attempts left", &a))
        snprintf(l_str, 1024, _("Authenticated failed, %d login attempts left"), a);
    else if (
        text.contains("attempt", Qt::CaseSensitive) && sscanf(str, "Authenticated failed, %d login attempt left", &a))
        snprintf(l_str, 1024, _("Authenticated failed, %d login attempt left"), a);
    else if (text.contains("days", Qt::CaseSensitive) && sscanf(str, "Account locked, %d days left", &a)) {
        // 这里的消息是多个字符串拼接起来的，无法在翻译文件中找到对应的句子，只能自己翻译
        strTrans = tr("Account locked,") + QString("%1 ").arg(a) + tr("days left");
        return strTrans;
    } else if (text.contains("hours", Qt::CaseSensitive) && sscanf(str, "Account locked, %d hours left", &a)) {
        strTrans = tr("Account locked,") + QString("%1 ").arg(a) + tr("hours left");
        return strTrans;
    } else if (text.contains("minutes", Qt::CaseSensitive) && sscanf(str, "Account locked, %d minutes left", &a)) {
        strTrans = tr("Account locked,") + QString("%1 ").arg(a) + tr("minutes left");
        return strTrans;
    } else if (text.contains("seconds", Qt::CaseSensitive) && sscanf(str, "Account locked, %d seconds left", &a)) {
        strTrans = tr("Account locked,") + QString("%1 ").arg(a) + tr("seconds left");
        return strTrans;
    } else if (
        text.contains("Warning: ", Qt::CaseSensitive)
        && sscanf(str, "Warning: your password will expire in %d day", &a)) {
        snprintf(l_str, 1024, _("Warning: your password will expire in %d day"), a);
    } else if (
        text.contains("Warning: ", Qt::CaseSensitive)
        && sscanf(str, "Warning: your password will expire in %d days", &a)) {
        snprintf(l_str, 1024, _("Warning: your password will expire in %d days"), a);
    } else {
        return _(str);
    }

    qDebug() << "l_str = " << l_str;
    return QString(l_str);
}

void MainWindow::setMessage(const QString &text, situation situat)
{
    if (situat == ERROR) {
        QPalette pe;
        pe.setColor(QPalette::Text, Qt::red);
        ui->lblMessage->setPalette(pe);
        //        ui->lePassword->setStyleSheet("QLineEdit{background-color: palette(Button);"
        //                                      "border-radius: 6px;border: 1px solid #F3222D;}");
    } else if (situat == TRUE) {
        QColor color = palette().color(QPalette::Text);
        QPalette pal(this->palette());
        pal.setColor(QPalette::Text, QColor(color));
        ui->lblMessage->setPalette(pal);
    }

    // qDebug()<<"receive：text = "<<text;
    if (text.indexOf("account locked") != -1 || text.indexOf("账户已锁定") != -1 || text.indexOf("Account locked") != -1
        || text.indexOf("永久锁定") != -1) {
        if (!m_timer) {
            m_timer = new QTimer(this);
            m_timer->setInterval(800);
            connect(m_timer, &QTimer::timeout, this, &MainWindow::unlock_countdown);
        }
        m_timer->start();
    } else if (text.indexOf("No password received, please input password") != -1) {
        ui->lblMessage->setText(tr("Password cannot be empty"));
        ui->lblMessage->setToolTip(tr("Password cannot be empty"));
    } else {
        ui->lblMessage->setText(text);
        ui->lblMessage->setToolTip(text);
    }
    // ui->lblMessage->adjustSize();
    // ui->widgetPasswdAuth->adjustSize();
    // ui->lblMessage->setText(text);
}

void MainWindow::setAuthResult(bool result, const QString &text)
{
    QString message = text;

    if (!result && text.isEmpty()) {
        message = tr("Authentication failed, please try again.");
    }

    if (authMode == PASSWORD) {
        switchWidget(PASSWORD);
        setMessage(message, ERROR);
    } else if (authMode == BIOMETRIC)
        setMessage(message, ERROR);
}

void MainWindow::clearEdit()
{
    ui->lePassword->setText("");
    ui->lePassword->setDisabled(true);
    ui->lePassword->setReadOnly(true);
    ui->ukeyPassword->setText("");
    ui->btnAuth->setDisabled(true);
}

void MainWindow::switchAuthMode(Mode mode)
{
    if (isbioSuccess) {
        accept(BIOMETRIC_SUCCESS);
        return;
    }
    int uid = getUid(userName);
    m_loginOptsWidget->setUser(uid);
    int count = m_loginOptsWidget->getLoginOptCount();
    if (!hasReviceBiometricPam && mode != BIOMETRIC) {
        count = 0; 
    }
    enableBioAuth = count > 0;
    if (count < 1) {
        setLoginTypeTip(tr("Input your password to authentication"));
        m_loginOptsWidget->hide();
    } else {
        m_loginOptsWidget->show();
        m_loginOptsWidget->setEnabled(true);
    }

    if (m_loginOptsWidget->getHasUkeyOptions()) {
        setLoginTypeTip(tr("Input your password to authentication"));
        m_loginOptsWidget->show();
    }

    switch (mode) {
        case PASSWORD: {
            qDebug() << "switch to password";

            authMode = mode;

            if (!enableBioAuth || !receiveBioPAM || useDoubleAuth) {
                ui->btnBioAuth->hide();
            } else {
                ui->btnBioAuth->show();
            }

            if (isHiddenSwitchButton) {
                ui->btnBioAuth->hide();
            }

            if (hasSendPassword && authMethod == "otp") {
                if (checkIsAdmin(userName)) {
                    setLoginTypeTip(
                        tr("Enter the two-factor authentication (2FA) OTP token for the administrator \"%1\"")
                            .arg(userName));
                } else {
                    setLoginTypeTip(
                        tr("Enter the two-factor authentication (2FA) OTP dynamic token for the user \"%1\"")
                            .arg(userName));
                }
                ui->cmbUsers->hide();
            } else if (hasSendPassword && authMethod == "ukey") {
                if (checkIsAdmin(userName)) {
                    setLoginTypeTip(
                        tr("Enter the two-factor authentication (2FA) security key for the administrator \"%1\"")
                            .arg(userName));
                } else {
                    setLoginTypeTip(
                        tr("Enter the two-factor authentication (2FA) security key for the user \"%1\"").arg(userName));
                }
                ui->cmbUsers->hide();
            }
        } break;
        case BIOMETRIC: {
            if (authMode == PASSWORD) {
                emit accept(BIOMETRIC_IGNORE);
                isFirstAuth = false;
                return;
            }
            authMode = mode;
            hasReviceBiometricPam = true;
            qDebug() << "switch to biometric";
            unlock_countdown();
            if (m_deviceInfo) {
                if (!m_loginOptsWidget->findDeviceById(m_deviceInfo->device_id)
                    || m_loginOptsWidget->isDeviceDisable(m_deviceInfo->device_id)) {
                    m_deviceInfo = DeviceInfoPtr();
                }
            }

            if (!enableBioAuth) {
                qDebug() << "It doesn't meet the condition for enabling biometric authentication, switch to password.";
                emit accept(BIOMETRIC_IGNORE);
                isFirstAuth = false;
                return;
            } else if (authMode == BIOMETRIC) {
                QString strDeviceName;
                if (isLoadingUkey) {
                    strDeviceName = m_loginOptsWidget->GetDefaultDevice(uid, UniT_General_Ukey);
                } else {
                    strDeviceName = m_loginOptsWidget->GetDefaultDevice(uid);
                }
                // 如果默认设备为空的话，第一次不启动生物识别认证
                if (strDeviceName.isEmpty() && !m_deviceInfo) {
                    qDebug() << "No default device";
                    emit accept(BIOMETRIC_IGNORE);
                    isFirstAuth = false;
                    return;
                }

                // 第一次，获取默认设备的设备信息，之后使用的则是从设备选择窗口传出的设备信息
                if (!m_deviceInfo) {
                    m_deviceInfo = m_loginOptsWidget->findDeviceByName(strDeviceName);
                }

                if (!m_deviceInfo) {
                    emit accept(BIOMETRIC_IGNORE);
                    isFirstAuth = false;
                    return;
                }

                if (m_failMap.contains(uid) && m_failMap[uid].contains(m_deviceInfo->device_id)
                    && m_failMap[uid][m_deviceInfo->device_id] >= maxFailedTimes) {
                    emit accept(BIOMETRIC_IGNORE);
                    isFirstAuth = false;
                    m_deviceInfo = DeviceInfoPtr();
                    return;
                }

                if (m_deviceInfo && m_loginOptsWidget->isDeviceDisable(m_deviceInfo->device_id)) {
                    emit accept(BIOMETRIC_IGNORE);
                    isFirstAuth = false;
                    m_deviceInfo = DeviceInfoPtr();
                    return;
                }

                if (m_deviceInfo->biotype == LOGINOPT_TYPE_GENERAL_UKEY) {
                    if (isFirstAuth) {
                        isFirstAuth = false;
                        m_deviceInfo = nullptr;
                        emit accept(BIOMETRIC_IGNORE);
                        return;
                    }
                    switchLoginOptType(LOGINOPT_TYPE_GENERAL_UKEY);
                    stopLoadingUkey();
                    startBioAuth();
                    m_loginOptsWidget->setCurrentDevice(m_deviceInfo);
                    m_loginOptsWidget->updateUIStatus(false);
                    return;
                }

                isFirstAuth = false;
                switchLoginOptType(m_loginOptsWidget->convertDeviceType(m_deviceInfo->biotype));
                startBioAuth();
                emit accept(BIOMETRIC_IGNORE);
                return;
            }
        } break;
        default:
            break;
    }
    switchWidget(mode);
}

/*** end of public memeber ***/

/*** private member ***/

uid_t MainWindow::getUid(const QString &userName)
{
    struct passwd *pwd = getpwnam(userName.toStdString().c_str());
    if (pwd == NULL) {
        qWarning() << "getpwnam error: " << strerror(errno);
        return -1;
    }
    return pwd->pw_uid;
}

void MainWindow::setDoubleAuth(bool val)
{
    useDoubleAuth = val;
}

void MainWindow::stopDoubleAuth()
{
    //    if(useDoubleAuth && widgetBioAuth)
    //        widgetBioAuth->stopAuth();
}

void MainWindow::switchWidget(Mode mode)
{
    ui->widgetPasswdAuth->hide();
    ui->btnAuth->hide();
    ui->btnLoading->hide();
    ui->btnAuth->setText(tr("Authenticate"));
    ui->btnBioAuth->setText(tr("Biometric"));
    ui->btnCancel->setText(tr("Cancel"));
    ui->returnButton->setText(tr("Use password"));
    ui->btnLoading->setDisabled(true);
    if (is_Mavis) {
        ui->cmbUsers->setFixedHeight(48);
        ui->lePassword->setFixedHeight(48);
        ui->ukeyPassword->setFixedHeight(48);
        ui->btnAuth->setFixedHeight(48);
        ui->btnCancel->setFixedHeight(48);
    } else {
        ui->cmbUsers->setFixedHeight(36);
        ui->lePassword->setFixedHeight(36);
        ui->ukeyPassword->setFixedHeight(36);
        ui->btnAuth->setFixedHeight(36);
        ui->btnBioAuth->setFixedHeight(36);
        ui->btnCancel->setFixedHeight(36);
        ui->returnButton->setFixedHeight(36);
    }
    switch (mode) {
        case PASSWORD: {
            setFixedWidth(420);

            if (m_deviceInfo && m_loginOptsWidget->findDeviceById(m_deviceInfo->device_id)) {
                switchLoginOptType(m_loginOptsWidget->convertDeviceType(m_deviceInfo->biotype));
            } else {
                switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
                m_loginOptsWidget->setSelectedPassword();
            }
            // 标题+间隔+内容头+间隔+类型提示+间隔+用户列表+间隔+
            // （密码输入框+间隔+验证提示+间隔+授权按钮+边距）/  --仅密码方式
            // （密码输入框+间隔+验证提示+间隔+登录选项+间隔+授权按钮+边距）/ --密码+生物识别 选择密码
            // （登录选项+间隔+授权按钮+边距）/ --密码+生物识别 选择生物识别
            // （ukey加载页+间隔+登录选项+间隔+授权按钮+边距）/（ukey认证页+间隔+登录选项+间隔+授权按钮+边距）
            // --选择Ukey
            int height = 40 + 8 + ui->lblHeader->height() + 8 + m_labelTip->height() + 8;
            switch (m_uCurLoginOptType) {
                case LOGINOPT_TYPE_PASSWORD: {
                    height += ui->cmbUsers->height() + 8 + ui->lePassword->height() + 8 + ui->lblMessage->height() + 8;
                } break;
                case LOGINOPT_TYPE_FACE:
                case LOGINOPT_TYPE_FINGERPRINT:
                case LOGINOPT_TYPE_FINGERVEIN:
                case LOGINOPT_TYPE_IRIS:
                case LOGINOPT_TYPE_VOICEPRINT:
                case LOGINOPT_TYPE_QRCODE: {
                    height += ui->cmbUsers->height() + 8;
                } break;
                case LOGINOPT_TYPE_GENERAL_UKEY:
                case LOGINOPT_TYPE_ADVANCED_UKEY: {
                    if (!ui->widgetUkeyAuth->isHidden()) {
                        height += ui->widgetUkeyAuth->height() + 8;
                    } else if (!ui->loadingUkeyWidget->isHidden()) {
                        height += ui->loadingUkeyWidget->height() + 8;
                    }
                } break;
                default:
                    break;
            }
            if (!m_loginOptsWidget->isHidden()) {
                height += m_loginOptsWidget->height() + 8;
            }
            height += ui->btnAuth->height() + 24;
            setFixedSize(width(), height);
            qInfo() << "MainWindow geometry:" << geometry() << height;

            // m_loginOptsWidget->updateUIStatus();
            ui->btnBioAuth->setStyleSheet("QPushButton{font-size:14px;}QPushButton:hover{border:none;color:#3E6CE5;}"
                                          "QPushButton:pressed{border:none;}");
            ui->btnBioAuth->setFlat(true);
            ui->btnBioAuth->setText(tr("Biometric"));
            ui->btnBioAuth->setAttribute(Qt::WA_UnderMouse, false);
            ui->btnBioAuth->adjustSize();
            if (m_uCurLoginOptType == LOGINOPT_TYPE_PASSWORD) {
                ui->widgetPasswdAuth->show();
                ui->lePassword->setFocus();
                // ui->lePassword->setAttribute(Qt::WA_InputMethodEnabled, false);
                if (m_modeButton && m_modeButton->isShowPwd()) {
                    ui->lePassword->setEchoMode(QLineEdit::Normal);
                } else {
                    ui->lePassword->setEchoMode(QLineEdit::Password);
                }
            }
            ui->btnAuth->show();
            ui->btnLoading->hide();
            if (w_timer && w_timer->isActive()) {
                w_timer->stop();
            }
            ui->btnCancel->show();
            // ui->lblContent->show();
            ui->returnButton->hide();
            ui->lblContent->hide();
        } break;

        case BIOMETRIC:
            setFixedWidth(420);
            ui->btnCancel->hide();
            ui->lblContent->hide();
            ui->btnBioAuth->hide();
            break;
        default:
            break;
    }

    //  adjustSize();
}

void MainWindow::unlock_countdown()
{
    int failed_count = 0;
    int time_left = 0;
    int deny = 0;
    int fail_time = 0;
    int unlock_time = 0;
    pam_tally_unlock_time_left(getUid(userName), &failed_count, &time_left, &deny, &fail_time, &unlock_time);

    // qDebug() << "failed_count:" << failed_count << "time_left:" <<time_left <<"deny:"<<deny<<"fail_time:"<<
    // fail_time<<"unlock_time:" << unlock_time;
    if (time_left >= 60) // 请多少分钟后重试
    {
        int nMinuteleft = time_left / 60;
        if (isLockingFlg == false) {
            m_nCurLockMin = unlock_time / 60; // 获取当前需要锁定的分钟数
        }

        // 如果当前设置的不是1min钟锁定，那么1min显示成2min，由2min直接跳到59s ||
        // 剩余分钟数小于当前设置的锁定时间，并且大于1min，自增+1
        if ((nMinuteleft == 1 && m_nCurLockMin != 1) || (nMinuteleft > 1 && nMinuteleft < m_nCurLockMin)) {
            nMinuteleft = nMinuteleft + 1;
        }
        setMessage(tr("Please try again in %1 minutes.").arg(nMinuteleft), ERROR);
        // ui->lblMessage->setToolTip(tr("Please try again in %1 minutes.").arg(nMinute));
        ui->lePassword->setText("");
        ui->lePassword->setDisabled(true);
        ui->lePassword->setReadOnly(true);
        ui->btnAuth->setDisabled(true);
        isLockingFlg = true;
        setBiometricAuthDisabledStatus(isLockingFlg);
        return;
    } else if (time_left > 0 && time_left < 60) // 请多少秒后重试
    {
        char ch[100] = { 0 };
        setMessage(tr("Please try again in %1 seconds.").arg(time_left % 60), ERROR);
        // ui->lblMessage->setToolTip(tr("Please try again in %1 seconds.").arg(time_left%60));
        ui->lePassword->setText("");
        ui->lePassword->setDisabled(true);
        ui->lePassword->setReadOnly(true);
        ui->btnAuth->setDisabled(true);
        isLockingFlg = true;
        setBiometricAuthDisabledStatus(isLockingFlg);
        return;
    } else if (failed_count == 0xFFFF) // 账号被永久锁定
    {
        ui->lblMessage->setText(tr("Account locked permanently."));
        ui->lblMessage->setToolTip(tr("Account locked permanently."));
        ui->lePassword->setText("");
        ui->lePassword->setDisabled(true);
        ui->lePassword->setReadOnly(true);
        ui->btnAuth->setDisabled(true);
        isLockingFlg = true;
        setBiometricAuthDisabledStatus(isLockingFlg);
        return;
    } else {
        if (ui->lePassword) {
            ui->lePassword->setDisabled(false);
            ui->lePassword->setReadOnly(false);
            ui->lePassword->setFocus();
        }
        if (ui->btnAuth) {
            ui->btnAuth->setDisabled(false);
        }

        if (isLockingFlg) {
            // setMessage(tr("Authentication failed, please try again."),ERROR);
            setMessage("");
            isLockingFlg = false;
            setBiometricAuthDisabledStatus(isLockingFlg);
        }
        if (m_timer) {
            m_timer->stop();
        }
    }
    return;
}

void MainWindow::root_unlock_countdown()
{
    int failed_count = 0;
    int time_left = 0;
    int deny = 0;
    int fail_time = 0;
    int unlock_time = 0;
    pam_tally_root_unlock_time_left(&failed_count, &time_left, &deny, &fail_time, &unlock_time);

    // qDebug() << "failed_count:" << failed_count << "time_left:" <<time_left <<"deny:"<<deny<<"fail_time:"<<
    // fail_time<<"unlock_time:" << unlock_time;
    int nMinuteleft = time_left / 60;
    if (time_left >= 60) // 请多少分钟后重试
    {
        int nMinuteleft = time_left / 60;
        if (isLockingFlg == false) {
            m_nCurLockMin = unlock_time / 60; // 获取当前需要锁定的分钟数
        }

        // 如果当前设置的不是1min钟锁定，那么1min显示成2min，由2min直接跳到59s ||
        // 剩余分钟数小于当前设置的锁定时间，并且大于1min，自增+1
        if ((nMinuteleft == 1 && m_nCurLockMin != 1) || (nMinuteleft > 1 && nMinuteleft < m_nCurLockMin)) {
            nMinuteleft = nMinuteleft + 1;
        }

        ui->lblMessage->setText(tr("Please try again in %1 minutes.").arg(nMinuteleft));
        ui->lblMessage->setToolTip(tr("Please try again in %1 minutes.").arg(nMinuteleft));
        ui->lePassword->setText("");
        ui->lePassword->setDisabled(true);
        ui->lePassword->setReadOnly(true);
        ui->btnAuth->setDisabled(true);
        isLockingFlg = true;
        setBiometricAuthDisabledStatus(isLockingFlg);
        return;
    } else if (time_left > 0 && time_left < 60) // 请多少秒后重试
    {
        char ch[100] = { 0 };
        ui->lblMessage->setText(tr("Please try again in %1 seconds.").arg(time_left % 60));
        ui->lblMessage->setToolTip(tr("Please try again in %1 seconds.").arg(time_left % 60));
        ui->lePassword->setText("");
        ui->lePassword->setDisabled(true);
        ui->lePassword->setReadOnly(true);
        ui->btnAuth->setDisabled(true);
        isLockingFlg = true;
        setBiometricAuthDisabledStatus(isLockingFlg);
        return;
    } else if (failed_count == 0xFFFF) // 账号被永久锁定
    {
        ui->lblMessage->setText(tr("Account locked permanently."));
        ui->lblMessage->setToolTip(tr("Account locked permanently."));
        ui->lePassword->setText("");
        ui->lePassword->setDisabled(true);
        ui->lePassword->setReadOnly(false);
        ui->btnAuth->setDisabled(true);
        isLockingFlg = true;
        setBiometricAuthDisabledStatus(isLockingFlg);
        return;
    } else {
        if (ui->lePassword) {
            ui->lePassword->setDisabled(false);
            ui->lePassword->setReadOnly(false);
            ui->lePassword->setFocus();
        }
        if (ui->btnAuth) {
            ui->btnAuth->setDisabled(false);
        }

        if (isLockingFlg) {
            // setMessage(tr("Authentication failed, please try again."),ERROR);
            setMessage("");
            isLockingFlg = false;
            setBiometricAuthDisabledStatus(isLockingFlg);
        }

        m_timer->stop();
    }
    return;
}

void MainWindow::setBiometricAuthDisabledStatus(bool locked)
{
    if (locked) {
        if (m_loginOptsWidget) {
            m_loginOptsWidget->stopAuth();
            m_loginOptsWidget->setAllDeviceDisable(true);
            if (m_bioTimer && m_bioTimer->isActive())
                m_bioTimer->stop();
        }
        if (m_deviceInfo && m_deviceInfo->biotype == BIOTYPE_FACE) {
            QImage imgFailed;
            m_loginOptsWidget->setFaceImg(imgFailed, 1);
        } else if (m_deviceInfo && m_deviceInfo->biotype == REMOTE_QRCODE_TYPE) {
            QImage nullImage;
            m_loginOptsWidget->setQRCode(nullImage);
        }
    } else {
        if (m_loginOptsWidget) {
            if (m_deviceInfo) {
                m_loginOptsWidget->startAuth(m_deviceInfo, getUid(userName));
            }
            m_loginOptsWidget->setAllDeviceDisable(false);
        }
    }
}

void MainWindow::onRespondUkey(const QString &text)
{
    if (m_loginOptsWidget) {
        m_loginOptsWidget->SetExtraInfo(text, "pincode");
    }
}

void MainWindow::switchLoginOptType(unsigned uLoginOptType)
{
    switch (uLoginOptType) {
        case LOGINOPT_TYPE_PASSWORD: {
            // m_loginOptsWidget->hide();
            ui->lePassword->show();
            ui->cmbUsers->show();
            ui->widgetPasswdAuth->show();
            ui->widgetUkeyAuth->hide();
            ui->ukeyPassword->clearFocus();
            ui->loadingUkeyWidget->hide();
            setFocusProxy(ui->lePassword);
            ui->cmbUsers->show();
            if (hasSendPassword && (authMethod == "ukey" || authMethod == "otp")) {
                ui->cmbUsers->hide();
            }
            m_loginOptsWidget->stopBioWaiting();
        } break;
        case LOGINOPT_TYPE_FACE:
        case LOGINOPT_TYPE_FINGERPRINT:
        case LOGINOPT_TYPE_IRIS:
        case LOGINOPT_TYPE_VOICEPRINT:
        case LOGINOPT_TYPE_FINGERVEIN:
        case LOGINOPT_TYPE_QRCODE: {
            switch (uLoginOptType) {
                case LOGINOPT_TYPE_FINGERPRINT:
                case LOGINOPT_TYPE_IRIS:
                case LOGINOPT_TYPE_VOICEPRINT:
                case LOGINOPT_TYPE_FINGERVEIN: {
                    m_loginOptsWidget->startBioWaiting(uLoginOptType);
                } break;
                default:
                    m_loginOptsWidget->stopBioWaiting();
                    break;
            }

            // 延迟检查错误状态
            m_isNetworkErr = false;
            QTimer::singleShot(500, this, [&, this]() {
                if (!this->m_isNetworkErr) {
                    m_loginOptsWidget->setQRCodeMsg("");
                }
            });

            ui->lePassword->show();
            ui->widgetUkeyAuth->hide();
            ui->loadingUkeyWidget->hide();
            ui->ukeyPassword->clearFocus();
            setFocusProxy(ui->lePassword);
            ui->cmbUsers->show();
            ui->widgetPasswdAuth->hide();
        } break;
        case LOGINOPT_TYPE_GENERAL_UKEY: {
            ui->lePassword->hide();
            ui->cmbUsers->hide();
            if (m_deviceInfo) {
                stopLoadingUkey();
                ui->widgetUkeyAuth->show();
            } else {
                startLoadingUkey();
            }
            setFocusProxy(ui->ukeyPassword);
            ui->ukeyPassword->setFocusPolicy(Qt::StrongFocus);
            ui->ukeyPassword->setFocus();
            ui->widgetPasswdAuth->hide();
            m_loginOptsWidget->stopBioWaiting();
        } break;
        default:
            return;
    }
    qDebug() << "CurOptType:" << uLoginOptType;
    if (m_deviceInfo && m_failMap.contains(getUid(userName))
        && m_failMap[getUid(userName)].contains(m_deviceInfo->device_id)
        && m_failMap[getUid(userName)][m_deviceInfo->device_id] >= maxFailedTimes) {
        if (m_deviceInfo->biotype == BIOTYPE_FACE) {
            QImage imgFailed;
            m_loginOptsWidget->setFaceImg(imgFailed, 1);
        }
        no_changes = true;
        if (m_deviceInfo->biotype == REMOTE_QRCODE_TYPE) {
            setLoginTypeTip(tr("Failed to verify %1, please enter password to unlock")
                                .arg(BioDevices::bioTypeToString_tr(m_deviceInfo->biotype)));
        } else if (m_deviceInfo->biotype == LOGINOPT_TYPE_GENERAL_UKEY) {
            setUkeyTypeTip(tr("Failed to verify %1, please enter password to unlock")
                               .arg(BioDevices::bioTypeToString_tr(m_deviceInfo->biotype)));
        } else {
            setLoginTypeTip(tr("Unable to verify %1, please enter password to unlock")
                                .arg(BioDevices::bioTypeToString_tr(m_deviceInfo->biotype)));
            m_loginOptsWidget->stopBioWaiting(true);
        }
        m_loginOptsWidget->setDeviceDisable(m_deviceInfo->device_id, true);
    } else {
        no_changes = false;
        if (uLoginOptType != m_uCurLoginOptType || (m_deviceInfo && m_deviceInfo->device_id != m_nLastDeviceId)) {
            switch (uLoginOptType) {
                case LOGINOPT_TYPE_PASSWORD: {
                    setLoginTypeTip(tr("Input your password to authentication"));
                } break;
                case LOGINOPT_TYPE_FACE: {
                    setLoginTypeTip(tr("Facial recognition to authorization"));
                } break;
                case LOGINOPT_TYPE_FINGERPRINT: {
                    setLoginTypeTip(tr("Fingerprint recognition to authorization"));
                } break;
                case LOGINOPT_TYPE_VOICEPRINT: {
                    setLoginTypeTip(tr("Voiceprint recognition to authorization"));
                } break;
                case LOGINOPT_TYPE_FINGERVEIN: {
                    setLoginTypeTip(tr("Finger veins recognition to authorization"));
                } break;
                case LOGINOPT_TYPE_IRIS: {
                    setLoginTypeTip(tr("Iris recognition to authorization"));
                } break;
                case LOGINOPT_TYPE_QRCODE: {
                    QString strDrivName = tr("wechat");
                    if (m_deviceInfo && m_deviceInfo->device_shortname != "wechat_driver") {
                        strDrivName = m_deviceInfo->device_shortname;
                    }
                    setLoginTypeTip(tr("Use the bound %1 scanning code to authorization").arg(strDrivName));
                } break;
                case LOGINOPT_TYPE_GENERAL_UKEY: {
                    setLoginTypeTip("");
                } break;
                default:
                    return;
            }
        }
    }
    if (m_deviceInfo) {
        m_nLastDeviceId = m_deviceInfo->device_id;
    } else {
        m_nLastDeviceId = -1;
    }
    m_uCurLoginOptType = uLoginOptType;
    m_loginOptsWidget->updateUIStatus(true, m_uCurLoginOptType);
}

void MainWindow::setLoginTypeTip(QString strLoginTypeTip)
{
    m_strLoginTypeTip = strLoginTypeTip;
    if (m_strLoginTypeTip.isEmpty()) {
        m_labelTip->hide();
    } else {
        QPalette pe;
        if (no_changes) {
            pe.setColor(QPalette::Text, Qt::red);
            m_labelTip->setPalette(pe);
        } else {
            QColor color = palette().color(QPalette::Text);
            QPalette pal(this->palette());
            pal.setColor(QPalette::Text, QColor(color));
            m_labelTip->setPalette(pal);
        }
        m_labelTip->setText(m_strLoginTypeTip);
        m_labelTip->show();
    }
}

void MainWindow::startBioAuthDelay()
{
    if (m_deviceInfo) {
        if (m_deviceInfo->biotype == LOGINOPT_TYPE_GENERAL_UKEY) {

        } else {
            m_loginOptsWidget->startAuth(m_deviceInfo, getUid(userName));
        }
        switchLoginOptType(m_loginOptsWidget->convertDeviceType(m_deviceInfo->biotype));
    } else {
        switchLoginOptType(LOGINOPT_TYPE_PASSWORD);
    }
    m_bioTimer->stop();
}

void MainWindow::startBioAuth(unsigned uTimeout)
{
    m_loginOptsWidget->stopAuth();

    if (m_deviceInfo && m_deviceInfo->biotype == LOGINOPT_TYPE_GENERAL_UKEY) {
        //因为ukey是在输完密码后才开始调用认证接口，而当前设备是在开始认证时才设置上，
	//因此ukey需要提示设置当前设备属性，避免登录选项切换ukey后，登录选项ui选中密码
	//的问题
        m_loginOptsWidget->setCurrentDevice(m_deviceInfo);
        switchLoginOptType(m_loginOptsWidget->convertDeviceType(m_deviceInfo->biotype));
    }

    if (!m_bioTimer) {
        m_bioTimer = new QTimer(this);
        connect(m_bioTimer, SIGNAL(timeout()), this, SLOT(startBioAuthDelay()));
    }
    ui->lblMessage->clear();
    ui->lblMessage->setToolTip("");
    m_bioTimer->start(uTimeout);
}

void MainWindow::onUpdateBioAuthMsg(QString strMsg)
{
    setMessage(strMsg, TRUE);
}

void MainWindow::updatePixmap()
{
    ui->btnAuth->hide();
    ui->btnAuth->setDisabled(true);
    ui->btnLoading->show();
    ui->lePassword->setDisabled(true);
    ui->lePassword->setReadOnly(true);
    QMatrix matrix;
    matrix.rotate(90.0);
    m_waitingPixmap = m_waitingPixmap.transformed(matrix, Qt::FastTransformation);
    ui->btnLoading->setIcon(m_waitingPixmap);
}

void MainWindow::onUpdateWndSize(unsigned uLoginOptType, unsigned uLoginOptSize)
{
    ui->lblContent->hide();
    ui->lblHeader->adjustSize();

    // 标题+间隔+内容头+间隔+类型提示+间隔+用户列表+间隔+
    // （密码输入框+间隔+验证提示+间隔+授权按钮+边距）/  --仅密码方式
    // （密码输入框+间隔+验证提示+间隔+登录选项+间隔+授权按钮+边距）/ --密码+生物识别 选择密码
    // （登录选项+间隔+授权按钮+边距）/ --密码+生物识别 选择生物识别
    // （ukey加载页+间隔+登录选项+间隔+授权按钮+边距）/（ukey认证页+间隔+登录选项+间隔+授权按钮+边距）
    // --选择Ukey
    int height = 40 + 8 + ui->lblHeader->height() + 8 + m_labelTip->height() + 8;
    switch (m_uCurLoginOptType) {
        case LOGINOPT_TYPE_PASSWORD: {
            height += ui->cmbUsers->height() + 8 + ui->lePassword->height() + 8 + ui->lblMessage->height() + 8;
        } break;
        case LOGINOPT_TYPE_FACE:
        case LOGINOPT_TYPE_FINGERPRINT:
        case LOGINOPT_TYPE_FINGERVEIN:
        case LOGINOPT_TYPE_IRIS:
        case LOGINOPT_TYPE_VOICEPRINT:
        case LOGINOPT_TYPE_QRCODE: {
            height += ui->cmbUsers->height() + 8;
        } break;
        case LOGINOPT_TYPE_GENERAL_UKEY:
        case LOGINOPT_TYPE_ADVANCED_UKEY: {
            if (!ui->widgetUkeyAuth->isHidden()) {
                height += ui->widgetUkeyAuth->height() + 8;
            } else if (!ui->loadingUkeyWidget->isHidden()) {
                height += ui->loadingUkeyWidget->height() + 8;
            }
        } break;
        default:
            break;
    }
    if (!m_loginOptsWidget->isHidden()) {
        height += m_loginOptsWidget->height() + 8;
    }
    height += ui->btnAuth->height() + 24;
    setFixedSize(width(), height);
    qInfo() << "MainWindow geometry update:" << geometry() << height;
}

void MainWindow::setEditInputMethod(bool bEnable)
{
    qDebug() << "setEditInputMethod:" << bEnable;
    if (bEnable) {
        ui->lePassword->setAttribute(Qt::WA_InputMethodEnabled, true);
    } else {
        ui->lePassword->setAttribute(Qt::WA_InputMethodEnabled, false);
    }
}

void MainWindow::setCurProject(bool isMavis)
{
    is_Mavis = isMavis;
}

void MainWindow::onOptionSelected(unsigned uCurLoginOptType, const DeviceInfoPtr &deviceInfo)
{
    isLoadingUkey = false;

    if (uCurLoginOptType == LOGINOPT_TYPE_GENERAL_UKEY) {
        stopLoadingUkey();
    }

    if (uCurLoginOptType == LOGINOPT_TYPE_PASSWORD) {
        switchLoginOptType(uCurLoginOptType);
        if (m_loginOptsWidget) {
            m_loginOptsWidget->stopAuth();
        }
        m_deviceInfo = nullptr;
        authMode = PASSWORD;
        emit restartAuth();
        return;
    }

    if (uCurLoginOptType != LOGINOPT_TYPE_GENERAL_UKEY && !deviceInfo)
        return;

    if (deviceInfo)
        qDebug() << "device changed: " << deviceInfo->biotype;

    if (deviceInfo && m_failMap[getUid(userName)][deviceInfo->device_id] >= maxFailedTimes) {
        qDebug() << "Failed MAX!!";
        return;
    }

    if (uCurLoginOptType == LOGINOPT_TYPE_GENERAL_UKEY && !deviceInfo) {
        isLoadingUkey = true;
        startLoadingUkey();
        m_loginOptsWidget->updateUkeyUIStatus(LOGINOPT_TYPE_GENERAL_UKEY);
	//清空输入密码以授权提示
        setLoginTypeTip("");
    } else if (uCurLoginOptType == LOGINOPT_TYPE_GENERAL_UKEY && deviceInfo) {
        stopLoadingUkey();
        m_loginOptsWidget->updateUkeyUIStatus(LOGINOPT_TYPE_GENERAL_UKEY);
    }

    if (deviceInfo == m_deviceInfo) {
        return;
    }

    if (uCurLoginOptType != LOGINOPT_TYPE_GENERAL_UKEY && deviceInfo == m_deviceInfo) {
        return;
    }

    if (m_bioTimer)
        m_bioTimer->stop();
    m_deviceInfo = deviceInfo;
    switchLoginOptType(uCurLoginOptType);
    if (!isbioSuccess && m_deviceInfo) {
        startBioAuth();
    }
}

void MainWindow::onNotifyOptionsChanged(unsigned uOptionsCount)
{
    if (uOptionsCount > 0) {
        m_loginOptsWidget->show();
    } else {
        // m_loginOptsWidget->hide();
    }

    if (m_deviceInfo && m_deviceInfo->biotype == LOGINOPT_TYPE_GENERAL_UKEY) {
        stopLoadingUkey();
    }

    if (!m_deviceInfo || !m_loginOptsWidget->findDeviceById(m_deviceInfo->device_id)
        || m_loginOptsWidget->isDeviceDisable(m_deviceInfo->device_id)) {
        m_loginOptsWidget->stopAuth();
        if (authMode != UNDEFINED) {
            authMode = UNDEFINED;
            emit switchToBiometric();
        }
    }
}

void MainWindow::onLoginOptionsAuthCompleted(uid_t uid, bool ret, int nStatus)
{
    qDebug() << "biometric authentication complete: " << uid << ret << nStatus;
    if (m_deviceInfo && m_deviceInfo->biotype == LOGINOPT_TYPE_GENERAL_UKEY && ui->widgetUkeyAuth->isVisible()) {
        ui->ukeyPassword->setText("");
    }

    if (uid == getUid(userName) && ret) {
        isbioSuccess = true;
        emit switchToBiometric();
        authMode = UNDEFINED;
    } else {
        if (nStatus == 5 && m_deviceInfo) {
            if (w_timer && w_timer->isActive())
                w_timer->stop();
            QImage imgFailed;
            m_loginOptsWidget->setFaceImg(imgFailed, 2);
            return;
        } else if (nStatus >= 2 && nStatus != 5)
            if (nStatus >= 2) {
                if (m_deviceInfo) {
                    uid_t curUid = getUid(userName);
                    if (m_failMap.contains(curUid) && m_failMap[curUid].contains(m_deviceInfo->device_id)) {
                        m_failMap[curUid][m_deviceInfo->device_id] = m_failMap[curUid][m_deviceInfo->device_id] + 1;
                    } else {
                        m_failMap[curUid][m_deviceInfo->device_id] = 1;
                    }
                    qDebug() << "Failed count:" << m_failMap[curUid][m_deviceInfo->device_id]
                             << ",Max:" << maxFailedTimes;
                    if (m_deviceInfo->biotype == BIOTYPE_FACE) {
                        QImage imgFailed;
                        m_loginOptsWidget->setFaceImg(imgFailed, 1);
                    }
                    if (m_failMap[curUid][m_deviceInfo->device_id] >= maxFailedTimes) {
                        no_changes = true;
                        if (m_deviceInfo->biotype == REMOTE_QRCODE_TYPE) {
                            setLoginTypeTip(tr("Failed to verify %1, please enter password to unlock")
                                                .arg(BioDevices::bioTypeToString_tr(m_deviceInfo->biotype)));
                            QImage nullImage;
                            m_loginOptsWidget->setQRCode(nullImage);
                        } else if (m_deviceInfo->biotype == LOGINOPT_TYPE_GENERAL_UKEY) {
                            ui->ukeyPassword->setReadOnly(true);
                            setUkeyTypeTip(tr("Unable to verify %1, please enter password to unlock")
                                               .arg(BioDevices::bioTypeToString_tr(m_deviceInfo->biotype)));
                        } else {
                            setLoginTypeTip(tr("Unable to verify %1, please enter password to unlock")
                                                .arg(BioDevices::bioTypeToString_tr(m_deviceInfo->biotype)));
                            m_loginOptsWidget->stopBioWaiting(true);
                        }

                        m_loginOptsWidget->setDeviceDisable(m_deviceInfo->device_id, true);
                        useDoubleAuth = false;
                        return;
                    }
                    no_changes = false;
                    if (m_deviceInfo->biotype == LOGINOPT_TYPE_GENERAL_UKEY) {
                        setUkeyTypeTip(tr("Failed to verify %1, you still have %2 verification opportunities")
                                           .arg(BioDevices::bioTypeToString_tr(m_deviceInfo->biotype))
                                           .arg(maxFailedTimes - m_failMap[curUid][m_deviceInfo->device_id]));
                    } else {
                        setLoginTypeTip(tr("Failed to verify %1, you still have %2 verification opportunities")
                                            .arg(BioDevices::bioTypeToString_tr(m_deviceInfo->biotype))
                                            .arg(maxFailedTimes - m_failMap[curUid][m_deviceInfo->device_id]));
                    }
                }
            }
        if (!isbioSuccess) {
            // dbus error
            if (nStatus == -1) {
                qDebug() << "Biometric dbus error:" << authMode;
            }
            if (m_uCurLoginOptType == LOGINOPT_TYPE_QRCODE && nStatus == 1) {
                m_isNetworkErr = true;
                m_loginOptsWidget->setQRCodeMsg(tr("Abnormal network"));
                startBioAuth(10000);
            } else if (m_uCurLoginOptType == LOGINOPT_TYPE_QRCODE && nStatus == -4) {
                m_isNetworkErr = true;
                m_loginOptsWidget->setQRCodeMsg(tr("Acquisition failure"));
                startBioAuth(10000);
            } else {
                startBioAuth();
            }
            if (nStatus >= 2 && m_deviceInfo) {
                if (m_deviceInfo->biotype == BIOTYPE_FACE) {
                    QImage imgFailed;
                    m_loginOptsWidget->setFaceImg(imgFailed, 1);
                }
            }
        }
    }
}

void MainWindow::updateAuthMethod()
{
    authMethod = bioDevices.getAuthMethod();
}

void MainWindow::resetSendPassword()
{
    hasSendPassword = false;
}

bool MainWindow::checkIsAdmin(QString userName)
{
    int ngroups = 512;
    struct passwd *pw;
    struct group *gr;
    gid_t groups[512];

    pw = getpwnam(userName.toLocal8Bit().data());
    if (pw == NULL) {
        qDebug() << userName << "is not exists";
        return false;
    }

    if (getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups) == -1) {
        qDebug() << userName << "get group failed";
        return false;
    }

    for (size_t j = 0; j < ngroups; j++) {
        gr = getgrgid(groups[j]);
        if (gr != NULL) {
            if (strcmp("sudo", gr->gr_name) == 0)
                return true;
        }
    }

    return false;
}

/*** end of private member ***/
