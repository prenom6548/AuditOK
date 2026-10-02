/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef LEFT_WIDGET_H
#define LEFT_WIDGET_H

#include <QObject>
#include <QLayout>
#include <QScrollArea>
#include <QWidget>
#include <QLayout>
#include <QList>
#include "device_list_button.h"
#include "custom_push_button.h"

class LeftWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LeftWidget(QWidget *parent = nullptr);

    DeviceListButton *getCurrentSelectPrinter() const;
    void setQSSFontSize(QFont curFont);
signals:
    void printerIsSelected(const UkuiUsbPrinter&);
    void noBtnIsChecked();
    void addDevice();
    void delDevice(QString);
public slots:
    void updateListSector(const QList<UkuiUsbPrinter>& printersVector,const UkuiUsbPrinter& currentPrinter);

private slots:
    void updateBtnConnections(DeviceListButton*);
    void delDeviceSlot();
    void addPrinterToListSector(const UkuiUsbPrinter& printersVector);
    void updateDefaultPrinter(QString printerName);
private:

    QList<UkuiUsbPrinter> m_printersVector;
    QLabel      *m_deviceLabel;
    CustomPushButton *m_addDeviceBtn;
    CustomPushButton *m_removeDeviceBtn;

    QList<DeviceListButton*> m_deviceList;

    QVBoxLayout *m_VLayoutNavigator;
    QHBoxLayout *m_HLayoutDeviceOP;
    QHBoxLayout *m_HLayoutDeviceList;

    QVBoxLayout *m_VLayoutDeviceBtnList;

    QWidget *m_deviceListWidget;
    QWidget *m_mainWin;
    QScrollArea *m_ScrollArea;

    void initDeviceOperateSector();
    void initDeviceListSector();

    void updateDeviceList();

    DeviceListButton *m_currentSelectBtn;

    bool m_isUpdatingFlag;
    QFont m_font14;
};

#endif // LEFT_WIDGET_H
