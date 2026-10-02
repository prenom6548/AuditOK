#include "mainwindow.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <QHBoxLayout>
#include <QDebug>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QMessageBox>
#include "frame.h"
#include "kom-buriedpoint.h"
#include "user_experience_dialog.h"
#include "diagnosis_subentry_widget.h"

namespace fault_diagnosis {

struct LessThan {
    QHash<QString, int> category_order = {
            {"网络检测", 0},
            {"系统组件与性能检测", 1},
            {"应用程序检测", 2}
    };

    // 升序排序，如果第一个参数小于第二个参数，返回 true
    bool operator()(const QString &a, const QString &b) {
        int a_a = -1, b_b = -1;

        auto a_iter = category_order.find(a);
        if (a_iter != category_order.end()) {
            a_a = a_iter.value();
        }

        auto b_iter = category_order.find(b);
        if (b_iter != category_order.end()) {
            b_b = b_iter.value();
        }

        if (a_a != -1 && b_b != -1) {
            return a_a < b_b;
        }
        if (a_a == -1 && b_b != -1) {
            return true;
        }
        if (a_a != -1 && b_b == -1) {
            return false;
        }

        return a < b;
    }
} less_than;

MainWindow::MainWindow()
    : status_icon_btn_(new QPushButton(this)),
      status_describe_label_(new K::TextLabel(this)),
      status_detail_label_(new K::TextLabel(this)),
      cancel_btn_(new QPushButton(this)),
      return_btn_(new QPushButton(this)),
      diagnosis_btn_(new QPushButton(this)),
      repair_btn_(new QPushButton(this)),
      progress_bar_(new kdk::KProgressBar(this)),
      diagnosis_information_widget_(new QTreeWidget(this)),
      diagnosed_count_(0),
      diagnostic_error_count_(0),
      diagnosed_success_count_(0),
      diagnosed_fail_count_(0),
      error_count_(0),
      need_repair_total_(0),
      fixed_count_(0),
      fixed_success_count_(0),
      fixed_fail_count_(0),
      status_(MainWindow::Status::kNoStatus),
      mode_(Mode::kNormalMode),
      diagnosis_type_(DiagnosisType::kNone),
      kSystemVersion(SystemVersion()),
      kSystemArch(SystemArch()),
      specific_error_module_(SpecificErrorModule::kNone),
      specific_error_widget_(nullptr)
{
    setObjectName("fault_diagnosis_mainwindow");
    // 头部
    status_icon_btn_->setFixedSize(64, 64);
    status_icon_btn_->setIconSize(QSize(64, 64));
    status_icon_btn_->setIcon(QIcon(":/fault_diagnosis/data/icons/normal.svg"));
    status_icon_btn_->setStyleSheet(PUSHBUTTON_ICON_STYLE);

    status_describe_label_->SetPixelSize(24, true);
    auto *status_label_vbox_layout_ = new QVBoxLayout;
    status_label_vbox_layout_->setSpacing(0);
    status_label_vbox_layout_->setContentsMargins(0, 0, 0, 0);
    status_label_vbox_layout_->addWidget(status_describe_label_);
    status_label_vbox_layout_->addWidget(status_detail_label_);

    cancel_btn_->setText(tr("Cancel"));
    return_btn_->setText(tr("Return"));
    auto *head_hbox_layout = new QHBoxLayout;
    head_hbox_layout->setSpacing(0);
    head_hbox_layout->setContentsMargins(40, 0, 40, 0);
    head_hbox_layout->addWidget(status_icon_btn_);
    head_hbox_layout->addSpacing(16);
    head_hbox_layout->addLayout(status_label_vbox_layout_);
    head_hbox_layout->addStretch();
    head_hbox_layout->addSpacing(10);
    head_hbox_layout->addWidget(return_btn_);
    head_hbox_layout->addSpacing(10);
    head_hbox_layout->addWidget(cancel_btn_);
    head_hbox_layout->addWidget(diagnosis_btn_);
    head_hbox_layout->addWidget(repair_btn_);
    connect(cancel_btn_, &QPushButton::clicked, this, &MainWindow::on_Cancel);
    connect(return_btn_, &QPushButton::clicked, this, &MainWindow::sig_Return);
    connect(diagnosis_btn_, &QPushButton::clicked, this, &MainWindow::on_StartDiagnosis);
    connect(repair_btn_, &QPushButton::clicked, this, &MainWindow::on_StartRepair);

    // 进度条
    progress_bar_->setFixedHeight(4);
    progress_bar_->setOrientation(Qt::Horizontal);
    progress_bar_->setTextVisible(false);
    progress_bar_->setRange(0, 100);
    progress_bar_->setValue(0);

    // 核心区域
    diagnosis_information_widget_->setFrameStyle(QFrame::NoFrame);
    diagnosis_information_widget_->setColumnCount(1);
    diagnosis_information_widget_->setIndentation(0);
    diagnosis_information_widget_->header()->setVisible(false);
    diagnosis_information_widget_->setStyleSheet("QTreeWidget::item:selected { background-color: transparent; }"
                                                 "QTreeWidget::item:hover { background-color: transparent; }");

    // 总布局
    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setSpacing(0);
    vbox_layout->setContentsMargins(0, 24, 0, 0);
    vbox_layout->addLayout(head_hbox_layout);
    vbox_layout->addSpacing(23);
    vbox_layout->addWidget(progress_bar_);
    vbox_layout->addSpacing(16);
    vbox_layout->addWidget(diagnosis_information_widget_);
    setLayout(vbox_layout);
}

MainWindow::~MainWindow() {

}

void MainWindow::on_DiagnosticsListFinished(fault_diagnosis::CategoryDiagnosticEntry category_diagnostic_entrys) {
    category_diagnostic_entrys_ = category_diagnostic_entrys;

    auto categorys = category_diagnostic_entrys.keys();
    qSort(categorys.begin(), categorys.end(), less_than);
    for (const auto &category: categorys) {
        auto diagnosis_entrys = category_diagnostic_entrys[category];
        if (diagnosis_entrys.isEmpty()) {
            continue;
        }
        auto *diagnosis_group_item = new QTreeWidgetItem(diagnosis_information_widget_);
        auto *diagnosis_group_widget = new DiagnosisGroupWidget(this);
        diagnosis_group_widget->SetIcon(CategoryIcon(category));
        diagnosis_group_widget->SetCategory(category);
        diagnosis_group_widget->SetTotal(0);
        connect(diagnosis_group_widget, &DiagnosisGroupWidget::sig_Expanded, this, &MainWindow::on_Expanded);
        diagnosis_information_widget_->setItemWidget(diagnosis_group_item, 0, diagnosis_group_widget);

        for (const auto &entry: diagnosis_entrys) {
            auto *diagnosis_entry_item = new QTreeWidgetItem(diagnosis_group_item);
            auto *diagnosis_entry_widget = new DiagnosisEntryWidget(entry, this);
            connect(diagnosis_entry_widget, &DiagnosisEntryWidget::sig_Expanded, this, &MainWindow::on_Expanded);
            connect(diagnosis_entry_widget, &DiagnosisEntryWidget::sig_SelectChange, this, &MainWindow::on_DiagnosisEntryWidgetSelectChange);
            diagnosis_information_widget_->setItemWidget(diagnosis_entry_item, 0, diagnosis_entry_widget);
            // 判断是否存在三级检测项
            if (!entry.item.subitem.empty()) {
                for (const auto &subentry: entry.item.subitem) {
                    auto *diagnosis_subentry_item = new QTreeWidgetItem(diagnosis_entry_item);
                    auto *diagnosis_subentry_widget = new DiagnosisSubentryWidget(subentry, this);
                    connect(diagnosis_subentry_widget, &DiagnosisSubentryWidget::sig_SelectChange, this, &MainWindow::on_SubitemEntryWidgetSelectChange);
                    diagnosis_information_widget_->setItemWidget(diagnosis_subentry_item, 0, diagnosis_subentry_widget);
                }
            }
        }
    }
    // QTreeWidget 的最后一个顶级 item，会稍微靠左，对不齐
    // 不知是 Qt 的 BUG 还是什么原因，此处多加一个空白 item，来规避该问题
    diagnosis_information_widget_->addTopLevelItem(new QTreeWidgetItem(diagnosis_information_widget_));

    // 实际开始检测
    // 收集诊断项
    need_diagnosis_.clear();
    need_diagnosis_ = category_diagnostic_entrys_;
    for (auto iter = need_diagnosis_.cbegin(); iter != need_diagnosis_.cend(); iter++) {
        auto &base_diagnosis_entrys = iter.value();
        for (const auto &entry: base_diagnosis_entrys) {
            if (!entry.item.subitem.empty()) {
                need_diagnosis_total_ += entry.item.subitem.size();
            } else {
                ++need_diagnosis_total_;
            }
        }
    }

    // 修改界面
    SwitchToDiagnosing();

    Q_EMIT sig_StartDiagnosis(mode_);

    NextDiagnosis();
}

void MainWindow::on_StartDiagnosis() {
    // 还原
    need_diagnosis_total_ = 0;
    diagnosed_count_ = 0;
    diagnostic_error_count_ = 0;
    diagnosis_group_diagnosed_count_ = 0;
    diagnosed_success_count_ = 0;
    diagnosed_fail_count_ = 0;
    cancel_ = false;
    Reset();

    switch (mode_) {
        case Mode::kDiagnosisSpecificTypeMode: {
            Q_EMIT sig_DiagnosticsList(mode_, QVariant::fromValue(diagnosis_type_));
            break;
        }
        case Mode::kRepairSpecificErrorMode: {
            if (specific_error_widget_ != nullptr) {
                switch (specific_error_module_) {
                    case SpecificErrorModule::kUpgrade:
                        specific_error_widget_->SetTitle(tr("Update detection"));
                        break;
                    case SpecificErrorModule::kSoftware:
                        specific_error_widget_->SetTitle(tr("Application detection"));
                        break;
                    default:
                        specific_error_widget_->SetTitle("");
                        break;
                }
                specific_error_widget_->SwitchRunning();
            }
            Q_EMIT sig_StartDiagnosis(mode_);

            // 错误码转换：将升级和软件商店的错误码转换成故障诊断后端所使用的错误码
            // 系统升级：0x11_0001_02_00000101
            // 软件商店：0x10_0001_02_00000001
            std::stringstream ss;
            switch (specific_error_module_) {
                case SpecificErrorModule::kUpgrade:
                    ss << "0x" << "11" << "0001" << "02";
                    break;
                case SpecificErrorModule::kSoftware:
                    ss << "0x" << "10" << "0001" << "02";
                    break;
                default:
                    ss << "0x" << "00" << "0000" << "00";
                    break;
            }
            ss << std::setfill('0') << std::setw(8) << specific_error_code_.toStdString();

            CheckEntry check_entry;
            check_entry.diagnosis_mark = "specific_error";
            check_entry.entry_mark = QString::fromStdString(ss.str());
            check_entry.data = specific_error_data_;
            Q_EMIT sig_Check(check_entry);
            break;
        }
        default: {
            Q_EMIT sig_DiagnosticsList(Mode::kNormalMode);
            break;
        }
    }
}

void MainWindow::SwitchToDiagnosing() {
    status_ = MainWindow::Status::kDiagnosising;
    status_icon_btn_->setIcon(QIcon(":/fault_diagnosis/data/icons/normal.svg"));
    status_describe_label_->SetText(tr("Detecting..."));
    UpdateStatusDetailLabel();
    cancel_btn_->show();
    return_btn_->hide();
    diagnosis_btn_->hide();
    repair_btn_->hide();
    progress_bar_->setValue(0);
}

void MainWindow::SwitchToDiagnosisCanceling() {
    status_ = MainWindow::Status::kDiagnosisCanceling;
    status_icon_btn_->setIcon(QIcon(":/fault_diagnosis/data/icons/normal.svg"));
    status_describe_label_->SetText(tr("Canceling in progress..."));
    UpdateStatusDetailLabel();
    cancel_btn_->show();
    return_btn_->hide();
    diagnosis_btn_->hide();
    repair_btn_->hide();
}

void MainWindow::SwitchToDiagnosisCancelComplete() {
    status_ = MainWindow::Status::kDiagnosisCancelComplete;
    if (diagnostic_error_count_ == 0) {
        status_icon_btn_->setIcon(QIcon(":/fault_diagnosis/data/icons/success.svg"));
        status_describe_label_->SetText(tr("Detection interruption, no issues found"));
    } else {
        status_icon_btn_->setIcon(QIcon(":/fault_diagnosis/data/icons/error.svg"));
        status_describe_label_->SetText(tr("Detected interruption and found %1 issues").arg(diagnostic_error_count_));
    }
    UpdateStatusDetailLabel();
    cancel_btn_->hide();
    return_btn_->show();
    diagnosis_btn_->setText(tr("Rediagnosis"));
    diagnosis_btn_->show();
    repair_btn_->hide();
}

void MainWindow::SwitchToDiagnosisComplete() {
    status_ = MainWindow::Status::kDiagnosisComplete;
    cancel_btn_->hide();
    return_btn_->show();
    if (diagnostic_error_count_ == 0) {
        status_icon_btn_->setIcon(QIcon(":/fault_diagnosis/data/icons/success.svg"));
        status_describe_label_->SetText(tr("No issues fount"));
        diagnosis_btn_->setText(tr("Rediagnosis"));
        diagnosis_btn_->show();
        repair_btn_->hide();
    } else {
        status_icon_btn_->setIcon(QIcon(":/fault_diagnosis/data/icons/error.svg"));
        status_describe_label_->SetText(tr("Found %1 issues").arg(diagnostic_error_count_));
        diagnosis_btn_->hide();
        repair_btn_->setText(tr("One click repair"));
        repair_btn_->show();
    }
    UpdateStatusDetailLabel();
    progress_bar_->setValue(100);
}

void MainWindow::SwitchToRepairing() {
    status_ = MainWindow::Status::kRepairing;
    status_icon_btn_->setIcon(QIcon(":/fault_diagnosis/data/icons/normal.svg"));
    status_describe_label_->SetText(tr("Repairing..."));
    UpdateStatusDetailLabel();
    cancel_btn_->show();
    return_btn_->hide();
    diagnosis_btn_->hide();
    repair_btn_->hide();
}

void MainWindow::SwitchToRepairCanceling() {
    status_ = MainWindow::Status::kRepairCanceling;
    status_icon_btn_->setIcon(QIcon(":/fault_diagnosis/data/icons/normal.svg"));
    status_describe_label_->SetText(tr("Canceling in progress..."));
    UpdateStatusDetailLabel();
    cancel_btn_->show();
    return_btn_->hide();
    diagnosis_btn_->hide();
    repair_btn_->hide();
}

void MainWindow::SwitchToRepairCancelComplete() {
    status_ = MainWindow::Status::kRepairCancelComplete;
    if (error_count_ > 0) {
        status_icon_btn_->setIcon(QIcon(":/fault_diagnosis/data/icons/error.svg"));
        status_describe_label_->SetText(tr("Repair failed"));
        repair_btn_->setText(tr("Repair"));
        repair_btn_->show();
    } else {
        status_icon_btn_->setIcon(QIcon(":/fault_diagnosis/data/icons/success.svg"));
        status_describe_label_->SetText(tr("Repair successful"));
        repair_btn_->hide();
    }
    UpdateStatusDetailLabel();
    cancel_btn_->hide();
    return_btn_->show();
    diagnosis_btn_->hide();
}

void MainWindow::SwitchToRepairComplete() {
    status_ = MainWindow::Status::kRepairComplete;
    cancel_btn_->hide();
    return_btn_->show();
    diagnosis_btn_->hide();
    if (error_count_ > 0) {
        status_icon_btn_->setIcon(QIcon(":/fault_diagnosis/data/icons/error.svg"));
        status_describe_label_->SetText(tr("Repair failed"));
        repair_btn_->setText(tr("Repair"));
        repair_btn_->show();
    } else {
        status_icon_btn_->setIcon(QIcon(":/fault_diagnosis/data/icons/success.svg"));
        status_describe_label_->SetText(tr("Repair successful"));
        repair_btn_->hide();
    }
    UpdateStatusDetailLabel();
    progress_bar_->setValue(100);
}

void MainWindow::UpdateStatusDetailLabel() {
    switch (status_) {
    case MainWindow::Status::kDiagnosising:
        if (!diagnostic_error_count_) {
            status_detail_label_->SetText(tr("%1 items have been tested, no issues found").arg(diagnosed_count_));
        } else {
            status_detail_label_->SetText(tr("%1 items have been tested and %2 issues have been found").arg(diagnosed_count_).arg(diagnostic_error_count_));
        }
        break;
    case MainWindow::Status::kDiagnosisCanceling:
        break;
    case MainWindow::Status::kDiagnosisCancelComplete:
        if (!diagnostic_error_count_) {
            status_detail_label_->SetText(tr("I suggest you conduct a complete inspection again"));
        } else {
            status_detail_label_->SetText(tr("Please repair and retest"));
        }
        break;
    case MainWindow::Status::kDiagnosisComplete:
        if (!diagnostic_error_count_) {
            status_detail_label_->SetText(tr("Please continue to maintain and regularly undergo physical examinations"));
        } else {
            status_detail_label_->SetText(tr("The scan has been completed. if there are any issues, please handle them as soon as possible"));
        }
        break;
    case MainWindow::Status::kRepairing:
        status_detail_label_->SetText(tr("Please wait for the repair to complete"));
        break;
    case MainWindow::Status::kRepairCanceling:
        break;
    case MainWindow::Status::kRepairCancelComplete:
        [[fallthrough]];
    case MainWindow::Status::kRepairComplete:
        if (error_count_ > 0) {
            status_detail_label_->SetText(tr("There is still %1 issue that has not been successfully fixed. it is recommended to manually fix it").arg(error_count_));
        } else {
            status_detail_label_->SetText(tr("Regular physical examinations can effectively improve the user experience"));
        }
        break;
    default:
        break;
    }
}

void MainWindow::NextDiagnosis() {
    QList<QString> erase_keys;

    auto categorys = need_diagnosis_.keys();
    qSort(categorys.begin(), categorys.end(), less_than);
    for (const auto &category: categorys) {
        const auto diagnosis_entry = need_diagnosis_[category];
        if (!diagnosis_entry.empty()) {
            // 判断是否需要暂停
            if (cancel_) {
                qInfo() << "Diagnosis cancel.";
                SwitchToDiagnosisCancelComplete();
                break;
            }

            CheckEntry check_entry;
            check_entry.diagnosis_mark = diagnosis_entry.front().diagnosis_mark;
            check_entry.entry_mark = diagnosis_entry.front().item.base.mark;
            if (mode_ == Mode::kRepairSpecificErrorMode) {
                check_entry.data = specific_error_data_;
            }

            // 检测哪个分类需要展开该分类
            auto *diagnosis_group_widget = GetDiagnosisGroupWidget(category);
            if (diagnosis_group_widget != nullptr) {
                diagnosis_group_widget->SetExpanded(true);
            }
            // 如果基础检测项有子检测项，还需要展开基础检测项，同时设置状态为检测中
            auto diagnosis_entry_pointer = GetDiagnosisEntryWithCategory(category, check_entry.diagnosis_mark, check_entry.entry_mark);
            auto *diagnosis_entry_item = diagnosis_entry_pointer.first;
            auto *diagnosis_entry_widget = diagnosis_entry_pointer.second;
            if (diagnosis_entry_item != nullptr && diagnosis_entry_widget != nullptr) {
                diagnosis_entry_widget->SetExpanded(true);
                diagnosis_entry_widget->SetStatus(DiagnosisEntryWidget::Status::kDiagnosising);

                auto subitem_count = diagnosis_entry_item->childCount();
                for (int i = 0; i < subitem_count; i++) {
                    auto subitem_item = diagnosis_entry_item->child(i);
                    auto subitem_widget = static_cast<DiagnosisSubentryWidget *>(diagnosis_information_widget_->itemWidget(subitem_item, 0));
                    subitem_widget->SetStatus(DiagnosisSubentryWidget::Status::kDiagnosising);
                }
            }

            Q_EMIT sig_Check(check_entry);
            break;
        } else {
            // 该分类下所有项已经检测完毕，从 need_diagnosis_ 中移除该项
            erase_keys.push_back(category);

            // 复原记录诊断组已诊断的数量
            diagnosis_group_diagnosed_count_ = 0;

            // 判断该分类是否需要合并
            auto *diagnosis_group_widget = GetDiagnosisGroupWidget(category);
            if (!CategoryExistError(category) && diagnosis_group_widget != nullptr) {
                diagnosis_group_widget->SetExpanded(false);
            }
        }
    }

    // 清除诊断项为空的项
    for (const auto &k: erase_keys) {
        need_diagnosis_.remove(k);
    }

    if (need_diagnosis_.empty()) {
        SwitchToDiagnosisComplete();

        // 展开所有有错误的项
        auto top_level_item_count = diagnosis_information_widget_->topLevelItemCount();
        for (int i = 0; i < top_level_item_count - 1; i++) {
            bool have_error = false;
            auto *diagnosis_group_item = diagnosis_information_widget_->topLevelItem(i);
            auto *diagnosis_group_widget = static_cast<DiagnosisGroupWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_group_item, 0));
            auto diagnosis_entry_count = diagnosis_group_item->childCount();
            for (int j = 0; j < diagnosis_entry_count; j++) {
                auto *diagnosis_entry_item = diagnosis_group_item->child(j);
                auto *diagnosis_entry_widget = static_cast<DiagnosisEntryWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_entry_item, 0));
                auto subitem_count = diagnosis_entry_item->childCount();
                if (!subitem_count) {
                    // 没有子检测项
                    if (diagnosis_entry_widget->ExistError()) {
                        have_error = true;
                    }
                } else {
                    // 存在子检测项
                    for (int m = 0; m < subitem_count; m++) {
                        auto *subitem_item = diagnosis_entry_item->child(m);
                        auto *subitem_widget = static_cast<DiagnosisSubentryWidget *>(diagnosis_information_widget_->itemWidget(subitem_item, 0));
                        if (subitem_widget->ExistError()) {
                            diagnosis_entry_widget->SetExpanded(true);
                            have_error = true;
                        }
                        break;
                    }
                }
            }
            if (have_error) {
                diagnosis_group_widget->SetExpanded(true);
            }
        }

        // 埋点
        QMap<QString, QString> custom_property = {
            {"systemVersion", kSystemVersion},
            {"systemArch", kSystemArch},
            {"totalNumber", QString::number(need_diagnosis_total_)},
            {"numberOfSuccessful", QString::number(diagnosed_success_count_)},
            {"numberOfFailed", QString::number(diagnosed_fail_count_)}
        };
        kom::BuriedPoint::uploadMessage(kom::BuriedPointPage::kFaultDiagnosis,
                                        "fault_diagnosis",
                                        custom_property);

        qDebug() << "Diagnosis complete";
    }
}

std::tuple<QString, QTreeWidgetItem *, DiagnosisEntryWidget *>
MainWindow::GetDiagnosisEntry(const QString &diagnosis_mark, const QString &diagnosis_entry_mark) {
    auto top_level_item_count = diagnosis_information_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count - 1; i++) {
        auto diagnosis_group_item = diagnosis_information_widget_->topLevelItem(i);
        auto *diagnosis_group_widget = static_cast<DiagnosisGroupWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_group_item, 0));
        auto diagnosis_entry_count = diagnosis_group_item->childCount();
        for (int j = 0; j < diagnosis_entry_count; j++) {
            auto *diagnosis_entry_item = diagnosis_group_item->child(j);
            auto *diagnosis_entry_widget = static_cast<DiagnosisEntryWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_entry_item, 0));
            if (diagnosis_entry_widget->DiagnosisMark() == diagnosis_mark &&
                diagnosis_entry_widget->Mark() == diagnosis_entry_mark)
            {
                return std::make_tuple(diagnosis_group_widget->Category(), diagnosis_entry_item, diagnosis_entry_widget);
            }
        }
    }

    qCritical() << "No DiagnosisEntryWidget pointer found";
    return std::make_tuple("", nullptr, nullptr);
}

QPair<QTreeWidgetItem *, DiagnosisEntryWidget *>
MainWindow::GetDiagnosisEntryWithCategory(const QString &category, const QString &diagnosis_mark, const QString &diagnosis_entry_mark) {
    auto top_level_item_count = diagnosis_information_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count - 1; i++) {
        auto *diagnosis_group_item = diagnosis_information_widget_->topLevelItem(i);
        auto *diagnosis_group_widget = static_cast<DiagnosisGroupWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_group_item, 0));
        if (diagnosis_group_widget->Category() == category) {
            auto diagnosis_entry_count = diagnosis_group_item->childCount();
            for (int j = 0; j < diagnosis_entry_count; j++) {
                auto *diagnosis_entry_item = diagnosis_group_item->child(j);
                auto *diagnosis_entry_widget = static_cast<DiagnosisEntryWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_entry_item, 0));
                if (diagnosis_entry_widget->DiagnosisMark() == diagnosis_mark &&
                    diagnosis_entry_widget->Mark() == diagnosis_entry_mark)
                {
                    return {diagnosis_entry_item, diagnosis_entry_widget};
                }
            }
        }
    }

    qCritical() << "No DiagnosisEntryWidget pointer found by Category";
    return {nullptr, nullptr};
}

DiagnosisGroupWidget *MainWindow::GetDiagnosisGroupWidget(const QString &category) {
    auto top_level_item_count = diagnosis_information_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count - 1; i++) {
        auto *diagnosis_group_item = diagnosis_information_widget_->topLevelItem(i);
        auto *diagnosis_group_widget = static_cast<DiagnosisGroupWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_group_item, 0));
        if (diagnosis_group_widget->Category() == category) {
            return diagnosis_group_widget;
        }
    }

    qCritical() << "No DiagnosisGroupWidget pointer found";
    return nullptr;
}

bool MainWindow::CategoryExistError(const QString &category) {
    auto top_level_item_count = diagnosis_information_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count - 1; i++) {
        auto *diagnosis_group_item = diagnosis_information_widget_->topLevelItem(i);
        auto *diagnosis_group_widget = static_cast<DiagnosisGroupWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_group_item, 0));
        if (diagnosis_group_widget->Category() == category) {
            auto diagnosis_entry_count = diagnosis_group_item->childCount();
            for (int j = 0; j < diagnosis_entry_count; j++) {
                auto *diagnosis_entry_item = diagnosis_group_item->child(j);
                auto *diagnosis_entry_widget = static_cast<DiagnosisEntryWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_entry_item, 0));
                auto subitem_count = diagnosis_entry_item->childCount();
                if (!subitem_count) {
                    if (diagnosis_entry_widget->ExistError()) {
                        return true;
                    }
                } else {
                    for (int m = 0; m < subitem_count; m++) {
                        auto *subitem_item = diagnosis_entry_item->child(m);
                        auto *subitem_widget = static_cast<DiagnosisSubentryWidget *>(diagnosis_information_widget_->itemWidget(subitem_item, 0));
                        if (subitem_widget->ExistError()) {
                            return true;
                        }
                    }
                }
            }
            return false;
        }
    }

    return false;
}

void MainWindow::Reset() {
    // QTreeWidget 删除同级别子项时，索引会发生变化，此处从后往前删
    auto top_level_item_count = diagnosis_information_widget_->topLevelItemCount();
    for (int i = top_level_item_count - 1; i >= 0; i--) {
        auto *diagnosis_group_item = diagnosis_information_widget_->topLevelItem(i);
        auto *diagnosis_group_widget = static_cast<DiagnosisGroupWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_group_item, 0));

        auto diagnosis_entry_count = diagnosis_group_item->childCount();
        for(int j = diagnosis_entry_count - 1; j >= 0; j--) {
            auto *diagnosis_entry_item = diagnosis_group_item->child(j);
            auto *diagnosis_entry_widget = static_cast<DiagnosisEntryWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_entry_item, 0));

            // 删除所有的子检测项
            auto subitem_entry_count = diagnosis_entry_item->childCount();
            for (int m = subitem_entry_count - 1; m >= 0; m--) {
                auto *subitem_entry_item = diagnosis_entry_item->child(m);
                auto *subitem_entry_widget = static_cast<DiagnosisSubentryWidget *>(diagnosis_information_widget_->itemWidget(subitem_entry_item, 0));
                auto *subitem_entry_item_remove = diagnosis_entry_item->takeChild(m);
                if (subitem_entry_item_remove) {
                    delete subitem_entry_item_remove;
                }
                if (subitem_entry_widget) {
                    delete subitem_entry_widget;
                }
            }

            // 删除诊断项
            auto *diagnosis_entry_item_remove = diagnosis_group_item->takeChild(j);
            if (diagnosis_entry_item_remove) {
                delete diagnosis_entry_item_remove;
            }
            if (diagnosis_entry_widget) {
                delete diagnosis_entry_widget;
            }
        }

        // 删除诊断组
        auto *diagnosis_group_item_remove = diagnosis_information_widget_->takeTopLevelItem(i);
        if (diagnosis_group_item_remove) {
            delete diagnosis_group_item_remove;
        }
        if (diagnosis_group_widget) {
            delete diagnosis_group_widget;
        }
    }
}

void MainWindow::HandleSpecificErrorChecked(const DiagnosticResult &diagnostic_result) {
    // 目前只考虑只有一个错误码的情况
    if (diagnostic_result.specific_errors.isEmpty()) {
        qCritical() << "Specific error check result is empty";
        return;
    }
    auto result = diagnostic_result.specific_errors.front();
    if (result.can_repair == "0") {
        specific_error_widget_->SetState(RepairSpecificErrorWidget::State::kRepairing);
        on_SpecificErrorRepair(result.error_code);
    }

    RepairSpecificErrorWidget::ShowContent show_content;
    show_content.error_code = result.error_code;
    show_content.report_path = specific_error_report_path_;
    if (result.can_repair == "1") {
        show_content.ok = false;
        show_content.desc = result.desc;
        show_content.report_issues = true;
        specific_error_widget_->SwitchResult(show_content);
        return;
    }
    if (result.can_repair == "2") {
        show_content.ok = false;
        show_content.desc = result.desc;
        if (result.network_state == "1") {
            show_content.network_state = RepairSpecificErrorWidget::NetworkState::kNormal;
        } else if (result.network_state == "2") {
            show_content.network_state = RepairSpecificErrorWidget::NetworkState::kException;
        }
        show_content.report_issues = true;
        specific_error_widget_->SwitchResult(show_content);
        return;
    }
    if (result.can_repair == "3") {
        // 垃圾清理
        show_content.ok = false;
        show_content.desc = result.desc;
        show_content.report_issues = true;
        specific_error_widget_->SwitchResult(show_content);
        return;
    }
    if (result.can_repair == "4") {
        show_content.ok = false;
        show_content.desc = result.desc;
        if (result.network_state == "1") {
            show_content.network_state = RepairSpecificErrorWidget::NetworkState::kNormal;
        } else if (result.network_state == "2") {
            show_content.network_state = RepairSpecificErrorWidget::NetworkState::kException;
        }
        show_content.upgrade_run = true;
        show_content.report_issues = true;
        specific_error_widget_->SwitchResult(show_content);
        return;
    }
    if (result.can_repair == "5") {
        show_content.ok = false;
        show_content.desc = result.desc;
        show_content.software_run = true;
        show_content.report_issues = true;
        specific_error_widget_->SwitchResult(show_content);
        return;
    }
}

void MainWindow::HandleSpecificErrorCheckFinished(const QString entry_mark, bool ok) {
    if (specific_error_widget_->State() == RepairSpecificErrorWidget::State::kDiagnosising) {
        // 未接收到检测结果的信号
        RepairSpecificErrorWidget::ShowContent show_content;
        show_content.report_path = specific_error_report_path_;
        if (ok) {
            show_content.ok = true;
            show_content.desc = tr("Repair successful");
            specific_error_widget_->SwitchResult(show_content);
        } else {
            show_content.ok = false;
            show_content.desc = tr("Repair failed");
            show_content.report_issues = true;
            specific_error_widget_->SwitchResult(show_content);
        }
    }
}

// 只处理可以具体确定的项
void MainWindow::on_Checked(QString diagnosis_mark, fault_diagnosis::DiagnosticResult diagnostic_result) {
    if (diagnosis_mark == "specific_error") {
        HandleSpecificErrorChecked(diagnostic_result);
        return;
    }

    auto diagnosis_entry = GetDiagnosisEntryWithCategory(diagnostic_result.category, diagnosis_mark, diagnostic_result.base_entry_mark);
    auto *diagnosis_entry_item = diagnosis_entry.first;
    auto *diagnosis_entry_widget = diagnosis_entry.second;
    if (diagnosis_entry_item == nullptr || diagnosis_entry_widget == nullptr) {
        return;
    }

    DescribeGuard describe_guard(*this, diagnostic_result.category);

    if (diagnostic_result.error_code.isEmpty() && diagnostic_result.details.empty()) {
        auto subitem_count = diagnosis_entry_item->childCount();
        if (!subitem_count) {
            // 没有子检测项，则可以明确此检测结果为主检测项的检测结果，且检测结果正确
            diagnosis_entry_widget->SetDiagnosisResult(true);
            ++diagnosed_count_;
            ++diagnosis_group_diagnosed_count_;
            ++diagnosed_success_count_;
        }
        return;
    }

    if (!diagnostic_result.error_code.isEmpty()) {
        // 只有主检测项，且出错
        diagnosis_entry_widget->SetDiagnosisResult(false, diagnostic_result.error_code, diagnostic_result.describe);
        ++diagnosed_count_;
        ++diagnosis_group_diagnosed_count_;
        ++diagnostic_error_count_;
        ++diagnosed_fail_count_;

        // 埋点
        QMap<QString, QString> custom_property = {
            {"systemVersion", kSystemVersion},
            {"systemArch", kSystemArch},
            {"detectionItemMark", diagnostic_result.base_entry_mark},
            {"detectionItemName", diagnostic_result.base_entry_name}
        };
        kom::BuriedPoint::uploadMessage(kom::BuriedPointPage::kFaultDiagnosis,
                                        "diagnosis_failed",
                                        custom_property);

        return;
    }

    if (!diagnostic_result.details.empty()) {
        // 子检测项出错
        // 子检测项出错，主检测项也需要显示错误，此处只修改状态，基础检测项的复选框需要在基础检测项检测完毕后再显示
        diagnosis_entry_widget->SetStatus(DiagnosisEntryWidget::Status::kException);

        auto subitem_count = diagnosis_entry_item->childCount();
        for (int i = 0; i < subitem_count; i++) {
            auto *subitem_item = diagnosis_entry_item->child(i);
            auto *subitem_widget = static_cast<DiagnosisSubentryWidget *>(diagnosis_information_widget_->itemWidget(subitem_item, 0));
            auto subitem_mark = subitem_widget->Mark();
            for (const auto &subitem_error: diagnostic_result.details) {
                if (subitem_error.mark == subitem_mark) {
                    subitem_widget->SetDiagnosisResult(false, subitem_error.error_code, subitem_error.describe);
                    ++diagnosed_count_;
                    ++diagnosis_group_diagnosed_count_;
                    ++diagnostic_error_count_;
                    ++diagnosed_fail_count_;

                    // 埋点
                    QMap<QString, QString> custom_property = {
                        {"systemVersion", kSystemVersion},
                        {"systemArch", kSystemArch},
                        {"detectionItemMark", subitem_error.mark},
                        {"detectionItemName", subitem_error.mark}
                    };
                    kom::BuriedPoint::uploadMessage(kom::BuriedPointPage::kFaultDiagnosis,
                                                    "diagnosis_failed",
                                                    custom_property);
                    break;
                }
            }
        }
    }
}

// 如果此时检测项仍处于 “检测中...” 状态，则根据 ok 的值进行处理
// 如果 ok 为 true，则所有 “检测中...” 的项认为是正确
// 如果 ok 为 false 则所有 “检测中...” 的项认为是错误
void MainWindow::on_CheckFinished(QString diagnosis_mark, QString entry_mark, bool ok) {
    if (diagnosis_mark == "specific_error") {
        HandleSpecificErrorCheckFinished(entry_mark, ok);
        return;
    }

    auto diagnosis_entry = GetDiagnosisEntry(diagnosis_mark, entry_mark);
    auto category = std::get<0>(diagnosis_entry);
    auto diagnosis_entry_item = std::get<1>(diagnosis_entry);
    auto diagnosis_entry_widget = std::get<2>(diagnosis_entry);
    if (diagnosis_entry_item == nullptr || diagnosis_entry_widget == nullptr) {
        throw std::runtime_error("Check finished no found corresponding widget");
    }

    // 处理状态仍为 “检测中...” 的项
    auto subitem_count = diagnosis_entry_item->childCount();
    if (!subitem_count) {
        // 只有主检测项
        if (diagnosis_entry_widget->Status() == DiagnosisEntryWidget::Status::kDiagnosising) {
            if (ok) {
                diagnosis_entry_widget->SetDiagnosisResult(true);
                ++diagnosed_count_;
                ++diagnosis_group_diagnosed_count_;
                ++diagnosed_success_count_;
            } else {
                diagnosis_entry_widget->SetDiagnosisResult(false, "", tr("Backend detection exception"));
                ++diagnosed_count_;
                ++diagnosis_group_diagnosed_count_;
                ++diagnostic_error_count_;
                ++diagnosed_fail_count_;
            }
        }
    } else {
        // 存在子检测项
        for (int i = 0; i < subitem_count; i++) {
            auto *subitem_item = diagnosis_entry_item->child(i);
            auto *subitem_widget = static_cast<DiagnosisSubentryWidget *>(diagnosis_information_widget_->itemWidget(subitem_item, 0));
            if (subitem_widget->Status() == DiagnosisSubentryWidget::Status::kDiagnosising) {
                if (ok) {
                    subitem_widget->SetDiagnosisResult(true);
                    ++diagnosed_count_;
                    ++diagnosis_group_diagnosed_count_;
                    ++diagnosed_success_count_;
                } else {
                    subitem_widget->SetDiagnosisResult(false, "", tr("Backend detection exception"));
                    ++diagnosed_count_;
                    ++diagnosis_group_diagnosed_count_;
                    ++diagnostic_error_count_;
                    ++diagnosed_fail_count_;
                }
            }
        }
    }

    if (subitem_count) {
        bool subitem_have_error{false};
        bool subitem_allow_select{false};
        int subitem_selected_count{0};
        for (int i = 0; i < subitem_count; i++) {
            auto *subitem_item = diagnosis_entry_item->child(i);
            auto *subitem_widget = static_cast<DiagnosisSubentryWidget *>(diagnosis_information_widget_->itemWidget(subitem_item, 0));
            if (subitem_widget->ExistError()) {
                subitem_have_error = true;
            }
            if (subitem_widget->AllowSelect()) {
                subitem_allow_select = true;
            }
            if (subitem_widget->Selected()) {
                ++subitem_selected_count;
            }
        }

        // 设置存在子检测项时，主检测项的检测结果。如果子检测项错误，主检测项也应错误
        if (subitem_have_error) {
            diagnosis_entry_widget->SetDiagnosisResult(false);
        } else {
            diagnosis_entry_widget->SetDiagnosisResult(true);
        }

        // 修正存在子检测项时，主检测项复选框的状态
        if (subitem_allow_select) {
            diagnosis_entry_widget->SetCheckedEnable(true);
            if (!subitem_selected_count) {
                diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Unchecked);
            } else if (subitem_selected_count == subitem_count) {
                diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Checked);
            } else {
                diagnosis_entry_widget->SetCheckedState(Qt::CheckState::PartiallyChecked);
            }
        } else {
            diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Unchecked);
            diagnosis_entry_widget->SetCheckedEnable(false);
        }

        // 如果子检测项全部正确，则合并基础检测项
        if (!subitem_have_error) {
            diagnosis_entry_widget->SetExpanded(false);
        }
    }

    // 清除 need_diagnosis_ 中对应的项
    auto categorys = need_diagnosis_.keys();
    qSort(categorys.begin(), categorys.end(), less_than);
    for (const auto &c: categorys) {
        auto iter = need_diagnosis_.find(c);
        if (iter == need_diagnosis_.end()) {
            throw std::runtime_error("Category not found in need diagnosis");
        }
        for (auto diagnosis_entry_iter = iter.value().begin(); diagnosis_entry_iter != iter.value().end(); diagnosis_entry_iter++) {
            if (diagnosis_entry_iter->diagnosis_mark == diagnosis_mark &&
                diagnosis_entry_iter->item.base.mark == entry_mark)
            {
                iter.value().erase(diagnosis_entry_iter);
                goto NEXT_DIAGNOSIS_FLAG;
            }
        }
    }

NEXT_DIAGNOSIS_FLAG:
    {
        DescribeGuard describe_guard(*this, category);
    }

    NextDiagnosis();
}

void MainWindow::on_Expanded(bool is_expanded) {
    auto *sender_pointer = sender();

    auto top_level_item_count = diagnosis_information_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count - 1; i++) {
        auto *diagnosis_group_item = diagnosis_information_widget_->topLevelItem(i);
        if (diagnosis_information_widget_->itemWidget(diagnosis_group_item, 0) == sender_pointer) {
            diagnosis_group_item->setExpanded(is_expanded);
            return;
        }

        auto diagnosis_entry_count = diagnosis_group_item->childCount();
        for (int j = 0; j < diagnosis_entry_count; j++) {
            auto *diagnosis_entry_item = diagnosis_group_item->child(j);
            if (diagnosis_information_widget_->itemWidget(diagnosis_entry_item, 0) == sender_pointer) {
                diagnosis_entry_item->setExpanded(is_expanded);
                return;
            }
        }
    }
}

void MainWindow::on_SubitemEntryWidgetSelectChange() {
    auto *sender_pointer = static_cast<DiagnosisSubentryWidget *>(sender());

    auto top_level_item_count = diagnosis_information_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count - 1; i++) {
        auto *diagnosis_group_item = diagnosis_information_widget_->topLevelItem(i);
        auto diagnosis_entry_count = diagnosis_group_item->childCount();
        for (int j = 0; j < diagnosis_entry_count; j++) {
            auto *diagnosis_entry_item = diagnosis_group_item->child(j);
            auto *diagnosis_entry_widget = static_cast<DiagnosisEntryWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_entry_item, 0));
            auto subitem_entry_count = diagnosis_entry_item->childCount();
            for (int m = 0; m < subitem_entry_count; m++) {
                auto *subitem_entry_item = diagnosis_entry_item->child(m);
                auto *subitem_entry_widget = static_cast<DiagnosisSubentryWidget *>(diagnosis_information_widget_->itemWidget(subitem_entry_item, 0));
                if (subitem_entry_widget == sender_pointer) {
                    // 找到了对应的子检测项
                    auto target_subitem_entry_selected_count{0};
                    auto target_subitem_entry_count = diagnosis_entry_item->childCount();
                    for (int n = 0; n < target_subitem_entry_count; n++) {
                        auto *target_subitem_entry_item = diagnosis_entry_item->child(n);
                        auto *target_subitem_entry_widget = static_cast<DiagnosisSubentryWidget *>(diagnosis_information_widget_->itemWidget(target_subitem_entry_item, 0));
                        if (target_subitem_entry_widget->Selected()) {
                            target_subitem_entry_selected_count++;
                        }
                    }

                    if (target_subitem_entry_selected_count == 0) {
                        diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Unchecked);
                    } else if (target_subitem_entry_selected_count == target_subitem_entry_count) {
                        diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Checked);
                    } else {
                        diagnosis_entry_widget->SetCheckedState(Qt::CheckState::PartiallyChecked);
                    }
                    return;
                }
            }
        }
    }
}

void MainWindow::on_DiagnosisEntryWidgetSelectChange() {
    auto *sender_pointer = static_cast<DiagnosisEntryWidget *>(sender());

    auto top_level_item_count = diagnosis_information_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count - 1; i++) {
        auto *diagnosis_group_item = diagnosis_information_widget_->topLevelItem(i);
        auto diagnosis_entry_count = diagnosis_group_item->childCount();
        for (int j = 0; j < diagnosis_entry_count; j++) {
            auto *diagnosis_entry_item = diagnosis_group_item->child(j);
            auto *diagnosis_entry_widget = static_cast<DiagnosisEntryWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_entry_item, 0));
            if (diagnosis_entry_widget == sender_pointer) {
                // 找到了对应的诊断项
                int subitem_entry_selected_count{0};
                bool is_selected = diagnosis_entry_widget->Selected();
                auto subitem_entry_count = diagnosis_entry_item->childCount();

                if (is_selected) {
                    int theory_selected{0};    // 理论上被选中的项
                    for (int n = 0; n < subitem_entry_count; n++) {
                        auto *subitem_entry_item = diagnosis_entry_item->child(n);
                        auto *subitem_entry_widget = static_cast<DiagnosisSubentryWidget *>(diagnosis_information_widget_->itemWidget(subitem_entry_item, 0));
                        if (subitem_entry_widget->Selected() || !subitem_entry_widget->AllowSelect()) {
                            theory_selected++;
                        }
                    }
                    if (theory_selected == subitem_entry_count) {
                        diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Unchecked);
                        is_selected = false;
                    }
                }

                for (int m = 0; m < subitem_entry_count; m++) {
                    auto *subitem_entry_item = diagnosis_entry_item->child(m);
                    auto *subitem_entry_widget = static_cast<DiagnosisSubentryWidget *>(diagnosis_information_widget_->itemWidget(subitem_entry_item, 0));
                    if (subitem_entry_widget->AllowSelect()) {
                        if (is_selected) {
                            subitem_entry_widget->SetCheckedState(Qt::CheckState::Checked);
                            subitem_entry_selected_count++;
                        } else {
                            subitem_entry_widget->SetCheckedState(Qt::CheckState::Unchecked);
                        }
                    }
                }
                // 给诊断项设置是完全选中还是部分选中
                if (is_selected) {
                    if (subitem_entry_selected_count == subitem_entry_count) {
                        diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Checked);
                    } else {
                        diagnosis_entry_widget->SetCheckedState(Qt::CheckState::PartiallyChecked);
                    }
                }
                return;
            }
        }
    }
}

void MainWindow::on_StartRepair() {
    cancel_ = false;

    // 重置
    error_count_ = AllErrors();
    progress_bar_->setValue(0);
    need_repair_total_ = 0;
    fixed_count_ = 0;
    fixed_success_count_ = 0;
    fixed_fail_count_ = 0;
    need_repairs_.clear();

    // 收集修复项
    auto top_level_item_count = diagnosis_information_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count - 1; i++) {
        auto *diagnosis_group_item = diagnosis_information_widget_->topLevelItem(i);
        auto *diagnosis_group_widget = static_cast<DiagnosisGroupWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_group_item, 0));
        auto diagnosis_entry_count = diagnosis_group_item->childCount();
        for (int j = 0; j < diagnosis_entry_count; j++) {
            auto *diagnosis_entry_item = diagnosis_group_item->child(j);
            auto *diagnosis_entry_widget = static_cast<DiagnosisEntryWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_entry_item, 0));

            RepairEntry repair_entry;
            repair_entry.diagnosis_mark = diagnosis_entry_widget->DiagnosisMark();
            repair_entry.entry_mark = diagnosis_entry_widget->Mark();

            auto subitem_entry_count = diagnosis_entry_item->childCount();
            if (!subitem_entry_count) {
                // 只有主检测项
                if (diagnosis_entry_widget->Selected()) {
                    repair_entry.errors.push_back(diagnosis_entry_widget->ErrorCode());
                    need_repair_total_++;
                }
            } else {
                for (int m = 0; m < subitem_entry_count; m++) {
                    auto *subitem_entry_item = diagnosis_entry_item->child(m);
                    auto *subitem_entry_widget = static_cast<DiagnosisSubentryWidget *>(diagnosis_information_widget_->itemWidget(subitem_entry_item, 0));
                    if (subitem_entry_widget->Selected()) {
                        repair_entry.errors.push_back(subitem_entry_widget->ErrorCode());
                        need_repair_total_++;
                    }
                }
            }

            if (!repair_entry.errors.empty()) {
                need_repairs_[diagnosis_group_widget->Category()].push_back(repair_entry);
            }
        }
    }

    if (!need_repair_total_) {
        QMessageBox box(QMessageBox::Icon::Information, "", tr("Please select the item that needs to be repaired."));
        box.exec();
        return;
    }

    // 更改界面
    SwitchToRepairing();

    NextRepair();
}

void MainWindow::NextRepair() {
    QList<QString> erase_keys;
    auto categorys = need_repairs_.keys();
    qSort(categorys.begin(), categorys.end(), less_than);
    for (const auto &category: categorys) {
        const auto iter = need_repairs_.find(category);
        auto repair_item_iter = iter.value().cbegin();
        if (repair_item_iter != iter.value().cend()) {
            // 判断是否需要取消
            if (cancel_) {
                qInfo() << "Repair cancel.";
                SwitchToRepairCancelComplete();
                break;
            }

            // 更改界面状态为修复中...
            auto diagnosis_entry = GetDiagnosisEntry(repair_item_iter->diagnosis_mark, repair_item_iter->entry_mark);
            auto *diagnosis_entry_item = std::get<1>(diagnosis_entry);
            auto *diagnosis_entry_widget = std::get<2>(diagnosis_entry);
            if (diagnosis_entry_item != nullptr && diagnosis_entry_widget != nullptr) {
                auto subitem_entry_count = diagnosis_entry_item->childCount();
                if (!subitem_entry_count) {
                    // 只有主检测项
                    diagnosis_entry_widget->SetStatus(DiagnosisEntryWidget::Status::kRepairing);
                } else {
                    for (int i = 0; i < subitem_entry_count; i++) {
                        auto *subitem_entry_item = diagnosis_entry_item->child(i);
                        auto *subitem_entry_widget = static_cast<DiagnosisSubentryWidget *>(diagnosis_information_widget_->itemWidget(subitem_entry_item, 0));
                        if (repair_item_iter->errors.contains(subitem_entry_widget->ErrorCode())) {
                            subitem_entry_widget->SetStatus(DiagnosisSubentryWidget::Status::kRepairing);
                        }
                    }
                    // 子项为 “修复中...” 那么主检测项也应为 “修复中...”
                    diagnosis_entry_widget->SetStatus(DiagnosisEntryWidget::Status::kRepairing);
                }
            }
            Q_EMIT sig_Repair(*qAsConst(repair_item_iter));
            break;
        } else {
            // 该分类已经修复完毕
            erase_keys.push_back(iter.key());

            // 如果该分类所有错误项都被修复，则合并该分类
            if (CategoryFixAll(category)) {
                auto *diagnosis_group_widget = GetDiagnosisGroupWidget(category);
                if (diagnosis_group_widget != nullptr) {
                    diagnosis_group_widget->SetExpanded(false);
                }
            }
        }
    }

    // 移除掉已修复完成的分类
    for (const auto &k: erase_keys) {
        need_repairs_.remove(k);
    }

    if (need_repairs_.empty()) {
        // 全部修复完毕
        SwitchToRepairComplete();
        qDebug() << "Repair completed";

        // 埋点
        QMap<QString, QString> custom_property = {
            {"systemVersion", kSystemVersion},
            {"systemArch", kSystemArch},
            {"totalNumber", QString::number(need_repair_total_)},
            {"numberOfSuccessful", QString::number(fixed_success_count_)},
            {"numberOfFailed", QString::number(fixed_fail_count_)}
        };
        kom::BuriedPoint::uploadMessage(kom::BuriedPointPage::kFaultDiagnosis,
                                        "fault_repair",
                                        custom_property);

        // 弹窗收集用户体验信息
        K::UserExperienceDialog user_experience_dialog;
        user_experience_dialog.SetFeedBackCallback([]() {
            Frame::SwitchToProblemFeedback();
        });
        user_experience_dialog.exec();
    }
}

bool MainWindow::CategoryFixAll(const QString &category) {
    auto top_level_item_count = diagnosis_information_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count - 1; i++) {
        auto *diagnosis_group_item = diagnosis_information_widget_->topLevelItem(i);
        auto *diagnosis_group_widget = static_cast<DiagnosisGroupWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_group_item, 0));
        if (diagnosis_group_widget->Category() == category) {
            auto diagnosis_entry_count = diagnosis_group_item->childCount();
            for (int j = 0; j < diagnosis_entry_count; j++) {
                auto *diagnosis_entry_item = diagnosis_group_item->child(j);
                auto *diagnosis_entry_widget = static_cast<DiagnosisEntryWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_entry_item, 0));
                auto subitem_entry_count = diagnosis_entry_item->childCount();
                if (!subitem_entry_count) {
                    // 只有主检测项
                    if (diagnosis_entry_widget->ExistError()) {
                        return false;
                    }
                } else {
                    for (int m = 0; m < subitem_entry_count; m++) {
                        auto *subitem_entry_item = diagnosis_entry_item->child(m);
                        auto *subitem_entry_widget = static_cast<DiagnosisSubentryWidget *>(diagnosis_information_widget_->itemWidget(subitem_entry_item, 0));
                        if (subitem_entry_widget->ExistError()) {
                            return false;
                        }
                    }
                }
            }
            return true;
        }
    }
    return false;
}

void MainWindow::on_RepairFinished(fault_diagnosis::RepairEntryResult repair_entry_result) {
    if (repair_entry_result.diagnosis_mark == "specific_error") {
        HandleSpecificErrorRepairFinish(repair_entry_result.ok);
        return;
    }

    auto diagnosis_entry = GetDiagnosisEntry(repair_entry_result.diagnosis_mark, repair_entry_result.entry_mark);
    auto *diagnosis_entry_item = std::get<1>(diagnosis_entry);
    auto *diagnosis_entry_widget = std::get<2>(diagnosis_entry);
    if (diagnosis_entry_item == nullptr || diagnosis_entry_widget == nullptr) {
        throw std::runtime_error("Repair finish no found corresponding widget");
    }

    // 处理状态仍为 “修复中...” 的项
    const auto subitem_entry_count = diagnosis_entry_item->childCount();
    if (!subitem_entry_count) {
        // 只有主检测项
        if (diagnosis_entry_widget->Status() == DiagnosisEntryWidget::Status::kRepairing) {
            if (repair_entry_result.ok) {
                error_count_--;
                fixed_success_count_++;
                diagnosis_entry_widget->SetRepairResult(true);
            } else {
                fixed_fail_count_++;
                diagnosis_entry_widget->SetRepairResult(false);
            }
        }
    } else {
        for (int i = 0; i < subitem_entry_count; i++) {
            auto *subitem_entry_item = diagnosis_entry_item->child(i);
            auto *subitem_entry_widget = static_cast<DiagnosisSubentryWidget *>(diagnosis_information_widget_->itemWidget(subitem_entry_item, 0));
            if (subitem_entry_widget->Status() == DiagnosisSubentryWidget::Status::kRepairing) {
                if (repair_entry_result.ok) {
                    error_count_--;
                    fixed_success_count_++;
                    subitem_entry_widget->SetRepairResult(true);
                } else {
                    fixed_fail_count_++;
                    subitem_entry_widget->SetRepairResult(false);
                }
            }
        }
    }

    if (subitem_entry_count) {
        bool subitem_have_error{false};
        bool subitem_allow_select{false};
        int subitem_selected_count{0};
        for (int i = 0; i < subitem_entry_count; i++) {
            auto *subitem_entry_item = diagnosis_entry_item->child(i);
            auto *subitem_entry_widget = static_cast<DiagnosisSubentryWidget *>(diagnosis_information_widget_->itemWidget(subitem_entry_item, 0));
            if (subitem_entry_widget->ExistError()) {
                subitem_have_error = true;
            }
            if (subitem_entry_widget->AllowSelect()) {
                subitem_allow_select = true;
            }
            if (subitem_entry_widget->Selected()) {
                ++subitem_selected_count;
            }
        }

        // 设置存在子检测项时，主检测项的修复结果。如果子检测项存在错误，主检测项修复失败
        if (subitem_have_error) {
            diagnosis_entry_widget->SetRepairResult(false);
        } else {
            diagnosis_entry_widget->SetRepairResult(true);
        }

        // 修正存在子检测项时，主检测项复选框的状态
        if (subitem_allow_select) {
            diagnosis_entry_widget->SetCheckedEnable(true);
            if (!subitem_selected_count) {
                diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Unchecked);
            } else if (subitem_selected_count == subitem_entry_count) {
                diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Checked);
            } else {
                diagnosis_entry_widget->SetCheckedState(Qt::CheckState::PartiallyChecked);
            }
        } else {
            diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Unchecked);
            diagnosis_entry_widget->SetCheckedEnable(false);
        }

        // 如果子检测项全部修复，则合并基础检测项
        if (!subitem_have_error) {
            diagnosis_entry_widget->SetExpanded(false);
        }
    }

    // 清除 need_repair_ 中对应的项
    auto categorys = need_repairs_.keys();
    qSort(categorys.begin(), categorys.end(), less_than);
    for (const auto &category: categorys) {
        auto iter = need_repairs_.find(category);
        if (iter == need_repairs_.end()) {
            throw std::runtime_error("Category not found in need repairs");
        }
        for (auto repair_entry_iter = iter.value().begin(); repair_entry_iter != iter.value().end(); repair_entry_iter++) {
            if (repair_entry_iter->diagnosis_mark == repair_entry_result.diagnosis_mark &&
                repair_entry_iter->entry_mark == repair_entry_result.entry_mark)
            {
                iter.value().erase(repair_entry_iter);
                goto NEXT_REPAIR_FLAG;
            }
        }
    }

NEXT_REPAIR_FLAG:
    NextRepair();
}

void MainWindow::HandleSpecificErrorFixed(bool ok) {
    if (specific_error_widget_->State() == RepairSpecificErrorWidget::State::kRepairing) {
        // 自动修复的情况
        RepairSpecificErrorWidget::ShowContent show_content;
        show_content.report_path = specific_error_report_path_;
        if (ok) {
            show_content.ok = true;
            switch (specific_error_module_) {
                case SpecificErrorModule::kUpgrade:
                    show_content.desc = tr("The issue has been successfully fixed, please try updating again");
                    break;
                case SpecificErrorModule::kSoftware:
                    show_content.desc = tr("The issue has been successfully fixed, please reinstall the software package again");
                    break;
                default:
                    show_content.desc = tr("Repair successful");
                    break;
            }
        } else {
            show_content.ok = false;
            show_content.desc = tr("Repair failed");
            show_content.report_issues = true;
        }
        specific_error_widget_->SwitchResult(show_content);
    } else {
        specific_error_widget_->SetRepairResult(specific_error_module_, ok);
    }
    specific_error_widget_->SetState(RepairSpecificErrorWidget::State::kRepairFinish);
}

void MainWindow::HandleSpecificErrorRepairFinish(bool ok) {
    if (specific_error_widget_->State() != RepairSpecificErrorWidget::State::kRepairFinish) {
        // 未接收到修复结果
        HandleSpecificErrorFixed(ok);
    }
}

void MainWindow::on_Fixed(QString diagnosis_mark, QString error_code, bool ok, QString reason, qint64 solution_type, QString solution_context) {
    if (diagnosis_mark == "specific_error") {
        HandleSpecificErrorFixed(ok);
        return;
    }

    fixed_count_++;
    progress_bar_->setValue((double)fixed_count_ / need_repair_total_ * 100);

    // 埋点
    auto buried_point = [this](QString error_code, QString error_name, QString error_desc) {
        QMap<QString, QString> custom_property = {
                {"systemVersion", kSystemVersion},
                {"systemArch", kSystemArch},
                {"errorCode", error_code},
                {"errorName", error_name},
                {"errorDescription", error_desc},
        };
        kom::BuriedPoint::uploadMessage(kom::BuriedPointPage::kFaultDiagnosis,
                                        "repair_failed",
                                        custom_property);
    };

    bool match_error_code{false};
    const auto top_level_item_count = diagnosis_information_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count - 1; i++) {
        auto *diagnosis_group_item = diagnosis_information_widget_->topLevelItem(i);
        const auto diagnosis_entry_count = diagnosis_group_item->childCount();
        for (int j = 0; j < diagnosis_entry_count; j++) {
            auto *diagnosis_entry_item = diagnosis_group_item->child(j);
            auto *diagnosis_entry_widget = static_cast<DiagnosisEntryWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_entry_item, 0));
            if (diagnosis_entry_widget->DiagnosisMark() != diagnosis_mark) {
                continue;
            }
            const auto subitem_entry_count = diagnosis_entry_item->childCount();
            if (!subitem_entry_count) {
                // 只有主检测项
                if (diagnosis_entry_widget->ErrorCode() == error_code) {
                    match_error_code = true;

                    if (ok) {
                        error_count_--;
                        fixed_success_count_++;
                        diagnosis_entry_widget->SetRepairResult(true);
                    } else {
                        fixed_fail_count_++;
                        diagnosis_entry_widget->SetRepairResult(false);
                        buried_point(error_code, diagnosis_entry_widget->Name(), reason);
                    }
                }
            } else {
                for (int m = 0; m < subitem_entry_count; m++) {
                    auto *subitem_entry_item = diagnosis_entry_item->child(m);
                    auto *subitem_entry_widget = static_cast<DiagnosisSubentryWidget *>(diagnosis_information_widget_->itemWidget(subitem_entry_item, 0));
                    if (subitem_entry_widget->ErrorCode() == error_code) {
                        match_error_code = true;

                        if (ok) {
                            error_count_--;
                            fixed_success_count_++;
                            subitem_entry_widget->SetRepairResult(true);
                        } else {
                            fixed_fail_count_++;
                            subitem_entry_widget->SetRepairResult(false);
                            buried_point(error_code, subitem_entry_widget->Name(), reason);
                        }
                    }
                }
            }
        }
    }
    if (!match_error_code) {
        // 没找到错误码对应的检测项
        qCritical() << "No matching error code found";
    }
}

QString MainWindow::CategoryIcon(const QString &category) {
    if (category == "网络检测") {
        return "ukui-network-agent-symbolic";
    }
    if (category == "系统组件与性能检测") {
        return "ukui-bf-fast-symbolic";
    }
    if (category == "应用程序检测") {
        return "ukui-defaultapp-symbolic";
    }
    return "preferences-desktop-wallpaper-symbolic";
}

void MainWindow::on_Cancel() {
    cancel_ = true;
    if (status_ == MainWindow::Status::kDiagnosising) {
        SwitchToDiagnosisCanceling();
    } else if (status_ == MainWindow::Status::kRepairing) {
        SwitchToRepairCanceling();
    }
}

int MainWindow::AllErrors() const {
    int error_count{0};
    auto top_level_item_count = diagnosis_information_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count - 1; i++) {
        auto *diagnosis_group_item = diagnosis_information_widget_->topLevelItem(i);
        auto diagnosis_entry_count = diagnosis_group_item->childCount();
        for (int j = 0; j < diagnosis_entry_count; j++) {
            auto *diagnosis_entry_item = diagnosis_group_item->child(j);
            auto *diagnosis_entry_widget = static_cast<DiagnosisEntryWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_entry_item, 0));
            auto subitem_entry_count = diagnosis_entry_item->childCount();
            if (!subitem_entry_count) {
                // 只有主检测项
                if (diagnosis_entry_widget->Status() == DiagnosisEntryWidget::Status::kException ||
                    diagnosis_entry_widget->Status() == DiagnosisEntryWidget::Status::KReapriFailed)
                {
                    error_count++;
                }
            } else {
                for (int m = 0; m < subitem_entry_count; m++) {
                    auto *subitem_entry_item = diagnosis_entry_item->child(m);
                    auto *subitem_entry_widget = static_cast<DiagnosisSubentryWidget *>(diagnosis_information_widget_->itemWidget(subitem_entry_item, 0));
                    if (subitem_entry_widget->Status() == DiagnosisSubentryWidget::Status::kException ||
                        subitem_entry_widget->Status() == DiagnosisSubentryWidget::Status::kRepairFailed)
                    {
                        error_count++;
                    }
                }
            }
        }
    }
    return error_count;
}

void MainWindow::SetMode(enum fault_diagnosis::Mode mode) {
    mode_ = mode;
}

void MainWindow::on_SpecificErrorRepair(QString error_code) {
    RepairEntry repair_entry;
    repair_entry.diagnosis_mark = "specific_error";
    repair_entry.entry_mark = "specific_error";
    repair_entry.errors.push_back(error_code);

    Q_EMIT sig_Repair(repair_entry);
}

void MainWindow::SetSpecificError(RepairSpecificErrorWidget *widget, enum SpecificErrorModule module, QString path, QString error_code, const QString &error_data) {
    specific_error_widget_ = widget;
    specific_error_module_ = module;
    specific_error_report_path_ = path;
    specific_error_code_ = error_code;
    specific_error_data_ = error_data;

    connect(specific_error_widget_, &RepairSpecificErrorWidget::sig_Repair, this, &MainWindow::on_SpecificErrorRepair, Qt::UniqueConnection);
    connect(specific_error_widget_, &RepairSpecificErrorWidget::sig_Return, this, &MainWindow::sig_Return, Qt::UniqueConnection);
}

void MainWindow::SetSpecificDiagnosisType(enum fault_diagnosis::DiagnosisType diagnosis_type) {
    diagnosis_type_ = diagnosis_type;
}

bool MainWindow::Running() const {
    switch (status_) {
        case Status::kDiagnosising:
            [[fallthrough]];
        case Status::kDiagnosisCanceling:
            [[fallthrough]];
        case Status::kRepairing:
            [[fallthrough]];
        case Status::kRepairCanceling:
            return true;
        default:
            return false;
    }
}

QString MainWindow::SystemVersion() const {
    std::string version{"unknow"};

    // 读取 /etc/os-release 文件
    {
        std::ifstream ifs("/etc/os-release", std::ios::in);
        if (ifs.is_open()) {
            std::string line;
            while (std::getline(ifs, line)) {
                auto pos = line.find("VERSION=");
                if (pos == 0) {
                    version = line.substr(8);
                    break;
                }
            }
        }
    }

    // 读取 /etc/kylin-bild 文件
    {
        std::ifstream ifs("/etc/kylin-build", std::ios::in);
        if (ifs.is_open()) {
            std::string line;
            while (std::getline(ifs, line)) {
                auto pos = line.find("buildid:");
                if (pos == 0) {
                    version.append("-");
                    version.append(line);
                    break;
                }
            }
        }
    }

    return QString::fromStdString(version);
}

QString MainWindow::SystemArch() const {
    std::string arch{"unknow"};

    std::ifstream ifs("/proc/osinfo", std::ios::in);
    if (ifs.is_open()) {
        std::string line;
        while (std::getline(ifs, line)) {
            auto pos = line.find("Architecture:");
            if (pos == 0) {
                arch = line.substr(13);
                // 移除左侧空白字符
                auto start = arch.find_first_not_of(" \n\t\r");
                arch = arch.substr(start);
                break;
            }
        }
    }

    return QString::fromStdString(arch);
}

}
