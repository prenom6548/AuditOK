#include "fault_diagnosis.h"
#include <QApplication>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMessageBox>
#include "frame.h"
#include "mainwindow.h"
#include "diagnosis_core.h"
#include "home_page_widget.h"
#include "repair_specific_error_widget.h"

namespace fault_diagnosis {

KomApplicationInterface *FaultDiagnosisProvider::create() const {
    return new FaultDiagnosis;
}

FaultDiagnosis::FaultDiagnosis()
    : stacked_widget_(nullptr),
      mainwindow_(nullptr),
      thread_(new QThread),
      thread_started_(false),
      start_diagnosis_cache_(false)
{
    diagnosis_type_map_.insert("Network", DiagnosisType::kNetwork);
    diagnosis_type_map_.insert("PerformanceAndComponents", DiagnosisType::kPerformanceAndComponents);
    diagnosis_type_map_.insert("Application", DiagnosisType::kApplication);
    diagnosis_type_map_.insert("SystemUpdate", DiagnosisType::kSystemUpdate);
    diagnosis_type_map_.insert("SoftwareStore", DiagnosisType::kSoftwareStore);
    diagnosis_type_map_.insert("Disk", DiagnosisType::kDisk);
    diagnosis_type_map_.insert("WiredNetwork", DiagnosisType::kWiredNetwork);
    diagnosis_type_map_.insert("WirelessNetwork", DiagnosisType::kWirelessNetwork);

    specific_error_module_map_.insert("Upgrade", SpecificErrorModule::kUpgrade);
    specific_error_module_map_.insert("Software", SpecificErrorModule::kSoftware);
}

FaultDiagnosis::~FaultDiagnosis() {

}

void FaultDiagnosis::init(void (*frame_callback)(const char *function_name, ...)) {
    Frame::setFrameCallback(frame_callback);
}

void FaultDiagnosis::communication(enum KomApplicationInterface::CommunicationType communication_type, QVariant data) {
    switch (communication_type) {
        case CommunicationType::kRepairSpecificError: {
            QString module;
            QString path;
            QString error_code;
            QString error_data;
            const auto data_list = data.toByteArray().split('\n');
            if (data_list.size() > 0) {
                module = QByteArray::fromHex(data_list.at(0));
            }
            if (data_list.size() > 1) {
                path = QByteArray::fromHex(data_list.at(1));
            }
            if (data_list.size() > 2) {
                error_code = QByteArray::fromHex(data_list.at(2));
            }
            if (data_list.size() > 3) {
                error_data = QByteArray::fromHex(data_list.at(3));
            }
            Q_EMIT sig_RepairSpecificError(module, path, error_code, error_data);
            break;
        }
        case CommunicationType::kDiagnosisSpecificType: {
            QString diagnosis_type = QByteArray::fromHex(data.toByteArray());
            Q_EMIT sig_DiagnosisSpecificType(diagnosis_type);
            break;
        }
        case CommunicationType::kUserExperiencePopup: {
            auto *mainwindow_close_event = data.value<QCloseEvent *>();
            if (stacked_widget_->currentIndex() == StackedWidget::kMainWindow) {
                mainwindow_->HandleAppCloseEvent(mainwindow_close_event);
                return;
            }
            mainwindow_close_event->accept();
            break;
        }
        default: {
            break;
        }
    }
}

std::string FaultDiagnosis::name() {
    return "FaultDiagnosis";
}

std::string FaultDiagnosis::i18nName() {
    return QObject::tr("FaultDiagnosis").toStdString();
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
    qRegisterMetaType<DiagnosticResult>();
    qRegisterMetaType<RepairEntry>();
    qRegisterMetaType<RepairEntryResult>();
    qRegisterMetaType<Mode>();
    qRegisterMetaType<DiagnosisType>();

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

    stacked_widget_ = new QStackedWidget;
    mainwindow_ = new MainWindow;
    auto *home_page = new HomePageWidget;
    auto *repair_specific_error = new RepairSpecificErrorWidget;
    stacked_widget_->insertWidget(StackedWidget::kHomePage, home_page);
    stacked_widget_->insertWidget(StackedWidget::kMainWindow, mainwindow_);
    stacked_widget_->insertWidget(StackedWidget::kRepairSpecificError, repair_specific_error);
    stacked_widget_->setCurrentIndex(StackedWidget::kHomePage);

    auto start_diagnosis = [this]() {
        if (mainwindow_->Running()) {
            // 处于运行状态
            QMessageBox::warning(nullptr, tr("Fault Diagnosis"), tr("Currently running, please try again after finishing."));
        } else {
            mainwindow_->on_StartDiagnosis();
        }
    };
    connect(home_page, &HomePageWidget::sig_OneClickDetection, [this, start_diagnosis, home_page]() {
        mainwindow_->SetMode(Mode::kNormalMode);
        if (thread_started_) {
            start_diagnosis();
        } else {
            start_diagnosis_cache_ = true;
        }
        home_page->SetState(HomePageWidget::State::kPreparing);
    });
    connect(home_page, &HomePageWidget::sig_NetworkDetection, this, [this]() {
        Q_EMIT sig_DiagnosisSpecificType("Network");
    });
    connect(home_page, &HomePageWidget::sig_PerformanceAndComponentsDetection, this, [this]() {
        Q_EMIT sig_DiagnosisSpecificType("PerformanceAndComponents");
    });
    connect(home_page, &HomePageWidget::sig_ApplicationDetection, this, [this]() {
        Q_EMIT sig_DiagnosisSpecificType("Application");
    });
    connect(home_page, &HomePageWidget::sig_SystemUpdateDetection, this, [this]() {
        Q_EMIT sig_DiagnosisSpecificType("SystemUpdate");
    });
    connect(home_page, &HomePageWidget::sig_SoftwareStoreDetection, this, [this]() {
        Q_EMIT sig_DiagnosisSpecificType("SoftwareStore");
    });
    connect(home_page, &HomePageWidget::sig_DiskDetection, this, [this]() {
        Q_EMIT sig_DiagnosisSpecificType("Disk");
    });
    connect(home_page, &HomePageWidget::sig_WiredNetworkDetection, this, [this]() {
        Q_EMIT sig_DiagnosisSpecificType("WiredNetwork");
    });
    connect(home_page, &HomePageWidget::sig_WirelessNetworkDetection, this, [this]() {
        Q_EMIT sig_DiagnosisSpecificType("WirelessNetwork");
    });
    connect(mainwindow_, &MainWindow::sig_Return, [this, home_page]() {
        home_page->SetState(HomePageWidget::State::kInit);
        stacked_widget_->setCurrentIndex(StackedWidget::kHomePage);
    });
    connect(mainwindow_, &MainWindow::sig_StartDiagnosis, this, [this](enum fault_diagnosis::Mode mode) {
        switch (mode) {
            case Mode::kRepairSpecificErrorMode:
                stacked_widget_->setCurrentIndex(StackedWidget::kRepairSpecificError);
                break;
            default:
                stacked_widget_->setCurrentIndex(StackedWidget::kMainWindow);
                break;
        }
    });
    connect(this, &FaultDiagnosis::sig_RepairSpecificError, this, [this, start_diagnosis, home_page, repair_specific_error](QString module, QString path, QString error_code, QString error_data) {
        mainwindow_->SetMode(Mode::kRepairSpecificErrorMode);
        auto iter = specific_error_module_map_.find(module);
        if (iter == specific_error_module_map_.end()) {
            QMessageBox::critical(nullptr, "", tr("The module is not supported"));
            return;
        }
        mainwindow_->SetSpecificError(repair_specific_error, iter.value(), path, error_code, error_data);
        repair_specific_error->SetRawData(iter.value(), error_code);
        if (thread_started_) {
            start_diagnosis();
        } else {
            start_diagnosis_cache_ = true;
        }
        home_page->SetState(HomePageWidget::State::kPreparing);
    });
    connect(this, &FaultDiagnosis::sig_DiagnosisSpecificType, this, [this, start_diagnosis, home_page](QString type) {
        mainwindow_->SetMode(Mode::kDiagnosisSpecificTypeMode);
        const auto iter = diagnosis_type_map_.find(type);
        if (iter == diagnosis_type_map_.end()) {
            QMessageBox::critical(nullptr, "", tr("The diagnostic type is not supported"));
            return;
        }
        mainwindow_->SetSpecificDiagnosisType(iter.value());
        if (thread_started_) {
            start_diagnosis();
        } else {
            start_diagnosis_cache_ = true;
        }
        home_page->SetState(HomePageWidget::State::kPreparing);
    });

    // 后端服务
    auto *diagnosis_core = new DiagnosisCore;
    diagnosis_core->moveToThread(thread_);
    connect(thread_, &QThread::finished, diagnosis_core, &QObject::deleteLater);
    connect(thread_, &QThread::started, this, [this, start_diagnosis]() {
        thread_started_ = true;
        if (start_diagnosis_cache_) {
            start_diagnosis();
            start_diagnosis_cache_ = false;
        }
    });
    connect(mainwindow_, &MainWindow::sig_DiagnosticsList, diagnosis_core, &DiagnosisCore::on_DiagnosticsList);
    connect(diagnosis_core, &DiagnosisCore::sig_DiagnosticsListFinished, mainwindow_, &MainWindow::on_DiagnosticsListFinished);
    connect(mainwindow_, &MainWindow::sig_Check, diagnosis_core, &DiagnosisCore::on_Check);
    connect(diagnosis_core, &DiagnosisCore::sig_Checked, mainwindow_, &MainWindow::on_Checked);
    connect(diagnosis_core, &DiagnosisCore::sig_CheckFinished, mainwindow_, &MainWindow::on_CheckFinished);
    connect(mainwindow_, &MainWindow::sig_Repair, diagnosis_core, &DiagnosisCore::on_Repair);
    connect(diagnosis_core, &DiagnosisCore::sig_Fixed, mainwindow_, &MainWindow::on_Fixed);
    connect(diagnosis_core, &DiagnosisCore::sig_RepairFinished, mainwindow_, &MainWindow::on_RepairFinished);
    //connect(diagnosis_core, &DiagnosisCore::sig_CheckProgress, mainwindow, &MainWindow::on_CheckProgress);
    //connect(diagnosis_core, &DiagnosisCore::sig_RepairProgress, mainwindow, &MainWindow::on_RepairProgress);

    thread_->start();
    return stacked_widget_;
}

}
