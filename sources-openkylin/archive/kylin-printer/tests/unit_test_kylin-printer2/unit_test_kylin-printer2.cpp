#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>

#define private public  // hack complier
#define protected public

#include "mainwindow.h"
#include "add_printer_window.h"
#include "rename_printer_dialog.h"
#include "troubleshooting_dialog.h"
#include "base_notify_dialog.h"
#include "job_manager_window.h"
#include "smb_auth_info_dialog.h"
#include "property_window.h"
#include "choose_ppd_combo_box.h"
#include "check_uri_connected_helper.h"
#include "find_and_match_ppds.h"
#include "config_ip_address_dialog.h"
#include "job_properties_window.h"
#include "package_helper.h"
#include "ink_info_helper.h"
#include "page_info_helper.h"
#include "snmpwalk_browser.h"
#include "ukui_apt.h"
#include "upload_helper.h"
#include "cups_debug_logging_checkbox.h"
#include "ink_info_widget.h"
#include "upload_helper.h"
#include "property_itemwidget.h"
#include "daemonipcdbus.h"
#include "information_collector.h"
#include "system_notification.h"

class PeonyPrintPicturesInterfaceTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
       
    }

    static void TearDownTestSuite()
    {
        
    }

};


TEST_F(PeonyPrintPicturesInterfaceTest, a)
{
    AddPrinterWindow *w = new AddPrinterWindow(QPoint(), nullptr);
    w->onManualClicked();
    w->onAutoClicked();
    
    DeviceInformation info;
    
    w->onAutoDeviceAdd(info);
    w->onManualDeviceAdd(info);
    w->onDeviceUpdated(info);
    w->onNameEditChanged();
    w->checkPrinterNameIsLegal(true, "asdfasdfasdfasdfasdf");
    w->updateForwardButtonStatus();

    w->onAddrChanged("10.41.120.168");
    w->m_drvComboBox->setCurrentText("everywhere");
    w->m_drvComboBox->addItem("everywhere");
    w->m_nameEdit->setText("asdgasdgawef");
    w->onForwardClicked();
    // w->showThis(QPoint());
    //
    // w->deleteLater();
}

TEST_F(PeonyPrintPicturesInterfaceTest, b)
{
    CupsDebugLoggingCheckbox *w = new CupsDebugLoggingCheckbox();
    w->saveDebugLogging();
    w->~CupsDebugLoggingCheckbox();
}

TEST_F(PeonyPrintPicturesInterfaceTest, c)
{
    UkuiUsbPrinter printer;
    printer.name = "unit_test_printer";
    printer.device_uri = "cups-pdf:/";
    QWidget *ww = new QWidget();
    PropertyWindow *w = new PropertyWindow(printer, ww);
    w->checkOptionConflicts("aaa", "bbb");
    w->selectPPDDialogShow();
    w->doSave();
    // w->updatePPD("aaa");
    w->setQSSFontSize(QFont());

    w->~PropertyWindow();
}

TEST_F(PeonyPrintPicturesInterfaceTest, d)
{
    InkInfoWidget *w = new InkInfoWidget();
    w->setMainLabel("aaa");
    w->setInformativeLabel("bbb");
    w->setColor(QColor());
    w->~InkInfoWidget();
}

TEST_F(PeonyPrintPicturesInterfaceTest, e)
{
    InkInfoWidget *w = new InkInfoWidget();
    w->setMainLabel("aaa");
    w->setInformativeLabel("bbb");
    w->setColor(QColor());
    w->~InkInfoWidget();
}

TEST_F(PeonyPrintPicturesInterfaceTest, n)
{
    AptUtilHelper* aptHelper = new AptUtilHelper(DeviceInformation());
    aptHelper->startWorker();
    aptHelper->onInstalldebStatusChanged(1, "aaa", "bbb");
    aptHelper->onRecvApt(true, QStringList(), "aaa", "bbb");
    aptHelper->onRecvApt(false, QStringList(), "aaa", "bbb");
    aptHelper->processPkg();
    aptHelper->m_device.packageNameList << "aaa";
    aptHelper->processPkg();
    aptHelper->installPackage(QStringList());
    aptHelper->stopAptTask();
    aptHelper->m_installStatus = ukuiInstallStatus::UKUI_INSTALL_IN_PROGRESS;
    aptHelper->stopAptTask();
    aptHelper->handleCancel();
    aptHelper->finished();
}

TEST_F(PeonyPrintPicturesInterfaceTest, o)
{
    DebUtilHelper* aptHelper = new DebUtilHelper("/home/kylin/work/kylin-printer/tests/usb-creator-common_0.3.7kylin1_amd64.deb");
    aptHelper->startWorker();
    aptHelper->onInstalldebStatusChanged(1, "aaa", "bbb");
    aptHelper->onRecvApt(true, "aaa", "bbb");
    aptHelper->onRecvApt(false, "aaa", "bbb");
}

TEST_F(PeonyPrintPicturesInterfaceTest, p)
{
    DeviceInformation info;
    UploadHelper *w = new UploadHelper(info, 1);
    w->startWorker();
    w->dowork();
}

TEST_F(PeonyPrintPicturesInterfaceTest, q)
{
    AttrName attr;
    QList<AttrName> attrs;
    attrs << attr;

    ProperytItemWidget *w = new ProperytItemWidget("aaa", PropertyBoxStyle::TEXT_WITH_LINEEDIT_UNEDITABLE);
    w = new ProperytItemWidget("aaa", PropertyBoxStyle::TEXT_WITH_LINEEDIT_UNEDITABLE);    
    w = new ProperytItemWidget("aaa", PropertyBoxStyle::TEXT_WITH_LINEEDIT_UNEDITABLE, attr);
    w = new ProperytItemWidget("aaa", PropertyBoxStyle::TEXT_WITH_LINEEDIT_UNEDITABLE, attr, attrs);
    w->setfirstTitleText(QString());
    w->setfirstTitleText("aaa");
    w->showWarning(QString());
    w->showWarning("aaa");
    w->getChangedProperty();
}

TEST_F(PeonyPrintPicturesInterfaceTest, r)
{
    DaemonIpcDbus *m_dbus = DaemonIpcDbus::getInstance();
    m_dbus->callRequest(QStringList());
    m_dbus->show();
    m_dbus->sendarg(QStringList());
    m_dbus->showGuide("aaa");
    m_dbus->request(QStringList());
}

TEST_F(PeonyPrintPicturesInterfaceTest, s)
{
    for (int i = 0; i < 20; i++) {
        InformationCollector::getInstance().addPoint(InformationCollector::P(i));
    }
}

TEST_F(PeonyPrintPicturesInterfaceTest, t)
{
    SystemNotification().sendMessage("aaa");
    SystemNotification().sendMessage("aaa", "bbb", 1000);
}




