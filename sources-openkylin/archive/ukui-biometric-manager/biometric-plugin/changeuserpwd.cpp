/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "changeuserpwd.h"
#include <ukuiwindowhelper.h>
#include "passwdcheckutil.h"
#include "securityquestionanswer.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QKeyEvent>
#include <QDebug>
#include <QMessageBox>

#include <QDBusInterface>
#include <QDBusReply>

#include <QCoreApplication>

#include <QPainter>
#include <QPainterPath>
#include <QRegExp>
#include <QRegExpValidator>
#include <QCloseEvent>
#include <kysdk/applications/accessinfohelper.h>

#ifdef signals
#undef signals
#endif

extern "C"
{

#include <glib.h>
#include <unistd.h>
}

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

ChangeUserPwd::ChangeUserPwd(QString n, QWidget *parent)
    : KDialog(parent), name(n), m_pSecurityQuestionAnswer(new SecurityQuestionAnswer(this))
{

    // 判断是否是当前用户
    if (QString::compare(name, QString(g_get_user_name())) == 0) {
        isCurrentUser = true;
    } else {
        isCurrentUser = false;
    }

    // 密码校验状态
    isChecking = false;

    // 构造密码校验线程
    thread1ForCheckPwd = new PwdCheckThread();

    makeSurePwqualityEnabled();

    initUI();
    setupStatus(name);

    setupConnect();
}

ChangeUserPwd::~ChangeUserPwd() {}

void ChangeUserPwd::makeSurePwqualityEnabled()
{
#ifdef ENABLEPQ
    int ret;
    void *auxerror;
    char buf[255];

    settings = pwquality_default_settings();
    if (settings == NULL) {
        enablePwdQuality = false;
        qDebug() << "init pwquality settings failed";
    } else {
        enablePwdQuality = true;
    }

    ret = pwquality_read_config(settings, PWCONF, &auxerror);
    if (ret != 0) {
        enablePwdQuality = false;
        qDebug() << "Reading pwquality configuration file failed: "
                 << pwquality_strerror(buf, sizeof(buf), ret, auxerror);
    } else {
        enablePwdQuality = true;
    }

    if (PasswdCheckUtil::getCurrentPamState()) {
        enablePwdQuality = true;
    } else {
        enablePwdQuality = false;
    }

#else
    enablePwdQuality = false;
#endif
}

void ChangeUserPwd::initUI()
{
    setFixedSize(QSize(480, 360));
    // 去除窗管标题栏
    UkuiWindowHelper *pUkuiWindowHelper = new UkuiWindowHelper(this);
    pUkuiWindowHelper->removeTitleBar();
    this->setWindowTitle(tr("Change password"));
    this->setWindowIcon("ukui-control-center");
    // 当前密码
    currentPwdLabel = new QLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(
        currentPwdLabel, "Login Options", "ChangeUserPwd", "current password label of ChangeUserPwd dailog");
    currentPwdLabel->setFixedSize(100, 24);
    setTextDynamicInPwd(currentPwdLabel, tr("Current Pwd"));
    curTipLabel = new QLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(
        curTipLabel, "Login Options", "ChangeUserPwd", "current password tip label of ChangeUserPwd dailog");
    QFont ft;
    ft.setPixelSize(14);
    curTipLabel->setFont(ft);
    curTipLabel->setFixedSize(QSize(322, 24));
    curTipLabel->setStyleSheet("color:red;");
    curTipHorLayout = new QHBoxLayout;
    curTipHorLayout->setContentsMargins(110, 0, 0, 0);
    curTipHorLayout->addStretch();
    curTipHorLayout->addWidget(curTipLabel);
    currentPwdLineEdit = new KPasswordEdit(this);
    KDK_EXTEND_ALL_INFO_FORMAT(
        currentPwdLineEdit, "Login Options", "ChangeUserPwd", "current password edit of ChangeUserPwd dailog");
    currentPwdLineEdit->setFixedSize(QSize(322, 36));
    auto placehoderText = tr("Required");
    currentPwdLineEdit->setPlaceholderText(placehoderText);
    currentPwdLineEdit->setEchoMode(QLineEdit::Password);
    currentPwdLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    QRegExp inputRegExp("^[A-Za-z0-9`~!@#$%^&*()_-+=<>,.\\\/?:;\"'|\{\}\ ]+$");
    QRegExpValidator *inputLimits = new QRegExpValidator(inputRegExp, this);
    currentPwdLineEdit->setValidator(inputLimits);
    currentPwdLineEdit->setTextMargins(0, 0, 30, 0);
    currentPwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    currentPwdLineEdit->installEventFilter(this);

    currentPwdHorLayout = new QHBoxLayout;
    currentPwdHorLayout->setSpacing(8);
    currentPwdHorLayout->setContentsMargins(0, 0, 0, 0);
    currentPwdHorLayout->addWidget(currentPwdLabel);
    currentPwdHorLayout->addWidget(currentPwdLineEdit);

    // 新密码
    newPwdLabel = new QLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(
        newPwdLabel, "Login Options", "ChangeUserPwd", "new password label of ChangeUserPwd dailog");
    newPwdLabel->setFixedSize(100, 24);
    setTextDynamicInPwd(newPwdLabel, tr("New Pwd"));
    newTipLabel = new QLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(
        newTipLabel, "Login Options", "ChangeUserPwd", "new password tip label of ChangeUserPwd dailog");
    newTipLabel->setFont(ft);
    newTipLabel->setFixedSize(QSize(322, 24));
    newTipLabel->setStyleSheet("color:red;");
    newTipHorLayout = new QHBoxLayout;
    newTipHorLayout->setContentsMargins(110, 0, 0, 0);
    newTipHorLayout->addStretch();
    newTipHorLayout->addWidget(newTipLabel);
    newPwdLineEdit = new KPasswordEdit(this);
    KDK_EXTEND_ALL_INFO_FORMAT(
        newPwdLineEdit, "Login Options", "ChangeUserPwd", "new password edit of ChangeUserPwd dailog");
    newPwdLineEdit->setFixedSize(QSize(322, 36));
    newPwdLineEdit->setPlaceholderText(placehoderText);
    newPwdLineEdit->setEchoMode(QLineEdit::Password);
    newPwdLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    newPwdLineEdit->setValidator(inputLimits);
    newPwdLineEdit->setTextMargins(0, 0, 30, 0);
    newPwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    newPwdLineEdit->installEventFilter(this);

    newPwdHorLayout = new QHBoxLayout;
    newPwdHorLayout->setSpacing(8);
    newPwdHorLayout->setContentsMargins(0, 0, 0, 0);
    newPwdHorLayout->addWidget(newPwdLabel);
    newPwdHorLayout->addWidget(newPwdLineEdit);

    // 确认密码
    surePwdLabel = new QLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(
        surePwdLabel, "Login Options", "ChangeUserPwd", "sure password label of ChangeUserPwd dailog");
    surePwdLabel->setFixedSize(100, 24);
    setTextDynamicInPwd(surePwdLabel, tr("Sure Pwd"));

    surePwdLineEdit = new KPasswordEdit(this);
    KDK_EXTEND_ALL_INFO_FORMAT(
        surePwdLineEdit, "Login Options", "ChangeUserPwd", "sure password edit of ChangeUserPwd dailog");
    surePwdLineEdit->setFixedSize(QSize(322, 36));
    surePwdLineEdit->setPlaceholderText(placehoderText);
    surePwdLineEdit->setEchoMode(QLineEdit::Password);
    surePwdLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    surePwdLineEdit->setValidator(inputLimits);
    surePwdLineEdit->setTextMargins(0, 0, 30, 0);
    surePwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    surePwdLineEdit->installEventFilter(this);

    surePwdHorLayout = new QHBoxLayout;
    surePwdHorLayout->setSpacing(8);
    surePwdHorLayout->setContentsMargins(0, 0, 0, 0);
    surePwdHorLayout->addWidget(surePwdLabel);
    surePwdHorLayout->addWidget(surePwdLineEdit);

    tipLabel = new QLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(tipLabel, "Login Options", "ChangeUserPwd", "tip label of ChangeUserPwd dailog");
    tipLabel->setFont(ft);
    tipLabel->setFixedSize(QSize(322, 30));
    tipLabel->setStyleSheet("color:red;");

    tipHorLayout = new QHBoxLayout;
    tipHorLayout->setSpacing(0);
    tipHorLayout->setContentsMargins(110, 0, 0, 0);
    tipHorLayout->addStretch();
    tipHorLayout->addWidget(tipLabel);

    surePwdWithTipVerLayout = new QVBoxLayout;
    surePwdWithTipVerLayout->setSpacing(8);
    surePwdWithTipVerLayout->setContentsMargins(0, 0, 0, 0);
    surePwdWithTipVerLayout->addLayout(surePwdHorLayout);
    surePwdWithTipVerLayout->addLayout(tipHorLayout);

    // 密码提示
    m_pPwdTipLabel = new QLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(m_pPwdTipLabel, "Login Options", "ChangeUserPwd", "new password tip label of ChangeUserPwd dailog");
    m_pPwdTipLabel->setFixedSize(100, 24);
    setTextDynamicInPwd(m_pPwdTipLabel, tr("Password Prompt"));

    m_pPwdTipLineEdit = new QLineEdit(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pPwdTipLabel, "Login Options", "ChangeUserPwd", "new password tip lineedit of ChangeUserPwd dailog");
    m_pPwdTipLineEdit->setFixedSize(QSize(322, 36));
    m_pPwdTipLineEdit->setPlaceholderText(tr("Optional(Recommended)"));
    m_pPwdTipLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_pPwdTipLineEdit->installEventFilter(this);

    m_pPwdTipHorLayout = new QHBoxLayout;
    m_pPwdTipHorLayout->setSpacing(8);
    m_pPwdTipHorLayout->setContentsMargins(0, 0, 0, 0);
    m_pPwdTipHorLayout->addWidget(m_pPwdTipLabel);
    m_pPwdTipHorLayout->addWidget(m_pPwdTipLineEdit);

    m_pTipMessageLabel = new QLabel();
    m_pTipMessageLabel->setFont(ft);
    m_pTipMessageLabel->setFixedSize(QSize(322, 30));
    m_pTipMessageLabel->setStyleSheet("color:red;");

    m_pPwdTipMessageLayout = new QHBoxLayout;
    m_pPwdTipMessageLayout->setSpacing(0);
    m_pPwdTipMessageLayout->setContentsMargins(110, 0, 0, 0);
    m_pPwdTipMessageLayout->addStretch();
    m_pPwdTipMessageLayout->addWidget(m_pTipMessageLabel);

    m_pPwdTipWidthVerLayout = new QVBoxLayout;
    m_pPwdTipWidthVerLayout->setSpacing(8);
    m_pPwdTipWidthVerLayout->setContentsMargins(0, 0, 0, 0);
    m_pPwdTipWidthVerLayout->addLayout(m_pPwdTipHorLayout);
    m_pPwdTipWidthVerLayout->addLayout(m_pPwdTipMessageLayout);

    // 中部输入区域
    contentVerLayout = new QVBoxLayout;
    contentVerLayout->setSpacing(0);
    contentVerLayout->setContentsMargins(24, 0, 35, 0);
    if (isCurrentUser) {
        contentVerLayout->addLayout(currentPwdHorLayout);
        contentVerLayout->addLayout(curTipHorLayout);
    }
    contentVerLayout->addLayout(newPwdHorLayout);
    contentVerLayout->addLayout(newTipHorLayout);
    contentVerLayout->addLayout(surePwdWithTipVerLayout);
    contentVerLayout->addLayout(m_pPwdTipWidthVerLayout);

    // 底部“取消”、“确定”按钮
    cancelBtn = new QPushButton();
    KDK_EXTEND_ALL_INFO_FORMAT(cancelBtn, "Login Options", "ChangeUserPwd", "cancel button of ChangeUserPwd dailog");
    cancelBtn->setMinimumWidth(96);
    cancelBtn->setText(tr("Cancel"));
    cancelBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    cancelBtn->setProperty("useButtonPalette", true);
    confirmBtn = new QPushButton();
    KDK_EXTEND_ALL_INFO_FORMAT(confirmBtn, "Login Options", "ChangeUserPwd", "confirm button of ChangeUserPwd dailog");
    confirmBtn->setMinimumWidth(96);
    confirmBtn->setText(tr("Confirm"));
    confirmBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    confirmBtn->setProperty("isImportant", true);

    bottomBtnsHorLayout = new QHBoxLayout;
    bottomBtnsHorLayout->setSpacing(16);
    bottomBtnsHorLayout->setContentsMargins(0, 0, 25, 0);
    bottomBtnsHorLayout->addStretch();
    bottomBtnsHorLayout->addWidget(cancelBtn);
    bottomBtnsHorLayout->addWidget(confirmBtn);

    mainVerLayout = new QVBoxLayout;
    mainVerLayout->setContentsMargins(0, 10, 0, 24);
    mainVerLayout->addLayout(contentVerLayout);
    mainVerLayout->addStretch();
    mainVerLayout->addLayout(bottomBtnsHorLayout);

    this->mainWidget()->setLayout(mainVerLayout);
}

void ChangeUserPwd::test() {}

int ChangeUserPwd::isDomainUser(const char *username)
{
    FILE *fp;
    fp = fopen("/etc/passwd", "r");
    if (fp == NULL) {
        return 1;
    }
    char buf[1024], name[128];
    while (!feof(fp)) {
        if (fgets(buf, sizeof(buf), fp) == NULL) {
            break;
        }
        sscanf(buf, "%[^:]", name);
        if (strcmp(name, username) == 0) {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    return 1;
}

void ChangeUserPwd::onCompleted(const QString &re) {}

void ChangeUserPwd::setupConnect()
{
    // 通用的connect
    connect(cancelBtn, &QPushButton::clicked, this, [=] { close(); });

    connect(newPwdLineEdit, &QLineEdit::textEdited, [=] {
        checkPwdLegality();

        refreshConfirmBtnStatus();
    });

    connect(surePwdLineEdit, &QLineEdit::textEdited, [=](QString txt) {
        if (!txt.isEmpty() && txt != newPwdLineEdit->text()) {
            surePwdTip = tr("Inconsistency with pwd");
        } else {
            surePwdTip = "";
        }

        updateTipLableInfo(tipLabel, surePwdTip);

        checkPromptLegality();

        refreshConfirmBtnStatus();
    });

    connect(m_pPwdTipLineEdit, &QLineEdit::textEdited, [=] {
        checkPromptLegality();
        refreshConfirmBtnStatus();
    });

    // 需要区分的connect
    if (isCurrentUser) {
        connect(thread1ForCheckPwd, &PwdCheckThread::complete, this, &ChangeUserPwd::checkPwdFinishedSlot);

        connect(currentPwdLineEdit, &QLineEdit::textEdited, [=](QString txt) {
            if (!txt.isEmpty()) {
                curPwdTip = "";
                updateTipLableInfo(curTipLabel, curPwdTip);

                // 再次校验新密码，需要保证"与旧密码相同"等条件生效
                checkPwdLegality();
            }

            refreshConfirmBtnStatus();
        });

        connect(confirmBtn, &QPushButton::clicked, [=] {
            // 密码校验失败有几秒延时，防止用户重复点击确定按钮
            if (isChecking)
                return;

            thread1ForCheckPwd->setArgs(name, currentPwdLineEdit->text());
            thread1ForCheckPwd->start();

            // 开始当前密码校验
            isChecking = true;

            if (m_timer == nullptr) {
                m_timer = new QTimer(this);
                m_timer->setInterval(150);
                connect(m_timer, &QTimer::timeout, this, &ChangeUserPwd::updatePixmap);
                m_waitingPixmap = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(24, 24);
                confirmBtn->setIcon(QIcon(m_waitingPixmap));
                confirmBtn->setText(tr(""));
                m_timer->start();
            } else {
                confirmBtn->setIconSize(QSize(24, 24));
                confirmBtn->setText(tr(""));
                m_timer->start();
            }
        });
    } else {
        connect(confirmBtn, &QPushButton::clicked, this, [=] {
            // 修改其他用户密码
            QDBusInterface tmpiface(
                "com.control.center.qt.systemdbus", "/", "com.control.center.interface", QDBusConnection::systemBus());

            if (!tmpiface.isValid()) {
                qWarning() << "Create Client Interface Failed When : " << QDBusConnection::systemBus().lastError();
                return;
            }

            tmpiface.call("changeOtherUserPasswd", name, newPwdLineEdit->text());

            this->accept();
        });
    }

}

void ChangeUserPwd::checkPwdFinishedSlot(QString re)
{
    curPwdTip = re;

    //返回值为空，密码校验成功
    if (re.isEmpty()) {
        if (newPwdLineEdit->text().compare(currentPwdLineEdit->text(), Qt::CaseSensitive) == 0 && !newPwdLineEdit->text().isEmpty()) {
            newPwdTip = tr("Same with old pwd");
            updateTipLableInfo(newTipLabel, newPwdTip);
            newPwdLineEdit->setFocus();
            confirmBtn->setIconSize(QSize(0, 0));
            confirmBtn->setText(tr("Confirm"));
            m_timer->stop();
            refreshConfirmBtnStatus();

            // 密码校验完成
            isChecking = false;
        } else {
            //修改密码
            QString currentPwd = currentPwdLineEdit->text();
            QString newPwd = newPwdLineEdit->text();

            currentPwd = currentPwd.toLocal8Bit().toBase64();
            newPwd = newPwd.toLocal8Bit().toBase64();
            char *cmd = g_strdup_printf("/usr/bin/changeuserpwd %s %s", currentPwd.toLatin1().data(), newPwd.toLatin1().data());

            qDebug() << __FUNCTION__ << cmd;

            FILE * stream;
            QString result;
            char output[256];

            if ((stream = popen(cmd, "r")) != NULL){

                while(fgets(output, 256, stream) != NULL){
                    result = QString(output).simplified();
                }
                pclose(stream);
            }


            this->accept();

            if (isDomainUser(g_get_user_name())) {
                QString primaryText;
                primaryText = result.simplified().isEmpty() ? tr("Pwd Changed Succes") : output;
                qDebug() << "output of changeUserpwd = " << result;
                QMessageBox::warning(NULL, "", primaryText, QMessageBox::Yes);
            }
            m_pSecurityQuestionAnswer->SetUserPasswdHint(getuid(), m_pPwdTipLineEdit->text());
        }
    } else {
        if (re.contains("Failed")){
            curPwdTip = tr("Authentication failed, input authtok again!");
        }

        updateTipLableInfo(curTipLabel,curPwdTip);

        //重置当前密码输入框
        currentPwdLineEdit->setText("");
        currentPwdLineEdit->setFocus();

        confirmBtn->setIconSize(QSize(0,0));
        confirmBtn->setText(tr("Confirm"));
        m_timer->stop();
        refreshConfirmBtnStatus();

        //密码校验完成
        isChecking = false;
    }
}

void ChangeUserPwd::updatePixmap()
{
    QMatrix matrix;
    matrix.rotate(90.0);
    m_waitingPixmap = m_waitingPixmap.transformed(matrix, Qt::FastTransformation);
    confirmBtn->setIcon(m_waitingPixmap);
}

void ChangeUserPwd::setupStatus(QString n)
{

    curPwdTip = QString();
    newPwdTip = QString();
    surePwdTip = QString();

    // 初始化确定按钮状态
    refreshConfirmBtnStatus();
}

void ChangeUserPwd::updateTipLableInfo(QLabel *Label, QString info)
{

    if (setTextDynamicInPwd(Label, info)) {
        Label->setToolTip(info);
    } else {
        Label->setToolTip("");
    }
}

void ChangeUserPwd::refreshCloseBtnStatus()
{
    if (isChecking) {
        cancelBtn->setEnabled(false);
    } else {
        cancelBtn->setEnabled(true);
    }
}

void ChangeUserPwd::refreshConfirmBtnStatus()
{
    if (getuid() && isCurrentUser) {
        if (!tipLabel->text().isEmpty() || currentPwdLineEdit->text().isEmpty()
            || currentPwdLineEdit->text() == tr("Current Pwd") || newPwdLineEdit->text().isEmpty()
            || newPwdLineEdit->text() == tr("New Pwd") || surePwdLineEdit->text().isEmpty()
            || surePwdLineEdit->text() == tr("Sure Pwd") || !curPwdTip.isEmpty() || !newPwdTip.isEmpty()
            || !surePwdTip.isEmpty() || !m_pwdTipMessage.isEmpty())
            confirmBtn->setEnabled(false);
        else
            confirmBtn->setEnabled(true);
    } else {
        if (!tipLabel->text().isEmpty() || newPwdLineEdit->text().isEmpty() || newPwdLineEdit->text() == tr("New Pwd")
            || surePwdLineEdit->text().isEmpty() || surePwdLineEdit->text() == tr("Sure Pwd") || !newPwdTip.isEmpty()
            || !surePwdTip.isEmpty())
            confirmBtn->setEnabled(false);
        else
            confirmBtn->setEnabled(true);
    }
}

bool ChangeUserPwd::isContainLegitimacyChar(QString word)
{
    foreach (QChar ch, word){
        if (int(ch.toLatin1() <= 0 || int(ch.toLatin1()) > 127)){
            return false;
        }
    }
    return true;
}

void ChangeUserPwd::checkPwdLegality()
{

    // 判断用户是否输入了非法字符
    if (!isContainLegitimacyChar(newPwdLineEdit->text())) {
        newPwdTip = tr("Contains illegal characters!");
    } else {
        if (enablePwdQuality) {

#ifdef ENABLEPQ
            void *auxerror;
            int ret;
            const char *msg;
            char buf[256];

            QByteArray ba = newPwdLineEdit->text().toLatin1();
            QByteArray ba1 = currentPwdLineEdit->text().toLatin1();

            if (isCurrentUser) {
                ret = pwquality_check(settings, ba.data(), ba1.data(), name.toLatin1().data(), &auxerror);
            } else {
                ret = pwquality_check(settings, ba.data(), NULL, name.toLatin1().data(), &auxerror);
            }

            if (ret == PWQ_ERROR_SAME_PASSWORD) {
                ret = pwquality_check(settings, ba.data(), NULL, name.toLatin1().data(), &auxerror);
            }
            if (ret < 0 && newPwdLineEdit->text().length() > 0) {
                msg = pwquality_strerror(buf, sizeof(buf), ret, auxerror);
                newPwdTip = QString(msg);
            } else {
                newPwdTip = "";
            }
#endif

        } else {
            // 系统未开启pwdquality模块
            newPwdTip = "";
        }
    }

    // 防止先输入确认密码，再输入密码后tipLabel无法刷新
    if (!surePwdLineEdit->text().isEmpty()) {
        if (QString::compare(newPwdLineEdit->text(), surePwdLineEdit->text()) == 0) {
            surePwdTip = "";
        } else {
            surePwdTip = tr("Inconsistency with pwd");
        }
    }

    // 设置新密码的提示
    if (!newPwdLineEdit->text().isEmpty() || newPwdLineEdit->hasFocus()) {
        updateTipLableInfo(newTipLabel, newPwdTip);
    }

    updateTipLableInfo(tipLabel, surePwdTip);

    updateTipLableInfo(curTipLabel, curPwdTip);
}

void ChangeUserPwd::checkPromptLegality()
{
    if (!m_pPwdTipLineEdit->text().isEmpty() && !surePwdLineEdit->text().isEmpty()) {
        if (m_pPwdTipLineEdit->text().contains(surePwdLineEdit->text())) {
            m_pwdTipMessage = tr("Your password cannot be included in the password prompt.");
        } else {
            m_pwdTipMessage = "";
        }
        updateTipLableInfo(m_pTipMessageLabel, m_pwdTipMessage);
    }
}

bool ChangeUserPwd::setTextDynamicInPwd(QLabel *label, QString string)
{

    bool isOverLength = false;
    QFontMetrics fontMetrics(label->font());
    int fontSize = fontMetrics.width(string);

    QString str = string;
    int pSize = label->width();
    if (fontSize > pSize) {
        str = fontMetrics.elidedText(string, Qt::ElideRight, pSize);
        isOverLength = true;
        label->setToolTip(string);
    } else {
        label->setToolTip("");
    }
    label->setText(str);
    return isOverLength;
}

void ChangeUserPwd::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Escape:
            break;
        case Qt::Key_Enter:
            if (confirmBtn->isEnabled())
                confirmBtn->clicked();
            break;
        case Qt::Key_Return:
            if (confirmBtn->isEnabled())
                confirmBtn->clicked();
            break;
        default:
            QDialog::keyPressEvent(event);
    }
}

void ChangeUserPwd::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
}

// 事件过滤如下
// 当接收到这些事件时，需要被过滤掉，所以返回true
bool ChangeUserPwd::eventFilter(QObject *target, QEvent *event)
{
    if (target == currentPwdLineEdit || target == newPwdLineEdit || target == surePwdLineEdit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

            if (keyEvent->matches(QKeySequence::Copy) || keyEvent->matches(QKeySequence::Cut)) {
                qDebug() << "Copy || Cut";
                return true;
            }
        }
    }
    if (event->type() == QEvent::FocusOut) {
        if (target == currentPwdLineEdit) {
            if (currentPwdLineEdit->text().isEmpty()) {
                curPwdTip = tr("current pwd cannot be empty!");
                updateTipLableInfo(curTipLabel, curPwdTip);
            }
        } else if (target == newPwdLineEdit) {
            if (newPwdLineEdit->text().isEmpty()) {
                newPwdTip = tr("new pwd cannot be empty!");
                updateTipLableInfo(newTipLabel, newPwdTip);
            }
        } else if (target == surePwdLineEdit) {
            if (surePwdLineEdit->text().isEmpty()) {
                surePwdTip = tr("sure pwd cannot be empty!");
                updateTipLableInfo(tipLabel, surePwdTip);
            }
        }
    }

    return QWidget::eventFilter(target, event);
    // 继续传递该事件到被观察者，由其本身调用相应的事件。
}

void ChangeUserPwd::closeEvent(QCloseEvent *event)
{
    if (isChecking) {
        qInfo() << "closeEvent:" << isChecking;
        event->ignore();
    } else {
        event->accept();
    }
}
