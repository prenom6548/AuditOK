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
#ifndef DEVICE_LIST_BUTTON_H
#define DEVICE_LIST_BUTTON_H

#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include "device_monitor.h"
#include "custom_label.h"
#include "ukui_printer.h"

class DeviceListButton : public QPushButton
{
    Q_OBJECT
public:
    explicit DeviceListButton(const UkuiUsbPrinter &printerStruct, QWidget *parent = 0);//编写带父类参数的构造函数
    void freshStatus();
    const QString getPrinterName() const;
    void setDefaultPrinter(bool);
    bool getDefaultStatus();
    void setQSSFontSize(QFont curFont);
protected:

private:
    void initUI(const UkuiUsbPrinter &printerStruct);

    UkuiUsbPrinter m_printerStruct;

    QHBoxLayout *m_HLwholeBtn;
    QVBoxLayout *m_VLtextInfo;
    QHBoxLayout *m_HLtextInfo;

    QPushButton *m_typeIcon;

    CustomLabel *m_printerNameLabel;
    QString m_printerNameText;
    QLabel *m_connectStatus;
    QPushButton *m_statusIcon;
    bool m_isConnectedFlag;
    QLabel *m_isDefault;
    bool m_isDefaultFlag;

    QMenu *m_menu;
    QAction *m_actionSetDefault;
    QAction *m_actionEnabled;
    QAction *m_actionShared;
signals:
    void thisBtnIsChecked(DeviceListButton*);
    void thisBtnIsUnchecked(DeviceListButton*);
    void changeDefaultPrinter(QString);
protected slots:
    void showMenu(const QPoint &point);

    void buttonCheckState(bool isChecked);
    bool eventFilter(QObject *obj, QEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
private slots:
    void menuHide();
    void triggerMenu(QAction *action);
};

#endif // DEVICE_LIST_BUTTON_H
