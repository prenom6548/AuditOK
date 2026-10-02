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
#include "timewidget.h"
#include "../lock-dialog/lockdialogmodel.h"

#define TIME_FONT_SIZE 36
#define TIME_DATE_SIZE 18
#define TIMEWIDGET_SIZE 1000, 100

TimeWidget::TimeWidget(LockDialogModel *model, QWidget *parent) : QWidget(parent), m_modelLockDialog(model)
{
    this->setMinimumSize(TIMEWIDGET_SIZE);

    m_curFontSize = m_modelLockDialog->getCurFontSize();
    m_ptToPx = m_modelLockDialog->getPtToPx();
    QVBoxLayout *m_VBLayout = new QVBoxLayout(this);

    m_t_label = new CustomLabel(this);
    m_t_label->setAlignment(Qt::AlignCenter);
    m_t_label->setFontSize(TIME_FONT_SIZE);
    m_t_label->setStyleSheet("QLabel{color: white;}"
                             "QToolTip{border-radius:4px; background-color:#FFFFFF; color:black;}");
    m_t_label->setText(getLongFormatDate(TIME));
    m_t_label->adjustSize();

    m_d_label = new CustomLabel(this);
    m_d_label->setAlignment(Qt::AlignCenter);
    m_d_label->setStyleSheet("QLabel{color: white;}"
                             "QToolTip{border-radius:4px; background-color:#FFFFFF; color:black;}");
    m_d_label->setText(getLongFormatDate(DATE));
    m_d_label->setFontSize(TIME_DATE_SIZE);
    m_d_label->adjustSize();

    m_VBLayout->addWidget(m_t_label);
    m_VBLayout->addWidget(m_d_label);
    this->setLayout(m_VBLayout);
    this->adjustSize();

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update_datatime()));
    this->timerStart();
}

TimeWidget::~TimeWidget()
{
    m_timer->deleteLater();
}

void TimeWidget::updateUI()
{
    this->setFixedSize(this->width(), this->height());
    m_t_label->setFontSize((TIME_FONT_SIZE + m_curFontSize) * m_ptToPx);
    m_d_label->setFontSize((TIME_DATE_SIZE + m_curFontSize) * m_ptToPx);
}

void TimeWidget::timerStart()
{
    m_timer->start(1000);
}

void TimeWidget::timerStop()
{
    m_timer->stop();
}

void TimeWidget::updateTimeFont(QString fontFamily)
{
    if (m_t_label) {
        m_t_label->setFontFamily(fontFamily);
    }
    if (m_d_label) {
        m_d_label->setFontFamily(fontFamily);
    }
    update_datatime();
}

void TimeWidget::updateTimeFontSize(double fontSize)
{
    if (m_t_label) {
        m_t_label->setFontSize((TIME_FONT_SIZE + fontSize) * m_ptToPx);
    }
    if (m_d_label) {
        m_d_label->setFontSize((TIME_DATE_SIZE + fontSize) * m_ptToPx);
    }
    update_datatime();
}

QString TimeWidget::getLongFormatDate(int type)
{
    char *userName;
    QByteArray ba = m_modelLockDialog->currentUserName().toLatin1();
    userName = ba.data();
    kdk_logn_dateinfo *dateInfo = kdk_system_login_lock_dateinfo(userName);
    if (type == DATE) {
        QString date = dateInfo->date;
        QString week = dateInfo->week;
        if (dateInfo) {
            kdk_free_logn_dateinfo(dateInfo);
        }
        return date + " " + week;
    } else if (type == TIME) {
        QString time = dateInfo->time;
        if (dateInfo) {
            kdk_free_logn_dateinfo(dateInfo);
        }
        return time;
    }
}

void TimeWidget::update_datatime()
{
    m_t_label->setText(getLongFormatDate(TIME));
    m_d_label->setText(getLongFormatDate(DATE));
}
