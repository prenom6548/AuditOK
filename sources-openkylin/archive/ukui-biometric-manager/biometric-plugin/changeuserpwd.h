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
#ifndef CHANGEUSERPWD_H
#define CHANGEUSERPWD_H

#include <QDialog>
#include <QTimer>

#include <kpasswordedit.h>
#include <kdialog.h>
using namespace kdk;

#include "pwdcheckthread.h"
#include "passwdcheckutil.h"

#ifdef ENABLEPQ
extern "C" {

#include <pwquality.h>

}

#define PWCONF "/etc/security/pwquality.conf"
#define RFLAG 0x1
#define CFLAG 0x2

#endif

class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QMouseEvent;
class QKeyEvent;
class SecurityQuestionAnswer;

class ChangeUserPwd : public KDialog
{
    Q_OBJECT

public:
    explicit ChangeUserPwd(QString n, QWidget *parent = nullptr);
    ~ChangeUserPwd();

public:
    void initUI();
    void setupConnect();
    void setupStatus(QString n);

    void makeSurePwqualityEnabled();

    void updateTipLableInfo(QLabel *Label,QString info);
    void refreshConfirmBtnStatus();
    void refreshCloseBtnStatus();
public:
    QVBoxLayout * mainVerLayout;
    QHBoxLayout * titleHorLayout;
    QVBoxLayout * contentVerLayout;
    QHBoxLayout * currentPwdHorLayout;
    QHBoxLayout * newPwdHorLayout;
    QHBoxLayout * tipHorLayout;
    QVBoxLayout * surePwdWithTipVerLayout;
    QHBoxLayout * surePwdHorLayout;
    QHBoxLayout * bottomBtnsHorLayout;
    QHBoxLayout *curTipHorLayout;
    QHBoxLayout *newTipHorLayout;
    QHBoxLayout *sureTipHorLayout;
    QHBoxLayout *m_pPwdTipHorLayout;
    QVBoxLayout *m_pPwdTipWidthVerLayout;
    QHBoxLayout *m_pPwdTipMessageLayout;

    QPushButton * closeBtn;
    QPushButton * cancelBtn;
    QPushButton * confirmBtn;

    QPushButton *newPwdEyeBtn;
    QPushButton *surePwdEyeBtn;
    QPushButton *currentPwdEyeBtn;

    QLabel *logoLabel;
    QLabel *titleLabel;
    QLabel * currentPwdLabel;
    QLabel * newPwdLabel;
    QLabel * surePwdLabel;
    QLabel *m_pPwdTipLabel;
    QLabel * tipLabel;
    QLabel *curTipLabel;
    QLabel *newTipLabel;
    QLabel *m_pTipMessageLabel;

    KPasswordEdit * currentPwdLineEdit;
    KPasswordEdit * newPwdLineEdit;
    KPasswordEdit * surePwdLineEdit;
    QLineEdit *m_pPwdTipLineEdit;

protected:
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *);
    bool eventFilter(QObject *target, QEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    QString name;
    QString curPwdTip;
    QString newPwdTip;
    QString surePwdTip;
    QString m_pwdTipMessage;
    QPixmap m_waitingPixmap;
    QTimer *m_timer = nullptr;

    bool isCurrentUser;
    bool isChecking;
    bool enablePwdQuality;

    PwdCheckThread * thread1ForCheckPwd;

#ifdef ENABLEPQ
    pwquality_settings_t *settings;
#endif

    SecurityQuestionAnswer *m_pSecurityQuestionAnswer = nullptr;

private:
    bool setTextDynamicInPwd(QLabel * label, QString string);

    void checkPwdLegality();
    bool isContainLegitimacyChar(QString word);
    int isDomainUser(const char* username);
    void checkPromptLegality();

private Q_SLOTS:
    void test();
    void onCompleted(const QString &result);
    void checkPwdFinishedSlot(QString re);
    void updatePixmap();

};

#endif // CHANGEUSERPWD_H
