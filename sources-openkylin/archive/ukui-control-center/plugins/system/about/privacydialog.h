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
#ifndef PRIVACYDIALOG_H
#define PRIVACYDIALOG_H

#include <QFile>
#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QString>
#include <QTextBrowser>

#include "ukcccommon.h"

using namespace ukcc;

const static QString endUserAgreementSP1 = "/usr/share/protocol/Ukcc-Agreement-%1.txt";
const static QString endUserAgreementOK = "/usr/share/protocol/Ukcc-Agreement-OK-%1.txt";

class PrivacyDialog  : public QDialog
{
    Q_OBJECT
public:
    PrivacyDialog(QWidget *parent);
    ~PrivacyDialog();
private:
    void initUi();

    QString getEnduserAgreement();
    QString getLicenseText(const QString &filePath);
    QString getLicensePath(const QString &filePath);
    QString getProtocolTitle();
    QString getCompanyName();

};

#endif // PRIVACYDIALOG_H
