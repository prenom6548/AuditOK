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
#include "privacydialog.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

PrivacyDialog::PrivacyDialog(QWidget *parent) :
    QDialog(parent)
{
    this->setWindowFlags(Qt::Dialog);
    setWindowTitle(tr("Set"));
    initUi();
}
PrivacyDialog::~PrivacyDialog()
{

}

void PrivacyDialog::initUi()
{
    this->resize(560, 560);
    QVBoxLayout *mverticalLayout = new QVBoxLayout(this);
    mverticalLayout->setSpacing(8);
    mverticalLayout->setContentsMargins(24, 0, 24, 24);

    QLabel *mTitleLabel = new QLabel(getProtocolTitle(), this);
    mTitleLabel->setAlignment(Qt::AlignCenter);
    mverticalLayout->addWidget(mTitleLabel);
    QTextBrowser *mContentLabel = new QTextBrowser(this);
    // 协议内容
    QString mContent = getEnduserAgreement();
    mContentLabel->setText(mContent);
    mverticalLayout->addWidget(mContentLabel);
    QLabel *label_1 = new QLabel(getCompanyName() , this);
    label_1->setAlignment(Qt::AlignRight);
    mverticalLayout->addWidget(label_1);
}

QString PrivacyDialog::getEnduserAgreement()
{
    QString agreementPath = endUserAgreementSP1;
    if (UkccCommon::isOpenkylin()) {
        agreementPath = endUserAgreementOK;
    }
    const QString protocol = getLicenseText(agreementPath);
    return protocol;
}

QString PrivacyDialog::getLicenseText(const QString &filePath)
{
    QFile license(getLicensePath(filePath));
    if (!license.open(QIODevice::ReadOnly)) return QString();
    const QByteArray buf = license.readAll();
    license.close();
    return buf;
}

QString PrivacyDialog::getLicensePath(const QString &filePath)
{
    const QString &local {QLocale::system().name()};
    QString path = QString(filePath).arg(local);

    if (QFile::exists(path)) {
        return path;
    } else {
        return QString(filePath).arg("en_US");;
    }
}

QString PrivacyDialog::getProtocolTitle()
{
    if (UkccCommon::isOpenkylin()) {
        return QString(tr("End User License Agreement and Privacy Policy Statement of openKylin"));
    }
    return QString(tr("End User License Agreement and Privacy Policy Statement of Kylin"));
}

QString PrivacyDialog::getCompanyName()
{
    if (UkccCommon::isOpenkylin()) {
        return QString(tr("openKylin Community."));
    }
    return QString(tr("Kylinsoft Co., Ltd."));
}
