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
#ifndef MAIN_WIN_PROPERTY_WIDGET_H
#define MAIN_WIN_PROPERTY_WIDGET_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTableView>
#include <QStandardItemModel>

#include "ukui_printer.h"
#include "custom_label.h"
#include "custom_push_button.h"

class PropertyManagerModel;
namespace Ui {
class MainWinPropertyWidget;
}

class MainWinPropertyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWinPropertyWidget(QWidget *main ,QWidget *parent = nullptr);
    void updateText(const UkuiUsbPrinter&);
    void setQSSFontSize(QFont curFont);

signals:
    void ppdDialogShowRequest(const QString &);
    void sendOutCurrentPinterInfo(const UkuiUsbPrinter&);
    void sendOutCurrentPinterName(const QString&);
    void currentPinterInfoPrintTest(const UkuiUsbPrinter&);
    void currentPinterInfoTroubleshooting(const UkuiUsbPrinter&);
    void updateWholeMainWin(QString);
protected:
    void paintEvent(QPaintEvent *) override;

private slots:
    void onEditPrinterNameBtnPressed();
    void modifyCurrentPrinterName(const QString &printerName = QString());
private:
    Ui::MainWinPropertyWidget *m_ui;
    void setUI();
    void setFont(QFont ft, QLabel *label);
    void updateWin(QString newPrinterName);
    void setPrinterDriver(const QString &str);
    QPushButton *m_logoIcon;

    CustomLabel *m_printerShowName;
    QString     m_currentPrinterNameText;
    QPushButton *m_printerShowNameEditIcon;

    CustomPushButton *m_showPropertyBtn;
    CustomPushButton *m_showJobListBtn;
    CustomPushButton *m_printTestPageBtn;
    QTableView  *m_properytView;
    PropertyManagerModel *m_propertyManagerModel;
    CustomPushButton *m_propertyDriverButton;

    bool m_isPpdDialogShownFlag;

    UkuiUsbPrinter m_currentPrinter;

    QWidget *m_mainWinPtr;

    friend class MainWindow;
};

class PropertyManagerModel : public QStandardItemModel
{
    Q_OBJECT
public:

    explicit PropertyManagerModel(QStandardItemModel *parent = nullptr);
    void setPrinterModel(const QString &str);
    void setPrinterStatus(const QString &str);
    void setPrinterLocation(const QString &str);
    ~PropertyManagerModel();
};


#endif // MAINWINPROPERTYWIDGET_H
