/*
 * Copyright (C) 2024 KylinSoftCo., Ltd.
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
#include "autorotatescreen.h"
#include <QDebug>

AutoRotateScreen* AutoRotateScreen::instance(QObject *parent)
{
    static AutoRotateScreen* _instance = nullptr;
    QMutex mutex;
    mutex.lock();
    if(!_instance)
    {
        _instance = new AutoRotateScreen(parent);
    }
    mutex.unlock();
    return _instance;
}

AutoRotateScreen::AutoRotateScreen(QObject *parent):
    QObject(parent),
    m_pSensor(nullptr),
    m_curScreenDirection(RotateDir::Normal)
{
    init();
}

void AutoRotateScreen::init()
{
    m_pSensor = new QOrientationSensor(this);
    monitorWatcher = new MonitorWatcher(this);
    connect(m_pSensor, &QOrientationSensor::readingChanged, this, &AutoRotateScreen::onReadingChanged);
    connect(m_pSensor, &QOrientationSensor::activeChanged, this, &AutoRotateScreen::onActiveChanged);
}

void AutoRotateScreen::start()
{
    if(m_pSensor && !m_pSensor->isActive())
    {
        m_pSensor->start();
    }
}

void AutoRotateScreen::stop()
{
    if(m_pSensor && m_pSensor->isActive())
    {
        m_pSensor->stop();
    }
}

void AutoRotateScreen::setScreenDirection(const RotateDir dir)
{
    QStringList monitorList = monitorWatcher->getMonitorNames();
    QString cmd;
    switch (dir) {
    case RotateDir::Normal:
        for (int i = 0; i < monitorList.count(); i++) {
            cmd = QString("xrandr --output %1 --rotate normal").arg(monitorList.at(i));
            QProcess process;
            process.start(cmd);
            process.waitForFinished(-1);
        }
        break;
    case RotateDir::Left:
        for (int i = 0; i < monitorList.count(); i++) {
            cmd = QString("xrandr --output %1 --rotate left").arg(monitorList.at(i));
            QProcess process;
            process.start(cmd);
            process.waitForFinished(-1);
        }
        break;
    case RotateDir::Right:
        for (int i = 0; i < monitorList.count(); i++) {
            cmd = QString("xrandr --output %1 --rotate right").arg(monitorList.at(i));
            QProcess process;
            process.start(cmd);
            process.waitForFinished(-1);
        }
        break;
    case RotateDir::Inverted:
        for (int i = 0; i < monitorList.count(); i++) {
            cmd = QString("xrandr --output %1 --rotate inverted").arg(monitorList.at(i));
            QProcess process;
            process.start(cmd);
            process.waitForFinished(-1);
        }
        break;
    default:
        break;
    }
}


AutoRotateScreen::RotateDir AutoRotateScreen::getScreenDirection()
{
    return m_curScreenDirection;
}

void AutoRotateScreen::onReadingChanged()
{
    QOrientationReading *reading = m_pSensor->reading();
    switch (reading->orientation()) {
    case QOrientationReading::Undefined:
        qDebug() << "info: [AutoRotateScreen][onReadingChanged]: QOrientationReading::Undefined";
        break;
    case QOrientationReading::TopUp:
        qDebug() << "info: [AutoRotateScreen][onReadingChanged]: QOrientationReading::TopUp";   
        setScreenDirection(RotateDir::Normal);
        emit screenRotate(RotateDir::Normal);
        break;
    case QOrientationReading::TopDown:
        qDebug() << "info: [AutoRotateScreen][onReadingChanged]: QOrientationReading::TopDown";
        setScreenDirection(RotateDir::Inverted);
        emit screenRotate(RotateDir::Inverted);
        break;
    case QOrientationReading::LeftUp:
        qDebug() << "info: [AutoRotateScreen][onReadingChanged]: QOrientationReading::LeftUp";
        setScreenDirection(RotateDir::Left);
        emit screenRotate(RotateDir::Left);
        break;
    case QOrientationReading::RightUp:
        qDebug() << "info: [AutoRotateScreen][onReadingChanged]: QOrientationReading::RightUp";
        setScreenDirection(RotateDir::Right);
        emit screenRotate(RotateDir::Right);
        break;
    case QOrientationReading::FaceUp:
        qDebug() << "info: [AutoRotateScreen][onReadingChanged]: QOrientationReading::FaceUp";
        break;
    case QOrientationReading::FaceDown:
        qDebug() << "info: [AutoRotateScreen][onReadingChanged]: QOrientationReading::FaceDown";
        break;
    default:
        break;
    }
}

void AutoRotateScreen::onActiveChanged()
{
    onReadingChanged();
}
