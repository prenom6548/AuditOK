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
#include "blockwidget.h"
#include "klabel.h"
#include "statusbutton.h"
#include "../lock-dialog/lockdialogmodel.h"
#include "powerlistwidget.h"
#include <QLabel>
#include <QListView>
#include <QStandardItemModel>
#include <QDebug>
#include <QDateTime>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>

#define SHUTDOWN_MSG_TIME 60

BlockWidget::BlockWidget(LockDialogModel *model, QWidget *parent) : QWidget(parent), m_modelLockDialog(model)
{
    initUi();
    setFocusPolicy(Qt::NoFocus);
}

BlockWidget::~BlockWidget() {}

void BlockWidget::initUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    this->setLayout(mainLayout);

    m_tipLabel = new KLabel(this);
    m_tipLabel->setAlignment(Qt::AlignCenter);
    m_tipLabel->setWordWrap(true);
    m_msgTipLabel = new KLabel(this);
    m_msgTipLabel->setAlignment(Qt::AlignCenter);
    QHBoxLayout *listLayout = new QHBoxLayout();
    listLayout->setAlignment(Qt::AlignCenter);
    m_listView = new QListView(this);
    m_listView->setObjectName(QString::fromUtf8("applist"));
    listLayout->addWidget(m_listView);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_cancelButton = new QPushButton(this);
    m_cancelButton->setText(tr("Cancel"));
    m_cancelButton->setFocusPolicy(Qt::NoFocus);
    buttonLayout->addWidget(m_cancelButton);
    m_confirmButton = new QPushButton(this);
    m_confirmButton->setText(tr("Confrim"));
    m_confirmButton->setFocusPolicy(Qt::NoFocus);
    buttonLayout->addWidget(m_confirmButton);
    buttonLayout->setAlignment(Qt::AlignCenter);

    mainLayout->setSpacing(46);
    mainLayout->addStretch();
    mainLayout->addWidget(m_tipLabel);
    mainLayout->addWidget(m_msgTipLabel);
    mainLayout->addLayout(listLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();

    m_curFontSize = m_modelLockDialog->getCurFontSize();
    m_ptToPx = m_modelLockDialog->getPtToPx();
    connect(m_modelLockDialog, &LockDialogModel::fontSizeChanged, this, &BlockWidget::onFontSizeChanged);
    m_tipLabel->setFontSize((18 + m_curFontSize) * m_ptToPx);

    connect(m_cancelButton, &QPushButton::clicked, this, [&]() { 
        if(shutdownTimer){
            shutdownTimer->stop();
        }
        emit cancelButtonclicked(); 
    });
    connect(m_confirmButton, &QPushButton::clicked, this, [&]() { emit confirmButtonclicked(); });
}

void BlockWidget::hideEvent(QHideEvent *event)
{
    if(shutdownTimer){
        shutdownTimer->stop();
    }
}

void BlockWidget::onFontSizeChanged(double fontSize)
{
    m_curFontSize = fontSize;
    m_tipLabel->setFontSize((18 + m_curFontSize) * m_ptToPx);

    sysFont.setPointSize((16 + m_curFontSize) * m_ptToPx);
    m_confirmButton->setFont(sysFont);
    m_cancelButton->setFont(sysFont);
}

void BlockWidget::updateFontFamily(QString fontFamily)
{
    if (m_tipLabel) {
        m_tipLabel->setFontFamily(fontFamily);
    }
    sysFont.setFamily(fontFamily);
    if (m_cancelButton) {
        m_cancelButton->setFont(sysFont);
    }
    if (m_confirmButton) {
        m_confirmButton->setFont(sysFont);
    }
}

void BlockWidget::setTips(const QString tips)
{
    m_cancelButton->setFixedSize(96, 36);
    m_confirmButton->setFixedSize(96, 36);
    m_tipLabel->setFixedWidth(this->width());
    m_cancelButton->setStyleSheet("QPushButton{background: rgba(255, 255, 255, 0.2);border-radius: 8px;color: white;}"
                                  "QPushButton:hover{background: rgba(255, 255, 255, 0.4);border-radius: 8px;}"
                                  "QPushButton:pressed {background: rgba(255, 255, 255, 0.3);border-radius: 8px;}");
    m_confirmButton->setStyleSheet("QPushButton{background: rgba(255, 255, 255, 0.2);border-radius: 8px;color: white;}"
                                   "QPushButton:hover{background: rgba(255, 255, 255, 0.4);border-radius: 8px;}"
                                   "QPushButton:pressed {background: rgba(255, 255, 255, 0.3);border-radius: 8px;}");
    sysFont.setPointSize((20 + m_curFontSize) * m_ptToPx);
    m_confirmButton->setFont(sysFont);
    m_cancelButton->setFont(sysFont);
    m_confirmButton->show();
    m_tipLabel->show();
    m_listView->hide();
    m_tipLabel->setText(tips);
    QString cancelStrEText = getElidedText(font(), m_cancelButton->width() - m_cancelButton->contentsMargins().left() - m_cancelButton->contentsMargins().right(), tr("Cancel"));
    if (cancelStrEText == tr("Cancel")) {
        m_cancelButton->setText(tr("Cancel"));
    } else if (cancelStrEText != tr("Cancel")) {
        m_cancelButton->setText(cancelStrEText);
        m_cancelButton->setToolTip(tr("Cancel"));
    }
    QString strEText = getElidedText(font(), m_confirmButton->width() - m_confirmButton->contentsMargins().left() - m_confirmButton->contentsMargins().right(), tr("Confrim"));
    if (strEText == tr("Confrim")) {
        m_confirmButton->setText(tr("Confrim"));
    } else if (strEText != tr("Confrim")) {
        m_confirmButton->setText(strEText);
        m_confirmButton->setToolTip(tr("Confrim"));
    }
}

void BlockWidget::setMsgTips(int type)
{
    msg_type = type;
    QString tips = tr("If you do not perform any operation, the system will automatically %1 after %2 seconds.")
                       .arg(getHibited_tr_lowcase(msg_type))
                       .arg(SHUTDOWN_MSG_TIME);
    
    m_msgTipLabel->setText(tips);
    m_msgTipLabel->show();

    lastDateTime = QDateTime::currentDateTime();
    if (!shutdownTimer) {
        shutdownTimer = new QTimer(this);
        shutdownTimer->setInterval(500);
        connect(shutdownTimer, &QTimer::timeout, this, [&] {
            int interval = (60 - lastDateTime.secsTo(QDateTime::currentDateTime()));
            if (interval <= 0) {
                shutdownTimer->stop();
                m_confirmButton->clicked();
                m_msgTipLabel->setText("");
            } else {
                m_msgTipLabel->setText(
                    tr("If you do not perform any operation, the system will automatically %1 after %2 seconds.")
                        .arg(getHibited_tr_lowcase(msg_type))
                        .arg(interval));
            }
        });
    }
    shutdownTimer->start(500);

}

void BlockWidget::setWarning(QStringList list, int type)
{
    m_tipLabel->setFixedWidth(this->width());
    switch (type) {
        case 0:
            m_tipLabel->setText(tr(
                "The following programs prevent restarting, you can click \"Cancel\" and then close these programs."));
            break;
        case 1:
            m_tipLabel->setText(tr("The following programs prevent the shutdown, you can click \"Cancel\" and then "
                                   "close these programs."));
            break;
        case 2:
            m_tipLabel->setText(
                tr("The following programs prevent suspend, you can click \"Cancel\" and then close these programs."));
            break;
        case 3:
            m_tipLabel->setText(tr(
                "The following programs prevent hibernation, you can click \"Cancel\" and then close these programs."));
            break;
        case 4:
            m_tipLabel->setText(tr(
                "The following programs prevent you from logging out, you can click \"Cancel\" and then close them."));
            break;
        default:
            break;
    }
    m_listView->show();

    QStandardItemModel *model = new QStandardItemModel(this);
    QIcon icon;
    QString iconName = list.at(0);
    QString appName = list.at(1);

    if (!iconName.isEmpty() && QIcon::hasThemeIcon(iconName)) {
        icon = QIcon::fromTheme(iconName);
    } else if (QIcon::hasThemeIcon("application-x-desktop")) {
        icon = QIcon::fromTheme("application-x-desktop");
    }
    model->appendRow(new QStandardItem(icon, appName));

    m_listView->verticalScrollMode();
    m_listView->setStyleSheet(
        "QListView#applist{font:10pt;color:white;background-color: rgba(255,255,255,80);border-style: outset;border-width: 0px;border-radius: 6px;}\
                              QListView#applist::item{height:48px;margin-top:2px;border-radius: 6px;}\
                              QListView#applist::item::selected {background-color: rgba(255,255,255,80);border: 1px solid #296CD9;\
                              height:48px;margin-top:2px;border-radius: 6px;}\
                              QListView#applist::item::hover {background-color: rgba(255,255,255,80);height:48px;margin-top:2px;border-radius: 6px;}");

    sysFont.setPointSize((10 + m_curFontSize) * m_ptToPx);
    m_listView->setFont(sysFont);
    m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_listView->setIconSize(QSize(32, 32));
    m_listView->setModel(model);
    m_listView->setFixedSize(520, 320);
    m_cancelButton->setFixedSize(120, 48);

    m_confirmButton->hide();
    m_cancelButton->setStyleSheet("QPushButton{background: rgba(255, 255, 255, 0.2);border-radius: 24px;color: white;}"
                                  "QPushButton:hover{background: rgba(255, 255, 255, 0.4);border-radius:24px;}"
                                  "QPushButton:pressed {background: rgba(255, 255, 255, 0.3);border-radius: 24px;}");
    sysFont.setPointSize((16 + m_curFontSize) * m_ptToPx);
    m_cancelButton->setFont(sysFont);
    QString cancelStrEText = getElidedText(font(), m_cancelButton->width() - m_cancelButton->contentsMargins().left() - m_cancelButton->contentsMargins().right(), tr("Cancel"));
    if (cancelStrEText == tr("Cancel")) {
        m_cancelButton->setText(tr("Cancel"));
    } else if (cancelStrEText != tr("Cancel")) {
        m_cancelButton->setText(cancelStrEText);
        m_cancelButton->setToolTip(tr("Cancel"));
    }
    QString strEText = getElidedText(font(), m_confirmButton->width() - m_confirmButton->contentsMargins().left() - m_confirmButton->contentsMargins().right(), tr("Confrim"));
    if (strEText == tr("Confrim")) {
        m_confirmButton->setText(tr("Confrim"));
    } else if (strEText != tr("Confrim")) {
        m_confirmButton->setText(strEText);
        m_confirmButton->setToolTip(tr("Confrim"));
    }
}

QString BlockWidget::getHibited_tr_lowcase(int type)
{
    switch (type) {
        case SHUTDOWN:
            return tr("shut down");
            break;
        case REBOOT:
            return tr("restart");
            break;
        default:
            break;
    }
    return "";
}

QString BlockWidget::getElidedText(QFont font, int width, QString strInfo)
{
    QFontMetrics fontMetrics(font);
    //如果当前字体下，字符串长度大于指定宽度
    if (fontMetrics.width(strInfo) > width) {
        strInfo = QFontMetrics(font).elidedText(strInfo, Qt::ElideRight, width);
    }
    return strInfo;
}
