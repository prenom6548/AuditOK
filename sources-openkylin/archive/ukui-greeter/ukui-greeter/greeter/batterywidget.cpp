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
#include "batterywidget.h"
#include <QVBoxLayout>
#include <QDBusReply>
#include <QDebug>
#include <QStyleOption>
#include <QPainter>

BatteryWidget::BatteryWidget(QPoint point, QWidget *parent) :
    QWidget(parent),
    mPoint(point)
{
//    setWindowOpacity(0);
    setAttribute(Qt::WA_TranslucentBackground);
    initUi();
    setupComponent();
}

void BatteryWidget::initUi()
{
    QVBoxLayout *Lyt = new QVBoxLayout(this);
    Lyt->setContentsMargins(16, 16, 16, 16);
    Lyt->setSpacing(8);
    mModeLabel = new QLabel(this);
    mModeLabel->setFixedHeight(24);
    mModeLabel->setStyleSheet("QLabel{font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc;"
                              "font-weight: bold;}");

    QHBoxLayout *Lyt_1 = new QHBoxLayout();
    Lyt_1->setSpacing(4);

    mIconBtn = new QPushButton(this);
    mIconBtn->setFixedSize(48, 32);
    mIconBtn->setStyleSheet("QPushButton{\
                            color: rgb(255, 255, 255, 255);\
                            border: none;\
                            border-radius: 4px;\
                            outline: none;\
                        }");

    mValueLabel = new QLabel(this);
    mValueLabel->setFixedHeight(48);
    mValueLabel->setMinimumWidth(48);

    mStatusLabel = new QLabel(this);
    mStatusLabel->setFixedHeight(36);
    mStatusLabel->setAlignment(Qt::AlignRight);

    Lyt_1->addWidget(mIconBtn);
    Lyt_1->addWidget(mValueLabel);
    Lyt_1->addStretch();
    Lyt_1->addWidget(mStatusLabel);

    Lyt->addWidget(mModeLabel);
    Lyt->addLayout(Lyt_1);
    Lyt->addStretch();
}

void BatteryWidget::setupComponent()
{
    QString batteryPath = "";
    dface = new QDBusInterface(UPOWER_SERVICE, UPOWER_PATH, UPOWER_INTERFACE, QDBusConnection::systemBus(), this);
    QDBusReply<QList<QDBusObjectPath>> reply = dface->call("EnumerateDevices");
    if (dface->isValid()) {
        for (QDBusObjectPath op : reply.value()) {
            if (op.path().contains("battery_")) {
                batteryPath = op.path();
                qDebug() << "battery path is :" << batteryPath;
                break;
            }
        }
    } else {
        qDebug() << "Enumerate devices failed";
    }
    batInterface = new QDBusInterface(UPOWER_SERVICE, batteryPath, FREEDESKTOP_UPOWER, QDBusConnection::systemBus());

    QDBusConnection::systemBus().connect(
        UPOWER_SERVICE, batteryPath, FREEDESKTOP_UPOWER, "PropertiesChanged", this, SLOT(dealMessage(QDBusMessage)));
    iface = new QDBusInterface(UPOWER_SERVICE, UPOWER_PATH, FREEDESKTOP_UPOWER, QDBusConnection::systemBus());
    if (dface->isValid()) {
        mIconBtn->setIconSize(QSize(48, 32));
        mIconBtn->setFocusPolicy(Qt::NoFocus);
        mIconBtn->setIcon(QIcon::fromTheme(getBatteryIconName()));
    }
    ed = EngineDevice::getInstance();
    onBatteryChanged(ed->engine_get_state());

    int size;
    size = ed->devices.size();
    for (int i = 0; i < size; i++) {
        DEVICE *dv;
        dv = ed->devices.at(i);

        if (dv->m_dev.kind == UP_DEVICE_KIND_LINE_POWER) {
            continue;
        }

        connect(ed, &EngineDevice::engine_signal_Battery_State, this, &BatteryWidget::onBatteryChanged);
        continue;
    }
}

void BatteryWidget::setPoint(QPoint point)
{
    mPoint = point;
}

QString BatteryWidget::getBatteryIconName()
{
    if (dface->isValid() && iface->isValid()) {
        bool batteryState = false;
        QDBusReply<QVariant> reply = iface->call("Get", UPOWER_SERVICE, "OnBattery");
        if (reply.isValid()) {
            batteryState = reply.value().toBool();
        }
        double percentage = -1.0;
        QDBusReply<QVariant> percentage_reply = batInterface->call("Get", UPOWER_DIVICES_SERVICE, "Percentage");
        if (percentage_reply.isValid()) {
            percentage = percentage_reply.value().toDouble();
        }

        qDebug()<<batteryState<<"+++++++++++++++++";
        if (batteryState) {
            return QString("battery-level-%1-symbolic").arg((int)percentage / 10 * 10);
        } else {
            return QString("battery-level-%1-charging-symbolic").arg((int)percentage / 10 * 10);
        }
    }
    return QString();
}

void BatteryWidget::showEvent(QShowEvent *event)
{
    QPoint pos = QPoint(mPoint.x(),
                        mPoint.y() - 112);
    this->move(pos);
        QWidget::showEvent(event);
}

void BatteryWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.save();
    p.setBrush(opt.palette.color(QPalette::Base));
    p.setPen(Qt::transparent);
    p.setOpacity(0.75);
    p.drawRoundedRect(this->rect(), 16, 16);
    p.restore();
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void BatteryWidget::setBatteryIcon(QString icon)
{
    mIconBtn->setIcon(QIcon::fromTheme(icon));
}

void BatteryWidget::onBatteryChanged(QStringList args)
{
    int battery = args.at(0).toInt();
    int batteryState = args.at(1).toInt();
    if (batteryState == 1 || batteryState == 5) {
        mStatusLabel->setText(tr("Charging..."));
    } else if (batteryState == 4) {
        mStatusLabel->setText(tr("fully charged"));
    }
    if (batteryState == 4 || batteryState == 1 || batteryState == 5) {
        mStatusLabel->setVisible(true);
        mModeLabel->setText(tr("PowerMode"));
    } else {
        mStatusLabel->setVisible(false);
        mModeLabel->setText(tr("BatteryMode"));
    }

    mValueLabel->setText(QString("<font size='5';font color=#262626>%1</font>%").arg(battery));
}

void BatteryWidget::dealMessage(QDBusMessage)
{
    mIconBtn->setIcon(QIcon::fromTheme(getBatteryIconName()));
}
