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
#ifndef GRUBVERIFY_H
#define GRUBVERIFY_H

#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QEvent>
#include <QKeyEvent>

#include <kysdk/applications/kdialog.h>
#include <kysdk/applications/kpasswordedit.h>
#include <kysdk/applications/klabel.h>
#include <kysdk/applications/kpushbutton.h>
#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

class GrubVerify : public KDialog
{
    Q_OBJECT
public:
    explicit GrubVerify(QWidget *parent = nullptr);
    ~GrubVerify();

    QPushButton *getConfirmBtn();
    QString getPwd();

private:

    QVBoxLayout * mainVerLayout;
    QVBoxLayout * contentVerLayout;
    QHBoxLayout * userHorLayout;
    QHBoxLayout * newPwdHorLayout;
    QHBoxLayout * surePwdHorLayout;
    QHBoxLayout * bottomBtnsHorLayout;
    QHBoxLayout * newTipHorLayout;
    QHBoxLayout * sureTipHorLayout;
    QString newPwdTip;
    QString surePwdTip;

    KPushButton *m_pCancelBtn = nullptr;
    KPushButton *m_pConfirmBtn = nullptr;

    KLabel *m_pUserLabel = nullptr;
    KLabel *m_pUsernameLabel = nullptr;
    KLabel *m_pNewPwdLabel = nullptr;
    KLabel *m_pNewTipLabel = nullptr;
    KLabel *m_pSurePwdLabel = nullptr;
    KLabel *m_pSureTipLabel = nullptr;

    KPasswordEdit *m_pNewPwdLineEdit = nullptr;
    KPasswordEdit *m_pSurePwdLineEdit = nullptr;

private:
    void initUI();
    void setupConnect();
    void checkPwdLegality();
    void refreshConfirmBtnStatus();

protected:
    bool eventFilter(QObject *target, QEvent *event);
    void keyPressEvent(QKeyEvent *);

signals:


};

#endif // GRUBVERIFY_H
