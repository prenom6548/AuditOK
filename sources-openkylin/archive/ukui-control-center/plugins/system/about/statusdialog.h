/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#ifndef STATUSDIALOG_H
#define STATUSDIALOG_H

#include <QObject>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <klabel.h>
#include <QDBusInterface>
#include <kdialog.h>

using namespace kdk;

const QString vTen        = "v10";
const QString vTenEnhance = "v10.1";
const QString vFour = "v4";
const QString v11 = "v11";

#define THEME_STYLE_SCHEMA "org.ukui.style"
#define STYLE_NAME_KEY "style-name"
#define CONTAIN_STYLE_NAME_KEY "styleName"
#define UKUI_DEFAULT "ukui-default"
#define UKUI_DARK "ukui-dark"

class StatusDialog : public KDialog
{
    Q_OBJECT
public:
    StatusDialog(QWidget *parent = nullptr);
    ~StatusDialog();

    void getActivationCode();
    void getRegisterCode();

private:
    void initUI();

private slots:
    void activeSlot(int activeSignal);

public:
    QLabel *mLogoLabel;
    QLabel *mVersionLabel_1;
    KLabel *mVersionLabel_2;
    KLabel *mStatusLabel_1;
    QLabel *mStatusLabel_2;
    QLabel *mSerialLabel_1;
    KLabel *mSerialLabel_2;
    KLabel *mTimeLabel_1;
    KLabel *mTimeLabel_2;
    KLabel *mActivationCodeLabel_1;
    KLabel *mActivationCodeLabel_2;
    QPushButton *mExtentBtn;
    KLabel* m_registerCodeLabel_1 = nullptr;
    KLabel* m_registerCodeLabel_2 = nullptr;

private:
    QFrame *mVersionFrame;
    QFrame *mStatusFrame;
    QFrame *mSerialFrame;
    QFrame *mTimeFrame;
    QFrame *mActivationFrame;
    QFrame *m_registerFrame = nullptr;
    QDBusInterface* m_activeInterface = nullptr;
};

#endif // STATUSDIALOG_H
