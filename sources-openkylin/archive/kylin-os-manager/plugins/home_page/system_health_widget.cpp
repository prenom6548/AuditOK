#include "system_health_widget.h"
#include <cmath>
#include <thread>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QStyleOption>
#include <QPixmap>
#include <QFontMetrics>
#include <QDebug>
#include <QPalette>
#include <QApplication>
#include <QColor>
#include <QDebug>
#include <QList>
#include <QMetaType>
#include "text_label.h"
#include "complete_machine.h"
#include <frame.h>
#include <gsettingmonitor.h>

Q_DECLARE_METATYPE(fault_diagnosis::DiagnosticResult)

namespace home_page {

SystemHealthWidget::SystemHealthWidget(QWidget *parent)
    : QWidget(parent),
      score_widget_(new ScoreWidget(this)),
      network_fault_num_label_(new K::TextLabel(this)),
      system_fault_num_label_(new K::TextLabel(this)),
      one_click_repair_btn_(new QPushButton(this))
{
    qRegisterMetaType<fault_diagnosis::DiagnosticResult>();
    setMinimumSize(237, 342);

    // 标题
    auto *title_label = new K::TextLabel(this);
    title_label->SetPixelSize(16, true);
    title_label->SetText(tr("System health"));
    auto *title_hbox_layout = new QHBoxLayout;
    title_hbox_layout->setContentsMargins(0, 0, 0, 0);
    title_hbox_layout->setSpacing(0);
    title_hbox_layout->addStretch();
    title_hbox_layout->addWidget(title_label);
    title_hbox_layout->addStretch();

    // 分数
    score_widget_->setFixedSize(114, 114);
    score_widget_->SetScore(-1);
    auto *socre_hbox_layout = new QHBoxLayout;
    socre_hbox_layout->setContentsMargins(0, 0, 0, 0);
    socre_hbox_layout->setSpacing(0);
    socre_hbox_layout->addStretch();
    socre_hbox_layout->addWidget(score_widget_);
    socre_hbox_layout->addStretch();

    // 信息
    network_fault_num_label_->SetPixelSize(14, true);
    network_fault_num_label_->SetAlignment(Qt::AlignHCenter);
    network_fault_num_label_->SetText("--");
    auto *network_fault_text_label = new K::TextLabel(this);
    network_fault_text_label->SetAlignment(Qt::AlignHCenter);
    network_fault_text_label->SetFontColor(K::TextLabel::FontColor::kPlaceholderText);
    network_fault_text_label->SetText(tr("Network fault"));
    auto *network_fault_vbox_layout = new QVBoxLayout;
    network_fault_vbox_layout->setContentsMargins(0, 0, 0, 0);
    network_fault_vbox_layout->setSpacing(0);
    network_fault_vbox_layout->addWidget(network_fault_num_label_);
    network_fault_vbox_layout->addWidget(network_fault_text_label);

    system_fault_num_label_->SetPixelSize(14, true);
    system_fault_num_label_->SetAlignment(Qt::AlignHCenter);
    system_fault_num_label_->SetText("--");
    auto *system_fault_text_label = new K::TextLabel(this);
    system_fault_text_label->SetAlignment(Qt::AlignHCenter);
    system_fault_text_label->SetFontColor(K::TextLabel::FontColor::kPlaceholderText);
    system_fault_text_label->SetText(tr("System fault"));
    auto *system_fault_vbox_layout = new QVBoxLayout;
    system_fault_vbox_layout->setContentsMargins(0, 0, 0, 0);
    system_fault_vbox_layout->setSpacing(0);
    system_fault_vbox_layout->addWidget(system_fault_num_label_);
    system_fault_vbox_layout->addWidget(system_fault_text_label);

    auto *information_hbox_layout = new QHBoxLayout;
    information_hbox_layout->setContentsMargins(0, 0, 0, 0);
    information_hbox_layout->setSpacing(0);
    information_hbox_layout->addStretch();
    information_hbox_layout->addLayout(network_fault_vbox_layout);
    information_hbox_layout->addSpacing(32);
    information_hbox_layout->addLayout(system_fault_vbox_layout);
    information_hbox_layout->addStretch();

    // 一键修复
    one_click_repair_btn_->setProperty("isImportant", true);
    one_click_repair_btn_->setProperty("current_mode", BtnMode::kRepair);
    one_click_repair_btn_->setText(tr("One click repair"));
    one_click_repair_btn_->setEnabled(false);
    auto *one_click_repair_hbox_layout = new QHBoxLayout;
    one_click_repair_hbox_layout->setContentsMargins(0, 0, 0, 0);
    one_click_repair_hbox_layout->setSpacing(0);
    one_click_repair_hbox_layout->addStretch();
    one_click_repair_hbox_layout->addWidget(one_click_repair_btn_);
    one_click_repair_hbox_layout->addStretch();
    connect(one_click_repair_btn_, &QPushButton::clicked, this, &SystemHealthWidget::on_BtnClick);

    // 总布局
    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setContentsMargins(0, 24, 0, 32);
    vbox_layout->setSpacing(0);
    vbox_layout->addLayout(title_hbox_layout);
    vbox_layout->addSpacing(18);
    vbox_layout->addLayout(socre_hbox_layout);
    vbox_layout->addSpacing(24);
    vbox_layout->addLayout(information_hbox_layout);
    vbox_layout->addStretch();
    vbox_layout->addLayout(one_click_repair_hbox_layout);
    setLayout(vbox_layout);

    // 因为首页的故障检测和故障检测插件共用一个 kydoctor 后端，两者同时运行时会互相影响
    // 所以如果直接带参运行故障检测功能，会造成崩溃
    // 此处判断，是否为带参启动，如果是带参启动，则首页的检测功能不在运行
    auto *base_data = Frame::GetBaseData();
    if (base_data != nullptr && base_data->start_with_arg == false) {
        // 非带参启动的情况
        connect(&work_thread_, &home_page::SystemHealthWorkThread::sig_CheckFinish, this, &home_page::SystemHealthWidget::on_CheckFinish);
        connect(&work_thread_, &home_page::SystemHealthWorkThread::sig_RepairFinish, this, &home_page::SystemHealthWidget::on_RepairFinish);
        work_thread_.start();
    } else {
        // 带参启动则不再后台进行检测，按钮直接显示深度修复
        one_click_repair_btn_->setText(tr("Deep repair"));
        one_click_repair_btn_->setProperty("current_mode", BtnMode::kDeepRepair);
        one_click_repair_btn_->setEnabled(true);
    }
}

SystemHealthWidget::~SystemHealthWidget() {

}

void SystemHealthWidget::on_CheckFinish() {
    auto [score, network_fault_num, system_fault_num] = work_thread_.GetResult();
    network_fault_num_label_->SetText(tr("%1item").arg(network_fault_num));
    system_fault_num_label_->SetText(tr("%1item").arg(system_fault_num));
    score_widget_->SetScore(score);

    if (score != 100) {
        // 检测到了错误，按钮显示一键修复
        one_click_repair_btn_->setText(tr("One click repair"));
        one_click_repair_btn_->setProperty("current_mode", BtnMode::kRepair);
        one_click_repair_btn_->setEnabled(true);
    } else {
        // 没有检测到错误，按钮直接显示深度修复
        one_click_repair_btn_->setText(tr("Deep Repair"));
        one_click_repair_btn_->setProperty("current_mode", BtnMode::kDeepRepair);
        one_click_repair_btn_->setEnabled(true);
    }
}

void SystemHealthWidget::on_RepairFinish() {
    auto [score, network_fault_num, system_fault_num] = work_thread_.GetResult();
    network_fault_num_label_->SetText(tr("%1item").arg(network_fault_num));
    system_fault_num_label_->SetText(tr("%1item").arg(system_fault_num));
    score_widget_->SetScore(score);

    one_click_repair_btn_->setProperty("current_mode", BtnMode::kDeepRepair);
    one_click_repair_btn_->setEnabled(true);
    one_click_repair_btn_->setText(tr("Deep repair"));
}

void SystemHealthWidget::on_BtnClick() {
    auto current_mode = one_click_repair_btn_->property("current_mode").toInt();
    switch (current_mode) {
        case BtnMode::kRepair: {
            work_thread_.SetMode(SystemHealthWorkThread::Mode::kRepair);
            work_thread_.start();
            one_click_repair_btn_->setEnabled(false);
            one_click_repair_btn_->setText(tr("Repairing..."));
            break;
        }
        case BtnMode::kDeepRepair: {
            Frame::RunFaultDiagnosis();
            break;
        }
        default:
            break;
    }

}

void SystemHealthWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    //设置背景颜色
    QColor color = qApp->palette().color(QPalette::Window);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, QColor(color));
    this->setPalette(pal);
    QBrush brush = QBrush(color);
    painter.setBrush(brush);
    painter.setOpacity(0.8);
    painter.setPen(Qt::transparent);
    QStyleOption opt;
    opt.init(this);
    painter.drawRoundedRect(opt.rect, 8, 8);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);
}

ScoreWidget::ScoreWidget(QWidget *parent)
    : QWidget(parent),
      score_(-1),
      unit_text_(tr("points"))
{
    // 监听主题变化，当主题切换时重新绘制
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [this]() {
        update();  // 重新绘制
    });
}

ScoreWidget::~ScoreWidget() {

}

void ScoreWidget::SetScore(int score) {
    score_ = score;
    update();
}

void ScoreWidget::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    // 检测当前主题
    const auto current_theme = kdk::GsettingMonitor::getSystemTheme();
    bool is_dark_theme = (current_theme != "ukui-default" && current_theme != "ukui-light");

    // 根据主题和分数选择背景图片
    QString background;
    if (score_ >= 0 && score_ <= 30) {
        // 粉色
        if (is_dark_theme) {
            background = ":/home_page/data/icons/default_score_pink_dark.svg";
        } else {
            background = ":/home_page/data/icons/default_score_pink.svg";
        }
    } else if (score_ >= 31 && score_ <=60) {
        // 黄色
        if (is_dark_theme) {
            background = ":/home_page/data/icons/default_score_yellow_dark.svg";
        } else {
            background = ":/home_page/data/icons/default_score_yellow.svg";
        }
    } else if (score_ >= 61 && score_ <= 80) {
        // 蓝色
        if (is_dark_theme) {
            background = ":/home_page/data/icons/default_score_blue_dark.svg";
        } else {
            background = ":/home_page/data/icons/default_score_blue.svg";
        }
    } else {
        // 绿色
        if (is_dark_theme) {
            background = ":/home_page/data/icons/default_score_green_dark.svg";
        } else {
            background = ":/home_page/data/icons/default_score_green.svg";
        }
    }

    auto pixmap = QPixmap(background).scaled(size(), Qt::KeepAspectRatio);
    p.drawPixmap(0, 0, pixmap);

    // 画分数
    QFont num_font;
    num_font.setPixelSize(42);
    num_font.setBold(true);
    QFontMetrics num_metrics(num_font);

    QFont unit_font;
    unit_font.setPixelSize(14);
    QFontMetrics unit_metrics(unit_font);

    QString num;
    QString unit;
    if (score_ == -1) {
        num = "--";
    } else {
        num = QString::number(score_);
        unit = unit_text_;
    }
    auto num_width = num_metrics.horizontalAdvance(num);
    auto unit_width = unit_metrics.horizontalAdvance(unit);
    auto total_width = num_width + unit_width;

    auto num_start_x = (width() - total_width) / 2;
    auto num_start_y = height() / 2 + qRound((num_metrics.ascent() - num_metrics.descent() + num_metrics.capHeight()) / 4.0);
    p.setFont(num_font);
    p.drawText(num_start_x, num_start_y, num);

    auto unit_start_x = num_start_x + num_width;
    auto unit_start_y = num_start_y ;
    p.setFont(unit_font);
    p.drawText(unit_start_x, unit_start_y, unit);
}

SystemHealthWorkThread::SystemHealthWorkThread(QObject *parent)
    : QThread(parent),
      low_score_(13),
      medium_score_(29),
      high_score_(43),
      mode_(Mode::kCheck)
{
    // 系统在线更新检测
    diagnosis_category_.insert("0x050001", DiagnosisCategory::kSystemUpdate);        // 关键库检测
    //diagnosis_category_.insert("0x050002", DiagnosisCategory::kSystemUpdate);        // boot 分区空间检测
    diagnosis_category_.insert("0x050005", DiagnosisCategory::kSystemUpdate);        // 仓库地址权限检测
    diagnosis_category_.insert("0x050006", DiagnosisCategory::kSystemUpdate);        // 本地环境检查
    // 系统组件与性能检测
    //diagnosis_category_.insert("0x020002", DiagnosisCategory::kSystemComponents);    // 硬盘检测
    //diagnosis_category_.insert("0x020003", DiagnosisCategory::kSystemComponents);    // 系统内存
    //diagnosis_category_.insert("0x020004", DiagnosisCategory::kSystemComponents);    // 内核运行
    //diagnosis_category_.insert("0x020006", DiagnosisCategory::kSystemComponents);    // 开关机时长
    //diagnosis_category_.insert("0x020007", DiagnosisCategory::kSystemComponents);    // 系统性能
    //diagnosis_category_.insert("0x020008", DiagnosisCategory::kSystemComponents);    // 系统日志
    //diagnosis_category_.insert("0x020009", DiagnosisCategory::kSystemComponents);    // 界面显示
    //diagnosis_category_.insert("0x02000a", DiagnosisCategory::kSystemComponents);    // 系统组件与功能
    //diagnosis_category_.insert("0x02000c", DiagnosisCategory::kSystemComponents);    // 蓝牙
    //diagnosis_category_.insert("0x02000d", DiagnosisCategory::kSystemComponents);    // 声音检测
    // 应用程序检测
    diagnosis_category_.insert("0x030001", DiagnosisCategory::kApplication);         // 远程桌面

    // 系统在线更新检测
    diagnosis_items_.insert("0x050001", DiagnosisLevel::kMedium);    // 关键库检测
    //diagnosis_items_.insert("0x050002", DiagnosisLevel::kMedium);    // boot 分区空间检测
    diagnosis_items_.insert("0x050005", DiagnosisLevel::kMedium);    // 仓库地址权限检测
    diagnosis_items_.insert("0x050006", DiagnosisLevel::kMedium);    // 本地环境检查
    // 系统组件与性能检测
    //diagnosis_items_.insert("0x020002", DiagnosisLevel::kHigh);      // 硬盘检测
    //diagnosis_items_.insert("0x020003", DiagnosisLevel::kHigh);      // 系统内存
    //diagnosis_items_.insert("0x020004", DiagnosisLevel::kHigh);      // 内核运行
    //diagnosis_items_.insert("0x020006", DiagnosisLevel::kLow);       // 开关机时长
    //diagnosis_items_.insert("0x020007", DiagnosisLevel::kHigh);      // 系统性能
    //diagnosis_items_.insert("0x020008", DiagnosisLevel::kHigh);      // 系统日志
    //diagnosis_items_.insert("0x020009", DiagnosisLevel::kHigh);      // 界面显示
    //diagnosis_items_.insert("0x02000a", DiagnosisLevel::kHigh);      // 系统组件与功能
    //diagnosis_items_.insert("0x02000c", DiagnosisLevel::kHigh);      // 蓝牙
    //diagnosis_items_.insert("0x02000d", DiagnosisLevel::kHigh);      // 声音检测
    // 应用程序检测
    diagnosis_items_.insert("0x030001", DiagnosisLevel::kLow);       // 远程桌面
}

SystemHealthWorkThread::~SystemHealthWorkThread() {

}

void SystemHealthWorkThread::SetMode(enum Mode mode) {
    mode_ = mode;
}

void SystemHealthWorkThread::run() {
    switch (mode_) {
        case Mode::kCheck:
            Check();
            break;
        case Mode::kRepair:
            Repair();
            break;
        default:
            qWarning() << "unsupported operation mode, system health work thread exits";
            break;
    }
}

void SystemHealthWorkThread::Check() {
    diagnosis_result_mutex_.lock();
    diagnosis_result_.clear();

    // 申请 kydoctor 使用权
    while (true) {
        bool ok{false};
        Frame::ApplyKydoctor((char *)"home_page", &ok);
        if (ok) {
            break;
        }

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
    }

    auto iter = diagnosis_items_.cbegin();

    QMap<QString, QList<QString>> result;
    fault_diagnosis::CompleteMachine complete_machine;

    auto diagnosis_current_item = [this, &complete_machine, &iter, &result]() {
        while (true) {
            if (iter == diagnosis_items_.cend()) {
                // 检测完毕
                qInfo() << "all diagnosis items have been completed";
                diagnosis_result_.swap(result);
                diagnosis_result_mutex_.unlock();
                Frame::ReleaseKydoctor((char *)"home_page");
                Q_EMIT sig_CheckFinish();
                // 退出事件循环, 结束线程
                quit();
                return;
            }

            if (complete_machine.Check(iter.key())) {
                qInfo() << "start diagnosis " << iter.key() << "...";
                break;
            } else {
                qWarning() << "failed to diagnosis item " << iter.key() << ", try to diagnosis the next item";
                iter++;
            }
        }
    };

    QObject obj;
    connect(&complete_machine, &fault_diagnosis::CompleteMachine::sig_Checked, &obj, [this, &result](QString mark, fault_diagnosis::DiagnosticResult diagnosis_result) {
        if (!diagnosis_result.error_code.isEmpty()) {
            // 该检测项只有主检测项，并出错
            QString error_code = diagnosis_result.error_code;
            QString error_desc = diagnosis_result.describe;
            // 检测错误码是否可自动修复，如果不可自动修复，在错误码前插入 ! 号
            if (!AutoRepair(error_desc)) {
                error_code.insert(0, '!');
                qWarning() << "cannot auto repair error code: " << error_code;
            }
            result[diagnosis_result.base_entry_mark].push_back(error_code);
            qInfo() << "error in diagnosis " << diagnosis_result.base_entry_mark;
        } else {
            // 判断是否存在子检测项出错
            if (!diagnosis_result.details.isEmpty()) {
                for (const auto &subitem_error: diagnosis_result.details) {
                    QString error_code = subitem_error.error_code;
                    QString error_desc = subitem_error.describe;
                    if (!AutoRepair(error_desc)) {
                        error_code.insert(0, '!');
                        qWarning() << "cannot auto repair subitem error code: " << error_code;
                    }
                    result[diagnosis_result.base_entry_mark].push_back(error_code);
                }
                qInfo() << "error in diagnosis " << diagnosis_result.base_entry_mark;
            }
        }
    });
    connect(&complete_machine, &fault_diagnosis::CompleteMachine::sig_CheckFinish, &obj, [&iter, &diagnosis_current_item](QString mark, QString base_diagnosis_mark, bool ok) {
        qInfo() << "diagnosis item " << iter.key() << " has been completed";
        iter++;
        diagnosis_current_item();
    });

    diagnosis_current_item();

    exec();
}

void SystemHealthWorkThread::Repair() {
    diagnosis_result_mutex_.lock();

    // 申请 kydoctor 使用权
    while (true) {
        bool ok{false};
        Frame::ApplyKydoctor((char *)"home_page", &ok);
        if (ok) {
            break;
        }

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
    }

    // 退出线程闭包
    auto exit_thread = [this]() {
        diagnosis_result_mutex_.unlock();
        Frame::ReleaseKydoctor((char *)"home_page");
        Q_EMIT sig_RepairFinish();
        quit();
    };

    QList<fault_diagnosis::RepairItem> repair_items;
    for (auto iter = diagnosis_result_.cbegin(); iter != diagnosis_result_.cend(); iter++) {
        for (const auto &error_code: iter.value()) {
            if (!error_code.startsWith('!')) {
                // 非 ! 开头的错误码，是可以自动修复的错误码
                fault_diagnosis::RepairItem repair_item;
                repair_item.error_code = error_code;
                repair_items.push_back(repair_item);
            }
        }
    }

    if (repair_items.isEmpty()) {
        qInfo() << "there are no error to be fixed, system health work thread exits";
        exit_thread();
        return;
    }

    QObject obj;
    fault_diagnosis::CompleteMachine complete_machine;
    connect(&complete_machine, &fault_diagnosis::CompleteMachine::sig_Fixed, &obj, [this](QString mark, QString error_code, bool ok, QString reason, qint64 soiution_type, QString solution_context) {
        if (ok) {
            qInfo() << "error fixed: " << error_code;
            for (auto iter = diagnosis_result_.begin(); iter != diagnosis_result_.end(); iter++) {
                for (auto error_code_iter = iter.value().begin(); error_code_iter != iter.value().end(); error_code_iter++) {
                    if (*error_code_iter == error_code) {
                        iter.value().erase(error_code_iter);
                    }
                }
            }
        } else {
            qCritical() << "error repair failed: " << error_code << ", reason: " << reason << ", solution type: " << soiution_type << ", solution context: " << solution_context;
        }
    });
    connect(&complete_machine, &fault_diagnosis::CompleteMachine::sig_RepairFinish, &obj, [this, &exit_thread](QString mark, QString base_diagnosis_mark, bool ok) {
        qInfo() << "repair completed, system health work thread exits";
        qInfo() << "current repair result: " << diagnosis_result_;
        exit_thread();
    });

    if (!complete_machine.Repair("", repair_items)) {
        qCritical() << "repair failed, system health work thread exited";
        qInfo() << "current repair result: " << diagnosis_result_;
        exit_thread();
        return;
    }

    exec();
}

bool SystemHealthWorkThread::AutoRepair(QString error_desc) {
    QString flag;
    QString content;
    if (error_desc.contains("*|*")) {
        const auto list = error_desc.split("*|*");
        if (list.size() > 1) {
            flag = list.at(0).trimmed();
            content = list.at(1).trimmed();
        } else {
            content = error_desc;
        }
    } else if (error_desc == "0") {
        flag = "0";
    } else if (error_desc == "1") {
        flag = "1";
    } else if (error_desc == "3") {
        flag = "3";
    } else {
        content = error_desc;
    }

    if (error_desc.isEmpty() || flag == "0") {
        return true;
    }
    return false;
}

std::tuple<int, int, int> SystemHealthWorkThread::GetResult() {
    double score{100};
    int network_fault_num{0};
    int system_fault_num{0};

    QMutexLocker locker(&diagnosis_result_mutex_);
    if (diagnosis_result_.isEmpty()) {
        // 无错误发生
        return {100, 0, 0};
    }

    for (auto iter = diagnosis_result_.cbegin(); iter != diagnosis_result_.cend(); iter++) {
        if (!iter.value().isEmpty() &&
            diagnosis_items_.find(iter.key()) != diagnosis_items_.end() &&
            diagnosis_category_.find(iter.key()) != diagnosis_category_.end())
        {
            // 扣除分数
            switch (diagnosis_items_[iter.key()]) {
                case DiagnosisLevel::kHigh:
                    score -= high_score_;
                    break;
                case DiagnosisLevel::kMedium:
                    score -= medium_score_;
                    break;
                case DiagnosisLevel::kLow:
                    score -= low_score_;
                    break;
                default:
                    break;
            }

            // 判断分类
            if (diagnosis_category_[iter.key()] == DiagnosisCategory::kNetwork) {
                network_fault_num++;
            } else {
                system_fault_num++;
            }
        }
    }

    return {static_cast<int>(std::ceil(score)), network_fault_num, system_fault_num};
}

}
