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
#include "timeBtn.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDateTime>
#include "datetime.h"
#include "worldMap/zoneinfo.h"

TimeBtn::TimeBtn(const QString &timezone, const QString &kyTimezone)
{
    this->setFixedHeight(80);
    this->setObjectName("TimeBtn");
    this->setAttribute(Qt::WA_DeleteOnClose);

    QHBoxLayout *timeShowLayout = new QHBoxLayout(this);
    QWidget     *timeWid      = new QWidget(this);
    QVBoxLayout *timeLayout   = new QVBoxLayout(timeWid);
    labelInfo    = new KLabel(this);    //时间,和标题字号一致
    labelTime    = new LightLabel(this);    //日期
    deleteBtn    = new QPushButton(this);

    areaInterface = new QDBusInterface("org.ukui.ukcc.session",
                                       "/Area",
                                       "org.ukui.ukcc.session.Area",
                                       QDBusConnection::sessionBus(),
                                       this);
    if (!areaInterface->isValid()) {
        qCritical() << "org.ukui.ukcc.session.Area DBus error:" << areaInterface->lastError();
    }

    timeShowLayout->setContentsMargins(0,0,18,0);
    timeLayout->setContentsMargins(18,0,18,0);
    timeShowLayout->addWidget(timeWid);
    timeShowLayout->addWidget(deleteBtn);
    deleteBtn->setFixedSize(36, 36);
    deleteBtn->setProperty("useButtonPalette", true);
    deleteBtn->setFlat(true);
    deleteBtn->setIcon(QIcon::fromTheme("edit-delete-symbolic"));
    deleteBtn->setVisible(false);

    timeLayout->addStretch();
    timeLayout->addWidget(labelInfo);
    timeLayout->addWidget(labelTime);
    timeLayout->addStretch();
    labelInfo->setObjectName("DateTime_Info");
    labelTime->setObjectName("DateTime_Time");

    thisZone = QTimeZone(timezone.toLatin1().data());
    int utcOff = thisZone.offsetFromUtc(QDateTime::currentDateTime())/3600;
    QString gmData;
    if (utcOff >= 0) {
        gmData = QString("(GMT+%1:%2)").arg(utcOff, 2, 10, QLatin1Char('0')).arg(utcOff / 60, 2, 10, QLatin1Char('0'));
    } else {
        gmData = QString("(GMT%1:%2)").arg(utcOff, 3, 10, QLatin1Char('0')).arg(utcOff / 60, 2, 10, QLatin1Char('0'));
    }
    ZoneInfo m_zoneinfo;
    const QString locale = QLocale::system().name();
    if (kyTimezone.isEmpty()) {
        labelInfo->setText(m_zoneinfo.getLocalTimezoneName(timezone, locale) + "   " + gmData);
    } else {
        labelInfo->setText(m_zoneinfo.getLocalTimezoneName(kyTimezone, locale) + "   " + gmData);
    }
    QFont font;
    m_fontSetting = new QGSettings("org.ukui.style", QByteArray(), this);
    font.setFamily(m_fontSetting->get("systemFont").toString());
    font.setPixelSize(m_fontSetting->get("systemFontSize").toInt() * 18 / 11);  //设置的是pt，按照公式计算为px,标题默认字为18px
    font.setWeight(QFont::Medium);
    labelInfo->setFont(font);
    connect(m_fontSetting , &QGSettings::changed, this, [=](const QString &key) {
        styleSettingChangeSlot(key);
    });

    timerId = startTimer(1000);

    updateTime(areaInterface->property("timeFormat").toString() == "24");
    connect(deleteBtn, &QPushButton::clicked, this, [=](){
        this->close();
        Q_EMIT deleted();
    });
}

void TimeBtn::styleSettingChangeSlot(const QString& key)
{
    if("systemFontSize" == key) {
        QFont font;
        font.setFamily(m_fontSetting->get("systemFont").toString());
        font.setPixelSize(m_fontSetting->get("systemFontSize").toInt() * 18 / 11);
        font.setWeight(QFont::Medium);
        labelInfo->setFont(font);
    }
}

TimeBtn::~TimeBtn()
{
    killTimer(timerId);
}

void TimeBtn::enterEvent(QEvent *event) {
    Q_UNUSED(event);
    deleteBtn->setVisible(true);
}

void TimeBtn::leaveEvent(QEvent *event) {
    Q_UNUSED(event);
    deleteBtn->setVisible(false);
}

void TimeBtn::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == timerId) {
        updateGMT();
        updateTime(areaInterface->property("timeFormat").toString() == "24");
    }
}

void TimeBtn::updateTime(bool hour_24) {
    QString localizedTimezone = "";
    QTimeZone currentZone  = QTimeZone(localizedTimezone.toLatin1().data());
    QString time;
    QDateTime thisZoneTime;
    const double timeDelta = (thisZone.offsetFromUtc(QDateTime::currentDateTime()) - currentZone.offsetFromUtc(QDateTime::currentDateTime())) / 3600.0;
    QString dateLiteral;
    if ((QDateTime::currentDateTime().time().hour() + timeDelta) >= 24) {
        dateLiteral = tr("Tomorrow");
    } else if ((QDateTime::currentDateTime().time().hour() + timeDelta) < 0) {
        dateLiteral = tr("Yesterday");
    } else {
        dateLiteral = tr("Today");
    }
    int decimalNumber = 1;
    //小时取余,再取分钟,将15分钟的双倍只显示一位小数,其他的都显示两位小数
    switch ((thisZone.offsetFromUtc(QDateTime::currentDateTime()) - currentZone.offsetFromUtc(QDateTime::currentDateTime())) / 3600 / 60 / 15) {
    case -1:
    case -3:
    case 1:
    case 3:
        decimalNumber = 2;
        break;
    default:
        decimalNumber = 1;
        break;
    }

    QString compareLiteral;
    if (timeDelta > 0) {
        compareLiteral = tr("%1 hours earlier than local").arg(QString::number(timeDelta, 'f', decimalNumber));
    } else {
        compareLiteral = tr("%1 hours later than local").arg(QString::number(-timeDelta, 'f', decimalNumber));
    }

    thisZoneTime = QDateTime::currentDateTime().toTimeZone(thisZone);
    if (hour_24) {
        time = thisZoneTime.toString("hh : mm : ss");
    } else {
        QLocale l1ocale = QLocale::system();
        if (l1ocale.name() != "en_US") {
            if ("bo_CN" == l1ocale.name())
                l1ocale = QLocale::Tibetan;
            else if ("mn_MN" == l1ocale.name())
                l1ocale = QLocale::Mongolian;
            else
                l1ocale = QLocale::Chinese;
            time = l1ocale.toString(thisZoneTime, "AP hh: mm : ss");
        } else {
            l1ocale = QLocale::English;
            time = l1ocale.toString(thisZoneTime, "hh: mm : ss AP");
        }
    }

    labelTime->setText(QString("%1  %2     %3").arg(dateLiteral).arg(time).arg(compareLiteral));
}

void TimeBtn::updateGMT()
{
    const int utcOff = thisZone.offsetFromUtc(QDateTime::currentDateTime()) / 3600;
    QString gmData;
    if (utcOff >= 0) {
        gmData = QString("(GMT+%1:%2)").arg(utcOff, 2, 10, QLatin1Char('0')).arg(utcOff / 60, 2, 10, QLatin1Char('0'));
    } else {
        gmData = QString("(GMT%1:%2)").arg(utcOff, 3, 10, QLatin1Char('0')).arg(utcOff / 60, 2, 10, QLatin1Char('0'));
    }

    const QString currentText = labelInfo->text();
    const int separator = currentText.lastIndexOf("   ");
    if (separator >= 0) {
        labelInfo->setText(currentText.left(separator) + "   " + gmData);
    }
}
