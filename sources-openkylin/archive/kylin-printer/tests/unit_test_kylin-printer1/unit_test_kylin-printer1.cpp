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
#include "ukui_printer.h"


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



TEST_F(PeonyPrintPicturesInterfaceTest, d)
{
    UkuiUsbPrinter printer;
    printer.name = "unit_test_printer";
    printer.device_uri = "cups-pdf:/";
    TroubleshootingDialog *w = new TroubleshootingDialog(printer, nullptr);
    w->check1();
    w->check2();
    w->check3();
    w->check4();
}

TEST_F(PeonyPrintPicturesInterfaceTest, e)
{
    g_findAndMatchWorkThread->start();
    g_findAndMatchWorkThread->updatePPDsMap();
    g_findAndMatchWorkThread->isBusy();
    g_findAndMatchWorkThread->hasPPDsMap();
    g_findAndMatchWorkThread->buildMatchPPDsThreadsAndConnections();

    QString ppd;
    QStringList ppdList;
    g_findAndMatchWorkThread->matchPPDToPrinter("aaa", "bbb", "ccc", ppd, ppdList);
    auto m_ppdMap = new ppdPrinterMap();
    g_findAndMatchWorkThread->getCurrentPPDsMap(*m_ppdMap);

    MatchPPDsThread::getInstance()->start();

    QMap<QString,PPDsAndAttr> ppdinfo;
    ppdPrinterMap data;
    ppdPrinterMap extra;
    data.insert("aaa", ppdinfo);

    MatchPPDsThread::getInstance()->initPPDMatch("aaa", "bbb", "ccc", data, extra, ppd, ppdList);
    MatchPPDsThread::getInstance()->initPPDMatch("", "bbb", "ccc", data, extra, ppd, ppdList);
    MatchPPDsThread::getInstance()->initPPDMatch("aaa", "", "ccc", data, extra, ppd, ppdList);
    MatchPPDsThread::getInstance()->initPPDMatch("aaa", "bbb", "", data, extra, ppd, ppdList);
    MatchPPDsThread::getInstance()->initPPDMatch("aaa", "", "ccc", ppdPrinterMap{}, extra, ppd, ppdList);


    MatchPPDsThread::getInstance()->~MatchPPDsThread();
    g_findAndMatchWorkThread->destroyCupsConnection();
    g_findAndMatchWorkThread->destroyMatchPPDsConnections();
    g_findAndMatchWorkThread->~FindAndMatchPPDs();
}

TEST_F(PeonyPrintPicturesInterfaceTest, f)
{
    QWidget *ww = new QWidget(nullptr);
    ChoosePpdComboBox *w = new ChoosePpdComboBox(ww);
    w->onChooseClicked();
    w->showPopup();
    w->hidePopup();
}


TEST_F(PeonyPrintPicturesInterfaceTest, g)
{
    ConfigIPAddressDialog *w = new ConfigIPAddressDialog(nullptr);
    w->getConfigStatus();
    w->onSettingsChanged();
    w->onApplyBtnClicked();
    w->onCancelBtnClicked();
}

TEST_F(PeonyPrintPicturesInterfaceTest, h)
{
    InkProperty p;
    qDebug() << p;
}

TEST_F(PeonyPrintPicturesInterfaceTest, i)
{
    PrinterAdvancedAttribute attr;
    UkuiPrinterManager::getInstance().addIppAttribute(attr);
    UkuiPrinterManager::getInstance().renamePrinter("aaa", "bbb");
    QList<UkuiUsbPrinter> printerNameList;
    UkuiPrinterManager::getInstance().printerIsInstalled("cups-pdf:/", true, printerNameList);
    UkuiPrinterManager::getInstance().restartJob(1);
    UkuiPrinterManager::getInstance().cancelJob("unit_test_printer", 1);
    UkuiPrinterManager::getInstance().deleteJob("unit_test_printer", 1);
    UkuiPrinterManager::getInstance().moveJob("aaa", "bbb", 1);
    UkuiPrinterManager::getInstance().moveAllJob("aaa", "bbb");
    UkuiPrinterManager::getInstance().holdJob(1);
    UkuiPrinterManager::getInstance().releaseJob(1);
    UkuiPrinterManager::getInstance().setJobHoldUntil(1, "aaa");
    UkuiPrinterManager::getInstance().generatePrinterName();
    QMap<QString, QVariant> properties;
    UkuiUsbPrinter printer;
    printer.name = "unit_test_printer";
    printer.device_uri = "cups-pdf:/";
    UkuiPrinterManager::getInstance().getAdvancedPrinterProperties(printer, properties);
    QList<PrinterAdvancedAttribute> printerAttributes;
    UkuiPrinterManager::getInstance().getAllProperties("cups-pdf:/", printerAttributes);
    UkuiPrinterManager::getInstance().startModifyAttr("unit_test_printer", printer);
    UkuiPrinterManager::getInstance().modifyPrinterAttributes(printer, properties);
    UkuiPrinterManager::getInstance().convertProperties(printerAttributes, properties);
    UkuiPrinterManager::getInstance().changeProperties(QStringList() << "aaa" << "bbb");
    UkuiPrinterManager::getInstance().getDefaultPrinter();
    UkuiPrinterManager::getInstance().setInfo("cups-pdf:/", "aaa");
    UkuiPrinterManager::getInstance().setUri("cups-pdf:/", "cups-pdf:/");
    UkuiPrinterManager::getInstance().getInkInfo(printer);
    QList<conflictPair> conflicts;
    // UkuiPrinterManager::getInstance().getConflictResults("aaa", conflicts);
    // UkuiPrinterManager::getInstance().duplicatePrinter("aaa", "unit_test_printer");
    UkuiPrinterManager::getInstance().setPrinterShared("unit_test_printer", true);
    UkuiPrinterManager::getInstance().setPrinterEnabled("unit_test_printer", true);
    UkuiPrinterManager::getInstance().isRemotePrinter("unit_test_printer");
}

