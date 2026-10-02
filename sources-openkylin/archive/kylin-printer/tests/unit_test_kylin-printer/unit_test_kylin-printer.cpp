#include <gtest/gtest.h>
#include <gtest/gtest-death-test.h>
#include <iostream>

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
    UkuiUsbPrinter printer;
    printer.name = "unit_test_printer";
    printer.device_uri = "cups-pdf:/";
    
    QList<UkuiUsbPrinter> printers;
    printers << printer;

    MainWindow::getInstance()->show();
    MainWindow::getInstance()->updateMainWin();
    MainWindow::getInstance()->modifyPPD("aaa");
    MainWindow::getInstance()->addPrinterToListSector(printer);
    MainWindow::getInstance()->updateListSector(printers, printer);
    MainWindow::getInstance()->addDeviceBtnClicked();
    MainWindow::getInstance()->updateRightWidget(printer);
    MainWindow::getInstance()->addNewPrinter(printer);
    MainWindow::getInstance()->clearRightWidget();
    MainWindow::getInstance()->showPPDSelectDialog();
    MainWindow::getInstance()->getCurFontSize();
    MainWindow::getInstance()->topWindow();
    MainWindow::getInstance()->onBtnClicked();
    MainWindow::getInstance()->updateMainWindowByAdd("unit_test_printer");
    MainWindow::getInstance()->updateMainWindowByDel("unit_test_printer");
    MainWindow::getInstance()->ppdDialogShow("aaa");
    MainWindow::getInstance()->updateByChangePPD("aaa");
    MainWindow::getInstance()->modifyPrinterPPD("aaa");
    MainWindow::getInstance()->propertyDialogShow(printer);
    MainWindow::getInstance()->jobListDialogShow("unit_test_printer");
    MainWindow::getInstance()->printTestPage(printer);
    MainWindow::getInstance()->troubleshooting(printer);
    MainWindow::getInstance()->updateByRename("unit_test_printer");
    MainWindow::getInstance()->setQSSFontSize(QFont());

}


TEST_F(PeonyPrintPicturesInterfaceTest, c)
{
    RenamePrinterDialog *w = new RenamePrinterDialog("unit_test_printer", nullptr);
    w->acceptButtonClicked();
    w->getPrinterName();
    delete w;
}


TEST_F(PeonyPrintPicturesInterfaceTest, e)
{
    BaseNotifyDialog *w = new BaseNotifyDialog(nullptr);
    w->dialogSetType(BaseNotifyDialogType::WAITTING);
    w->dialogSetType(BaseNotifyDialogType::ERROR);
    w->dialogSetType(BaseNotifyDialogType::WARNING);
    w->dialogSetType(BaseNotifyDialogType::SUCCEED);
    w->labelSetText("aaa");
    w->setMovieDialog();
    w->gifPlay();
    w->gifPlay();
    w->gifPlay();
    w->gifPlay();
    w->gifPlay();
    w->gifPlay();
    w->gifPlay();
    w->gifPlay();
}

TEST_F(PeonyPrintPicturesInterfaceTest, f)
{
    SMBAuthData data;
    data.host = "192.17.120.168";
    data.share = "aaa";
    data.workgroup = "WORKGROUP";
    data.username = "kylin";
    data.password = "kk123123";
    SmbAuthInfoDialog *w = new SmbAuthInfoDialog(data, nullptr);
    w->acceptButtonClicked();
    w->show();
}

TEST_F(PeonyPrintPicturesInterfaceTest, g)
{
    QString name = "unit_test_printer";
    UkuiUsbPrinter selectPrinter;
    QList<UkuiUsbPrinter> printers;
    g_printerManger.enumPrinters(printers);
    for(auto printer : printers)
    {
        if(printer.name == name)
        {
            selectPrinter = printer;
        }
    }

    QWidget *ww = new QWidget();
    PropertyWindow *w = new PropertyWindow(selectPrinter, ww);
    QString option;
    QMap<QString, QString> conflicts;
    w->setAllConflicts(option, conflicts);
    w->updatePPD("aaa");
    // w->onShow();
}

TEST_F(PeonyPrintPicturesInterfaceTest, h)
{
    CheckUriConnectedHelper *w = new CheckUriConnectedHelper("socket://10.41.120.168");
    w->dowork();
}

TEST_F(PeonyPrintPicturesInterfaceTest, i)
{
    JobMenu menu;

    auto ww = JobManagerWindow::getInstance("unit_test_printer",UkuiQueueStatus::UKUI_PRINTER_QUEUE_ALL,nullptr);
    JobManagerWindow m_jobListDialog("unit_test_printer",UkuiQueueStatus::UKUI_PRINTER_QUEUE_ALL,nullptr);
    m_jobListDialog.showThis();
    m_jobListDialog.m_model->setPageInfo(1,1);
    m_jobListDialog.updateModel(false);

    QList<PrintJob> a;
    QList<PrintJob> b;
    m_jobListDialog.compareJobs(a, b);
    // m_jobListDialog.ShowContextMenu(QPoint());
    for (int i = -1; i <= 10; i++) {
        m_jobListDialog.setMenuActionEnabled(menu, i, true);
    }
    PrintJob job;
    m_jobListDialog.getJob(job, 1);

    QAction action;
    m_jobListDialog.triggerMenu(&action);

    JobPropertiesWindow *w = new JobPropertiesWindow(1, nullptr);

    JobManagerModel m("unit_test_printer");
    m.resetFont(QFont());

    

}

TEST_F(PeonyPrintPicturesInterfaceTest, j)
{
    DeviceInformation info;
    info.packageNameList << "aaa" << "bbb";
    PackageHelper *w = new PackageHelper(info);
    w->startWorker();
    w->dowork();

    info.packageNameList << "aaa" << "bbb";
    w = new PackageHelper(info);
    w->dowork();
}

TEST_F(PeonyPrintPicturesInterfaceTest, k)
{
    UkuiUsbPrinter printer;
    printer.name = "unit_test_printer";
    printer.device_uri = "ipp://10.41.120.168";

    InkInfoHelper *w = new InkInfoHelper(printer);
    w->dowork();
}

TEST_F(PeonyPrintPicturesInterfaceTest, l)
{
    UkuiUsbPrinter printer;
    printer.name = "unit_test_printer";
    printer.device_uri = "ipp://10.41.120.168";

    PageInfoHelper *w = new PageInfoHelper("ipp://10.41.120.168");
    w->dowork();
}

TEST_F(PeonyPrintPicturesInterfaceTest, m)
{
    std::vector<std::string> buf;

    std::vector<HostNode> hosts = {
        { "172.17.30.111", "public" },
    };

    std::vector<OidNode> oids = {
        { "1.3.6.1.2.1.4.20.1.1" },
        { "1.3.6.1.2.1.4.20.1.3" },
        { "1.3.6.1.2.1.1.5" },
        { "1.3.6.1.2.1.43.9.2.1.8" },
        { "1.3.6.1.2.1.25.3.2.1.3" },
        { "1.3.6.1.2.1.43.5.1.1.17" },
    };
    const char *a = "1.3.6.1.2.1.1.5";
    const char *b = "1.3.6.1.2.1.25.3.2.1.3";
    std::vector<OidNode> oids1 = {
        { a },
    };
    std::vector<OidNode> oids2 = {
        { b },
    };
 
    snmpwalkBrowser_initialize(oids);
    int ans = snmpwalkBrowser_synchronous(buf, hosts, oids);
    for (int i = 0; i < buf.size(); i++) {
        printf("%s\n", buf[i].c_str());
    }

    // buf.clear();
    // snmpwalkBrowser_initialize(oids2);
    // ans = snmpwalkBrowser_synchronous(buf, hosts, oids2);
    // for (int i = 0; i < buf.size(); i++) {
    //     printf("%s\n", buf[i].c_str());
    // }
    qDebug() << buf.size();
}


TEST_F(PeonyPrintPicturesInterfaceTest, n)
{
    DebInstallWindow w;
    w.onForwardBtnClicked();
    w.onCancelBtnClicked();
}

TEST_F(PeonyPrintPicturesInterfaceTest, o)
{
    MenuModule w;
    w.helpAction();
}



