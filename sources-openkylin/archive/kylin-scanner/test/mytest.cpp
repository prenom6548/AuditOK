/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "mytest.h"
#include "../src/titlebar/titlebar.h"
#include "kpushbutton.h"
#include <QTest>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include "../src/device/connect.h"
#include "../src/device/deviceFinder.h"
#include "../src/imageOp/imageoperationbase.h"
#include "../src/imageOp/imageoperationbeauty.h"
#include "../src/imageOp/imageoperationmirror.h"
#include "../src/imageOp/imageoperationocr.h"
#include "../src/imageOp/imageoperationrectify.h"
#include <QStack>
#include "../src/device/snmpFinder.h"
#include "../src/saneobject.h"

static bool uriCheck(const QString &uri, const QString &scheme);

Mytest::Mytest(QObject *parent)
    : QObject(parent)
{
    // 每个testcase需要独立运行
//        testCase1();
//        testCase2();
//        testCase3();
    // 456 要求运行有纸
//        testCase4();
//        testCase5();
//        testCase6();
//        testCase7();
//        testCase8();
//        testCase9();
//        testCase10();
//        testCase11();
//        testCase12();
//        testCase13();
//    testCase14();
//    testCase15();
//    testCase16();
    testCase17();

}

void Mytest::testCase1()
{
    // 启动应用 测试标题栏按钮
    MainWidget *w = new MainWidget;
    w->show();
    QTest::mouseClick(w->findChild<QPushButton*>("m_maxBtn"), Qt::LeftButton);
    QTest::mouseClick(w->findChild<QPushButton*>("m_minBtn"), Qt::LeftButton);
    QTest::mouseClick(w->findChild<QPushButton*>("m_maxBtn"), Qt::LeftButton);
    QTest::mouseDClick(w->findChild<TitleBar*>(), Qt::LeftButton);
    QTest::mouseDClick(w->findChild<TitleBar*>(), Qt::LeftButton);
    QTimer::singleShot(3000,[=](){
        TitleBar *titlebar = w->findChild<TitleBar*>();
        int result = titlebar->warnCloseWindow();
        titlebar->fontSizeChanged();
        QMenu *menu = titlebar->findChild<QMenu*>();
        QList<QAction*> actions = menu->actions();
        for (int i = 0; i < actions.length(); ++i) {
            actions.at(i)->trigger();
        }
    });
}

void Mytest::testCase2()
{
    qDebug() << "执行testcase2";
    // show一些正常流程不会有的页面
    FailedPageWidget *widget = new FailedPageWidget;
    widget->show();
    widget->fontSizeChanged();
    widget->rotateChangedSlot(true);
    widget->rotateChangedSlot(false);
    widget->clickfiledPageButton();
    widget->close();

    QStringList a;
    a.append("1");
    a.append("2");
    a.append("3");
    CustomTableWidget *tablewidge = new CustomTableWidget(a);
    tablewidge->insertItem(3, a);
    tablewidge->getSelectedRow();
    tablewidge->show();
    tablewidge->close();
    QString text = "11111111111111111111111111111111111111";
    CustomPushButton *custompushbtn = new CustomPushButton(text);
    custompushbtn->setTextLimitShrink(text, 10);
    custompushbtn->setTextLimitExpand(text);
    custompushbtn->fullText();


    DisplayWidget *displaywidget = new DisplayWidget();
    displaywidget->show();
    displaywidget->showFailedPageSlot();
    displaywidget->showSuccessImageHandlePageSlot();
    displaywidget->close();

    CustomLabel *customlabel = new CustomLabel(text);
    customlabel->show();
    customlabel->setTextLimitShrink(text, 10);
    customlabel->setTextLimitExpand(text);
    customlabel->fullText();
    customlabel->elideText();

    QWidget *widget2 = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(custompushbtn);
    layout->addWidget(customlabel);
    widget2->setLayout(layout);
    widget2->setFixedSize(100,100);
    widget2->show();
    QTimer::singleShot(2000,[=](){widget2->close();});

    QString add = "10.41.120.45";
    try_connect(add.toLocal8Bit(),22);

    QString uri1 = "http://www.example.com";
    QString scheme1 = "http";
    QVERIFY(uriCheck(uri1, scheme1));

    QList<DeviceInformation> info ;
    DeviceInformation oneinfo;
    info.append(oneinfo);
    deviceFinder *devfinder = new deviceFinder;
    devfinder->buriedDeviceInfomation(info);

    LeftImageHandleSuccessPageWidget *leftsuccesswidget = new LeftImageHandleSuccessPageWidget;
    leftsuccesswidget->show();
    leftsuccesswidget->showOcrWidgetSlot();
    leftsuccesswidget->showImageWidgetSlot();
    leftsuccesswidget->updatOcrText();
    leftsuccesswidget->close();

    ImageOperationBase *imgOpBase = new ImageOperationBase;
    QStack<QImage> *stackImage = new QStack<QImage>;
    QImage *img = new QImage("/home/fanyuchen/scanner-v11/kylin-scanner/test/a.png");
    stackImage->append(*img);
    imgOpBase->ImageOP();
    imgOpBase->imageCopy(img, stackImage);
    imgOpBase->interuptOp();
    imgOpBase->getIsInterrupt();

    ImageOperationBeauty *imgOpBeauty = new ImageOperationBeauty(img, stackImage);
    imgOpBeauty->ImageOP();
    imgOpBeauty->beauty(*img);

    ImageOperationMirror *imgOpMirror = new ImageOperationMirror(img, stackImage);
    imgOpMirror->ImageOP();
    imgOpMirror->mirror(img);

    ImageOperationOCR *imgOpOCR = new ImageOperationOCR(img);
    imgOpOCR->ImageOP();

    ImageOperationRectify *imgOpRectify = new ImageOperationRectify(img, stackImage);
    imgOpRectify->ImageOP();


    WatermarkDialog *water = new WatermarkDialog();
    water->fontSizeChangedSlot();
    QTest::keyClick(water, Qt::Key_Return);
    water->getLineEditText();
    water->repaint();

    //runnin
    RunningDialog *run = new RunningDialog(nullptr,"123");
    run->isDarkTheme();
    run->getFileListNum();
    run->setWaitText("text");
    run->disconnectCancelButton();
    run->m_closeButtondisconnect();
    run->showPictures();
    run->hideCancelButton();

    ToolBarWidget *toolbar = new ToolBarWidget;
    toolbar->rotateChangedSlot(false);

    ScanSettingsWidget *scanseting= new ScanSettingsWidget;
    scanseting->updateSettingBtnSlotForInstallDriver();
    scanseting->setScanIconDisable();
    scanseting->updateSettingsForSwitchDevices();
    scanseting->updateSettingsForDetectDevices();

    Navigator *navi = new Navigator;
    navi->showNavigation(QPixmap(":/default-connect-page/detect_fail_dark.svg"));
    navi->getHighLightRegion(QPoint(0,0),QPoint(1,1));
    navi->close();
    qDebug() << "testcase2完成";
}

void Mytest::testCase3()
{
    // 启动应用安装驱动界面
    MainWidget *w = new MainWidget;
    w->show();
    QTimer::singleShot(10000,[=](){
        DisplayWidget *m_displaywidget = w->findChild<DisplayWidget*>();
        SuccessPageWidget *m_successWidget = m_displaywidget->findChild<SuccessPageWidget*>();
        ScanSettingsWidget *m_scanSettingWidge = m_successWidget->findChild<ScanSettingsWidget*>();
        m_scanSettingWidge->showWaittingDialogSlot();
        QTimer::singleShot(5000,[=](){w->close();});
    });

}

void Mytest::testCase4()
{
    // 需要扫描仪有纸，走一遍扫描流程
    MainWidget *w = new MainWidget;
    w->show();
    QTimer::singleShot(10000,[=](){
        DisplayWidget *m_displaywidget = w->findChild<DisplayWidget*>();
        SuccessPageWidget *m_successWidget = m_displaywidget->findChild<SuccessPageWidget*>();
        LeftImageHandleSuccessPageWidget *left = m_successWidget->findChild<LeftImageHandleSuccessPageWidget*>();
        ThumbnailWidget *thumb = left->findChild<ThumbnailWidget*>();
        ScanSettingsWidget *m_scanSettingWidge = m_successWidget->findChild<ScanSettingsWidget*>();
        QComboBox *pageNum = m_scanSettingWidge->findChild<QComboBox*>("m_pageNumberComboBox");
        QComboBox *type = m_scanSettingWidge->findChild<QComboBox*>("m_typeComboBox");
        QComboBox *m_colorComboBox = m_scanSettingWidge->findChild<QComboBox*>("m_colorComboBox");
        QComboBox *m_resolutionComboBox = m_scanSettingWidge->findChild<QComboBox*>("m_resolutionComboBox");
        QComboBox *m_sizeComboBox = m_scanSettingWidge->findChild<QComboBox*>("m_sizeComboBox");
        QComboBox *m_formatComboBox = m_scanSettingWidge->findChild<QComboBox*>("m_formatComboBox");
        QLineEdit *m_saveNameEdit = m_scanSettingWidge->findChild<QLineEdit*>("m_saveNameEdit");
        QLineEdit *m_saveDirectoryButton = m_scanSettingWidge->findChild<QLineEdit*>("m_saveDirectoryButton");
        m_scanSettingWidge->fontSizeChanged();
        pageNum->setCurrentIndex(1);
        type->setCurrentIndex(1);
        m_colorComboBox->setCurrentIndex(1);
        m_colorComboBox->setCurrentIndex(2);
        m_resolutionComboBox->setCurrentIndex(5);
        m_resolutionComboBox->setCurrentIndex(2);
        m_resolutionComboBox->setCurrentIndex(3);
        m_resolutionComboBox->setCurrentIndex(4);
        m_resolutionComboBox->setCurrentIndex(1);
        m_sizeComboBox->setCurrentIndex(1);
        m_sizeComboBox->setCurrentIndex(2);
        m_formatComboBox->setCurrentIndex(1);
        m_formatComboBox->setCurrentIndex(2);
        m_formatComboBox->setCurrentIndex(3);
        m_formatComboBox->setCurrentIndex(4);
        m_formatComboBox->setCurrentIndex(5);
        m_saveNameEdit->setText("11111111111111111中1111111111111111中111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111中");

        //调用开始扫描信号
        emit g_user_signal->exitOcrWhenScanSignal();
        m_scanSettingWidge->scanButtonClickedSlot();

        QTimer::singleShot(5000,[=](){
            LeftImageHandleSuccessPageWidget *left = m_successWidget->findChild<LeftImageHandleSuccessPageWidget*>();
            ShowImageWidget *showimg = left->findChild<ShowImageWidget*>();
            ToolBarWidget *toolbar = left->findChild<ToolBarWidget*>();
            toolbar->beautyButtonClickedSlot();
            QTimer::singleShot(5000,[=](){toolbar->rectifyButtonClickedSlot();});
            QTimer::singleShot(10000,[=](){toolbar->ocrButtonClickedSlot();});
            QTimer::singleShot(15000,[=](){toolbar->ocrButtonClickedSlot();});
//            QTimer::singleShot(20000,[=](){toolbar->cropButtonClickedSlot();toolbar->cropDoubleClickedSlot();showimg->cropCompleteSlot();});
            QTimer::singleShot(25000,[=](){toolbar->rotateButtonClickedSlot();});
            QTimer::singleShot(30000,[=](){toolbar->mirrorButtonClickedSlot();});
            QTimer::singleShot(35000,[=](){toolbar->zoominButtonClickedSlot();});
            QTimer::singleShot(35500,[=](){toolbar->zoominButtonClickedSlot();});
            QTimer::singleShot(36000,[=](){toolbar->zoomoutButtonClickedSlot();});
            QTimer::singleShot(37000,[=](){
                g_user_signal->exitWindowWithSaveFlag = false;
            });

            QTimer::singleShot(38000,[=](){toolbar->rotateChangedSlot(false);});
            QTimer::singleShot(39000,[=](){w->close();});
        });

    });
}

void Mytest::testCase5()
{
    // 需要扫描仪有纸，走一遍扫描流程保存为tiff
    MainWidget *w = new MainWidget;
    w->show();
    QTimer::singleShot(10000,[=](){
        DisplayWidget *m_displaywidget = w->findChild<DisplayWidget*>();
        SuccessPageWidget *m_successWidget = m_displaywidget->findChild<SuccessPageWidget*>();
        ScanSettingsWidget *m_scanSettingWidge = m_successWidget->findChild<ScanSettingsWidget*>();
        QComboBox *pageNum = m_scanSettingWidge->findChild<QComboBox*>("m_pageNumberComboBox");
        QComboBox *type = m_scanSettingWidge->findChild<QComboBox*>("m_typeComboBox");
        QComboBox *m_colorComboBox = m_scanSettingWidge->findChild<QComboBox*>("m_colorComboBox");
        QComboBox *m_resolutionComboBox = m_scanSettingWidge->findChild<QComboBox*>("m_resolutionComboBox");
        QComboBox *m_sizeComboBox = m_scanSettingWidge->findChild<QComboBox*>("m_sizeComboBox");
        QComboBox *m_formatComboBox = m_scanSettingWidge->findChild<QComboBox*>("m_formatComboBox");
        QLineEdit *m_saveNameEdit = m_scanSettingWidge->findChild<QLineEdit*>("m_saveNameEdit");
        QLineEdit *m_saveDirectoryButton = m_scanSettingWidge->findChild<QLineEdit*>("m_saveDirectoryButton");
        m_scanSettingWidge->fontSizeChanged();
        pageNum->setCurrentIndex(1);
        type->setCurrentIndex(1);
        m_colorComboBox->setCurrentIndex(1);
        m_colorComboBox->setCurrentIndex(2);
        m_resolutionComboBox->setCurrentIndex(5);
        m_resolutionComboBox->setCurrentIndex(2);
        m_resolutionComboBox->setCurrentIndex(3);
        m_resolutionComboBox->setCurrentIndex(4);
        m_resolutionComboBox->setCurrentIndex(1);
        m_sizeComboBox->setCurrentIndex(1);
        m_sizeComboBox->setCurrentIndex(2);
        m_formatComboBox->setCurrentIndex(1);
        m_formatComboBox->setCurrentIndex(2);
        m_formatComboBox->setCurrentIndex(3);
        m_formatComboBox->setCurrentIndex(4);
        m_saveNameEdit->setText("1111111112111111中1111111111111111中111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111中");

        //调用开始扫描信号
        emit g_user_signal->exitOcrWhenScanSignal();
        m_scanSettingWidge->scanButtonClickedSlot();

        QTimer::singleShot(5000,[=](){w->close();});
    });
}


void Mytest::testCase6()
{
    // 需要扫描仪有纸，走一遍扫描流程保存为pdf
    MainWidget *w = new MainWidget;
    w->show();
    QTimer::singleShot(10000,[=](){
        DisplayWidget *m_displaywidget = w->findChild<DisplayWidget*>();
        SuccessPageWidget *m_successWidget = m_displaywidget->findChild<SuccessPageWidget*>();
        ScanSettingsWidget *m_scanSettingWidge = m_successWidget->findChild<ScanSettingsWidget*>();
        QComboBox *pageNum = m_scanSettingWidge->findChild<QComboBox*>("m_pageNumberComboBox");
        QComboBox *type = m_scanSettingWidge->findChild<QComboBox*>("m_typeComboBox");
        QComboBox *m_colorComboBox = m_scanSettingWidge->findChild<QComboBox*>("m_colorComboBox");
        QComboBox *m_resolutionComboBox = m_scanSettingWidge->findChild<QComboBox*>("m_resolutionComboBox");
        QComboBox *m_sizeComboBox = m_scanSettingWidge->findChild<QComboBox*>("m_sizeComboBox");
        QComboBox *m_formatComboBox = m_scanSettingWidge->findChild<QComboBox*>("m_formatComboBox");
        QLineEdit *m_saveNameEdit = m_scanSettingWidge->findChild<QLineEdit*>("m_saveNameEdit");
        QLineEdit *m_saveDirectoryButton = m_scanSettingWidge->findChild<QLineEdit*>("m_saveDirectoryButton");
        m_scanSettingWidge->fontSizeChanged();
        pageNum->setCurrentIndex(1);
        type->setCurrentIndex(1);
        m_colorComboBox->setCurrentIndex(1);
        m_colorComboBox->setCurrentIndex(2);
        m_resolutionComboBox->setCurrentIndex(5);
        m_resolutionComboBox->setCurrentIndex(2);
        m_resolutionComboBox->setCurrentIndex(3);
        m_resolutionComboBox->setCurrentIndex(4);
        m_resolutionComboBox->setCurrentIndex(1);
        m_sizeComboBox->setCurrentIndex(1);
        m_sizeComboBox->setCurrentIndex(2);
        m_formatComboBox->setCurrentIndex(1);
        m_formatComboBox->setCurrentIndex(2);

        m_saveNameEdit->setText("111121111111111111中1111111111111111中111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111中");

        //调用开始扫描信号
        emit g_user_signal->exitOcrWhenScanSignal();
        m_scanSettingWidge->scanButtonClickedSlot();
        QTimer::singleShot(5000,[=](){w->close();});
    });
}

void Mytest::testCase7()
{
    // 测试 apt 安装接口
    QWidget *a= new QWidget;
    a->show();
    DeviceInformation dev;
    dev.name = "hp";
    dev.vendor = "hp";
    dev.model = "hp";
    QStringList strlist;
    QString str = "hplip";
    strlist.append(str);
    dev.packageNameList = strlist;
    AptUtilHelper *aptutils = new AptUtilHelper(dev);
    aptutils->onInstalldebStatusChanged(20, "23 ", "12 ");
    aptutils->onRecvApt(true, strlist, "123", "123");
    aptutils->onRecvApt(false, strlist, "123", "123");
    QObject::connect(aptutils, &AptUtilHelper::succeed, [=](){a->close();});
    QObject::connect(aptutils, &AptUtilHelper::failed, [=](){a->close();});
    QObject::connect(aptutils, &AptUtilHelper::destroyed, [=](){a->close();});
    aptutils->startWorker();

}

void Mytest::testCase8()
{
    // 测试 deb 安装接口
    QWidget *a= new QWidget;
    a->show();
    DebUtilHelper *debutils = new DebUtilHelper("hplip");
    debutils->onInstalldebStatusChanged(20,"123", "213");
    debutils->onRecvApt(true, "123", "123");
    debutils->onRecvApt(false, "123", "123");
    QObject::connect(debutils, &DebUtilHelper::succeed, [=](){a->close();});
    QObject::connect(debutils, &DebUtilHelper::failed, [=](){a->close();});
    QObject::connect(debutils, &DebUtilHelper::destroyed, [=](){a->close();});

    debutils->startWorker();
}


void Mytest::testCase9()
{
    // 调用mail页面
    QWidget *a= new QWidget;
    a->show();

    NoMailDialog *nomail = new NoMailDialog;
    nomail->close();

    nomail->closeNoMailWindow();
    nomail->fontSizeChangedSlot();
    //    nomail->installMailApp();

    _getAppList(QString("123123").toLocal8Bit().data());
    getAppIdList(QString("123123").toLocal8Bit().data());

    SendMailDialog *sendmail = new SendMailDialog;
    sendmail->setMailSelectComboboxItems();
    sendmail->closeSendMailWindowSlot();
    sendmail->openSelectMailClientSlot("123");

    a->close();
    exit(0);

}

void Mytest::testCase10()
{
    QWidget *a= new QWidget;
    a->show();

    DeviceInformation dev;
    dev.name = "hp";
    dev.vendor = "hp";
    dev.model = "hp";
    QStringList strlist;
    QString str = "hplip";
    strlist.append(str);
    dev.packageNameList = strlist;
    QList<DeviceInformation> devlist;
    devlist.append(dev);
    buriedNoDriverDevice(dev);

    newDeviceListPage *newdev = new newDeviceListPage(devlist);
    newdev->installDriverSlot();
    newdev->showSuccessDialog();
    newdev->showFailDialog("???");
    newdev->reDetectDevicesSlot();
    //    newdev->nextButtonClickSlot();
    //    newdev->installHandlerSlot();

    a->close();
    exit(0);
}

void Mytest::testCase11()
{
    QWidget *a= new QWidget;
    a->show();

    g_sane_object->refreshListSlots();
//    g_sane_object->getOptionResolutionsRange(75,75);
//    g_sane_object->getOptionResolutionsRange(100,100);
//    g_sane_object->getOptionResolutionsRange(150,150);
//    g_sane_object->getOptionResolutionsRange(200,200);
//    g_sane_object->getOptionResolutionsRange(300,300);
//    g_sane_object->getOptionResolutionsRange(600,600);
//    g_sane_object->getOptionResolutionsRange(1200,1200);
//    g_sane_object->getOptionResolutionsRange(2400,2400);
//    g_sane_object->getOptionResolutionsRange(4800,2400);
    g_sane_object->getSaneResolutionByUser("150 dpi");
    g_sane_object->getSaneResolutionByUser("200 dpi");
    g_sane_object->getSaneResolutionByUser("300 dpi");
    g_sane_object->getSaneResolutionByUser("600 dpi");
    g_sane_object->getSaneResolutionByUser("1200 dpi");
    g_sane_object->getSaneResolutionByUser("2400 dpi");
    g_sane_object->getSaneResolutionByUser("4800 dpi");

    a->close();
    exit(0);
}

void Mytest::testCase12()
{
    // 一些难以跑到的代码
//    MainWidget *w = new MainWidget;
//    w->show();
//    QTimer::singleShot(10000,[=](){
//        w->resizeTitleBar();
//        w->detectScanDeviceThreadFinishedSlot(false);
//        w->onPrepareForSleep(true);
//        w->onPrepareForSleep(false);
//        w->showBeautyRunningDialog();
//        w->hideBeautyRunningDialog();
//        w->showRectifyRunningDialog();
//        w->hideRectifyRunningDialog(false);
//        w->close();
//    });
}

void Mytest::testCase13()
{
    // 发送右键按钮点击
    QWidget *a= new QWidget;
    a->show();

    ScanSettingsWidget *scan = new ScanSettingsWidget;
    QTimer::singleShot(100,[=](){exit(0);});
    scan->sendMailButtonClickedSlot();
    a->close();

}

void Mytest::testCase14()
{
    // 发送右键按钮点击
    QWidget *a= new QWidget;
    a->show();

    ScanSettingsWidget *scan = new ScanSettingsWidget;
    QTimer::singleShot(100,[=](){exit(0);});
    scan->saveAsButtonClickedSlot(false);
    a->close();
}

void Mytest::testCase15()
{
    QWidget *a= new QWidget;
    a->show();
    CropLabel cropLabel;
    cropLabel.testCropLabelEvents();
    a->close();
    exit(0);
}

void Mytest::testCase16()
{

    QPdfWriter *writer = new QPdfWriter("/home/fanyuchen/scanner-v11/kylin-scanner/test/1.pdf");
    ShowImageWidget *imgwidget = new ShowImageWidget;
    imgwidget->setPdfSize(writer, "A");
    imgwidget->setPdfSize(writer, "A0");
    imgwidget->setPdfSize(writer, "A1");
    imgwidget->setPdfSize(writer, "A2");
    imgwidget->setPdfSize(writer, "A3");
    imgwidget->setPdfSize(writer, "A4");
    imgwidget->setPdfSize(writer, "A5");
    imgwidget->setPdfSize(writer, "A6");
    QImage *img = new QImage("/home/fanyuchen/scanner-v11/kylin-scanner/test/a.png");
    const QImage img2("/home/fanyuchen/scanner-v11/kylin-scanner/test/a.png");

    imgwidget->saveToPdf(*img, "/home/fanyuchen/scanner-v11/kylin-scanner/test/1.pdf");
    imgwidget->imageSave("/home/fanyuchen/scanner-v11/kylin-scanner/test/1.pdf");
    imgwidget->pictureDeepen(img2,QSize(1,1),QPoint(1,1));
    imgwidget->setPixmapScaledByProportion(2.5);
    imgwidget->createNavigation();
    imgwidget->showImageAfterClickedThumbnail("/home/fanyuchen/scanner-v11/kylin-scanner/test/a.png");
    imgwidget->rotateSlot();
//    imgwidget->loadImageFinishedSlot("/home/fanyuchen/scanner-v11/kylin-scanner/test/a.png", 20, *img);
    exit(0);
}

void Mytest::testCase17()
{
    ShowImageWidget *showimgwidget = new ShowImageWidget();
    QPdfWriter *pdf = new QPdfWriter("/home/fanyuchen/scanner-v11/kylin-scanner/test/1.pdf");
    showimgwidget->setPdfSize(pdf, "A0");
    showimgwidget->setPdfSize(pdf, "A1");
    showimgwidget->setPdfSize(pdf, "A2");
    showimgwidget->setPdfSize(pdf, "A3");
    showimgwidget->setPdfSize(pdf, "A4");
    showimgwidget->setPdfSize(pdf, "A5");
    showimgwidget->setPdfSize(pdf, "A6");
    showimgwidget->setPdfSize(pdf, "A7");
    QImage img("/home/fanyuchen/scanner-v11/kylin-scanner/test/a.png");

    QSize a(1,1);
    QPoint p(1,1);
    showimgwidget->pictureDeepen(img, a, p);
    showimgwidget->minNumIsZero(0,1);
    showimgwidget->setPixmapScaledByProportion(0.2);
    showimgwidget->drawShadow();
    showimgwidget->createNavigation();
    showimgwidget->clickNavigation(p);
//    showimgwidget->showImageAfterClickedThumbnail("/home/fanyuchen/scanner-v11/kylin-scanner/test/a.png");
    UsbHotplugThread thread;
    QStringList a1;
    a1.append("lsusb");
//    thread.getRetFromCommand(a1);
//    thread.isRemoveScanner("123");
//    thread.getConnectedScanner();
//    thread.getConnectedScanner2();
//    thread.initSane();
//    thread.usbRemoveSlot();
//    thread.usbAddSlot("123");
//    thread.usbDeviceIdentify("123");
//    thread.usbDeviceAdd("123");
//    thread.usbDeviceRemove("123");
    exit(0);
}

void Mytest::testCase18()
{
    qDebug() << "123";
}

static bool uriCheck(const QString &uri, const QString &scheme)
{
    const QString uriSplit = "://";
    if (uri.isEmpty() || scheme.isEmpty()) {
        return false;
    }

    if (!uri.contains(uriSplit)) {
        return false;
    }

    QString uriScheme = uri.left(uri.indexOf(uriSplit));
    if (scheme != uriScheme) {
        return false;
    }

    if (uri.rightRef(uri.size() - uriSplit.size() - uri.indexOf(uriSplit)).isEmpty()) {
        return false;
    }

    return true;
}

