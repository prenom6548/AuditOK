#include <QThread>
#include <QApplication>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>
#include "frame.h"
#include "mainwindow.h"
#include "diagnosis_core.h"
#include "home_page_widget.h"
#include "fault_diagnosis.h"

namespace fault_diagnosis {

KomApplicationInterface *FaultDiagnosisProvider::create() const {
    return new FaultDiagnosis;
}

FaultDiagnosis::FaultDiagnosis()
    : thread_(new QThread)
{

}

FaultDiagnosis::~FaultDiagnosis() {

}

void FaultDiagnosis::init(void (*frame_callback)(const char *function_name, ...)) {
    Frame::setFrameCallback(frame_callback);
}

std::string FaultDiagnosis::name() {
    return "Fault diagnosis";
}

std::string FaultDiagnosis::i18nName() {
    return "故障诊断";
}

std::string FaultDiagnosis::icon() {
    return "ukui-troubleshooting-symbolic";
}

int FaultDiagnosis::sort() {
    return 1;
}

QWidget *FaultDiagnosis::createWidget() {
    qRegisterMetaType<CategoryDiagnosticEntry>();
    qRegisterMetaType<CheckEntry>();
    qRegisterMetaType<CheckEntryResult>();
    qRegisterMetaType<RepairEntry>();
    qRegisterMetaType<RepairEntryResult>();

    auto *translator = new QTranslator;
    if (!translator->load(QLocale(), "fault-diagnosis", "_", "/usr/share/kylin-os-manager/fault-diagnosis/translations")) {
        qCritical() << "Fault diagnosis load translate file fail.";
    } else {
        qApp->installTranslator(translator);
    }

    auto *qt_translator = new QTranslator;
    if (!qt_translator->load(QLocale(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        qCritical() << "Fault diagnosis load qt translate file fail";
    } else {
        qApp->installTranslator(qt_translator);
    }

    auto *sdk_translator = new QTranslator;
    if (!sdk_translator->load(":/translations/gui_" + QLocale::system().name() + ".qm")) {
        qCritical() << "Fault diagnosis load sdk translate file fail";
    } else {
        qApp->installTranslator(sdk_translator);
    }

    auto *stacked_widget = new QStackedWidget;
    auto *home_page = new HomePageWidget;
    auto *mainwindow = new MainWindow;
    stacked_widget->insertWidget(StackedWidget::kHomePage, home_page);
    stacked_widget->insertWidget(StackedWidget::kMainWindow, mainwindow);
    stacked_widget->setCurrentIndex(StackedWidget::kHomePage);
    QObject::connect(home_page, &HomePageWidget::sig_OneClickDetection, [stacked_widget, mainwindow]() {
        mainwindow->on_StartDiagnosis();
        stacked_widget->setCurrentIndex(StackedWidget::kMainWindow);
    });
    QObject::connect(mainwindow, &MainWindow::sig_Return, [stacked_widget]() {
        stacked_widget->setCurrentIndex(StackedWidget::kHomePage);
    });

    auto *diagnosis_core = new DiagnosisCore;
    diagnosis_core->moveToThread(thread_.get());
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, [&]() {
        thread_->quit();
        thread_->wait();
    });
    QObject::connect(thread_.get(), &QThread::finished, diagnosis_core, &QObject::deleteLater);
    QObject::connect(thread_.get(), &QThread::started, mainwindow, &MainWindow::sig_DiagnosticsList);
    QObject::connect(mainwindow, &MainWindow::sig_DiagnosticsList, diagnosis_core, &DiagnosisCore::on_DiagnosticsList);
    QObject::connect(diagnosis_core, &DiagnosisCore::sig_DiagnosticsListFinished, mainwindow, &MainWindow::on_DiagnosticsListFinished);
    QObject::connect(mainwindow, &MainWindow::sig_Check, diagnosis_core, &DiagnosisCore::on_Check);
    QObject::connect(diagnosis_core, &DiagnosisCore::sig_CheckFinished, mainwindow, &MainWindow::on_CheckFinished);
    QObject::connect(mainwindow, &MainWindow::sig_Repair, diagnosis_core, &DiagnosisCore::on_Repair);
    QObject::connect(diagnosis_core, &DiagnosisCore::sig_Fixed, mainwindow, &MainWindow::on_Fixed);
    QObject::connect(diagnosis_core, &DiagnosisCore::sig_RepairFinished, mainwindow, &MainWindow::on_RepairFinished);
    //QObject::connect(diagnosis_core, &DiagnosisCore::sig_CheckProgress, mainwindow, &MainWindow::on_CheckProgress);
    //QObject::connect(diagnosis_core, &DiagnosisCore::sig_RepairProgress, mainwindow, &MainWindow::on_RepairProgress);

    thread_->start();
    return stacked_widget;
}

}
