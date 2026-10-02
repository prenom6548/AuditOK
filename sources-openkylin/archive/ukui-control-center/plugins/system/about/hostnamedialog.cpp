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
#include "hostnamedialog.h"
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QProcess>
#include <QRegExp>
#include <QDebug>
#include <QRegExpValidator>
#include <regex>
#include <string>
#include "klabel.h"
#include "klineedit.h"
using namespace kdk;
using namespace std;

HostNameDialog::HostNameDialog(QWidget *parent):
    KDialog(parent)
{
    this->setWindowFlags(Qt::Dialog);
    setWindowIcon("ukui-control-center");
    setWindowTitle(tr("Set HostName"));
    InitUi();
    setEdit();
    setupComponent();
    initConnect();
}

HostNameDialog::~HostNameDialog()
{

}

void HostNameDialog::InitUi()
{
    setFixedSize(424, 188);
    QVBoxLayout *mInputPwdLyt = new QVBoxLayout(this);
    mInputPwdLyt->setContentsMargins(24, 12, 24, 24);

    QFrame *mInputPwdFrame = new QFrame(this);
    mInputPwdFrame->setFrameShape(QFrame::NoFrame);

    KLabel *hostNameLabel = new KLabel(mInputPwdFrame);
    hostNameLabel->setText(tr("HostName"));

    mHostNameEdit = new KLineEdit(mInputPwdFrame);
    mHostNameEdit->setFixedSize(240, 36);
    mHostNameEdit->setClearButtonEnabled(true);
    mHostNameEdit->installEventFilter(this);
    mHostNameEdit->pushbutton()->hide();

    QHBoxLayout *mLyt_1= new QHBoxLayout(mInputPwdFrame);
    mLyt_1->setContentsMargins(0, 0, 0, 0);
    mLyt_1->addWidget(hostNameLabel);
    mLyt_1->addStretch();
    mLyt_1->addWidget(mHostNameEdit);

    m_tipLabel = new KLabel(this);
    QFont ft;
    ft.setPixelSize(12);
    m_tipLabel->setFont(ft);
    m_tipLabel->setFixedSize(240, 24);
    m_tipLabel->setContentsMargins(0, 0, 0, 0);
    m_tipLabel->setFontColor(Qt::red);
    m_tipLabel->setVisible(false);

    QFrame *mInputPwdFrame_1 = new QFrame(this);
    mInputPwdFrame_1->setFrameShape(QFrame::NoFrame);

    mConfirmBtn = new QPushButton(mInputPwdFrame_1);
    mConfirmBtn->setMinimumWidth(88);
    mConfirmBtn->setText(tr("Confirm"));
    mConfirmBtn->setProperty("isImportant", true);

    mCancelBtn = new QPushButton(mInputPwdFrame_1);
    mCancelBtn->setMinimumWidth(88);
    mCancelBtn->setText(tr("Cancel"));
    mCancelBtn->setProperty("useButtonPalette", true);


    QHBoxLayout *mLyt_2= new QHBoxLayout(mInputPwdFrame_1);
    mLyt_2->setContentsMargins(0, 0, 0, 0);
    mLyt_2->addStretch();
    mLyt_2->addWidget(mCancelBtn);
    mLyt_2->addWidget(mConfirmBtn);

    mInputPwdLyt->addWidget(mInputPwdFrame);
    mInputPwdLyt->addWidget(m_tipLabel, 0, Qt::AlignRight);
    mInputPwdLyt->addStretch();
    mInputPwdLyt->addWidget(mInputPwdFrame_1);

    this->mainWidget()->setLayout(mInputPwdLyt);

    hostNameLabel->setAccessibleName(tr("HostName"));
    hostNameLabel->setBuddy(mHostNameEdit);
    mHostNameEdit->setAccessibleName(tr("HostName"));
    mCancelBtn->setAccessibleName(tr("Cancel"));
    mConfirmBtn->setAccessibleName(tr("Confirm"));
}

void HostNameDialog::hostNameEditSlot(QString pwd) {
    if (pwd.isEmpty()) {
        m_tipLabel->setText(tr("Must be 1-64 characters long"));
        m_tipLabel->setVisible(true);
    } else if (pwd.length() > 64) {
        m_tipLabel->setText("");
        mHostNameEdit->setText(pwd.mid(0, 64));
        m_tipLabel->setVisible(true);
    } else {
        m_tipLabel->setVisible(!isMatch(pwd));
    }

    if (!m_tipLabel->text().isEmpty() || mHostNameEdit->text().isEmpty()) {
        mConfirmBtn->setEnabled(false);
    } else {
        mConfirmBtn->setEnabled(true);
    }
}

void HostNameDialog::initConnect()
{
    connect(mHostNameEdit, &QLineEdit::textEdited, this, [=](QString pwd) {
        hostNameEditSlot(pwd);
    });

    connect(mCancelBtn, &QPushButton::clicked, this, [=]() {
        this->close();
    });

    connect(mConfirmBtn, &QPushButton::clicked, this, [=]() {
        if (mfirsthostname != mHostNameEdit->text()) {
            setHostname(mHostNameEdit->text());
        }
        this->close();
    });
}

void HostNameDialog::setEdit()
{
    QRegExp rx("[a-zA-Z0-9.-]*");
    QRegExpValidator *validator = new QRegExpValidator(rx , this);
    mHostNameEdit->setValidator(validator);
}

void HostNameDialog::setupComponent()
{
    mfirsthostname = UkccCommon::getHostName();
    mHostNameEdit->setText(mfirsthostname);
}

void HostNameDialog::setHostname(QString hostname)
{
    //修改/etc/hosts文件中的主机名
    QDBusInterface *sethostnameDbus = new QDBusInterface("com.control.center.qt.systemdbus",
                                                             "/",
                                                             "com.control.center.interface",
                                                             QDBusConnection::systemBus());

    QDBusReply<bool> reply = sethostnameDbus->call("sethostname", hostname);
    qDebug() << Q_FUNC_INFO << reply.isValid() << hostname;
    if (reply.isValid() && reply.value()) {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("LANG","en_US");
        QProcess *process = new QProcess;
        process->setProcessEnvironment(env);
        process->start(QString("%1%2").arg("hostnamectl set-hostname ").arg(hostname));
        process->waitForFinished();
        delete process;
    }
    delete sethostnameDbus;
    sethostnameDbus = nullptr;
}

bool HostNameDialog::isMatch(QString hostname)
{
    bool ismatch = true;
    string str = hostname.toStdString();
    string::const_iterator iter_begin = str.cbegin();
    string::const_iterator iter_end = str.cend();

    if (regex_search(iter_begin, iter_end, regex("^[-.]")) || regex_search(iter_begin, iter_end, regex("[-.]$"))) {
        m_tipLabel->setText(tr("Hostname must start or end with a number and a letter"));
        ismatch = false;
    }
    if (regex_search(iter_begin, iter_end, regex("[-][.]")) || regex_search(iter_begin, iter_end, regex("[.][-]"))) {
        int position = mHostNameEdit->cursorPosition();
        if (regex_search(iter_begin, iter_end, regex("[-][.]"))) {
           mHostNameEdit->setText(regex_replace(str, regex("[-][.]"), "-").c_str());
        } else {
            mHostNameEdit->setText(regex_replace(str, regex("[.][-]"), ".").c_str());
        }
        mHostNameEdit->setCursorPosition(position - 1);
        m_tipLabel->setText(tr("Hostname cannot have consecutive ' - ' and ' . '"));
        ismatch = false;
    }
    if (regex_search(iter_begin, iter_end, regex("[.][.]"))) {
        int position = mHostNameEdit->cursorPosition();
        mHostNameEdit->setText(regex_replace(str, regex("[.][.]"), ".").c_str());
        mHostNameEdit->setCursorPosition(position - 1);
        m_tipLabel->setText(tr("Hostname cannot have consecutive ' . '"));
        ismatch = false;
    }
    if (ismatch) {
        m_tipLabel->setText("");
    }
    return ismatch;
}
