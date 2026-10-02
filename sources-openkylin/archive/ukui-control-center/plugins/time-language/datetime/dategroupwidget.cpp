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
#include "dategroupwidget.h"
#include <QDebug>
#include <kysdk/applications/accessinfohelper.h>

using namespace kdk;

DategroupWidget::DategroupWidget(QWidget *parent)
    : QWidget(parent)
{
    m_lunarDateEdit    = new KLunarDateEdit(this); // 日历
    hourComboBox      = new QComboBox(this); // 时
    minComboBox       = new QComboBox(this); // 分
    secComboBox       = new QComboBox(this); // 秒
    hourMinColonLabel = new QLabel(":");
    MinSecColonLabel  = new QLabel(":");

    QHBoxLayout *layout = new QHBoxLayout(this);
    QSpacerItem *setDateSpacer = new QSpacerItem(16, 20, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_lunarDateEdit);
    layout->addSpacerItem(setDateSpacer);
    layout->addWidget(hourComboBox);
    layout->addWidget(hourMinColonLabel);
    layout->addWidget(minComboBox);
    layout->addWidget(MinSecColonLabel);
    layout->addWidget(secComboBox);
    layout->addStretch();

    m_lunarDateEdit->setFixedSize(160, 36);
    m_lunarDateEdit->getDateEdit()->setDate(QDate::currentDate());
    m_lunarDateEdit->getDateEdit()->setCalendarPopup(true);
    hourComboBox->setFixedWidth(100);
    minComboBox->setFixedWidth(64);
    secComboBox->setFixedWidth(64);
    hourMinColonLabel->setFixedWidth(16);
    MinSecColonLabel->setFixedWidth(16);
    hourMinColonLabel->setAlignment(Qt::AlignCenter);
    MinSecColonLabel->setAlignment(Qt::AlignCenter);
#ifdef Nile
    resetDateEdit(kdk_system_get_shortformat());
#endif

    areaInterface = new QDBusInterface("org.ukui.ukcc.session",
                                       "/Area",
                                       "org.ukui.ukcc.session.Area",
                                       QDBusConnection::sessionBus(),
                                       this);
    if (!areaInterface->isValid()) {
        qCritical() << "org.ukui.ukcc.session.Area DBus error:" << areaInterface->lastError();
    }
    QDBusConnection::sessionBus().connect("org.ukui.ukcc.session",
                                          "/Area",
                                          "org.ukui.ukcc.session.Area",
                                          "changed",
                                          this,
                                          SLOT(dataChangedSlot(QString)));
#ifdef Nile
    QDBusConnection::sessionBus().connect("com.kylin.kysdk.DateServer",
                                          "/com/kylin/kysdk/Date",
                                          "com.kylin.kysdk.DateInterface",
                                          "ShortDateSignal",
                                          this,
                                          SLOT(resetDateEdit(QString)));
#endif
    initHour();
    initMinAndSec();
    initConnect();
    updateTime();
    timerID = startTimer(1000);

    KDK_EXTEND_ALL_INFO_FORMAT(m_lunarDateEdit, "Datetime", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(hourComboBox, "Datetime", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(minComboBox, "Datetime", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(secComboBox, "Datetime", "", "");

    m_lunarDateEdit->getDateEdit()->setAccessibleName(tr("Date"));
    hourComboBox->setAccessibleName(tr("Hour"));
    minComboBox->setAccessibleName(tr("Minute"));
    secComboBox->setAccessibleName(tr("Second"));
}

DategroupWidget::~DategroupWidget()
{
    killTimer(timerID);
}

void DategroupWidget::dataChangedSlot(QString key)
{
    if (key == "timeFormat") {
        initHour();
    }
}

void DategroupWidget::resetDateEdit(QString date) {
    QString dateformat;
    int locale = 0;
    int i = 0;


    QLocale l1ocale = QLocale::system();
    if ("zh_CN" == l1ocale.name()){
        locale = 1;
    } else if ("bo_CN" == l1ocale.name()){
        locale = 2;
    }

    dateformat = date;
    while(i < dateformat.length()) {
        if (dateformat.at(i) != 'M' &&
                dateformat.at(i) != 'd' &&
                dateformat.at(i) != 'y')
            break;
        else
            ++i;
    }

    QString split = dateformat.at(i);
    if (locale == 0) {
        if (dateformat.at(dateformat.length() - 3) == split)
            m_lunarDateEdit->getDateEdit()->setDisplayFormat("M" + split + "d" + split + "yy");
        else
            m_lunarDateEdit->getDateEdit()->setDisplayFormat("MM" + split + "dd" + split + "yyyy");
    } else {
        if (dateformat.at(2) == split)
            m_lunarDateEdit->getDateEdit()->setDisplayFormat("yy" + split + "M" + split + "d");
        else
            m_lunarDateEdit->getDateEdit()->setDisplayFormat("yyyy" + split + "MM" + split + "dd");
    }
}

void DategroupWidget::initHour()
{
    hourComboBox->clear();
    QLocale l1ocale = QLocale::system();
    if ("zh_CN" == l1ocale.name()){
        l1ocale = QLocale::Chinese;
    } else if ("bo_CN" == l1ocale.name()){
        l1ocale = QLocale::Tibetan;
    } else if ("mn_MN" == l1ocale.name()){
        l1ocale = QLocale::Mongolian;
    } else if ("kk_KZ" == l1ocale.name()) {
        l1ocale = QLocale::Kazakh;
    } else if ("kk_KG" == l1ocale.name()) {
        l1ocale = QLocale::Kirghiz;
    } else if ("ug_CN" == l1ocale.name()) {
        l1ocale = QLocale::Uighur;
    } else {
        l1ocale = QLocale::English;
    }
    QString AMname = l1ocale.amText();
    QString PMname = l1ocale.pmText();
    qDebug() << "initHour AMname" << AMname << PMname << PMname << "" << QLocale::system().name() << "loccale name" << l1ocale.name();
    if (areaInterface->property("timeFormat").toString() != "24") {
        if (AMname == QString("上午") || AMname == QString("སྔ་དྲོ་")) {
            hourComboBox->addItem(AMname + QString::number(12));
            for (int i = 1 ; i <= 11; i++) {
                hourComboBox->addItem(AMname + QString::number(i));
            }
            hourComboBox->addItem(PMname + QString::number(12));
            for (int i = 1 ; i <= 11; i++) {
                hourComboBox->addItem(PMname + QString::number(i));
            }
        } else {
            hourComboBox->addItem(QString::number(12) + " " + AMname);
            for (int i = 1 ; i <= 11; i++) {
                hourComboBox->addItem(QString::number(i) + " " +AMname);
            }
            hourComboBox->addItem(QString::number(12) + " " +PMname);
            for (int i = 1 ; i <= 11; i++) {
                hourComboBox->addItem(QString::number(i) + "  " + PMname);
            }
        }
    } else {
        for (int h = 0; h < 24; h++){
            hourComboBox->addItem(QString::number(h));
        }
    }
}

void DategroupWidget::initMinAndSec()
{
    for (int m = 0; m < 60; m++) {
        minComboBox->addItem(QString::number(m));
    }

    for (int s = 0; s < 60; s++) {
        secComboBox->addItem(QString::number(s));
    }
}

void DategroupWidget::initConnect()
{
    connect(m_lunarDateEdit, &KLunarDateEdit::dateChanged, this, [=](QDate date) {
        qDebug() << Q_FUNC_INFO << "dateChanged" << date.toString();
        QTime t(hourComboBox->currentIndex(), minComboBox->currentIndex(), secComboBox->currentIndex());
        Q_EMIT dateChanged(date, t);
    });

    connect(hourComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, [=]() {
        QDate d(m_lunarDateEdit->getDateEdit()->date());
        QTime t(hourComboBox->currentIndex(), minComboBox->currentIndex(), secComboBox->currentIndex());
        Q_EMIT dateChanged(d, t);
    });

    connect(minComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, [=]() {
        QDate d(m_lunarDateEdit->getDateEdit()->date());
        QTime t(hourComboBox->currentIndex(), minComboBox->currentIndex(), secComboBox->currentIndex());
        Q_EMIT dateChanged(d, t);
    });

    connect(secComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, [=]() {
        QDate d(m_lunarDateEdit->getDateEdit()->date());
        QTime t(hourComboBox->currentIndex(), minComboBox->currentIndex(), secComboBox->currentIndex());
        Q_EMIT dateChanged(d, t);
    });
}

void DategroupWidget::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == timerID) {
        updateTime();
    }
}

void DategroupWidget::updateTime(bool force)
{
    QDateTime Ctime = QDateTime::currentDateTime();
    m_lunarDateEdit->blockSignals(true);
    hourComboBox->blockSignals(true);
    minComboBox->blockSignals(true);
    secComboBox->blockSignals(true);
    if (force || !m_lunarDateEdit->getDateEdit()->hasFocus()) {
        m_lunarDateEdit->getDateEdit()->setDate(QDate::currentDate());
    }
    hourComboBox->setCurrentIndex(Ctime.time().hour());
    minComboBox->setCurrentIndex(Ctime.time().minute());
    secComboBox->setCurrentIndex(Ctime.time().second());

    m_lunarDateEdit->blockSignals(false);
    hourComboBox->blockSignals(false);
    minComboBox->blockSignals(false);
    secComboBox->blockSignals(false);
}
