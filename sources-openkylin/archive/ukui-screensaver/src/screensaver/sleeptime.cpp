/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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

#include "sleeptime.h"
#include <QLabel>
#include <QDebug>
#include <QListWidget>
#include <QApplication>

SleepTime::SleepTime(QWidget *parent)
    : QWidget(parent)
    , sleepTime(0)
    , m_nLastSleepLeave(0)
    , m_nLastSleepTimeSecs(0)
    , configuration(SCConfiguration::instance())
{
    init();
}

SleepTime::~SleepTime() {}

void SleepTime::init()
{
    layout = new QHBoxLayout(this);
    QLocale local;
    QString systemLang = local.name();
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        layout->setDirection(QBoxLayout::LeftToRight);
    } else {
        layout->setDirection(QBoxLayout::RightToLeft);
    }
//    layout->setDirection(QBoxLayout::RightToLeft);
    layout->setSpacing(8);
    curFontSize = configuration->getFontSize();
    sysFont = qApp->font();
    sysFont.setPointSize((20 + curFontSize) * m_ptToPx);
    for (int i = 0; i < 3; i++) {
        QLabel *label = new QLabel(this);
        label->setText("0");
        label->setFixedSize(40, 40);
        label->setObjectName("clockTime");
        label->setFont(sysFont);
        list.append(label);
        if (i < 2) {
            QLabel *colon = new QLabel(this);
            colon->setText(":");
            colon->setObjectName("colon");
            colon->setFont(sysFont);
            list.append(colon);
        }
    }

    for (int i = 0; i < list.count(); i++) {
        layout->addWidget(list.at(i));
    }

    restTime = new QLabel(this);
    sysFont.setPointSize((20 + curFontSize) * m_ptToPx);
    restTime->setFont(sysFont);
    restTime->setText(tr("You have rested"));
    restTime->setObjectName("restTime");
    restTime->setAlignment(Qt::AlignBottom);
    restTime->adjustSize();

    layout->addSpacing(8);
    layout->addWidget(restTime);

    initTime = QDateTime::currentDateTime();
    m_lastTime = initTime;
}

int SleepTime::setTime(QDateTime time)
{
    // 与上一次取时间的时间差
    long nNewSleepTime = qAbs(m_lastTime.msecsTo(time));
    sleepTime = qAbs(initTime.msecsTo(time));
    // 时间差大于1s，则认为时间异常变化，保存已过去的时间
    if (nNewSleepTime > 1000) {
        m_nLastSleepLeave += qAbs(m_lastTime.msecsTo(initTime));
        sleepTime = 0;
        initTime = time;
    }
    m_lastTime = time;
    // 当前时间差+异常情况过去的时间
    sleepTime += m_nLastSleepLeave;
    sleepTime = sleepTime / 1000;

    if (m_nLastSleepTimeSecs == 0 || qAbs(sleepTime - m_nLastSleepTimeSecs) >= 1) {
        int hour = sleepTime / 3600;
        int sec = sleepTime % 3600 % 60;
        int min = sleepTime % 3600 / 60;
        setHour(hour);
        setSeconds(sec);
        setMinute(min);
        m_nLastSleepTimeSecs = sleepTime;
    }

    return true;
}

void SleepTime::setHour(int hour)
{
    QString time;
    if (hour >= 100) {
        QLabelSetText(list.at(4), QString::number(hour));
    } else if (hour < 10) {
        time = "0" + QString::number(hour);
        list.at(4)->setText(time);
    } else {
        time = QString::number(hour);
        list.at(4)->setText(time);
    }
}

void SleepTime::setSeconds(int seconds)
{
    QString time;
    if (seconds < 10) {
        time = "0" + QString::number(seconds);
    } else {
        time = QString::number(seconds);
    }
    list.at(0)->setText(time);
}

void SleepTime::setMinute(int minutes)
{
    QString time;
    if (minutes < 10) {
        time = "0" + QString::number(minutes);
    } else {
        time = QString::number(minutes);
    }
    list.at(2)->setText(time);
}

void SleepTime::setSmallMode()
{
    for (int i = 0; i < 5; i++)
        list.at(i)->setFixedSize(10, 10);
    adjustSize();
}

bool SleepTime::QLabelSetText(QLabel *label, QString string)
{
    bool is_over_length = false;
    QFontMetrics fontMetrics(label->font());
    int fontSize = fontMetrics.width(string);
    QString str = string;
    if (fontSize > (label->width() - 5)) {
        str = fontMetrics.elidedText(string, Qt::ElideRight, label->width());
        is_over_length = true;
    }
    label->setText(str);
    return is_over_length;
}

void SleepTime::changeEvent(QEvent *event)
{
    //    if (event->type() == QEvent::LanguageChange) {
    //        if (restTime) {
    //            s_strRestTimeTranslation = tr("You have rested");
    //            restTime->setText(s_strRestTimeTranslation);
    //            restTime->adjustSize();
    //            qDebug() << "RestTime trans1:" << s_strRestTimeTranslation;
    //        }
    //        adjustSize();
    //    }
}
