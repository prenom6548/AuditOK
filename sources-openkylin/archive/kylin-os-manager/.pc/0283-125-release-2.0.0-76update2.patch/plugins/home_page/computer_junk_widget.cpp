#include "computer_junk_widget.h"
#include <cmath>
#include <QPainter>
#include <QPixmap>
#include <QFont>
#include <QFontMetrics>
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QMutexLocker>
#include <QMetaType>
#include "frame.h"
#include "utils.h"

namespace home_page {

ComputerJunkWidget::ComputerJunkWidget(QWidget *parent)
    : QWidget(parent),
      size_widget_(new SizeWidget(this)),
      system_junk_size_label_(new K::TextLabel(this)),
      internet_junk_size_label_(new K::TextLabel(this)),
      usage_traces_size_label_(new K::TextLabel(this)),
      one_click_clean_btn_(new QPushButton(this))
{
    qRegisterMetaType<junk_clean::JunkItem>();
    setMinimumSize(237, 342);

    // 标题
    auto *title_label = new K::TextLabel(this);
    title_label->SetPixelSize(16, true);
    title_label->SetText(tr("Computer junk"));
    auto *title_hbox_layout = new QHBoxLayout;
    title_hbox_layout->setContentsMargins(0, 0, 0, 0);
    title_hbox_layout->setSpacing(0);
    title_hbox_layout->addStretch();
    title_hbox_layout->addWidget(title_label);
    title_hbox_layout->addStretch();

    // 大小
    size_widget_->setFixedSize(122, 122);
    auto *size_hbox_layout = new QHBoxLayout;
    size_hbox_layout->setContentsMargins(0, 0, 0, 0);
    size_hbox_layout->setSpacing(0);
    size_hbox_layout->addStretch();
    size_hbox_layout->addWidget(size_widget_);
    size_hbox_layout->addStretch();

    // 信息
    system_junk_size_label_->SetPixelSize(14, true);
    system_junk_size_label_->SetAlignment(Qt::AlignHCenter);
    system_junk_size_label_->SetText("--");
    auto *system_junk_text_label = new K::TextLabel(this);
    system_junk_text_label->SetAlignment(Qt::AlignHCenter);
    system_junk_text_label->SetFontColor(K::TextLabel::FontColor::kPlaceholderText);
    system_junk_text_label->SetText(tr("System junk"));
    auto *system_junk_vbox_layout = new QVBoxLayout;
    system_junk_vbox_layout->setContentsMargins(0, 0, 0, 0);
    system_junk_vbox_layout->setSpacing(0);
    system_junk_vbox_layout->addWidget(system_junk_size_label_);
    system_junk_vbox_layout->addWidget(system_junk_text_label);

    internet_junk_size_label_->SetPixelSize(14, true);
    internet_junk_size_label_->SetAlignment(Qt::AlignHCenter);
    internet_junk_size_label_->SetText("--");
    auto *internet_junk_text_label = new K::TextLabel(this);
    internet_junk_text_label->SetAlignment(Qt::AlignHCenter);
    internet_junk_text_label->SetFontColor(K::TextLabel::FontColor::kPlaceholderText);
    internet_junk_text_label->SetText(tr("Internet junk"));
    auto *internet_junk_vbox_layout = new QVBoxLayout;
    internet_junk_vbox_layout->setContentsMargins(0, 0, 0, 0);
    internet_junk_vbox_layout->setSpacing(0);
    internet_junk_vbox_layout->addWidget(internet_junk_size_label_);
    internet_junk_vbox_layout->addWidget(internet_junk_text_label);

    usage_traces_size_label_->SetPixelSize(14, true);
    usage_traces_size_label_->SetAlignment(Qt::AlignHCenter);
    usage_traces_size_label_->SetText("--");
    auto *usage_traces_text_label = new K::TextLabel(this);
    usage_traces_text_label->SetAlignment(Qt::AlignHCenter);
    usage_traces_text_label->SetFontColor(K::TextLabel::FontColor::kPlaceholderText);
    usage_traces_text_label->SetText(tr("Usage traces"));
    auto *usage_traces_vbox_layout = new QVBoxLayout;
    usage_traces_vbox_layout->setContentsMargins(0, 0, 0, 0);
    usage_traces_vbox_layout->setSpacing(0);
    usage_traces_vbox_layout->addWidget(usage_traces_size_label_);
    usage_traces_vbox_layout->addWidget(usage_traces_text_label);

    auto *information_hbox_layout = new QHBoxLayout;
    information_hbox_layout->setContentsMargins(0, 0, 0, 0);
    information_hbox_layout->setSpacing(0);
    information_hbox_layout->addStretch();
    information_hbox_layout->addLayout(system_junk_vbox_layout);
    information_hbox_layout->addSpacing(12);
    information_hbox_layout->addLayout(internet_junk_vbox_layout);
    information_hbox_layout->addSpacing(12);
    information_hbox_layout->addLayout(usage_traces_vbox_layout);
    information_hbox_layout->addStretch();

    // 一键清理
    one_click_clean_btn_->setProperty("isImportant", true);
    one_click_clean_btn_->setProperty("current_mode", BtnMode::kClean);
    one_click_clean_btn_->setText(tr("One click clean"));
    one_click_clean_btn_->setEnabled(false);
    auto *one_click_clean_hbox_layout = new QHBoxLayout;
    one_click_clean_hbox_layout->setContentsMargins(0, 0, 0, 0);
    one_click_clean_hbox_layout->setSpacing(0);
    one_click_clean_hbox_layout->addStretch();
    one_click_clean_hbox_layout->addWidget(one_click_clean_btn_);
    one_click_clean_hbox_layout->addStretch();
    connect(one_click_clean_btn_, &QPushButton::clicked, this, &ComputerJunkWidget::on_BtnClick);

    // 总布局
    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setContentsMargins(0, 24, 0, 32);
    vbox_layout->setSpacing(0);
    vbox_layout->addLayout(title_hbox_layout);
    vbox_layout->addSpacing(18);
    vbox_layout->addLayout(size_hbox_layout);
    vbox_layout->addSpacing(16);
    vbox_layout->addLayout(information_hbox_layout);
    vbox_layout->addStretch();
    vbox_layout->addLayout(one_click_clean_hbox_layout);
    setLayout(vbox_layout);

    // 因为首页的垃圾清理实例出了所有的清理项，部分清理项会依赖 dbus 接口
    // 这部分清理项和垃圾清理插件的这部分清理项会共用一个 dbus 后端，两者同时运行时会互相影响
    // 所以如果直接带参运行垃圾清理功能，会造成崩溃
    // 此处判断，是否为带参启动，如果是带参启动，则首页的扫描功能不在运行
    // 一种可行的方式：首页只实例和使用不依赖 dbus 接口并且级别为无影响的这部分清理项
    auto base_data = Frame::GetBaseData();
    if (base_data != nullptr && base_data->start_with_arg == false) {
        // 非带参启动情况
        connect(&work_thread_, &home_page::ComputerJunkWorkThread::sig_ScanFinish, this, &home_page::ComputerJunkWidget::on_ScanFinish);
        connect(&work_thread_, &home_page::ComputerJunkWorkThread::sig_CleanFinish, this, &home_page::ComputerJunkWidget::on_CleanFinish);
        work_thread_.start();
    } else {
        // 带参启动则不再进行后台扫描，直接显示深度清理
        one_click_clean_btn_->setText(tr("Deep clean"));
        one_click_clean_btn_->setProperty("current_mode", BtnMode::kDeepClean);
        one_click_clean_btn_->setEnabled(true);
    }
}

ComputerJunkWidget::~ComputerJunkWidget() {

}

void ComputerJunkWidget::SetResult(quint64 total_size, quint64 system_junk_size, quint64 internet_junk_size, quint64 usage_traces) {
    auto [size, unit] = TotalSizeForDisplay(total_size);
    size_widget_->SetSize(size, unit);
    system_junk_size_label_->SetText(K::Utils::ByteToString(system_junk_size));
    internet_junk_size_label_->SetText(K::Utils::ByteToString(internet_junk_size));
    usage_traces_size_label_->SetText(K::Utils::ByteToString(usage_traces));
}

std::pair<QString, QString> ComputerJunkWidget::TotalSizeForDisplay(quint64 total_size) {
    static const QList<QString> units = {"B", "KB", "MB", "GB"};
    int unit_index{0};
    auto value = static_cast<double>(total_size);

    do {
        // 进位
        while (value >= 1000.0 && unit_index < units.size() - 1) {
            value /= 1024;
            unit_index++;
        }

        if (value < 100.0) {
            break;
        }

        // 四舍五入取整，取整后判断是否需要进位
        auto rounded = std::round(value);
        if (rounded < 1000.0 || unit_index >= units.size() - 1) {
            value = rounded;
            break;
        } else {
            value = rounded / 1024.0;
            unit_index++;
        }
    } while (true);

    if (value < 100.0) {
        auto target = QString::number(value, 'f', 1);
        if (target.contains('.') && target.back() == '0') {
            // 移除 0 和 .
            target.chop(2);
        }
        return std::make_pair(target, units[unit_index]);
    }

    return std::make_pair(QString::number(static_cast<quint64>(value)), units[unit_index]);
}

void ComputerJunkWidget::on_ScanFinish() {
    auto [total_size, system_junk_size, internet_junk_size, usage_traces] = work_thread_.GetResult();
    SetResult(total_size, system_junk_size, internet_junk_size, usage_traces);

    if (total_size == 0) {
        // 未扫到垃圾，直接显示深度清理
        one_click_clean_btn_->setText(tr("Deep clean"));
        one_click_clean_btn_->setProperty("current_mode", BtnMode::kDeepClean);
        one_click_clean_btn_->setEnabled(true);
    } else {
        // 扫描到垃圾，按钮显示一键清理
        one_click_clean_btn_->setText(tr("One click clean"));
        one_click_clean_btn_->setProperty("current_mode", BtnMode::kClean);
        one_click_clean_btn_->setEnabled(true);
    }
}

void ComputerJunkWidget::on_CleanFinish() {
    auto [total_size, system_junk_size, internet_junk_size, usage_traces] = work_thread_.GetResult();
    SetResult(total_size, system_junk_size, internet_junk_size, usage_traces);

    one_click_clean_btn_->setText(tr("Deep clean"));
    one_click_clean_btn_->setProperty("current_mode", BtnMode::kDeepClean);
    one_click_clean_btn_->setEnabled(true);
}

void ComputerJunkWidget::on_BtnClick() {
    auto current_mode = one_click_clean_btn_->property("current_mode").toInt();
    switch (current_mode) {
        case BtnMode::kClean: {
            one_click_clean_btn_->setText(tr("Cleaning..."));
            one_click_clean_btn_->setEnabled(false);
            work_thread_.SetMode(ComputerJunkWorkThread::Mode::kClean);
            work_thread_.start();
            break;
        }
        case BtnMode::kDeepClean: {
            Frame::RunJunkClean();
            break;
        }
        default:
            break;
    }
}

SizeWidget::SizeWidget(QWidget *parent)
    : QWidget(parent),
      size_("--"),
      unit_("")
{

}

SizeWidget::~SizeWidget() {

}

void SizeWidget::SetSize(QString size, QString unit) {
    size_ = size;
    unit_ = unit;
    update();
}

void SizeWidget::paintEvent(QPaintEvent *evnet) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    // 画背景图
    auto pixmap = QPixmap(":/home_page/data/icons/default_computer_junk_background.svg").scaled(size(), Qt::KeepAspectRatio);
    p.drawPixmap(0, 0, pixmap);

    // 画大小
    QFont num_font;
    num_font.setPixelSize(42);
    QFontMetrics num_metrics(num_font);

    QFont unit_font;
    unit_font.setPixelSize(14);
    QFontMetrics unit_metrics(unit_font);

    auto num_width = num_metrics.horizontalAdvance(size_);
    auto unit_width = unit_metrics.horizontalAdvance(unit_);
    auto total_width = num_width + unit_width;

    auto num_start_x = (width() - total_width) / 2;
    auto num_start_y = height() / 2 + (num_metrics.ascent() - num_metrics.descent()) / 2;
    p.setFont(num_font);
    p.drawText(num_start_x, num_start_y, size_);

    auto unit_start_x = num_start_x + num_width;
    auto unit_start_y = num_start_y;
    p.setFont(unit_font);
    p.drawText(unit_start_x, unit_start_y, unit_);
}

ComputerJunkWorkThread::ComputerJunkWorkThread(QObject *parent)
    : QThread(parent),
      cleaner_manager_(nullptr),
      mode_(Mode::kScan)
{

}

ComputerJunkWorkThread::~ComputerJunkWorkThread() {

}

void ComputerJunkWorkThread::SetMode(enum Mode mode) {
    mode_ = mode;
}

void ComputerJunkWorkThread::run() {
    switch (mode_) {
        case Mode::kScan:
            Scan();
            break;
        case Mode::kClean:
            Clean();
            break;
        default:
            qWarning() << "unsupported operation mode, computer junk work thread exits";
            break;
    }
}

void ComputerJunkWorkThread::Scan() {
    scan_result_mutex_.lock();
    scan_result_.clear();

    auto cleaners = cleaner_manager_.Cleaners();
    auto iter = cleaners.cbegin();

    QMap<QString, QList<junk_clean::JunkItem>> scan_result;
    auto scan_current_item = [this, &iter, &cleaners, &scan_result]() {
        while (true) {
            if (iter == cleaners.cend()) {
                // 扫描完毕
                qInfo() << "all clean item have been scanned and completed";
                cleaner_manager_.disconnect(this);
                scan_result_.swap(scan_result);
                scan_result_mutex_.unlock();
                Q_EMIT sig_ScanFinish();
                quit();
                return;
            }

            if (iter.value()->Level() == junk_clean::Level::kNoEffect &&
                iter.value()->Mark() != "boot_partition_cleaner")
            {
                // boot 分区清理项不进行扫描
                qInfo() << "start scan " << iter.value()->Mark() << "...";
                iter.value()->Scan();
                break;
            } else {
                iter++;
            }
        }
    };

    connect(&cleaner_manager_, &junk_clean::CleanerManager::sig_ScanForJunk, this, [&scan_result](QString mark, junk_clean::JunkItem junk_item) {
        scan_result[mark].push_back(junk_item);
    });
    connect(&cleaner_manager_, &junk_clean::CleanerManager::sig_ScanFinish, this, [&iter, &scan_current_item](QString mark, quint64 total_size) {
        qInfo() << "clean item " << mark << " has been scanned and completed";
        iter++;
        scan_current_item();
    });

    scan_current_item();

    exec();
}

void ComputerJunkWorkThread::Clean() {
    scan_result_mutex_.lock();

    auto cleaners = cleaner_manager_.Cleaners();
    auto iter = scan_result_.begin();

    auto clean_current_item = [this, &iter, &cleaners]() {
        while (true) {
            if (iter == scan_result_.end()) {
                // 清理完毕
                qInfo() << "all clean items have been cleared";
                cleaner_manager_.disconnect(this);
                scan_result_mutex_.unlock();
                Q_EMIT sig_CleanFinish();
                quit();
                return;
            }

            if (iter.value().isEmpty()) {
                iter++;
                continue;
            }

            auto target_iter = cleaners.find(iter.key());
            if (target_iter == cleaners.end()) {
                iter++;
                continue;
            }

            QList<quint64> junk_marks;
            for (const auto &junk_item: iter.value()) {
                junk_marks.push_back(junk_item.mark);
            }

            qInfo()<< "start cleaning item " << iter.key();
            target_iter.value()->Clean(junk_marks);
            break;
        }
    };

    connect(&cleaner_manager_, &junk_clean::CleanerManager::sig_CleanForJunk, this, [this](QString cleaner_mark, quint64 junk_mark) {
        auto target_iter = scan_result_.find(cleaner_mark);
        if (target_iter == scan_result_.end()) {
            qWarning() << "junk that cannot find clean item has been cleared";
            return;
        }
        for (auto junk_item_iter = target_iter.value().begin(); junk_item_iter != target_iter.value().end(); junk_item_iter++) {
            if (junk_item_iter->mark == junk_mark) {
                qInfo() << "clean: " <<  junk_item_iter->path;
                target_iter.value().erase(junk_item_iter);
                break;
            }
        }
    });
    connect(&cleaner_manager_, &junk_clean::CleanerManager::sig_CleanFinish, this, [&iter, &clean_current_item](QString cleaner_mark) {
        qInfo() << "cleanup item " << cleaner_mark << " has been completed";
        iter++;
        clean_current_item();
    });

    clean_current_item();

    exec();
}

std::tuple<quint64, quint64, quint64, quint64> ComputerJunkWorkThread::GetResult() {
    QMutexLocker locker(&scan_result_mutex_);

    quint64 total_size{0};
    quint64 system_junk_size{0};
    quint64 internet_junk_size{0};
    quint64 usage_traces{0};
    auto cleaners = cleaner_manager_.Cleaners();

    if (scan_result_.isEmpty()) {
        // 无垃圾
        return {0, 0, 0, 0};
    }

    for (auto iter = scan_result_.cbegin(); iter != scan_result_.cend(); iter++) {
        if (iter.value().isEmpty()) {
            continue;
        }

        auto target_iter = cleaners.find(iter.key());
        if (target_iter == cleaners.end()) {
            qWarning() << "no " << iter.key() << " clean item found, skip count this clean item";
            continue;
        }

        auto type = target_iter.value()->Type();
        switch (type) {
            case junk_clean::Type::kSystemJunk: {
                for (const auto &junk_item: iter.value()) {
                    system_junk_size += junk_item.size;
                    total_size += junk_item.size;
                }
                break;
            }
            case junk_clean::Type::kInternetJunk: {
                for (const auto &junk_item: iter.value()) {
                    internet_junk_size += junk_item.size;
                    total_size += junk_item.size;
                }
                break;
            }
            case junk_clean::Type::kUsageTraces: {
                for (const auto &junk_item: iter.value()) {
                    usage_traces += junk_item.size;
                    total_size += junk_item.size;
                }
                break;
            }
            default: {
                qWarning() << "there are no match type";
                break;
            }
        }
    }

    return {total_size, system_junk_size, internet_junk_size, usage_traces};
}

}
