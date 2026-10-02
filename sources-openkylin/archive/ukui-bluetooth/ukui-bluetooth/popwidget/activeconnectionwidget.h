/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef ACTIVECONNECTIONWIDGET_H
#define ACTIVECONNECTIONWIDGET_H

#include <QIcon>
#include <QLabel>
#include <QTimer>
#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QCheckBox>
#include <QGSettings>
#include <QPushButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QMessageBox>

#define TRANSPARENCY_SETTINGS       "org.ukui.control-center.personalise"
#define TRANSPARENCY_KEY            "transparency"

class ActiveConnectionWidget : public QMessageBox
{
    Q_OBJECT
public:
    explicit ActiveConnectionWidget(QString address, QString name, QString type, int rssi);
    ~ActiveConnectionWidget();

private slots:
    void onClick_connect_btn();
    void onClick_cancel_btn();
    void toggled(bool checked);

public slots:
    void autoConnChanged(bool activate);

signals:
    void replyActiveConnection(QString,bool);

private:
    QPushButton * m_cancel_btn   = nullptr;
    QPushButton * m_connect_btn  = nullptr;

    QCheckBox   * m_actCnt_box   = nullptr;

    int m_devRSSI;
    bool m_mutex = false;
    QString m_devAddr;
    QString m_devName;
    QString m_devType;
};

#endif // ACTIVECONNECTIONWIDGET_H
