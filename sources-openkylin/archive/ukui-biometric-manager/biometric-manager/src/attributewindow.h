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
#ifndef ATTRIBUTEWINDOW_H
#define ATTRIBUTEWINDOW_H
#include <QDialog>
#include <QPainter>
#include <QPainterPath>
#include <QListWidget>
#include <QDebug>
#include <QPushButton>
#include <QFileSystemWatcher>
#include <QMovie>
#include <QDBusInterface>
#include <QDBusArgument>
#include <QDBusReply>
#include <QMessageBox>
#include <QCoreApplication>

#include "attributewindow.h"
#include "mainwindow.h"

namespace Ui {
class AttributeWindow;
}

class AttributeWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AttributeWindow(DeviceInfo *deviceinfo, QWidget *parent = nullptr);

    ~AttributeWindow();


protected:
    void paintEvent(QPaintEvent * event);



private:
    Ui::AttributeWindow *ui;

    void setupInit();

    QString transferBioType(int type);
    QString transferVerifyType(int type);
    QString transferStorageType(int type);
    QString transferBusType(int type);
    QString transferIdentifyType(int type);

    DeviceInfo* deviceInfo;
//    SwitchButton  *defaultDeviceBtn;
    QFileSystemWatcher  *mWatcher;

private slots:

};
#endif // ATTRIBUTEWINDOW_H



