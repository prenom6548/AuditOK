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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "custom_widget.h"
#include <QScrollArea>
#include <QLineEdit>

#include "ukui_printer.h"
#include "add_printer_window.h"
#include "menumodule.h"

#include "right_widget.h"
#include "left_widget.h"
#include "select_ppd_dialog.h"
#include "property_window.h"
#include "job_manager_window.h"
#include "deb_install_window.h"

#include "ukui_settings_monitor.h"
#include "menumodule.h"

#define g_printerManger UkuiPrinterManager::getInstance()
class MainWindow : public CustomWidget
{
    Q_OBJECT
public:
    static MainWindow *getInstance();
    kdk::WindowId windowId = 0;
    void init();
protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
signals:
    void updateMainWin();
    void modifyPPD(QString);
    void addPrinterToListSector(const UkuiUsbPrinter &printer);
    void updateListSector(const QList<UkuiUsbPrinter> &printersVector,const UkuiUsbPrinter& currentPrinter);
    void addDeviceBtnClicked();
    void updateRightWidget(const UkuiUsbPrinter&);
    void addNewPrinter(const UkuiUsbPrinter&);
    void clearRightWidget();
    void showPPDSelectDialog();
    void getCurFontSize();
public:
    bool mainWindowIsHidden(){
        return this->isHidden();
    }
public slots:
    void updateRightWidgetByCheckBtn(const UkuiUsbPrinter&);
    void updateMainWindowByAdd(QString);
    void updateByChangePPD(QString);
    void setQSSFontSize(QFont currentFont);
private slots:
    void topWindow();
    void onBtnClicked();
    void ppdDialogShow(const QString &str);
    void propertyDialogShow(const UkuiUsbPrinter&);
    void jobListDialogShow(const QString&);
    void printTestPage(const UkuiUsbPrinter&printer);
    void troubleshooting(const UkuiUsbPrinter&printer);

    void updateMainWindowByDel(QString);

    void modifyPrinterPPD(QString);
    void updateCurrentSelectedPrinter(const UkuiUsbPrinter&printer);

    void updateImmediately();
    void updateByRename(QString printerName);
    void enableMaximum();
    void disableMaximum();

private:
    ~MainWindow();
    explicit MainWindow(QWidget *parent = nullptr);
    MainWindow(MainWindow const&);
    MainWindow& operator=(MainWindow const&);


    bool m_isProcessing = false;

    DaemonIpcDbus *m_dbus;
    AddPrinterWindow *m_AddWindow;

    MenuModule *m_menu;
    LeftWidget  *m_leftWidget;
    RightWidget *m_rightWidget;
    std::unique_ptr<QHBoxLayout> m_leftLayout{nullptr};
    std::unique_ptr<QHBoxLayout> m_rightLayout{nullptr};


    PropertyWindow *m_propertyDialog = nullptr;
    JobManagerWindow *m_jobListDialog = nullptr;

    UkuiUsbPrinter m_currentSelectedPrinter;

    void initModule();
    void initQSS();
    void initConnect();

    QList<UkuiUsbPrinter> m_allPrintersVector;

    QTimer *m_timer;

    DebInstallWindow* m_pDebInstallWindow = nullptr;

    QFont m_curFont;
    double m_curFontPointSize;


    //=======================test接口=========================

protected:
    virtual void handlePCMode();
    virtual void handlePADMode();

};

#endif // MAINWINDOW_H
