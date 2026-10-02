/*
 * Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDebug>
#include "batteryinfo.h"

BatteryInfo::BatteryInfo(QObject *parent) : QObject(parent)
{
    const QByteArray id("org.ukui.power-manager");
    mPowerManagementGsettings = new QGSettings(id);
    mUpower = new UPowerInfo;
    QString batteryPath = mUpower->getBatteryPath();
    iface = new QDBusInterface(UPOWER_SERVICE, batteryPath, FREEDESKTOP_UPOWER, QDBusConnection::systemBus());
    initBatteryInfo(mBat1);
    showBatteryInfo();
    connect(mPowerManagementGsettings, &QGSettings::changed, this, [=] { readSettings(); });
    if (batteryPath.contains("battery_")) {
        QDBusConnection::systemBus().connect(UPOWER_SERVICE,
                                             batteryPath,
                                             FREEDESKTOP_UPOWER,
                                             "PropertiesChanged",
                                             this,
                                             SLOT(dealMessage(QDBusMessage)));
    }
}

BatteryInfo::~BatteryInfo()
{
    delete iface;
    delete mUpower;
    delete mPowerManagementGsettings;
}

void BatteryInfo::initBatteryInfo(BatInfo &data)
{
    readSettings();
    mLowBatteryState = false;
    QDBusMessage msg = iface->call("GetAll","org.freedesktop.UPower.Device");
    qDebug() << "get all bat msg type :" << msg.type();
    if (msg.type() == QDBusMessage::ReplyMessage) {
        const QDBusArgument &dbusArg = msg.arguments().at(0).value<QDBusArgument>();
        QMap<QString,QVariant> map;
        dbusArg >> map;
        data.isPresent = (map.value(QString("IsPresent")).toBool());
        data.percentage = map.value(QString("Percentage")).toDouble();
        data.percentage = ( (float)( (int)( (data.percentage + 0.05) * 10 ) ) ) / 10;
        data.state = map.value(QString("State")).toInt();
        data.timeToEmpty = map.value(QString("TimeToEmpty")).toLongLong();
        data.timeToFull = map.value(QString("TimeToFull")).toLongLong();
        data.warnlevel = map.value(QString("WarningLevel")).toInt();
    } else {
        qDebug() << "init battery data faile";
    }
}

void BatteryInfo::dealMessage(QDBusMessage msg)
{
    readSettings();
    getBatteryInfo();
    dealLowBatteryMessage(batteryState);
    dealBatteryIconMessage(batteryState, mPercentage);
    msgAnalysis(msg);
}

void BatteryInfo::readSettings()
{
    mSetPercentageLow = mPowerManagementGsettings->get("percentageLow").toInt();
    mPercentageAction = mPowerManagementGsettings->get("percentageAction").toInt();
}

void BatteryInfo::getBatteryInfo()
{
    QThread::msleep(500);
    batteryState = mUpower->OnBattery();
    mPercentage = Percentage();
}

void BatteryInfo::msgAnalysis(QDBusMessage batData)
{
    const QDBusArgument &dbusArgs = batData.arguments().at(1).value<QDBusArgument>();
    QMap<QString,QVariant> map;
    dbusArgs >> map;
    putBatteryInfo(map,mBat1);
    showBatteryInfo();
}

void BatteryInfo::putBatteryInfo(QMap<QString, QVariant> &map, BatInfo &data)
{
    if (map.contains("TimeToFull")) {
        data.timeToFull = map.value(QString("TimeToFull")).toLongLong();
    } if (map.contains("TimeToEmpty")) {
        data.timeToEmpty = map.value(QString("TimeToEmpty")).toLongLong();
    } if (map.contains("WarnLevel")) {
        data.warnlevel = map.value(QString("WarnLevel")).toInt();
    } if (map.contains("State")) {
        data.state = map.value(QString("State")).toUInt();
    } if (map.contains("Percentage")) {
        data.percentage = map.value(QString("Percentage")).toDouble();
        data.percentage = ( (float)( (int)( (data.percentage + 0.05) * 10 ) ) ) / 10;
        emit PercentageChanged(data.percentage);
    } if (map.contains("IsPresent")) {
        data.isPresent = (map.value(QString("IsPresent")).toBool());
    }
}

void BatteryInfo::showBatteryInfo()
{
    qDebug() << "TimeToFull" << mBat1.timeToFull;
    qDebug() << "TimeToEmpty" << mBat1.timeToEmpty;
    qDebug() << "WarnLevel" << mBat1.warnlevel;
    qDebug() << "State" << mBat1.state;
    qDebug() << "Percentage" << mBat1.percentage;
    qDebug() << "IsPresent" << mBat1.isPresent;
}

QString BatteryInfo::IconName()
{
    getBatteryInfo();
    double num = mPercentage;
    if(!IsPresent()){
        mIconMessage = QString("battery-error-charging-symbolic");
    }else if (true == batteryState) {
        mIconMessage = QString("battery-level-%1-symbolic").arg(qRound(num / 10.0) * 10);
    } else {
        mIconMessage = QString("battery-level-%1-charging-symbolic").arg(qRound(num / 10.0) * 10);
    }
    return mIconMessage;
}

double BatteryInfo::Percentage()
{
    double value;
    QDBusReply<QVariant> reply = iface->call("Get", UPOWER_DIVICES_SERVICE, "Percentage");
    if (reply.isValid()) {
        value = reply.value().toDouble();
        return value;
    } else {
        qDebug() << "Get percentage failed";
        return -1.0;
    }
}

int BatteryInfo::TimeToFull()
{
    //    QDBusInterface iface(UPOWER_SERVICE,UPOWER_DISPLAY_PATH,
    //                          FREEDESKTOP_UPOWER,
    //                          QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = iface->call("Get", UPOWER_DIVICES_SERVICE, "TimeToFull");
    if (reply.isValid()) {
        return reply.value().toInt();
    } else {
        qDebug() << "Get time to full failed";
        return -1;
    }
}

int BatteryInfo::TimeToEmpty()
{
    QDBusInterface iface(UPOWER_SERVICE, UPOWER_DISPLAY_PATH, FREEDESKTOP_UPOWER, QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = iface.call("Get", UPOWER_DIVICES_SERVICE, "TimeToEmpty");
    if (reply.isValid()) {
        return reply.value().toInt();
    } else {
        qDebug() << "Get time to empty failed";
        return -1;
    }
}

bool BatteryInfo::IsPresent()
{
    QDBusReply<QVariant> reply = iface->call("Get", UPOWER_DIVICES_SERVICE, "IsPresent");
    if (reply.isValid()) {
        return reply.value().toInt();
    } else {
        qDebug() << "Get is present failed";
        return false;
    }
}

bool BatteryInfo::LowBatteryState()
{
    readSettings();
    getBatteryInfo();
    if (mSetPercentageLow > mPercentage) {
        return true;
    } else {
        qDebug() << "Get low battery state failed";
        return false;
    }
}

int BatteryInfo::getBatteryState()
{
    QDBusReply<QVariant> reply = iface->call("Get", UPOWER_DIVICES_SERVICE, "State");
    if (reply.isValid()) {
        return reply.value().toInt();
    } else {
        qDebug() << "Get Battery State failed";
        return -1;
    }
}

void BatteryInfo::dealLowBatteryMessage(const bool &state)
{
    if (state) {
        if (mSetPercentageLow >= mPercentage) {
            if (!mLowBatteryState) {
                mLowBatteryState = true;
                emit LowBatteryChanged(mLowBatteryState);
            }
            if (mPercentageAction >= mPercentage) {
                mVeryLowBatteryState = true;
                emit VeryLowBatteryChanged(mVeryLowBatteryState);
            }
        } else {
            if (mLowBatteryState) {
                mLowBatteryState = false;
                emit LowBatteryChanged(mLowBatteryState);
            }
            if (mVeryLowBatteryState) {
                mVeryLowBatteryState = false;
                emit VeryLowBatteryChanged(mVeryLowBatteryState);
            }
        }
    } else {
        if (mLowBatteryState) {
            mLowBatteryState = false;
            emit LowBatteryChanged(mLowBatteryState);
        }
        if (mVeryLowBatteryState) {
            mVeryLowBatteryState = false;
            emit VeryLowBatteryChanged(mVeryLowBatteryState);
        }
    }
}

void BatteryInfo::dealBatteryIconMessage(const bool &state, const double &precentage)
{
    double num = precentage;
    if(!IsPresent()){
        mIconMessage = QString("battery-error-charging-symbolic");
    }
    else if (true == state) {
        mIconMessage = QString("battery-level-%1-symbolic").arg((int)num / 10 * 10);
    } else {
        mIconMessage = QString("battery-level-%1-charging-symbolic").arg((int)num / 10 * 10);
    }
    emit BatteryIconChanged(mIconMessage);
}
