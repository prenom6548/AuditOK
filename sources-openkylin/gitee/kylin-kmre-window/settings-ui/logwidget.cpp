/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
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

#include "logwidget.h"
#include "utils.h"
#include "processresult.h"
#include "global.h"

#include <QDebug>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QDateTime>
#include <QFile>
#include <QProcess>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDBusInterface>
#include <QtDBus>
#include <QDBusPendingCallWatcher>
#include <QDesktopServices>
#include <QMovie>

using namespace Global;

LogWidget::LogWidget(const QString &userName, const QString &userId, QWidget *parent) :
    SettingScrollContent(parent)
    , m_loginUserName(userName)
    , m_loginUserId(userId)
{
//    setTitleTip(tr("Log collection"));
    QFrame *centralWidget = new QFrame;

    m_tipsLabel = new QLabel();
    m_tipsLabel->setAlignment(Qt::AlignCenter);
    m_tipsLabel->setVisible(false);
    m_tipsLabel->setFixedWidth(120);

    m_loading = new QLabel();
    QMovie *loading = new QMovie(":/res/loading.gif");
    QSize s(20,20);
    loading->setScaledSize(s);
    loading->setBackgroundColor("#3790FA");

    m_loading->setMovie(loading);
    //m_loading->setScaledContents(true);
    loading->start();
    m_loading->setFixedSize(120,36);
    m_loading->setFocusPolicy(Qt::NoFocus);
    m_loading->setStyleSheet("background-color:#3790FA;border-radius: 6px;");
    m_loading->setAlignment(Qt::AlignCenter);
    m_loading->setVisible(false);

    m_startButton = new QPushButton;
    m_startButton->setFixedSize(120, 36);
    m_startButton->setFocusPolicy(Qt::NoFocus);
    m_startButton->setText(tr("Start"));
    m_startButton->setStyleSheet("background-color:#3790FA;color: #FFFFFF");
    if (!isKylinOSPro() && !isOpenKylin()) {
        m_startButton->setStyleSheet("");
    }
    connect(m_startButton, SIGNAL(clicked()), this, SLOT(onStart()));

    m_backButton = new QPushButton;
    m_backButton->setVisible(false);
    m_backButton->setFixedSize(120, 36);
    m_backButton->setFocusPolicy(Qt::NoFocus);
    m_backButton->setText(tr("Back"));
    connect(m_backButton, &QPushButton::clicked, this, [=] () {
        m_tipsLabel->clear();
        m_tipsLabel->setVisible(false);
        m_backButton->setVisible(false);
        m_startButton->setVisible(true);
        m_title->setVisible(true);
        finish_pic->setVisible(false);
        log_pic->setVisible(true);
    });

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(m_startButton, 0, Qt::AlignVCenter);
    hlayout->addWidget(m_backButton, 0, Qt::AlignVCenter);
    hlayout->addWidget(m_loading, 0, Qt::AlignVCenter);
    hlayout->addStretch();

    QVBoxLayout *layout = new QVBoxLayout;
    m_title = new QLabel();
    log_pic = new QLabel;
    finish_pic = new QLabel;
    log_pic->resize(78,94);
    finish_pic->resize(64,64);
    finish_pic->setVisible(false);
    QImage *img1=new QImage;
    QImage *img2=new QImage;
    img1->load(":/res/log.png");
    img1->scaled(log_pic->size(),Qt::KeepAspectRatio);
    img2->load(":/res/finish.png");
    img2->scaled(finish_pic->size(),Qt::KeepAspectRatio);
    log_pic->setScaledContents(true);
    log_pic->setPixmap(QPixmap::fromImage(*img1));
    log_pic->setAlignment(Qt::AlignVCenter);
    log_pic->setProperty("useIconHighlightEffect",0x2);
    finish_pic->setScaledContents(true);
    finish_pic->setPixmap(QPixmap::fromImage(*img2));
    finish_pic->setAlignment(Qt::AlignVCenter);
    m_title->setText(tr("Log collection"));
    m_title->setAlignment(Qt::AlignCenter);

    QHBoxLayout *hpic = new QHBoxLayout;
    hpic->addStretch();
    hpic->addWidget(finish_pic, 0,Qt::AlignVCenter);
    hpic->addWidget(log_pic, 0,Qt::AlignVCenter);
    hpic->addStretch();

    layout->addStretch();
    layout->setContentsMargins(225,122,230,231);
    layout->addLayout(hpic);
    layout->setSpacing(21);
    layout->addWidget(m_title, 0, Qt::AlignHCenter);
    layout->addWidget(m_tipsLabel, 0, Qt::AlignHCenter);
    //layout->setMargin(10);
    layout->setSpacing(16);
    layout->addStretch();
    layout->addLayout(hlayout);
    layout->addStretch();

    centralWidget->setLayout(layout);
    this->setContent(centralWidget);
}

LogWidget::~LogWidget()
{

}

void LogWidget::onStart()
{
    m_logPath.clear();
    m_startButton->setVisible(false);
    m_title->setVisible(false);
    m_tipsLabel->setVisible(true);
    m_tipsLabel->setText(tr("Getting logs..."));
    m_loading->setVisible(true);

    this->startGetDisplayInfo();
}

void LogWidget::startGetDisplayInfo()
{
    m_displayInfo.clear();

    QDBusInterface interface("cn.kylinos.Kmre.Manager", "/cn/kylinos/Kmre/Manager", "cn.kylinos.Kmre.Manager", QDBusConnection::sessionBus());
    QDBusPendingReply<QString> reply = interface.asyncCall("getDisplayInformation");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] (QDBusPendingCallWatcher *w) {
        if (!w->isError()) {
            QDBusPendingReply<QString> reply = *w;// 无返回值时<>中无需填写类型,如: QDBusPendingReply<> r = *w;
            if (reply.isValid()) {
                m_displayInfo = reply.value();
            }
        }
        w->deleteLater();

        this->startGetServiceStatus();
        this->startCopyLogFils();
    });
}

void LogWidget::startGetServiceStatus()
{

}

void LogWidget::startCopyLogFils()
{
    QDBusInterface interface("cn.kylinos.Kmre.Pref", "/cn/kylinos/Kmre/Pref", "cn.kylinos.Kmre.Pref", QDBusConnection::systemBus());
    QDBusPendingReply<int> reply = interface.asyncCall("copyLogFiles", m_loginUserName, m_loginUserId, m_logPath);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] (QDBusPendingCallWatcher *w) {
        if (w->isError()) {
            w->deleteLater();
            m_tipsLabel->setText(tr("An error occurred while getting log!"));
            m_backButton->setVisible(true);
            return;
        }

        QDBusPendingReply<int> reply = *w;// 无返回值时<>中无需填写类型,如: QDBusPendingReply<> r = *w;
        if (reply.isValid()) {
            int ret = reply.value();
            Q_UNUSED(ret);
        }
        w->deleteLater();

        m_tipsLabel->setText(tr("Get log complete."));
        m_backButton->setVisible(true);
        m_loading->setVisible(false);
        log_pic->setVisible(false);
        finish_pic->setVisible(true);

        const QString path = QString("%1/KmreLog/").arg(QDir::homePath());
        bool openEnabled = QFileInfo(path).isDir();
        if (openEnabled) {
            QDesktopServices::openUrl(QUrl(QString("file:%1").arg(path), QUrl::TolerantMode));
        }
    });
}
