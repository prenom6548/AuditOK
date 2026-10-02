#include <QHBoxLayout>
#include <QDebug>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QMessageBox>
#include "mainwindow.h"

namespace fault_diagnosis {

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
      status_(MainWindow::Status::kNoStatus),
      diagnosed_count_(0),
      diagnostic_error_count_(0)
{
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
    for (auto iter = category_diagnostic_entrys.begin(); iter != category_diagnostic_entrys.end(); iter++) {
        auto category = iter.key();
        auto diagnostic_entrys = iter.value();
        if (diagnostic_entrys.isEmpty()) {
            continue;
        }
        auto *diagnostic_group_item = new QTreeWidgetItem(diagnosis_information_widget_);
        auto *diagnosis_group_widget = new DiagnosisGroupWidget(this);
        diagnosis_group_widget->SetIcon(CategoryIcon(category));
        diagnosis_group_widget->SetCategory(category);
        diagnosis_group_widget->SetTotal(0);
        diagnosis_group_widget->SetStatus(DiagnosisGroupWidget::Status::kNoState);
        connect(diagnosis_group_widget, &DiagnosisGroupWidget::sig_Expanded, this, &MainWindow::on_Expanded);
        diagnosis_information_widget_->setItemWidget(diagnostic_group_item, 0, diagnosis_group_widget);

        for (const auto &entry: diagnostic_entrys) {
            auto *diagnostic_entry_item = new QTreeWidgetItem(diagnostic_group_item);
            auto *diagnostic_entry_widget = new DiagnosisEntryWidget(entry, this);
            diagnostic_entry_widget->SetStatus(DiagnosisEntryWidget::Status::kNoState);
            connect(diagnostic_entry_widget, &DiagnosisEntryWidget::sig_Expanded, this, &MainWindow::on_Expanded);
            connect(diagnostic_entry_widget, &DiagnosisEntryWidget::sig_SelectChange, this, &MainWindow::on_DiagnosisEntryWidgetSelectChange);
            diagnosis_information_widget_->setItemWidget(diagnostic_entry_item, 0, diagnostic_entry_widget);
        }
    }
    // QTreeWidget 的最后一个顶级 item，会稍微靠左，对不齐
    // 不知是 Qt 的 BUG 还是什么原因，此处多加一个空白 item，来规避该问题
    diagnosis_information_widget_->addTopLevelItem(new QTreeWidgetItem(diagnosis_information_widget_));
}

void MainWindow::on_StartDiagnosis() {
    // 还原
    need_diagnosis_total_ = 0;
    diagnosed_count_ = 0;
    diagnostic_error_count_ = 0;
    diagnosis_group_diagnosed_count_ = 0;
    cancel_ = false;
    Reset();

    // 收集诊断项
    need_diagnosis_.clear();
    need_diagnosis_ = category_diagnostic_entrys_;
    for (auto iter = need_diagnosis_.cbegin(); iter != need_diagnosis_.cend(); iter++) {
        need_diagnosis_total_ += iter.value().size();
    }

    // 修改界面
    SwitchToDiagnosing();

    NextDiagnosis();
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
    for (auto iter = need_diagnosis_.begin(); iter != need_diagnosis_.end(); iter++) {
        auto diagnosis_entry_iter = iter.value().begin();
        if (diagnosis_entry_iter != iter.value().end()) {
            // 判断是否需要暂停
            if (cancel_) {
                qInfo() << "Diagnosis cancel.";
                SwitchToDiagnosisCancelComplete();
                break;
            }

            CheckEntry check_entry;
            check_entry.category = iter.key();
            check_entry.diagnosis_mark = diagnosis_entry_iter->diagnosis_mark;
            check_entry.entry_mark = diagnosis_entry_iter->item.mark;

            // 检测哪个分类需要展开该分类
            auto *diagnosis_group_widget = GetDiagnosisGroupWidget(check_entry.category);
            if (diagnosis_group_widget != nullptr) {
                diagnosis_group_widget->SetExpanded(true);
            }

            // 更改诊断项的状态
            auto *diagnosis_entry_widget = GetDiagnosisEntryWithCategory(check_entry.category,
                                                                         check_entry.diagnosis_mark,
                                                                         check_entry.entry_mark).second;
            if (diagnosis_entry_widget != nullptr) {
                diagnosis_entry_widget->SetStatus(DiagnosisEntryWidget::Status::kDiagnosising);
            }
            Q_EMIT sig_Check(check_entry);
            break;
        } else {
            // 该分类下所有项已经检测完毕，从 need_diagnosis_ 中移除该项
            erase_keys.push_back(iter.key());

            // 复原记录诊断组已诊断的数量
            diagnosis_group_diagnosed_count_ = 0;

            // 判断该分类是否需要合并
            auto *diagnosis_group_widget = GetDiagnosisGroupWidget(iter.key());
            if (!CategoryExistError(iter.key())) {
                if (diagnosis_group_widget != nullptr) {
                    diagnosis_group_widget->SetExpanded(false);
                    diagnosis_group_widget->SetStatus(DiagnosisGroupWidget::Status::kNormal);
                }
            } else {
                if (diagnosis_group_widget != nullptr) {
                    diagnosis_group_widget->SetStatus(DiagnosisGroupWidget::Status::kException);
                }
            }
        }
    }

    // 清除诊断项为空的项
    for (const auto &k: erase_keys)
        need_diagnosis_.remove(k);

    if (need_diagnosis_.empty()) {
        SwitchToDiagnosisComplete();

        // 展开所有的错误项
        auto top_level_item_count = diagnosis_information_widget_->topLevelItemCount();
        for (int i = 0; i < top_level_item_count - 1; i++) {
            bool have_error = false;
            auto *diagnosis_group_item = diagnosis_information_widget_->topLevelItem(i);
            auto *diagnosis_group_widget = static_cast<DiagnosisGroupWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_group_item, 0));
            auto diagnosis_entry_count = diagnosis_group_item->childCount();
            for (int j = 0; j < diagnosis_entry_count; j++) {
                auto *diagnosis_entry_item = diagnosis_group_item->child(j);
                auto *diagnosis_entry_widget = static_cast<DiagnosisEntryWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_entry_item, 0));
                if (diagnosis_entry_item->childCount() > 0) {
                    // 该诊断项存在错误
                    diagnosis_entry_widget->SetExpanded(true);
                    have_error = true;
                }
            }
            if (have_error) {
                diagnosis_group_widget->SetExpanded(true);
            }
        }

        qDebug() << "Diagnosis complete";
    }
}

QPair<QTreeWidgetItem *, DiagnosisEntryWidget *> MainWindow::GetDiagnosisEntryWithCategory(const QString &category,
                                                                                           const QString &diagnosis_mark,
                                                                                           qint64 diagnosis_entry_mark)
{
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
                if (diagnosis_entry_widget->ExistError()) {
                    return true;
                }
            }
            return false;
        }
    }

    return false;
}

void MainWindow::Reset() {
    auto top_level_item_count = diagnosis_information_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count - 1; i++) {
        auto *diagnosis_group_item = diagnosis_information_widget_->topLevelItem(i);
        auto *diagnosis_group_widget = static_cast<DiagnosisGroupWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_group_item, 0));
        diagnosis_group_widget->Reset();

        auto diagnosis_entry_count = diagnosis_group_item->childCount();
        for(int j = 0; j < diagnosis_entry_count; j++) {
            auto *diagnosis_entry_item = diagnosis_group_item->child(j);
            auto *diagnosis_entry_widget = static_cast<DiagnosisEntryWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_entry_item, 0));
            diagnosis_entry_widget->Reset();

            // 删除所有的错误项
            auto error_entry_count = diagnosis_entry_item->childCount();
            // QTreeWidget 删除同级别子项时，索引会发生变化，此处从后往前删
            for (int m = error_entry_count - 1; m >= 0; m--) {
                auto *error_entry_item = diagnosis_entry_item->child(m);
                auto *error_entry_widget = static_cast<ErrorEntryWidget *>(diagnosis_information_widget_->itemWidget(error_entry_item, 0));
                auto *error_entry_item_remove = diagnosis_entry_item->takeChild(m);
                if (error_entry_item_remove) {
                    delete error_entry_item_remove;
                }
                if (error_entry_widget) {
                    delete error_entry_widget;
                }
            }
        }
    }
}


void MainWindow::on_CheckFinished(fault_diagnosis::CheckEntryResult check_entry_result) {
    diagnosed_count_++;
    if (!check_entry_result.ok) {
        diagnostic_error_count_++;
    } else if (!check_entry_result.result.details.isEmpty()) {
        diagnostic_error_count_ += check_entry_result.result.details.size();
    }
    diagnosis_group_diagnosed_count_++;

    // 清除 need_diagnosis_ 中对应的项
    auto iter = need_diagnosis_.find(check_entry_result.check.category);
    if (iter == need_diagnosis_.end()) {
        throw std::runtime_error("Category not fount in need diagnosis");
    }
    for (auto diagnostic_entry_iter = iter.value().begin(); diagnostic_entry_iter != iter.value().end(); diagnostic_entry_iter++) {
        if (diagnostic_entry_iter->diagnosis_mark == check_entry_result.check.diagnosis_mark &&
            diagnostic_entry_iter->item.mark == check_entry_result.check.entry_mark)
        {
            iter.value().erase(diagnostic_entry_iter);
            break;
        }
    }

    // 更新状态细节 label
    UpdateStatusDetailLabel();

    // 更新进度
    if (need_diagnosis_total_) {
        progress_bar_->setValue((double)diagnosed_count_ / need_diagnosis_total_ * 100);
    }

    // 设置诊断组已扫描项
    auto *diagnosis_group_widget = GetDiagnosisGroupWidget(check_entry_result.check.category);
    if (diagnosis_group_widget != nullptr) {
        diagnosis_group_widget->SetTotal(diagnosis_group_diagnosed_count_);
    }

    // 给诊断项设置诊断结果
    const auto diagnosis_entry  = GetDiagnosisEntryWithCategory(check_entry_result.check.category,
                                                                check_entry_result.check.diagnosis_mark,
                                                                check_entry_result.check.entry_mark);
    auto *diagnosis_entry_item = diagnosis_entry.first;
    auto *diagnosis_entry_widget = diagnosis_entry.second;
    if (diagnosis_entry_item == nullptr || diagnosis_entry_widget == nullptr) {
        qCritical() << "No corresponding diagnostic item found.";
        return;
    }

    diagnosis_entry_widget->SetDiagnosticResult(check_entry_result);

    // 构建错误项
    for (const auto &e: check_entry_result.result.details) {
        auto *error_entry_item = new QTreeWidgetItem(diagnosis_entry_item);
        auto *error_entry_widget = new ErrorEntryWidget(e, check_entry_result.solution_records[e.error_code], this);
        connect(error_entry_widget, &ErrorEntryWidget::sig_SelectChange, this, &MainWindow::on_ErrorEntryWidgetSelectChange);
        diagnosis_information_widget_->setItemWidget(error_entry_item, 0, error_entry_widget);

        // QTreeWidget 在界面显示出来的情况下动态添加 widget，widget 不能立即放到指定的层级上，而是会显示在界面的顶部
        // 直到添加下一个 item 时，上一个 widget 才能被放到正确的位置
        // 所以此处在添加 ErrorEntryWidget 后，立即添加一个空的 item，然后立即删除掉，来规避该问题
        auto *tmp_item = new QTreeWidgetItem(diagnosis_entry_item);
        diagnosis_entry_item->removeChild(tmp_item);
        delete tmp_item;
    }

    // 设置诊断项复选框的状态
    int error_entry_selected_count{0};
    auto error_entry_count = diagnosis_entry_item->childCount();
    for (int i = 0; i < error_entry_count; i++) {
        auto *error_entry_item = diagnosis_entry_item->child(i);
        auto *error_entry_widget = static_cast<ErrorEntryWidget *>(diagnosis_information_widget_->itemWidget(error_entry_item, 0));
        if (error_entry_widget->Selected()) {
            error_entry_selected_count++;
        }
    }
    if (error_entry_count == 0) {
        // 没有错误
        diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Unchecked);
        diagnosis_entry_widget->SetCheckedEnable(false);
    } else if (error_entry_selected_count == 0) {
        // 错误项全部为手动修复
        diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Unchecked);
        diagnosis_entry_widget->SetCheckedEnable(false);
    } else if(error_entry_selected_count == error_entry_count) {
        diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Checked);
        diagnosis_entry_widget->SetCheckedEnable(true);
    } else {
        diagnosis_entry_widget->SetCheckedState(Qt::CheckState::PartiallyChecked);
        diagnosis_entry_widget->SetCheckedEnable(true);
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

void MainWindow::on_ErrorEntryWidgetSelectChange() {
    auto *sender_pointer = static_cast<ErrorEntryWidget *>(sender());

    auto top_level_item_count = diagnosis_information_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count - 1; i++) {
        auto *diagnosis_group_item = diagnosis_information_widget_->topLevelItem(i);
        auto diagnosis_entry_count = diagnosis_group_item->childCount();
        for (int j = 0; j < diagnosis_entry_count; j++) {
            auto *diagnosis_entry_item = diagnosis_group_item->child(j);
            auto *diagnosis_entry_widget = static_cast<DiagnosisEntryWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_entry_item, 0));
            auto error_entry_count = diagnosis_entry_item->childCount();
            for (int m = 0; m < error_entry_count; m++) {
                auto *error_entry_item = diagnosis_entry_item->child(m);
                auto *error_entry_widget = static_cast<ErrorEntryWidget *>(diagnosis_information_widget_->itemWidget(error_entry_item, 0));
                if (error_entry_widget == sender_pointer) {
                    // 找到了对应的错误项
                    auto target_error_entry_selected_count{0};
                    auto target_error_entry_count = diagnosis_entry_item->childCount();
                    for (int n = 0; n < target_error_entry_count; n++) {
                        auto *target_error_entry_item = diagnosis_entry_item->child(n);
                        auto *target_error_entry_widget = static_cast<ErrorEntryWidget *>(diagnosis_information_widget_->itemWidget(target_error_entry_item, 0));
                        if (target_error_entry_widget->Selected()) {
                            target_error_entry_selected_count++;
                        }
                    }

                    if (target_error_entry_selected_count == 0) {
                        diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Unchecked);
                    } else if (target_error_entry_selected_count == target_error_entry_count) {
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
                int error_entry_selected_count{0};
                bool is_selected = diagnosis_entry_widget->Selected();
                auto error_entry_count = diagnosis_entry_item->childCount();

                if (is_selected) {
                    int theory_selected{0};    // 理论上被选中的项
                    for (int n = 0; n < error_entry_count; n++) {
                        auto *error_entry_item = diagnosis_entry_item->child(n);
                        auto *error_entry_widget = static_cast<ErrorEntryWidget *>(diagnosis_information_widget_->itemWidget(error_entry_item, 0));
                        if (error_entry_widget->Selected() || error_entry_widget->ManualRepair() || error_entry_widget->GetStatus() == ErrorEntryWidget::Status::kFixed) {
                            theory_selected++;
                        }
                    }
                    if (theory_selected == error_entry_count) {
                        diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Unchecked);
                        is_selected = false;
                    }
                }

                for (int m = 0; m < error_entry_count; m++) {
                    auto *error_entry_item = diagnosis_entry_item->child(m);
                    auto *error_entry_widget = static_cast<ErrorEntryWidget *>(diagnosis_information_widget_->itemWidget(error_entry_item, 0));
                    if (is_selected) {
                        if (error_entry_widget->SetCheckState(Qt::CheckState::Checked)) {
                            error_entry_selected_count++;
                        }
                    } else {
                        error_entry_widget->SetCheckState(Qt::CheckState::Unchecked);
                    }
                }
                // 给诊断项设置是完全选中还是部分选中
                if (is_selected) {
                    if (error_entry_selected_count == error_entry_count) {
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
            repair_entry.category = diagnosis_group_widget->Category();
            repair_entry.entry_mark = diagnosis_entry_widget->Mark();

            auto error_entry_count = diagnosis_entry_item->childCount();
            for (int m = 0; m < error_entry_count; m++) {
                auto *error_entry_item = diagnosis_entry_item->child(m);
                auto *error_entry_widget = static_cast<ErrorEntryWidget *>(diagnosis_information_widget_->itemWidget(error_entry_item, 0));
                if (error_entry_widget->Selected()) {
                    repair_entry.errors[error_entry_widget->ErrorCode()] = error_entry_widget->SolutionRecord();
                    need_repair_total_++;
                }
            }

            if (!repair_entry.errors.isEmpty()) {
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
    for (auto iter = need_repairs_.cbegin(); iter != need_repairs_.cend(); iter++) {
        const auto repair_item_iter = iter.value().cbegin();
        if (repair_item_iter != iter.value().cend()) {
            // 判断是否需要取消
            if (cancel_) {
                qInfo() << "Repair cancel.";
                SwitchToRepairCancelComplete();
                break;
            }

            // 更改错误项界面状态
            auto diagnosis_entry = GetDiagnosisEntryWithCategory(repair_item_iter->category, repair_item_iter->diagnosis_mark, repair_item_iter->entry_mark);
            auto *diagnosis_entry_item = diagnosis_entry.first;
            if (diagnosis_entry_item != nullptr) {
                auto error_entry_count = diagnosis_entry_item->childCount();
                for (int i = 0; i < error_entry_count; i++) {
                    auto *error_entry_item = diagnosis_entry_item->child(i);
                    auto *error_entry_widget = static_cast<ErrorEntryWidget *>(diagnosis_information_widget_->itemWidget(error_entry_item, 0));
                    if (repair_item_iter->errors.find(error_entry_widget->ErrorCode()) != repair_item_iter->errors.end()) {
                        error_entry_widget->SetStatus(ErrorEntryWidget::Status::kRepairing);
                    }
                }
            }
            Q_EMIT sig_Repair(*qAsConst(repair_item_iter));
            break;
        } else {
            // 该分类已经修复完毕
            erase_keys.push_back(iter.key());

            // 如果该分类所有错误项都被修复，则合并该分类，同时状态更改为已修复
            auto *diagnosis_group_widget = GetDiagnosisGroupWidget(iter.key());
            if (CategoryFixAll(iter.key())) {
                if (diagnosis_group_widget != nullptr) {
                    diagnosis_group_widget->SetExpanded(false);
                    diagnosis_group_widget->SetStatus(DiagnosisGroupWidget::Status::kFix);
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
                auto error_entry_count = diagnosis_entry_item->childCount();
                for (int m = 0; m < error_entry_count; m++) {
                    auto *error_entry_item = diagnosis_entry_item->child(m);
                    auto *error_entry_widget = static_cast<ErrorEntryWidget *>(diagnosis_information_widget_->itemWidget(error_entry_item, 0));
                    if (error_entry_widget->GetStatus() != ErrorEntryWidget::Status::kFixed) {
                        return false;
                    }
                }
            }
            return true;
        }
    }
    return false;
}

void MainWindow::on_RepairFinished(fault_diagnosis::RepairEntryResult repair_entry_result) {
    // 清除 need_repair_ 中对应的项
    auto iter = need_repairs_.find(repair_entry_result.repair.category);
    if (iter == need_repairs_.end())
        throw std::runtime_error("Category not found in need repairs");

    for (auto repair_entry_iter = iter.value().begin(); repair_entry_iter != iter.value().end(); iter ++) {
        if (repair_entry_iter->diagnosis_mark == repair_entry_result.repair.diagnosis_mark &&
            repair_entry_iter->entry_mark == repair_entry_result.repair.entry_mark)
        {
            iter.value().erase(repair_entry_iter);
            break;
        }
    }

    // 更改界面
    auto diagnosis_entry = GetDiagnosisEntryWithCategory(repair_entry_result.repair.category,
                                                         repair_entry_result.repair.diagnosis_mark,
                                                         repair_entry_result.repair.entry_mark);
    auto *diagnosis_entry_item = diagnosis_entry.first;
    auto *diagnosis_entry_widget = diagnosis_entry.second;
    if (diagnosis_entry_item != nullptr && diagnosis_entry_widget != nullptr) {
        auto fixed_count{0};
        auto manual_repair_count{0};
        auto selected_count{0};
        auto error_entry_count = diagnosis_entry_item->childCount();
        for (int i = 0; i < error_entry_count; i++) {
            auto *error_entry_item = diagnosis_entry_item->child(i);
            auto *error_entry_widget = static_cast<ErrorEntryWidget *>(diagnosis_information_widget_->itemWidget(error_entry_item, 0));
            if (error_entry_widget->GetStatus() == ErrorEntryWidget::Status::kFixed) {
                fixed_count++;
            }
            if (error_entry_widget->ManualRepair()) {
                manual_repair_count++;
            }
            if (error_entry_widget->Selected()) {
                selected_count++;
            }
        }
        if (fixed_count == error_entry_count) {
            // 该诊断项下的错误项全部修复
            diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Unchecked);
            diagnosis_entry_widget->SetCheckedEnable(false);
            diagnosis_entry_widget->SetStatus(DiagnosisEntryWidget::Status::kFixed);
        } else if (fixed_count + manual_repair_count == error_entry_count) {
            // 该诊断项下只剩余手动修复项，其余皆以修复
            diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Unchecked);
            diagnosis_entry_widget->SetCheckedEnable(false);
            diagnosis_entry_widget->SetStatus(DiagnosisEntryWidget::Status::kException);
        } else if (selected_count == error_entry_count) {
            // 所有项都修复失败了
            diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Checked);
            diagnosis_entry_widget->SetCheckedEnable(true);
            diagnosis_entry_widget->SetStatus(DiagnosisEntryWidget::Status::kException);
        } else if (selected_count != 0 && selected_count < error_entry_count) {
            // 部分修复失败了
            diagnosis_entry_widget->SetCheckedState(Qt::CheckState::PartiallyChecked);
            diagnosis_entry_widget->SetCheckedEnable(true);
            diagnosis_entry_widget->SetStatus(DiagnosisEntryWidget::Status::kException);
        } else if (selected_count == 0 && (fixed_count + manual_repair_count < error_entry_count)) {
            // 存在未选中的修复项
            diagnosis_entry_widget->SetCheckedState(Qt::CheckState::Unchecked);
            diagnosis_entry_widget->SetCheckedEnable(true);
            diagnosis_entry_widget->SetStatus(DiagnosisEntryWidget::Status::kException);
        }
    }

    NextRepair();
}

void MainWindow::on_Fixed(QString diagnosis_mark, qint64 error_code, bool ok, QString reason) {
    fixed_count_++;
    progress_bar_->setValue((double)fixed_count_ / need_repair_total_ * 100);

    auto *error_entry_widget = GetErrorEntryWidget(diagnosis_mark, error_code);
    if (error_entry_widget != nullptr) {
        if (ok) {
            error_count_--;
            error_entry_widget->SetCheckState(Qt::CheckState::Unchecked);
            error_entry_widget->SetCheckEnable(false);
            error_entry_widget->SetDescribe("");
            error_entry_widget->SetStatus(ErrorEntryWidget::Status::kFixed);
        } else {
            error_entry_widget->SetDescribe(reason);
        }
    }
}

ErrorEntryWidget *MainWindow::GetErrorEntryWidget(const QString &diagnosis_mark, qint64 error_code) {
    auto top_level_item_count = diagnosis_information_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count - 1; i++) {
        auto *diagnosis_group_item = diagnosis_information_widget_->topLevelItem(i);
        auto diagnosis_entry_count = diagnosis_group_item->childCount();
        for (int j = 0; j < diagnosis_entry_count; j++) {
            auto *diagnosis_entry_item = diagnosis_group_item->child(j);
            auto *diagnosis_entry_widget = static_cast<DiagnosisEntryWidget *>(diagnosis_information_widget_->itemWidget(diagnosis_entry_item, 0));
            if (diagnosis_entry_widget->DiagnosisMark() == diagnosis_mark) {
                auto error_entry_count = diagnosis_entry_item->childCount();
                for (int m = 0; m < error_entry_count; m++) {
                    auto *error_entry_item = diagnosis_entry_item->child(m);
                    auto *error_entry_widget = static_cast<ErrorEntryWidget *>(diagnosis_information_widget_->itemWidget(error_entry_item, 0));
                    if (error_entry_widget->ErrorCode() == error_code) {
                        return error_entry_widget;
                    }
                }
            }
        }
    }
    return nullptr;
}

QString MainWindow::CategoryIcon(const QString &category) {
    if (category == "硬件检测") {
        return "ukui-zs-hard-disk-symbolic";
    }
    if (category == "内核检测") {
        return "ukui-zs-kernel-version-symbolic";
    }
    if (category == "系统服务检测") {
        return "ukui-zs-neicun-symbolic";
    }
    if (category == "系统应用检测") {
        return "ukui-bf-security-symbolic";
    }
    if (category == "应用软件检测") {
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
            auto error_entry_count = diagnosis_entry_item->childCount();
            for (int m = 0; m < error_entry_count; m++) {
                auto *error_entry_item = diagnosis_entry_item->child(m);
                auto *error_entry_widget = static_cast<ErrorEntryWidget *>(diagnosis_information_widget_->itemWidget(error_entry_item, 0));
                if (error_entry_widget->GetStatus() == ErrorEntryWidget::Status::kException) {
                    error_count++;
                }
            }
        }
    }
    return error_count;
}

}
