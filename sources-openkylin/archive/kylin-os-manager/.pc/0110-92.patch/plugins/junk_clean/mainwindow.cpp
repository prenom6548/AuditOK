#include "mainwindow.h"
#include <time.h>
#include <cmath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTreeWidgetItem>
#include <QDebug>
#include "utils.h"
#include "clean_up_group_widget.h"
#include "clean_up_entry_widget.h"
#include "junk_entry_widget.h"

namespace junk_clean {

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      icon_btn_(new QPushButton(this)),
      state_label_(new K::TextLabel(this)),
      detail_label_(new K::TextLabel(this)),
      return_btn_(new QPushButton(this)),
      one_click_clean_btn_(new QPushButton(this)),
      cancel_btn_(new QPushButton(this)),
      progress_bar_(new kdk::KProgressBar(this)),
      clean_up_items_widget_(new QTreeWidget(this)),
      state_(MainWindow::State::kUnknow),
      cancel_(false),
      selected_size_(0),
      selected_trace_(0),
      cleaned_size_(0),
      cleaned_file_count_(0),
      cleaning_start_time_(0),
      total_num_need_scan_(0),
      total_num_scanned_(0),
      total_num_need_clean_(0),
      total_num_cleaned_(0)
{
    icon_btn_->setFlat(true);
    icon_btn_->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    icon_btn_->setFixedSize(64, 64);
    icon_btn_->setIconSize(QSize(64, 64));
    icon_btn_->setIcon(QIcon(":/data/icons/clean_icon_dark.svg"));

    state_label_->SetPixelSize(24, true);
    auto head_label_vbox_layout_ = new QVBoxLayout;
    head_label_vbox_layout_->setContentsMargins(0, 0, 0, 0);
    head_label_vbox_layout_->setSpacing(0);
    head_label_vbox_layout_->addWidget(state_label_);
    head_label_vbox_layout_->addWidget(detail_label_);

    return_btn_->setMinimumSize(96, 36);
    return_btn_->setText(tr("Return"));
    one_click_clean_btn_->setMinimumSize(96, 36);
    one_click_clean_btn_->setText(tr("One click clean"));
    one_click_clean_btn_->setProperty("isImportant", true);
    cancel_btn_->setMinimumSize(96, 36);
    connect(one_click_clean_btn_, &QPushButton::clicked, this, &MainWindow::on_StartClean);
    connect(return_btn_, &QPushButton::clicked, this, &MainWindow::sig_SwitchHomepage);
    connect(cancel_btn_, &QPushButton::clicked, this, &MainWindow::on_Cancel);

    auto head_hbox_layout_ = new QHBoxLayout;
    head_hbox_layout_->setContentsMargins(40, 0, 40, 0);
    head_hbox_layout_->setSpacing(0);
    head_hbox_layout_->addWidget(icon_btn_);
    head_hbox_layout_->addSpacing(16);
    head_hbox_layout_->addLayout(head_label_vbox_layout_);
    head_hbox_layout_->addStretch();
    head_hbox_layout_->addWidget(return_btn_);
    head_hbox_layout_->addSpacing(10);
    head_hbox_layout_->addWidget(one_click_clean_btn_);
    head_hbox_layout_->addWidget(cancel_btn_);

    progress_bar_->setFixedHeight(4);
    progress_bar_->setOrientation(Qt::Horizontal);
    progress_bar_->setTextVisible(false);
    progress_bar_->setRange(0, 100);
    progress_bar_->setValue(0);

    clean_up_items_widget_->setFrameStyle(QFrame::NoFrame);
    clean_up_items_widget_->setColumnCount(1);
    clean_up_items_widget_->setIndentation(0);
    clean_up_items_widget_->header()->setVisible(false);
    clean_up_items_widget_->setStyleSheet("QTreeWidget::item::selected { background-color: transparent; }"
                                          "QTreeWidget::item::hover { background-color: transparent; }");

    auto vbox_layout = new QVBoxLayout;
    vbox_layout->setContentsMargins(0, 24, 0, 0);
    vbox_layout->setSpacing(0);
    vbox_layout->addLayout(head_hbox_layout_);
    vbox_layout->addSpacing(28);
    vbox_layout->addWidget(progress_bar_);
    vbox_layout->addSpacing(16);
    vbox_layout->addWidget(clean_up_items_widget_);
    setLayout(vbox_layout);
}

void MainWindow::on_CleanUpItemsFinish(junk_clean::TypeCleanUpItems type_clean_up_items) {
    for (auto iter = type_clean_up_items.cbegin(); iter != type_clean_up_items.cend(); iter++) {
        auto type = iter.key();
        auto cleaners = iter.value();

        auto *clean_up_group_item = new QTreeWidgetItem(clean_up_items_widget_);
        auto *clean_up_group_widget = new CleanUpGroupWidget(type, this);
        clean_up_group_widget->SetDescribe(cleaners.size(), 0, 0);
        clean_up_items_widget_->setItemWidget(clean_up_group_item, 0, clean_up_group_widget);
        connect(clean_up_group_widget, &CleanUpGroupWidget::sig_Expand, this, &MainWindow::on_Expand);

        for (const auto &i: cleaners) {
            auto *clean_up_entry_item = new QTreeWidgetItem(clean_up_group_item);
            auto *clean_up_entry_widget = new CleanUpEntryWidget(i.mark, type, i.level, this);
            clean_up_entry_widget->SetName(i.name);
            clean_up_entry_widget->SetDescribe(i.describe);
            clean_up_entry_widget->SetSize(0);
            clean_up_items_widget_->setItemWidget(clean_up_entry_item, 0, clean_up_entry_widget);
            connect(clean_up_entry_widget, &CleanUpEntryWidget::sig_Expand, this, &MainWindow::on_Expand);
            connect(clean_up_entry_widget, &CleanUpEntryWidget::sig_CheckBoxStateChanged, this, &MainWindow::on_CleanUpEntryCheckBoxStateChanged);
        }

        // 默认为展开状态
        clean_up_group_widget->SetExpand(true);
    }
}

void MainWindow::on_Expand(bool is_expand) const {
    auto *sender_pointer = sender();

    auto top_level_item_count = clean_up_items_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count; i++) {
        auto *clean_up_group_item = clean_up_items_widget_->topLevelItem(i);
        if (clean_up_items_widget_->itemWidget(clean_up_group_item, 0) == sender_pointer) {
            clean_up_group_item->setExpanded(is_expand);
            return;
        }

        auto clean_up_entry_count = clean_up_group_item->childCount();
        for (int j = 0; j < clean_up_entry_count; j++) {
            auto *clean_up_entry_item = clean_up_group_item->child(j);
            if (clean_up_items_widget_->itemWidget(clean_up_entry_item, 0) == sender_pointer) {
                clean_up_entry_item->setExpanded(is_expand);
                return;
            }
        }
    }
}

void MainWindow::SwitchToInitial() {
    state_ = MainWindow::State::kInitial;

    // 重置界面
    state_label_->SetText("");
    detail_label_->SetText("");
    progress_bar_->setValue(0);
    auto top_level_item_count = clean_up_items_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count; i++) {
        auto *clean_up_group_item = clean_up_items_widget_->topLevelItem(i);
        auto *clean_up_group_widget = static_cast<CleanUpGroupWidget *>(clean_up_items_widget_->itemWidget(clean_up_group_item, 0));
        auto clean_up_entry_count = clean_up_group_item->childCount();
        // 重置清理组
        clean_up_group_widget->SetDescribe(clean_up_entry_count, 0, 0);
        clean_up_group_widget->SetExpand(true);
        for (int j = 0; j < clean_up_entry_count; j++) {
            auto *clean_up_entry_item = clean_up_group_item->child(j);
            auto *clean_up_entry_widget = static_cast<CleanUpEntryWidget *>(clean_up_items_widget_->itemWidget(clean_up_entry_item, 0));
            // 重置清理项
            clean_up_entry_widget->Reset();
            auto junk_entry_count = clean_up_entry_item->childCount();
            // 删除所有垃圾项
            for (int m = junk_entry_count -1 ; m >= 0; m--) {
                auto *junk_entry_item = clean_up_entry_item->child(m);
                auto *junk_entry_widget = static_cast<JunkEntryWidget *>(clean_up_items_widget_->itemWidget(junk_entry_item, 0));
                clean_up_entry_item->removeChild(junk_entry_item);
                delete junk_entry_item;
                delete junk_entry_widget;
            }
        }
    }
}

void MainWindow::SwitchToScanning() {
    state_ = MainWindow::State::kScanning;
    state_label_->SetText(tr("Scanning in progress..."));
    detail_label_->SetText("");
    return_btn_->hide();
    one_click_clean_btn_->hide();
    cancel_btn_->setText(tr("Cancel scan"));
    cancel_btn_->show();
}

void MainWindow::SwitchToCancelingScan() {
    state_ = MainWindow::State::kCancelingScan;
    state_label_->SetText(tr("Canceling in progress"));
    return_btn_->hide();
    one_click_clean_btn_->hide();
    cancel_btn_->show();
}

void MainWindow::SwitchToCancelingClean() {
    state_ = MainWindow::State::kCancelingClean;
    state_label_->SetText(tr("Canceling in progress"));
    return_btn_->hide();
    one_click_clean_btn_->hide();
    cancel_btn_->show();
}

void MainWindow::SwitchToCleanFinished() {
    state_ = MainWindow::State::kCleanFinished;

    // 更新进度条
    if (!cancel_) {
        progress_bar_->setValue(100);
    }

    auto current_time = time(NULL);
    auto time_diff = current_time - cleaning_start_time_;
    if (time_diff == 0) {
        // 最小耗时为 1 秒
        time_diff = 1;
    }
    Q_EMIT sig_CleanFinished(cleaned_size_, cleaned_file_count_, time_diff);
}

void MainWindow::SwitchToScanFinished() {
    state_ = MainWindow::State::kScanFinished;

    auto [total_size, selected_size, total_trace, selected_trace] = JunkSelectState();
    selected_size_ = selected_size;
    selected_trace_ = selected_trace;
    if (total_size == 0) {
        state_label_->SetText(tr("No cleanable files found"));
    } else {
        state_label_->SetText(tr("Found %1 of cleanable files in total").arg(K::Utils::ByteToString(total_size)));
    }
    detail_label_->SetText(tr("Selected %1 of garbage and %2 usage traces").arg(K::Utils::ByteToString(selected_size)).arg(selected_trace));

    // 更新进度条
    if (!cancel_) {
        progress_bar_->setValue(100);
    }

    return_btn_->show();
    if (selected_size == 0 && selected_trace == 0) {
        one_click_clean_btn_->setEnabled(false);
    } else {
        one_click_clean_btn_->setEnabled(true);
    }
    one_click_clean_btn_->show();
    cancel_btn_->hide();
}

void MainWindow::SwitchToCleaning() {
    state_ = MainWindow::State::kCleaning;

    if (selected_size_ != 0) {
        state_label_->SetText(tr("%1 garbage cleaning in progress...").arg(K::Utils::ByteToString(selected_size_)));
    } else {
        state_label_->SetText(tr("Cleaning up %1 traces...").arg(selected_trace_));
    }
    detail_label_->SetText("");

    progress_bar_->setValue(0);

    return_btn_->hide();
    one_click_clean_btn_->hide();
    cancel_btn_->setText(tr("Canecl cleanup"));
    cancel_btn_->show();
}

void MainWindow::StartScan() {
    SwitchToInitial();
    need_scan_.clear();
    cancel_ = false;
    total_num_need_scan_ = 0;
    total_num_scanned_ = 0;

    // 获取需要扫描的项
    auto top_level_item_count = clean_up_items_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count; i++) {
        auto *clean_up_group_item = clean_up_items_widget_->topLevelItem(i);
        auto *clean_up_group_widget = static_cast<CleanUpGroupWidget *>(clean_up_items_widget_->itemWidget(clean_up_group_item, 0));
        auto clean_up_entry_count = clean_up_group_item->childCount();
        for (int j = 0; j < clean_up_entry_count; j++) {
            auto *clean_up_entry_item = clean_up_group_item->child(j);
            auto *clean_up_entry_widget = static_cast<CleanUpEntryWidget *>(clean_up_items_widget_->itemWidget(clean_up_entry_item, 0));
            need_scan_[clean_up_group_widget->Type()].push_back(clean_up_entry_widget->Mark());
            total_num_need_scan_++;
        }
    }

    SwitchToScanning();
    NextScan();
}

void MainWindow::NextScan() {
    QList<enum Type> erase_keys;
    for (auto iter = need_scan_.cbegin(); iter != need_scan_.cend(); iter++) {
        auto type = iter.key();
        auto cleaners = iter.value();
        if (!cleaners.empty()) {
            if (cancel_) {
                qInfo() << "Cancel scan.";
                SwitchToScanFinished();
                break;
            }
            auto cleaner = cleaners.first();
            // 更改清理组界面为“正在扫描垃圾...”
            auto [clean_up_group_item, clean_up_group_widget] = CleanUpGroupWithType(type);
            if (clean_up_group_item != nullptr && clean_up_group_widget != nullptr) {
                clean_up_group_widget->SetDescribe(tr("Scanning for garbage..."));
            }
            // 更改清理项界面为“正在扫描...”
            auto [clean_up_entry_item, clean_up_entry_widget] = CleanUpEntryWithMark(cleaner);
            if (clean_up_entry_item != nullptr && clean_up_entry_widget != nullptr) {
                clean_up_entry_widget->SetSize(CleanUpEntryWidget::SpecialSize::kScanning);
            }
            Q_EMIT sig_Scan(cleaner);
            break;
        } else {
            // 该分类已经扫描完毕
            erase_keys.push_back(type);
            // 更改清理组文本
            auto [clean_up_group_item, clean_up_group_widget] = CleanUpGroupWithType(type);
            if (clean_up_group_item != nullptr && clean_up_group_widget != nullptr) {
                auto [total_size, selected_size] = JunkByteForCategory(type);
                clean_up_group_widget->SetDescribe(clean_up_group_item->childCount(), total_size, selected_size);
            }
        }
    }
    // 移除已经扫描完成的分类
    for (const auto &key: erase_keys) {
        need_scan_.remove(key);
    }

    if (need_scan_.empty()) {
        // 扫描完成
        SwitchToScanFinished();
        qInfo() << "Scanning completed.";
    }
}

void MainWindow::on_ScanForJunk(QString mark, junk_clean::JunkItem junk_item) {
    // XXX: 目前显示具体的垃圾路径，设计稿为显示当前的扫描路径
    detail_label_->SetText(tr("Scanning: %1").arg(junk_item.path));

    auto [clean_up_entry_item, clean_up_entry_widget] = CleanUpEntryWithMark(mark);
    if (clean_up_entry_item == nullptr || clean_up_entry_widget == nullptr) {
        qCritical() << "Get clean up entry fail with " << mark;
        return;
    }
    auto *junk_entry_item = new QTreeWidgetItem(clean_up_entry_item);
    auto *junk_entry_widget = new JunkEntryWidget(junk_item.mark, clean_up_entry_widget->Type(), this);
    if (clean_up_entry_widget->Level() == Level::kNoEffect) {
        junk_entry_widget->SetCheckState(Qt::CheckState::Checked);
    } else {
        junk_entry_widget->SetCheckState(Qt::CheckState::Unchecked);
    }
    junk_entry_widget->SetContent(junk_item.path);
    junk_entry_widget->SetSize(junk_item.size);
    clean_up_items_widget_->setItemWidget(junk_entry_item, 0, junk_entry_widget);
    connect(junk_entry_widget, &JunkEntryWidget::sig_CheckBoxStateChanged, this, &MainWindow::on_JunkEntryCheckBoxStateChanged);
}

void MainWindow::on_ScanFinish(QString mark, quint64 totoal_size) {
    // 从待扫描中移除该扫描项
    for (auto iter = need_scan_.begin(); iter != need_scan_.end(); iter++) {
        if (iter.value().contains(mark)) {
            iter.value().removeAll(mark);
            break;
        }
    }

    // 更新进度条
    total_num_scanned_++;
    progress_bar_->setValue((static_cast<double>(total_num_scanned_) / total_num_need_scan_) * 100);

    // 设置清理项的复选框和下拉框
    auto [clean_up_entry_item, clean_up_entry_widget] = CleanUpEntryWithMark(mark);
    if (clean_up_entry_item != nullptr && clean_up_entry_widget != nullptr) {
        clean_up_entry_widget->ShowCheckBox(true);
        int junk_entry_selected_count{0};
        auto junk_entry_count = clean_up_entry_item->childCount();
        for (int i = 0; i < junk_entry_count; i++) {
            auto *junk_entry_item = clean_up_entry_item->child(i);
            auto *junk_entry_widget = static_cast<JunkEntryWidget *>(clean_up_items_widget_->itemWidget(junk_entry_item, 0));
            if (junk_entry_widget->CheckState() != Qt::CheckState::Unchecked) {
                junk_entry_selected_count++;
            }
        }
        if (junk_entry_count == 0 && clean_up_entry_widget->Level() != Level::kNoEffect) {
            clean_up_entry_widget->SetCheckState(Qt::CheckState::Unchecked);
        } else if (junk_entry_count == 0) {
            clean_up_entry_widget->SetCheckState(Qt::CheckState::Checked);
        } else if (junk_entry_selected_count == 0) {
            clean_up_entry_widget->SetCheckState(Qt::CheckState::Unchecked);
        } else if (junk_entry_selected_count == junk_entry_count) {
            clean_up_entry_widget->SetCheckState(Qt::CheckState::Checked);
        } else {
            clean_up_entry_widget->SetCheckState(Qt::CheckState::PartiallyChecked);
        }

        if (junk_entry_count != 0) {
            clean_up_entry_widget->ShowExpandBtn(true);
        }

        // 设置该清理项的垃圾总大小
        if (totoal_size == 0) {
            clean_up_entry_widget->SetSize(CleanUpEntryWidget::SpecialSize::kVeryClean);
        } else {
            clean_up_entry_widget->SetSize(totoal_size);
        }
    }

    NextScan();
}

std::pair<QTreeWidgetItem *, CleanUpEntryWidget *> MainWindow::CleanUpEntryWithMark(const QString &mark) const {
    auto top_level_item_count = clean_up_items_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count; i++) {
        auto *clean_up_group_item = clean_up_items_widget_->topLevelItem(i);
        auto clean_up_entry_count = clean_up_group_item->childCount();
        for (int j = 0; j < clean_up_entry_count; j++) {
            auto *clean_up_entry_item = clean_up_group_item->child(j);
            auto *clean_up_entry_widget = static_cast<CleanUpEntryWidget *>(clean_up_items_widget_->itemWidget(clean_up_entry_item, 0));
            if (clean_up_entry_widget->Mark() == mark) {
                return {clean_up_entry_item, clean_up_entry_widget};
            }
        }
    }
    return {nullptr, nullptr};
}

std::pair<QTreeWidgetItem *, CleanUpGroupWidget *> MainWindow::CleanUpGroupWithType(enum Type type) const {
    auto top_level_item_count = clean_up_items_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count; i++) {
        auto *clean_up_group_item = clean_up_items_widget_->topLevelItem(i);
        auto *clean_up_group_widget = static_cast<CleanUpGroupWidget *>(clean_up_items_widget_->itemWidget(clean_up_group_item, 0));
        if (clean_up_group_widget->Type() == type) {
            return {clean_up_group_item, clean_up_group_widget};
        }
    }
    return {nullptr, nullptr};
}

std::pair<QTreeWidgetItem *, CleanUpGroupWidget*> MainWindow::CleanUpGroupWithMark(const QString &mark) const {
    auto top_level_item_count = clean_up_items_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count; i++) {
        auto *clean_up_group_item = clean_up_items_widget_->topLevelItem(i);
        auto *clean_up_group_widget = static_cast<CleanUpGroupWidget *>(clean_up_items_widget_->itemWidget(clean_up_group_item, 0));
        auto clean_up_entry_count = clean_up_group_item->childCount();
        for (int j = 0; j < clean_up_entry_count; j++) {
            auto *clean_up_entry_item = clean_up_group_item->child(j);
            auto *clean_up_entry_widget = static_cast<CleanUpEntryWidget *>(clean_up_items_widget_->itemWidget(clean_up_entry_item, 0));
            if (clean_up_entry_widget->Mark() == mark) {
                return {clean_up_group_item, clean_up_group_widget};
            }
        }
    }
    return {nullptr, nullptr};
}

void MainWindow::on_JunkEntryCheckBoxStateChanged() {
    auto *sender_pointer = sender();

    auto top_level_item_count = clean_up_items_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count; i++) {
        auto *clean_up_group_item = clean_up_items_widget_->topLevelItem(i);
        auto *clean_up_group_widget = static_cast<CleanUpGroupWidget *>(clean_up_items_widget_->itemWidget(clean_up_group_item, 0));
        auto clean_up_entry_count = clean_up_group_item->childCount();
        for (int j = 0; j < clean_up_entry_count; j++) {
            auto *clean_up_entry_item = clean_up_group_item->child(j);
            auto *clean_up_entry_widget = static_cast<CleanUpEntryWidget *>(clean_up_items_widget_->itemWidget(clean_up_entry_item, 0));
            auto junk_entry_count = clean_up_entry_item->childCount();
            for (int m = 0; m < junk_entry_count; m++) {
                auto *junk_entry_item = clean_up_entry_item->child(m);
                auto *junk_entry_widget = clean_up_items_widget_->itemWidget(junk_entry_item, 0);
                if (junk_entry_widget == sender_pointer) {
                    // 找到了具体的垃圾项
                    int selected_count{0};
                    for (int n = 0; n < junk_entry_count; n++) {
                        auto *item = clean_up_entry_item->child(n);
                        auto *widget = static_cast<JunkEntryWidget *>(clean_up_items_widget_->itemWidget(item, 0));
                        if (widget->CheckState() == Qt::CheckState::Checked) {
                            selected_count++;
                        }
                    }
                    if (selected_count == 0) {
                        clean_up_entry_widget->SetCheckState(Qt::CheckState::Unchecked);
                    } else if (selected_count == junk_entry_count) {
                        clean_up_entry_widget->SetCheckState(Qt::CheckState::Checked);
                    } else {
                        clean_up_entry_widget->SetCheckState(Qt::CheckState::PartiallyChecked);
                    }
                    // 重新计算该清理组下已选中垃圾的大小
                    auto [total_size, selected_size] = JunkByteForCategory(clean_up_group_widget->Type());
                    clean_up_group_widget->SetDescribe(clean_up_group_item->childCount(), total_size, selected_size);

                    // 如果处于扫描结束状态，重新计算头部状态栏中选中垃圾的大小和选中痕迹的个数
                    // 此处直接复用 SwitchToSacnFinished 函数
                    if (state_ == MainWindow::State::kScanFinished) {
                        SwitchToScanFinished();
                    }
                    return;
                }
            }
        }
    }
}

void MainWindow::on_CleanUpEntryCheckBoxStateChanged() {
    auto *sender_pointer = sender();

    auto top_level_item_count = clean_up_items_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count; i++) {
        auto *clean_up_group_item = clean_up_items_widget_->topLevelItem(i);
        auto *clean_up_group_widget = static_cast<CleanUpGroupWidget *>(clean_up_items_widget_->itemWidget(clean_up_group_item, 0));
        auto clean_up_entry_count = clean_up_group_item->childCount();
        for (int j = 0; j < clean_up_entry_count; j++) {
            auto *clean_up_entry_item = clean_up_group_item->child(j);
            auto *clean_up_entry_widgdet = static_cast<CleanUpEntryWidget *>(clean_up_items_widget_->itemWidget(clean_up_entry_item, 0));
            if (clean_up_entry_widgdet == sender_pointer) {
                // 找到了具体的清理项
                auto check_state = clean_up_entry_widgdet->CheckState();
                // Qt::CheckState::PartiallyChecked 状态当作选中状态来处理
                if (check_state == Qt::CheckState::PartiallyChecked) {
                    clean_up_entry_widgdet->SetCheckState(Qt::CheckState::Checked);
                    check_state = Qt::CheckState::Checked;
                }
                auto junk_entry_count = clean_up_entry_item->childCount();
                for (int m = 0; m < junk_entry_count; m++) {
                    auto *junk_entry_item = clean_up_entry_item->child(m);
                    auto *junk_entry_widget = static_cast<JunkEntryWidget *>(clean_up_items_widget_->itemWidget(junk_entry_item, 0));
                    if (check_state == Qt::CheckState::Checked) {
                        junk_entry_widget->SetCheckState(Qt::CheckState::Checked);
                    } else {
                        junk_entry_widget->SetCheckState(Qt::CheckState::Unchecked);
                    }
                }
                // 重新计算该清理组中已选中垃圾项的大小
                auto [total_size, selected_size] = JunkByteForCategory(clean_up_group_widget->Type());
                clean_up_group_widget->SetDescribe(clean_up_group_item->childCount(), total_size, selected_size);

                // 如果处于扫描结束状态，重新计算头部状态栏中选中垃圾的大小和选中痕迹的个数
                // 此处直接复用 SwitchToSacnFinished 函数
                if (state_ == MainWindow::State::kScanFinished) {
                    SwitchToScanFinished();
                }
                return;
            }
        }
    }
}

std::pair<quint64, quint64> MainWindow::JunkByteForCategory(enum Type type) const {
    auto [clean_up_group_item, clean_up_group_widget] = CleanUpGroupWithType(type);
    if (clean_up_group_item == nullptr || clean_up_group_widget == nullptr) {
        return {0, 0};
    }
    quint64 total_size{0};
    quint64 selected_size{0};
    auto clean_up_entry_count = clean_up_group_item->childCount();
    for (int i = 0; i < clean_up_entry_count; i++) {
        auto *clean_up_entry_item = clean_up_group_item->child(i);
        auto *clean_up_entry_widget = static_cast<CleanUpEntryWidget *>(clean_up_items_widget_->itemWidget(clean_up_entry_item, 0));
        auto junk_entry_count = clean_up_entry_item->childCount();
        // 如果该清理项没有对应的垃圾项，就取该清理项的大小
        if (junk_entry_count == 0) {
            auto clean_up_entry_size = clean_up_entry_widget->Size();
            if (clean_up_entry_widget->CheckState() != Qt::CheckState::Unchecked) {
                total_size += clean_up_entry_size;
                selected_size += clean_up_entry_size;
            } else {
                total_size += clean_up_entry_size;
            }
            continue;
        }
        for (int j = 0; j < junk_entry_count; j++) {
            auto *junk_entry_item = clean_up_entry_item->child(j);
            auto *junk_entry_widget = static_cast<JunkEntryWidget *>(clean_up_items_widget_->itemWidget(junk_entry_item, 0));
            auto junk_entry_size = junk_entry_widget->Size();
            if (junk_entry_widget->CheckState() == Qt::CheckState::Checked) {
                total_size += junk_entry_size;
                selected_size += junk_entry_size;
            } else {
                total_size += junk_entry_size;
            }
        }
    }
    return {total_size, selected_size};
}

std::tuple<quint64, quint64, quint64, quint64> MainWindow::JunkSelectState() const {
    quint64 total_size{0};
    quint64 selected_size{0};
    quint64 total_trace{0};
    quint64 selected_trace{0};

    auto top_level_item_count = clean_up_items_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count; i++) {
        auto *clean_up_group_item = clean_up_items_widget_->topLevelItem(i);
        auto *clean_up_group_widget = static_cast<CleanUpGroupWidget *>(clean_up_items_widget_->itemWidget(clean_up_group_item, 0));
        auto [total, selected] = JunkByteForCategory(clean_up_group_widget->Type());
        switch (clean_up_group_widget->Type()) {
            case Type::kSystemJunk:
                [[fallthrough]];
            case Type::kInternetJunk:
                total_size += total;
                selected_size += selected;
                break;
            case Type::kUsageTraces:
                total_trace += total;
                selected_trace += selected;
                break;
            default:
                break;
        }
    }
    return {total_size, selected_size, total_trace, selected_trace};
}

void MainWindow::on_StartClean() {
    need_clean_.clear();
    cancel_ = false;
    cleaned_size_ = 0;
    cleaned_file_count_ = 0;
    cleaning_start_time_ = time(NULL);
    total_num_need_clean_ = 0;
    total_num_cleaned_ = 0;

    // 收集需要清理的项
    auto top_level_item_count = clean_up_items_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count; i++) {
        auto *clean_up_group_item = clean_up_items_widget_->topLevelItem(i);
        auto *clean_up_group_widget = static_cast<CleanUpGroupWidget *>(clean_up_items_widget_->itemWidget(clean_up_group_item, 0));
        auto clean_up_entry_count = clean_up_group_item->childCount();
        for (int j = 0; j < clean_up_entry_count; j++) {
            auto *clean_up_entry_item = clean_up_group_item->child(j);
            auto *clean_up_entry_widget = static_cast<CleanUpEntryWidget *>(clean_up_items_widget_->itemWidget(clean_up_entry_item, 0));
            if (clean_up_entry_widget->CheckState() == Qt::CheckState::Unchecked || clean_up_entry_widget->Size() == 0) {
                continue;
            }

            CleanTask clean_task;
            clean_task.cleaner_mark = clean_up_entry_widget->Mark();
            auto junk_entry_count = clean_up_entry_item->childCount();
            for (int m = 0; m < junk_entry_count; m++) {
                auto *junk_entry_item = clean_up_entry_item->child(m);
                auto *junk_entry_widget = static_cast<JunkEntryWidget *>(clean_up_items_widget_->itemWidget(junk_entry_item, 0));
                if (junk_entry_widget->CheckState() != Qt::CheckState::Unchecked) {
                    clean_task.junk_marks.push_back(junk_entry_widget->Mark());
                }
            }
            need_clean_[clean_up_group_widget->Type()].push_back(clean_task);
            total_num_need_clean_++;
        }
    }

    SwitchToCleaning();
    NextClean();
}

void MainWindow::NextClean() {
    QList<enum Type> erase_keys;
    for (auto iter = need_clean_.cbegin(); iter != need_clean_.cend(); iter++) {
        auto type = iter.key();
        auto clean_tasks = iter.value();
        if (!clean_tasks.empty()) {
            if (cancel_) {
                SwitchToCleanFinished();
                qInfo() << "Cancel clean.";
                break;
            }
            auto task = clean_tasks.first();
            // 把该清理项所在的清理组设置为“正在清理垃圾...”
            auto [clean_up_group_item, clean_up_group_widget] = CleanUpGroupWithType(type);
            if (clean_up_group_item != nullptr && clean_up_group_widget != nullptr) {
                clean_up_group_widget->SetDescribe(tr("Cleaning up garbage..."));
            }
            Q_EMIT sig_Clean(task);
            break;
        } else {
            // 该清理组清理完毕
            erase_keys.push_back(type);
            // 设置该清理组的描述文本
            auto [clean_up_group_item, clean_up_group_widget] = CleanUpGroupWithType(type);
            auto [total_size, selected_size] = JunkByteForCategory(type);
            if (clean_up_group_item != nullptr && clean_up_group_widget != nullptr) {
                clean_up_group_widget->SetDescribe(clean_up_group_item->childCount(), total_size, selected_size);
            }
        }
    }

    // 移除清理完毕的项
    for (const auto &key: erase_keys) {
        need_clean_.remove(key);
    }

    if (need_clean_.empty()) {
        // 清理完毕
        SwitchToCleanFinished();
        qInfo() << "Cleaning completed.";
    }
}

void MainWindow::on_CleanForJunk(QString cleaner_mark, quint64 junk_mark) {
    auto top_level_item_count = clean_up_items_widget_->topLevelItemCount();
    for (int i = 0; i < top_level_item_count; i++) {
        auto *clean_up_group_item = clean_up_items_widget_->topLevelItem(i);
        auto clean_up_entry_count = clean_up_group_item->childCount();
        for (int j = 0; j < clean_up_entry_count; j++) {
            auto *clean_up_entry_item = clean_up_group_item->child(j);
            auto *clean_up_entry_widget = static_cast<CleanUpEntryWidget *>(clean_up_items_widget_->itemWidget(clean_up_entry_item, 0));
            if (clean_up_entry_widget->Mark() == cleaner_mark) {
                auto junk_entry_count = clean_up_entry_item->childCount();
                for (int m = 0; m < junk_entry_count; m++) {
                    auto *junk_entry_item = clean_up_entry_item->child(m);
                    auto *junk_entry_widget = static_cast<JunkEntryWidget *>(clean_up_items_widget_->itemWidget(junk_entry_item, 0));
                    if (junk_entry_widget->Mark() == junk_mark) {
                        // 找到了具体的垃圾项
                        detail_label_->SetText(tr("Cleaning up: %1").arg(junk_entry_widget->Content()));
                        // 记录清理掉的垃圾大小和文件数量
                        cleaned_size_ += junk_entry_widget->Size();
                        cleaned_file_count_ += junk_entry_widget->FileCount();
                        // 移除掉该垃圾项
                        clean_up_entry_item->removeChild(junk_entry_item);
                        delete junk_entry_item;
                        delete junk_entry_widget;
                        // 重新计算该清理项中垃圾项的大小
                        quint64 total_size{0};
                        auto current_junk_count = clean_up_entry_item->childCount();
                        for (int n = 0; n < current_junk_count; n++) {
                            auto *current_junk_entry_item = clean_up_entry_item->child(n);
                            auto *current_junk_entry_widget = static_cast<JunkEntryWidget *>(clean_up_items_widget_->itemWidget(current_junk_entry_item, 0));
                            total_size += current_junk_entry_widget->Size();
                        }
                        if (total_size == 0) {
                            clean_up_entry_widget->SetSize(CleanUpEntryWidget::SpecialSize::kVeryClean);
                        } else {
                            clean_up_entry_widget->SetSize(total_size);
                        }

                        return;
                    }
                }
            }
        }
    }
}

void MainWindow::on_CleanFinish(QString cleaner_mark) {
    // 从 need_clean 中移除掉该清理项
    for (auto iter = need_clean_.begin(); iter != need_clean_.end(); iter ++) {
        for (auto task_iter = iter.value().begin(); task_iter != iter.value().end(); task_iter++) {
            if (task_iter->cleaner_mark == cleaner_mark) {
                iter.value().erase(task_iter);
                goto FLAG;
            }
        }
    }

FLAG:
    // 更新进度条
    total_num_cleaned_++;
    progress_bar_->setValue((static_cast<double >(total_num_cleaned_) / total_num_need_clean_) * 100);

    auto [clean_up_entry_item, clean_up_entry_widget] = CleanUpEntryWithMark(cleaner_mark);
    if (clean_up_entry_item != nullptr && clean_up_entry_widget != nullptr) {
        auto junk_entry_count = clean_up_entry_item->childCount();
        if (junk_entry_count == 0) {
            clean_up_entry_widget->ShowExpandBtn(false);
            clean_up_entry_widget->SetSize(CleanUpEntryWidget::SpecialSize::kVeryClean);
        }
    }

    NextClean();
}

void MainWindow::on_Cancel() {
    cancel_ = true;
    if (state_ == MainWindow::State::kScanning) {
        SwitchToCancelingScan();
    } else if (state_ == MainWindow::State::kCleaning) {
        SwitchToCancelingClean();
    }
}

}
