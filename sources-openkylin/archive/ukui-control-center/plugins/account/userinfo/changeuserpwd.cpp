/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
#include "changeuserpwd.h"

#include "passwdcheckutil.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QKeyEvent>
#include <QDebug>
#include <QMessageBox>

#include <QDBusInterface>
#include <QDBusReply>
#include <QRegExpValidator>
#include <QCoreApplication>

#include <QPainter>
#include <QPainterPath>
#include <QProcess>

#include <stdint.h>


#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <unistd.h>

#include <kysdk/applications/accessinfohelper.h>

#include "ukcccommon.h"
using namespace ukcc;

// 由于当前仓库还没有kylin_pwhistory.h模块，自定义返回值类型
#define KYLIN_PWHISTORY_SUCCESS       0
#define KYLIN_PWHISTORY_USED          100

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

ChangeUserPwd::ChangeUserPwd(QString n, QWidget *parent) :
    KDialog(parent),
    name(n)
{

    //判断是否是当前用户
    if (QString::compare(name, QString(g_get_user_name())) == 0){
        isCurrentUser = true;
    } else {
        isCurrentUser = false;
    }

    //密码校验状态
    isChecking = false;

    //构造密码校验线程
    thread1ForCheckPwd = new PwdCheckThread(this);

    m_pUniauthBackendDbus = new UniAuthService(this);
    // 将以后所有DBus调用的超时设置为 milliseconds
    m_pUniauthBackendDbus->setTimeout(2147483647); // -1 为默认的25s超时
    userSettingDbus =  new QDBusInterface("org.ukui.groupmanager",
                                          "/org/ukui/groupmanager",
                                          "org.ukui.groupmanager.interface",
                                          QDBusConnection::systemBus());
    // 将以后所有DBus调用的超时设置为 milliseconds
    userSettingDbus->setTimeout(2147483647); // -1 为默认的25s超时
    QDBusReply<bool> isPasswordMinAgeReached = userSettingDbus->call("isPasswordMinAgeReached", name);
    m_isPasswordMinAgeReached = isPasswordMinAgeReached;

    makeSurePwqualityEnabled();

    m_EnablePwdHistory = UkccCommon::checkPamPwHistory();

    initUI();
    setupStatus(name);

    setupConnect();
}

ChangeUserPwd::~ChangeUserPwd()
{
}

void ChangeUserPwd::makeSurePwqualityEnabled()
{
    if (UkccCommon::isDomainUser(g_get_user_name())){
        enablePwdQuality = false;
        return;
    }
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
    if (ret != 0){
        enablePwdQuality = false;
        qDebug() << "Reading pwquality configuration file failed: " << pwquality_strerror(buf, sizeof(buf), ret, auxerror);
    } else {
        enablePwdQuality = true;
    }

    if (PasswdCheckUtil::getCurrentPamState())
        enablePwdQuality = true;
    else
        enablePwdQuality = false;

#else
    enablePwdQuality = false;
#endif
}

void ChangeUserPwd::initUI()
{
    setWindowTitle(tr("Change password"));
    setWindowIcon("ukui-control-center");

    m_pTipMessage = new MessageBar;
    m_pTipMessage->setFixedSize(432, 36);
    m_pTipMessage->setContentsMargins(0,0,0,0);
    KLabel *warningLabel = new KLabel;
    warningLabel->setPixmap(QIcon::fromTheme("dialog-warning"), 16, 16);
    warningLabel->setText(tr("The password cannot be changed before the minimum password change interval has been reached."));
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setContentsMargins(16,0,16,0);
    hlayout->addWidget(warningLabel);
    m_pTipMessage->setLayout(hlayout);

    //当前密码
    currentPwdLabel = new KLabel();
    currentPwdLabel->setFixedWidth(100);
    currentPwdLabel->setText(tr("Current Pwd"));
    curTipLabel = new KLabel();
    QFont ft;
    ft.setPixelSize(12);
    curTipLabel->setFont(ft);
    curTipLabel->setFixedSize(QSize(322, 24));
    curTipLabel->setFontColor(Qt::red);
    curTipHorLayout = new QHBoxLayout;
    curTipHorLayout->setContentsMargins(110, 0, 0, 0);
    curTipHorLayout->addStretch();
    curTipHorLayout->addWidget(curTipLabel);
    currentPwdLineEdit = new KPasswordEdit(this);
    currentPwdLineEdit->setFixedWidth(322);
    auto placehoderText = tr("Required");
    currentPwdLineEdit->setPlaceholderText(placehoderText);
    currentPwdLineEdit->setEchoMode(QLineEdit::Password);
    QRegExp inputRegExp("^[A-Za-z0-9`~!@#$%^&*()_-+=<>,.\\\/?:;\"'|\{\}\ ]+$");
    QRegExpValidator *inputLimits = new QRegExpValidator(inputRegExp, this);
    currentPwdLineEdit->setValidator(inputLimits);
    currentPwdLineEdit->setTextMargins(0,0,30,0);
    currentPwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    currentPwdLineEdit->installEventFilter(this);
    currentPwdLineEdit->setFocus();
    currentPwdLineEdit->setAccessibleName(tr("Current Pwd"));

    currentPwdHorLayout = new QHBoxLayout;
    currentPwdHorLayout->setSpacing(8);
    currentPwdHorLayout->setContentsMargins(0, 0, 0, 0);
    currentPwdHorLayout->addWidget(currentPwdLabel);
    currentPwdHorLayout->addWidget(currentPwdLineEdit);

    //新密码
    newPwdLabel = new KLabel();
    newPwdLabel->setFixedWidth(100);
    newPwdLabel->setText(tr("New Pwd"));
    newPwdLineEdit = new KPasswordEdit(this);
    newPwdLineEdit->setFixedWidth(322);
    newPwdLineEdit->setPlaceholderText(placehoderText);
    newPwdLineEdit->setEchoMode(QLineEdit::Password);
    newPwdLineEdit->setValidator(inputLimits);
    newPwdLineEdit->setTextMargins(0,0,30,0);
    newPwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    newPwdLineEdit->installEventFilter(this);
    newPwdLineEdit->setAccessibleName(tr("New Pwd"));

    newPwdHorLayout = new QHBoxLayout;
    newPwdHorLayout->setSpacing(8);
    newPwdHorLayout->setContentsMargins(0, 0, 0, 0);
    newPwdHorLayout->addWidget(newPwdLabel);
    newPwdHorLayout->addWidget(newPwdLineEdit);

    newTipLabel = new KLabel();
    newTipLabel->setFont(ft);
    newTipLabel->setFixedSize(QSize(322, 24));
    newTipLabel->setFontColor(Qt::red);
    newTipHorLayout = new QHBoxLayout;
    newTipHorLayout->setContentsMargins(110, 0, 0, 0);
    newTipHorLayout->addStretch();
    newTipHorLayout->addWidget(newTipLabel);

    //确认密码
    surePwdLabel = new KLabel();
    surePwdLabel->setFixedWidth(100);
    surePwdLabel->setText(tr("Sure Pwd"));
    surePwdLineEdit = new KPasswordEdit(this);
    surePwdLineEdit->setFixedWidth(322);
    auto placehoder = tr("Verification");
    surePwdLineEdit->setPlaceholderText(placehoder);
    surePwdLineEdit->setEchoMode(QLineEdit::Password);
    surePwdLineEdit->setValidator(inputLimits);
    surePwdLineEdit->setTextMargins(0,0,30,0);
    surePwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    surePwdLineEdit->installEventFilter(this);
    surePwdLineEdit->setAccessibleName(tr("Sure Pwd"));
    surePwdHorLayout = new QHBoxLayout;
    surePwdHorLayout->setSpacing(8);
    surePwdHorLayout->setContentsMargins(0, 0, 0, 0);
    surePwdHorLayout->addWidget(surePwdLabel);
    surePwdHorLayout->addWidget(surePwdLineEdit);

    tipLabel = new KLabel();
    tipLabel->setFont(ft);
    tipLabel->setFixedSize(QSize(322, 30));
    tipLabel->setFontColor(Qt::red);
    tipHorLayout = new QHBoxLayout;
    tipHorLayout->setSpacing(0);
    tipHorLayout->setContentsMargins(110, 0, 0, 0);
    tipHorLayout->addStretch();
    tipHorLayout->addWidget(tipLabel);

    // 密码提示
    m_pPasswordPromptLabel = new KLabel();
    m_pPasswordPromptLabel->setFixedWidth(100);
    m_pPasswordPromptLabel->setText(tr("Password Prompt"));
    m_pPasswordPromptLineedit = new QLineEdit(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_pPasswordPromptLineedit, "UserInfo", "", "");
    m_pPasswordPromptLineedit->setFixedWidth(322);
    m_pPasswordPromptLineedit->setPlaceholderText(tr("Optional(Recommended)"));
    m_pPasswordPromptLineedit->setTextMargins(0,0,30,0);
    m_pPasswordPromptLineedit->setContextMenuPolicy(Qt::NoContextMenu);
    m_pPasswordPromptLineedit->installEventFilter(this);
    m_pPasswordPromptLineedit->setAccessibleName(tr("Password Prompt"));
    QHBoxLayout *pwdPromptHLayout = new QHBoxLayout;
    pwdPromptHLayout->setSpacing(8);
    pwdPromptHLayout->setContentsMargins(0, 0, 0, 0);
    pwdPromptHLayout->addWidget(m_pPasswordPromptLabel);
    pwdPromptHLayout->addWidget(m_pPasswordPromptLineedit);

    m_pPasswordPromptTips = new KLabel();
    m_pPasswordPromptTips->setFont(ft);
    m_pPasswordPromptTips->setFixedSize(QSize(322, 30));
    m_pPasswordPromptTips->setFontColor(Qt::red);
    QHBoxLayout *passwordPromptTipsHLayout = new QHBoxLayout;
    passwordPromptTipsHLayout->setSpacing(0);
    passwordPromptTipsHLayout->setContentsMargins(110, 0, 0, 0);
    passwordPromptTipsHLayout->addStretch();
    passwordPromptTipsHLayout->addWidget(m_pPasswordPromptTips);

    //中部输入区域
    contentVerLayout = new QVBoxLayout;
    contentVerLayout->setSpacing(0);
    contentVerLayout->setContentsMargins(24, 0, 35, 0);
    if (isCurrentUser) {
        if (!m_isPasswordMinAgeReached) {
            contentVerLayout->addWidget(m_pTipMessage);
            contentVerLayout->addSpacing(16);
            m_pTipMessage->show();
            setFixedSize(QSize(480, 420));
        } else {
            m_pTipMessage->hide();
            setFixedSize(QSize(480, 360));
        }

        contentVerLayout->addLayout(currentPwdHorLayout);
        contentVerLayout->addLayout(curTipHorLayout);

    } else {
        currentPwdLabel->setVisible(false);
        currentPwdLineEdit->setVisible(false);
        curTipLabel->setVisible(false);
        setFixedSize(480, 310);
    }
    contentVerLayout->addLayout(newPwdHorLayout);
    contentVerLayout->addLayout(newTipHorLayout);
    contentVerLayout->addLayout(surePwdHorLayout);
    contentVerLayout->addLayout(tipHorLayout);
    contentVerLayout->addLayout(pwdPromptHLayout);
    contentVerLayout->addLayout(passwordPromptTipsHLayout);

    //底部“取消”、“确定”按钮
    cancelBtn = new KPushButton();
    cancelBtn->setMinimumWidth(96);
    cancelBtn->setText(tr("Cancel"));
    confirmBtn = new KPushButton();
    confirmBtn->setMinimumWidth(96);
    confirmBtn->setText(tr("Confirm"));
    confirmBtn->setBackgroundColorHighlight(true);

    bottomBtnsHorLayout = new QHBoxLayout;
    bottomBtnsHorLayout->setSpacing(16);
    bottomBtnsHorLayout->setContentsMargins(24, 0, 24, 0);
    bottomBtnsHorLayout->addStretch();
    bottomBtnsHorLayout->addWidget(cancelBtn);
    bottomBtnsHorLayout->addWidget(confirmBtn);

    mainVerLayout = new QVBoxLayout;
    mainVerLayout->setContentsMargins(0, 10, 0, 24);
    mainVerLayout->addLayout(contentVerLayout);
    mainVerLayout->addStretch();
    mainVerLayout->addLayout(bottomBtnsHorLayout);

    mainWidget()->setLayout(mainVerLayout);
}

bool ChangeUserPwd::changePassword(const QString &programPath, const QString &oldPassword, const QString &newPassword, int *resultCode, QString *resultMsg)
{
    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("UKCC_SECURE_MODE", "1");
    process.setProcessEnvironment(env);

    /* 启动程序，不传递任何命令行参数 */
    process.start(programPath, QStringList());
    if (!process.waitForStarted(5000)) {
        qWarning() << "Failed to start process:" << process.errorString();
        return false;
    }

    /* 编码密码为 UTF-8 */
    QByteArray oldPwdBytes = oldPassword.toUtf8();
    QByteArray newPwdBytes = newPassword.toUtf8();

    /* 构造二进制数据包 */
    QByteArray packet;
    packet.reserve(8 + oldPwdBytes.size() + newPwdBytes.size());

    /* 旧密码长度（4字节，小端序） */
    uint32_t oldLen = static_cast<uint32_t>(oldPwdBytes.size());
    packet.append(reinterpret_cast<const char*>(&oldLen), sizeof(oldLen));
    packet.append(oldPwdBytes);

    /* 新密码长度（4字节，小端序） */
    uint32_t newLen = static_cast<uint32_t>(newPwdBytes.size());
    packet.append(reinterpret_cast<const char*>(&newLen), sizeof(newLen));
    packet.append(newPwdBytes);

    /* 写入 stdin */
    process.write(packet);
    process.closeWriteChannel();  /* 关闭写端，通知子进程数据发送完毕 */

    /* 等待子进程完成 */
    if (!process.waitForFinished(30000)) {
        qWarning() << "Process timeout or failed:" << process.errorString();
        process.kill();
        return false;
    }

    /* 读取 stdout 返回的结果 */
    QByteArray output = process.readAllStandardOutput();
    if (output.size() < 8) {
        qWarning() << "Invalid response from pipe, too short:" << output.size();
        return false;
    }

    /* 解析结果码 */
    uint32_t code = 0;
    memcpy(&code, output.constData(), sizeof(code));
    *resultCode = static_cast<int>(code);

    /* 解析消息长度 */
    uint32_t msgLen = 0;
    memcpy(&msgLen, output.constData() + 4, sizeof(msgLen));

    /* 解析消息 */
    if (msgLen > 0 && output.size() >= static_cast<int>(8 + msgLen)) {
        *resultMsg = QString::fromUtf8(output.constData() + 8, msgLen);
    } else {
        *resultMsg = QString();
    }

    return true;
}

void ChangeUserPwd::setupConnect()
{
    //通用的connect
    connect(cancelBtn, &QPushButton::clicked, this, [=]{
        close();
    });

    connect(newPwdLineEdit, &QLineEdit::textChanged, [=]{
        checkPwdLegality();

        passwordPromptChangedSlot();

        refreshConfirmBtnStatus();
    });

    connect(surePwdLineEdit, &QLineEdit::textChanged, [=](QString txt){
        if (!txt.isEmpty() && txt != newPwdLineEdit->text()){
            surePwdTip = tr("Inconsistency with pwd");
        } else {
            surePwdTip = "";
        }

        tipLabel->setText(surePwdTip);

        refreshConfirmBtnStatus();
    });

    connect(m_pPasswordPromptLineedit, &QLineEdit::textChanged, this, &ChangeUserPwd::passwordPromptChangedSlot);

    //需要区分的connect
    if (isCurrentUser) {
        connect(thread1ForCheckPwd, &PwdCheckThread::complete, this, &ChangeUserPwd::checkPwdFinishedSlot);

        connect(currentPwdLineEdit, &QLineEdit::textChanged, [=](QString txt){
            if (!txt.isEmpty()){
                curPwdTip = "";
                curTipLabel->setText(curPwdTip);

                //再次校验新密码，需要保证"与旧密码相同"等条件生效
                checkPwdLegality();

            }

            refreshConfirmBtnStatus();
        });

        connect(confirmBtn, &KPushButton::clicked, [=]{
            //密码校验失败有几秒延时，防止用户重复点击确定按钮
            if (isChecking)
                return;

            thread1ForCheckPwd->setArgs(name, currentPwdLineEdit->text());
            thread1ForCheckPwd->start();

            //开始当前密码校验
            isChecking = true;

            if(m_timer == nullptr)
            {
                m_timer = new QTimer(this);
                m_timer->setInterval(150);
                connect(m_timer, &QTimer::timeout, this, &ChangeUserPwd::updatePixmap);
                m_waitingPixmap = QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(24, 24);
                confirmBtn->setIcon(QIcon(m_waitingPixmap));
                confirmBtn->setText(tr(""));
                m_timer->start();
            } else {
                confirmBtn->setIconSize(QSize(24,24));
                confirmBtn->setText(tr(""));
                m_timer->start();
            }
        });
    } else {
        connect(confirmBtn, &KPushButton::clicked, this, [=]{
            //修改其他用户密码
            emit changeOtherUserPwd(newPwdLineEdit->text());

            // 绑定密码提示需要用户id
            if (m_pPasswordPromptTips->text().isEmpty()) {
                emit changeOtherUserPrompt(m_pPasswordPromptLineedit->text());
            }

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
            newPwdTip = tr("The new password cannot be the same sa the old one.");
            newTipLabel->setText(newPwdTip);
            newPwdLineEdit->setFocus();
            confirmBtn->setIconSize(QSize(0,0));
            confirmBtn->setText(tr("Confirm"));
            m_timer->stop();
            refreshConfirmBtnStatus();

            //密码校验完成
            isChecking = false;
        } else {
            //修改密码
            QString programPath = QString("/usr/bin/changeuserpwd");
            QString currentPwd = currentPwdLineEdit->text();
            QString newPwd = newPwdLineEdit->text();

            /* 直接调用类 A 的成员函数 */
            int resultCode = -1;
            QString resultMsg;

            if (changePassword(programPath, currentPwd, newPwd, &resultCode, &resultMsg)) {
                qDebug() << "Result code:" << resultCode;
                qDebug() << "Message:" << resultMsg;
            }

            // 绑定密码提示需要用户id
            if (m_pPasswordPromptTips->text().isEmpty()) {
                int ret = m_pUniauthBackendDbus->SetUserPasswordHint(getuid(), m_pPasswordPromptLineedit->text());
                qDebug() << "reply of SetUserPasswordHint in changeUserPwd = " << ret;
            }

            this->accept();

            if (UkccCommon::isDomainUser(g_get_user_name())) {
                QString primaryText;
                primaryText = resultCode == 0 ? tr("Pwd Changed Succes") : resultMsg;
                qDebug() << "output of changeUserpwd = " << resultMsg;
                QMessageBox::warning(NULL, "", primaryText, QMessageBox::Yes);
            }
        }
    } else {
        if (re.contains("Failed")){
            curPwdTip = tr("Authentication failed, input authtok again!");
        }

        curTipLabel->setText(curPwdTip);

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

void ChangeUserPwd::passwordPromptChangedSlot()
{
   if (!m_pPasswordPromptLineedit->text().isEmpty() && m_pPasswordPromptLineedit->text().contains(newPwdLineEdit->text())) {
       m_pPasswordPromptTips->setText(tr("Your password cannot be included in the password prompt."));
   } else {
       m_pPasswordPromptTips->setText("");
   }

   refreshConfirmBtnStatus();
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

    //初始化确定按钮状态
    refreshConfirmBtnStatus();
}


void ChangeUserPwd::refreshCloseBtnStatus(){
    if (isChecking){
//        closeBtn->setEnabled(false);
        cancelBtn->setEnabled(false);
    } else {
//        closeBtn->setEnabled(true);
        cancelBtn->setEnabled(true);
    }
}

void ChangeUserPwd::refreshConfirmBtnStatus()
{
    // 当前用户，未到最短密码使用期限，不允许修改密码
    if (isCurrentUser && !m_isPasswordMinAgeReached) {
        confirmBtn->setEnabled(false);
        return;
    }
    if (getuid() && isCurrentUser) {
        if (!tipLabel->text().isEmpty() ||
                currentPwdLineEdit->text().isEmpty() || currentPwdLineEdit->text() == tr("Current Pwd") ||
                newPwdLineEdit->text().isEmpty() || newPwdLineEdit->text() == tr("New Pwd") ||
                surePwdLineEdit->text().isEmpty() || surePwdLineEdit->text() == tr("Sure Pwd") ||
                !curPwdTip.isEmpty() || !newPwdTip.isEmpty() || !surePwdTip.isEmpty()
                || !m_pPasswordPromptTips->text().isEmpty()) {
             confirmBtn->setEnabled(false);
        } else {
            confirmBtn->setEnabled(true);
        }
    } else {
        if (!tipLabel->text().isEmpty() ||
                newPwdLineEdit->text().isEmpty() || newPwdLineEdit->text() == tr("New Pwd") ||
                surePwdLineEdit->text().isEmpty() || surePwdLineEdit->text() == tr("Sure Pwd") ||
                !newPwdTip.isEmpty() || !surePwdTip.isEmpty()
                || !m_pPasswordPromptTips->text().isEmpty()) {
            confirmBtn->setEnabled(false);
        } else {
            confirmBtn->setEnabled(true);
        }
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
    //判断用户是否输入了非法字符
    if (!isContainLegitimacyChar(newPwdLineEdit->text())){
        newPwdTip = tr("Contains illegal characters!");
    } else {
        if (enablePwdQuality){

#ifdef ENABLEPQ
            void * auxerror;
            int ret;
            const char * msg;
            char buf[256];

            QByteArray ba = newPwdLineEdit->text().toLatin1();
            QByteArray ba1 = currentPwdLineEdit->text().toLatin1();

            if (isCurrentUser){
               ret = pwquality_check(settings, ba.data(), ba1.data(), name.toLatin1().data(), &auxerror);
            } else {
               ret = pwquality_check(settings, ba.data(), NULL, name.toLatin1().data(), &auxerror);
            }

            if (ret == PWQ_ERROR_SAME_PASSWORD) {
                ret = pwquality_check(settings, ba.data(), NULL, name.toLatin1().data(), &auxerror);
            }
            if (ret < 0 && newPwdLineEdit->text().length() > 0){
                msg = pwquality_strerror(buf, sizeof(buf), ret, auxerror);
                newPwdTip = QString(msg);
            } else {
                newPwdTip = "";
            }
#endif

        } else {
            //系统未开启pwdquality模块
            newPwdTip = "";
        }
    }

    // 密码历史模块校验
    if (m_EnablePwdHistory && !newPwdLineEdit->text().isEmpty() && newPwdTip.isEmpty()) {
        int ret = m_pUniauthBackendDbus->CheckUserHistoryPassword(name, newPwdLineEdit->text());
        qDebug() << "kylin_check_and_save_password reply = " << ret;
        if (ret == KYLIN_PWHISTORY_USED) {
            newPwdTip = tr("The new password cannot be the same as any previous passwords.");
        } else if (ret != KYLIN_PWHISTORY_SUCCESS) {
            newPwdTip = tr("Password history module check failed! error num %1").arg(ret);
        }
    }

    //防止先输入确认密码，再输入密码后tipLabel无法刷新
    if (!surePwdLineEdit->text().isEmpty()){
        if (QString::compare(newPwdLineEdit->text(), surePwdLineEdit->text()) == 0){
            surePwdTip = "";
        } else {
            surePwdTip = tr("Inconsistency with pwd");
        }
    }

    //设置新密码的提示
    if (!newPwdLineEdit->text().isEmpty() || newPwdLineEdit->hasFocus()) {
        newTipLabel->setText(newPwdTip);
    }

    tipLabel->setText(surePwdTip);

    curTipLabel->setText(curPwdTip);

}

void ChangeUserPwd::keyPressEvent(QKeyEvent * event){
    switch (event->key())
    {
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

//事件过滤如下
//当接收到这些事件时，需要被过滤掉，所以返回true
bool ChangeUserPwd::eventFilter(QObject *target, QEvent *event)
{
    if (target == currentPwdLineEdit || target == newPwdLineEdit || target == surePwdLineEdit) {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

            if(keyEvent->matches(QKeySequence::Copy) || keyEvent->matches(QKeySequence::Cut))
            {
                qDebug() <<"Copy || Cut";
                return true;
            }
        }
    }
    if (event->type() == QEvent::FocusOut) {
        if (target == currentPwdLineEdit) {
            if (currentPwdLineEdit->text().isEmpty()) {
                curPwdTip = tr("Current pwd cannot be empty!");
                curTipLabel->setText(curPwdTip);
            }
        } else if (target == newPwdLineEdit) {
            if (newPwdLineEdit->text().isEmpty()) {
                newPwdTip = tr("New pwd cannot be empty!");
                newTipLabel->setText(newPwdTip);
            }
        } else if (target == surePwdLineEdit) {
            if (surePwdLineEdit->text().isEmpty()) {
                surePwdTip = tr("Sure pwd cannot be empty!");
                tipLabel->setText(surePwdTip);
            }
        }
    }

    return KDialog::eventFilter(target, event);
    //继续传递该事件到被观察者，由其本身调用相应的事件。
}

