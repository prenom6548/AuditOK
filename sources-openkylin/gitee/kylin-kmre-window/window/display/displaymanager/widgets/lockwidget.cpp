/*
 * Copyright (C) 2013 ~ 2020 KylinSoft Ltd.
 *
 * Authors:
 *  YangChenBoHuang    yangchenbohuang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "lockwidget.h"
#include "kmreenv.h"
#include "preferences.h"
#include "windowmanager.h"

#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QRegExpValidator>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QThread>
#include <QStandardPaths>
#include <QDir>

LockWidget::LockWidget(QWidget *parent)
    :QWidget(parent)
{
    this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    this->setWindowModality(Qt::ApplicationModal);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setAttribute(Qt::WA_DeleteOnClose);

    QFrame *centralWidget = new QFrame(this);
    m_lockFrame = new QFrame;
    QFrame *m_setcodeFrame = new QFrame;
    centralWidget->setFixedSize(300,400);
    code = KmreConfig::Preferences::getInstance()->m_code;
    m_stackedLayout = new QStackedLayout(centralWidget);
    m_stackedLayout->setSpacing(0);
    m_stackedLayout->setMargin(0);
    centralWidget->setStyleSheet("QFrame{background:palette(Base);border-radius:6px;}");
    m_title = new QLabel;
    m_title->setAttribute(Qt::WA_TranslucentBackground);
    m_title->setText(tr("Please enter your password"));
    m_line = new QLineEdit;
    QRegExp rx("([0-9]*$)");
    QValidator *validator = new QRegExpValidator(rx);
    m_line->setValidator(validator);
    m_line->setEchoMode(QLineEdit::Password);
    m_Btn0 = new QPushButton;
    m_Btn1 = new QPushButton;
    m_Btn2 = new QPushButton;
    m_Btn3 = new QPushButton;
    m_Btn4 = new QPushButton;
    m_Btn5 = new QPushButton;
    m_Btn6 = new QPushButton;
    m_Btn7 = new QPushButton;
    m_Btn8 = new QPushButton;
    m_Btn9 = new QPushButton;
    m_resetBtn = new QPushButton;
    m_back = new QPushButton;
    m_Btn0->setText("0");
    m_Btn1->setText("1");
    m_Btn2->setText("2");
    m_Btn3->setText("3");
    m_Btn4->setText("4");
    m_Btn5->setText("5");
    m_Btn6->setText("6");
    m_Btn7->setText("7");
    m_Btn8->setText("8");
    m_Btn9->setText("9");
    m_resetBtn->setText(tr("forget"));
    m_back->setIcon(QIcon::fromTheme("go-previous-symbolic"));
    m_Btn0->setFixedSize(50,50);
    m_Btn1->setFixedSize(50,50);
    m_Btn2->setFixedSize(50,50);
    m_Btn3->setFixedSize(50,50);
    m_Btn4->setFixedSize(50,50);
    m_Btn5->setFixedSize(50,50);
    m_Btn6->setFixedSize(50,50);
    m_Btn7->setFixedSize(50,50);
    m_Btn8->setFixedSize(50,50);
    m_Btn9->setFixedSize(50,50);
    m_resetBtn->setFixedSize(100,50);
    m_back->setFixedSize(50,50);
    m_Btn0->setStyleSheet("QPushButton{background:#E6E6E6;border-radius:25px;} QPushButton:hover{background:#DADADA;} QPushButton:pressed{background:#B3B3B3;}");
    m_Btn1->setStyleSheet("QPushButton{background:#E6E6E6;border-radius:25px;} QPushButton:hover{background:#DADADA;} QPushButton:pressed{background:#B3B3B3;}");
    m_Btn2->setStyleSheet("QPushButton{background:#E6E6E6;border-radius:25px;} QPushButton:hover{background:#DADADA;} QPushButton:pressed{background:#B3B3B3;}");
    m_Btn3->setStyleSheet("QPushButton{background:#E6E6E6;border-radius:25px;} QPushButton:hover{background:#DADADA;} QPushButton:pressed{background:#B3B3B3;}");
    m_Btn4->setStyleSheet("QPushButton{background:#E6E6E6;border-radius:25px;} QPushButton:hover{background:#DADADA;} QPushButton:pressed{background:#B3B3B3;}");
    m_Btn5->setStyleSheet("QPushButton{background:#E6E6E6;border-radius:25px;} QPushButton:hover{background:#DADADA;} QPushButton:pressed{background:#B3B3B3;}");
    m_Btn6->setStyleSheet("QPushButton{background:#E6E6E6;border-radius:25px;} QPushButton:hover{background:#DADADA;} QPushButton:pressed{background:#B3B3B3;}");
    m_Btn7->setStyleSheet("QPushButton{background:#E6E6E6;border-radius:25px;} QPushButton:hover{background:#DADADA;} QPushButton:pressed{background:#B3B3B3;}");
    m_Btn8->setStyleSheet("QPushButton{background:#E6E6E6;border-radius:25px;} QPushButton:hover{background:#DADADA;} QPushButton:pressed{background:#B3B3B3;}");
    m_Btn9->setStyleSheet("QPushButton{background:#E6E6E6;border-radius:25px;} QPushButton:hover{background:#DADADA;} QPushButton:pressed{background:#B3B3B3;}");
    m_back->setStyleSheet("QPushButton{background:#E6E6E6;border-radius:25px;} QPushButton:hover{background:#DADADA;} QPushButton:pressed{background:#B3B3B3;}");
    m_resetBtn->setStyleSheet("QPushButton{background:palette(Base);text-align:left;}");
    QHBoxLayout *hlayout1 = new QHBoxLayout;
    hlayout1->addStretch();
    hlayout1->setSpacing(16);
    hlayout1->addWidget(m_Btn1);
    hlayout1->addWidget(m_Btn2);
    hlayout1->addWidget(m_Btn3);
    hlayout1->addStretch();
    QHBoxLayout *hlayout2 = new QHBoxLayout;
    hlayout2->addStretch();
    hlayout2->setSpacing(16);
    hlayout2->addWidget(m_Btn4);
    hlayout2->addWidget(m_Btn5);
    hlayout2->addWidget(m_Btn6);
    hlayout2->addStretch();
    QHBoxLayout *hlayout3 = new QHBoxLayout;
    hlayout3->addStretch();
    hlayout3->setSpacing(16);
    hlayout3->addWidget(m_Btn7);
    hlayout3->addWidget(m_Btn8);
    hlayout3->addWidget(m_Btn9);
    hlayout3->addStretch();
    QHBoxLayout *hlayout4 = new QHBoxLayout;
    hlayout4->addStretch();
    hlayout4->setSpacing(16);
    hlayout4->addWidget(m_Btn0);
    hlayout4->addStretch();
    QHBoxLayout *hlayout5 = new QHBoxLayout;
    hlayout5->addStretch();
    hlayout5->addWidget(m_resetBtn);
    hlayout5->addSpacing(32);
    hlayout5->addWidget(m_back);
    hlayout5->addStretch();
    QPushButton *m_close = new QPushButton;
//    m_close->setText(tr("close"));
    m_close->setFixedSize(30,30);
    m_close->adjustSize();
    m_close->setProperty("isWindowButton", 0x02);
    m_close->setProperty("useIconHighlightEffect", 0x08);
    m_close->setVisible(false);
    m_close->setIcon(QIcon::fromTheme("window-close-symbolic"));
    if (KmreConfig::Preferences::getInstance()->m_code == "") {
        m_close->setVisible(true);
    }
    QVBoxLayout *m_layout = new QVBoxLayout(m_lockFrame);
    m_layout->addWidget(m_close, 0, Qt::AlignRight);
    m_layout->addWidget(m_title, 0, Qt::AlignHCenter);
    m_layout->addWidget(m_line, 0, Qt::AlignHCenter);
    m_layout->addLayout(hlayout1);
    m_layout->addLayout(hlayout2);
    m_layout->addLayout(hlayout3);
    m_layout->addLayout(hlayout4);
    m_layout->addLayout(hlayout5);
    m_layout->addStretch();

    m_tiplabel = new QLabel;
    m_reset = new QLabel;
    m_kylinid = new QLabel;
    m_kylincode = new QLabel;
    m_newcode = new QLabel;
    m_kylinid->setFixedWidth(100);
    m_kylincode->setFixedWidth(100);
    m_newcode->setFixedWidth(100);
    m_tiplabel->setText(tr("Account/Password is not correct"));
    m_tiplabel->setStyleSheet("QLabel{color:red;}");
    m_tiplabel->setVisible(false);
    m_reset->setText(tr("reset code"));
    m_kylinid->setText(tr("kylin id"));
    m_kylincode->setText(tr("kylin code"));
    m_newcode->setText(tr("new code"));
    m_idline = new QLineEdit;
    m_codeline = new QLineEdit;
    m_newcodeline = new QLineEdit;
    m_idline->setFixedWidth(150);
    m_codeline->setFixedWidth(150);
    m_newcodeline->setFixedWidth(150);
    m_codeline->setEchoMode(QLineEdit::Password);
    m_newcodeline->setEchoMode(QLineEdit::Password);
    m_newcodeline->setValidator(validator);
    QHBoxLayout *m_hlayout1 = new QHBoxLayout;
    m_hlayout1->addWidget(m_kylinid);
    m_hlayout1->addSpacing(10);
    m_hlayout1->addWidget(m_idline);
    m_hlayout1->addStretch();
    QHBoxLayout *m_hlayout2 = new QHBoxLayout;
    m_hlayout2->addWidget(m_kylincode);
    m_hlayout2->addSpacing(10);
    m_hlayout2->addWidget(m_codeline);
    m_hlayout2->addStretch();
    QHBoxLayout *m_hlayout3 = new QHBoxLayout;
    m_hlayout3->addWidget(m_newcode);
    m_hlayout3->addSpacing(10);
    m_hlayout3->addWidget(m_newcodeline);
    m_hlayout3->addStretch();
    m_backBtn = new QPushButton;
    m_backBtn->setText(tr("back"));
    m_backBtn->setProperty("isWindowButton", 0x01);
    m_okBtn = new QPushButton;
    m_okBtn->setText(tr("confirm"));
    m_okBtn->setProperty("isWindowButton", 0x01);
    m_resetBtn->setProperty("isWindowButton", 0x01);
    m_resetBtn->setFocusPolicy(Qt::NoFocus);
    QHBoxLayout *m_hlayout4 = new QHBoxLayout;
    m_hlayout4->addWidget(m_backBtn, 0, Qt::AlignHCenter);
    m_hlayout4->addWidget(m_okBtn, 0, Qt::AlignHCenter);
    QVBoxLayout *m_vlayout = new QVBoxLayout(m_setcodeFrame);
    m_vlayout->setSpacing(16);
    m_vlayout->addWidget(m_reset, 0, Qt::AlignHCenter);
    m_vlayout->addSpacing(20);
    m_vlayout->addLayout(m_hlayout1);
    m_vlayout->addLayout(m_hlayout2);
    m_vlayout->addLayout(m_hlayout3);
    m_vlayout->addSpacing(20);
    m_vlayout->addWidget(m_tiplabel);
    m_vlayout->addLayout(m_hlayout4);
//    m_vlayout->addWidget(m_backBtn, 0, Qt::AlignLeft);
    m_vlayout->addStretch();

    m_stackedLayout->addWidget(m_lockFrame);
    m_stackedLayout->addWidget(m_setcodeFrame);
    m_stackedLayout->setCurrentWidget(m_lockFrame);

    connect(m_close, &QPushButton::clicked, this, &LockWidget::closelock);
    connect(m_line, &QLineEdit::textChanged, this, [=]() {
        if (m_line->text() == code) {
            closelock();
        }
    });
    connect(m_Btn0, &QPushButton::clicked, this, [=]() {m_line->insert("0");});
    connect(m_Btn1, &QPushButton::clicked, this, [=]() {m_line->insert("1");});
    connect(m_Btn2, &QPushButton::clicked, this, [=]() {m_line->insert("2");});
    connect(m_Btn3, &QPushButton::clicked, this, [=]() {m_line->insert("3");});
    connect(m_Btn4, &QPushButton::clicked, this, [=]() {m_line->insert("4");});
    connect(m_Btn5, &QPushButton::clicked, this, [=]() {m_line->insert("5");});
    connect(m_Btn6, &QPushButton::clicked, this, [=]() {m_line->insert("6");});
    connect(m_Btn7, &QPushButton::clicked, this, [=]() {m_line->insert("7");});
    connect(m_Btn8, &QPushButton::clicked, this, [=]() {m_line->insert("8");});
    connect(m_Btn9, &QPushButton::clicked, this, [=]() {m_line->insert("9");});
    connect(m_back, &QPushButton::clicked, this, [=]() {m_line->backspace();});
    connect(m_resetBtn, &QPushButton::clicked, this, [=]() {m_stackedLayout->setCurrentWidget(m_setcodeFrame);});
    connect(m_backBtn, &QPushButton::clicked, this, [=]() {m_stackedLayout->setCurrentWidget(m_lockFrame);});
    connect(m_okBtn, &QPushButton::clicked, this, &LockWidget::checkkylinid);
}

void LockWidget::checkkylinid()
{
    QNetworkAccessManager *accessManager = new QNetworkAccessManager(this);
    QNetworkRequest request;
    request.setUrl(QUrl("https://id.kylinos.cn/v1/api/userLogin"));
    accessManager->get(request);
    QByteArray postData;
//    postData.append("kylinID=pengdaixin&pwd=Dp448283676..&form=0&validateCode=8888&imgId=kylincloud");
    postData.append(QString("kylinID=%1&pwd=%2&form=0&validateCode=8888&imgId=kylincloud").arg(m_idline->text()).arg(m_codeline->text()));
    QNetworkReply *reply = accessManager->post(request,postData);
    connect(accessManager, &QNetworkAccessManager::finished, this, &LockWidget::finishedSlot);
}

void LockWidget::finishedSlot(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QString content = reply->readAll();
        QJsonParseError jsonError;
        const QJsonDocument jsonDocument = QJsonDocument::fromJson(content.toUtf8(), &jsonError);
        if (!jsonDocument.isNull() && (jsonError.error == QJsonParseError::NoError)) {
            if (jsonDocument.isObject()) {
                QJsonObject obj = jsonDocument.object();
                QString message = obj.value("message").toString();
                if (message == "success") {
                    code = m_newcodeline->text();
                    KmreConfig::Preferences::getInstance()->m_code = m_newcodeline->text();
                    KmreConfig::Preferences::getInstance()->updatelockcode();
                    m_tiplabel->setVisible(false);
                    m_stackedLayout->setCurrentWidget(m_lockFrame);
                    m_idline->clear();
                    m_codeline->clear();
                    m_newcodeline->clear();
                }
                else {
                    QThread::usleep(500*1000);
                    m_tiplabel->setVisible(true);
                }
            }
        }
    }
}

void LockWidget::closelock()
{
    KmreWindowManager::getInstance()->lockScreen(false);
}

SimpleLockWidget::SimpleLockWidget(QWidget *parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_DeleteOnClose);
}