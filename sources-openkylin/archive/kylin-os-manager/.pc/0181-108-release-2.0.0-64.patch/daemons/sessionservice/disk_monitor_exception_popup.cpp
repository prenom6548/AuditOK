#include "disk_monitor_exception_popup.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QProcess>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <kwindowinfo.h>
#include <kwindowsystem.h>

DiskMonitorExceptionPopup::DiskMonitorExceptionPopup(const QString &config_file_name)
    : config_file_name_(config_file_name),
      title_label_(new K::TextLabel(this)),
      not_prompted_within_3_days_check_box_(new QCheckBox(this))
{
    setAttribute(Qt::WA_DeleteOnClose);

    setFixedSize(455, 192);
    setIcon(QIcon::fromTheme("kylin-os-manager"));
    setWidgetName(tr("Kylin Os Manager"));
    windowButtonBar()->minimumButton()->hide();
    windowButtonBar()->maximumButton()->hide();
    windowButtonBar()->menuButton()->hide();

    // 隐藏任务栏图标
    const KWindowInfo window_info(winId(), NET::WMState);
    if (!window_info.hasState(NET::SkipTaskbar) || !window_info.hasState(NET::SkipPager)) {
        KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager);
    }

    // 标题
    auto *icon_btn = new QPushButton(this);
    icon_btn->setAttribute(Qt::WA_TransparentForMouseEvents);
    icon_btn->setFixedSize(16 , 16);
    icon_btn->setIcon(QIcon::fromTheme("dialog-warning"));
    auto *title_hbox_layout = new QHBoxLayout;
    title_hbox_layout->setMargin(0);
    title_hbox_layout->setSpacing(0);
    title_hbox_layout->addWidget(icon_btn);
    title_hbox_layout->addSpacing(8);
    title_hbox_layout->addWidget(title_label_);
    title_hbox_layout->addStretch();

    // 3 天内不在提示
    not_prompted_within_3_days_check_box_->setFixedSize(16, 16);
    not_prompted_within_3_days_check_box_->setCheckState(Qt::CheckState::Unchecked);
    connect(not_prompted_within_3_days_check_box_, &QCheckBox::stateChanged, this, &DiskMonitorExceptionPopup::on_NotPromptedWithinThreeDays);
    auto *not_prompted_within_3_days_label = new K::TextLabel(this);
    not_prompted_within_3_days_label->SetText(tr("Not prompted within 3 days"));
    auto *not_prompted_within_3_days_hbox_layout = new QHBoxLayout;
    not_prompted_within_3_days_hbox_layout->setMargin(0);
    not_prompted_within_3_days_hbox_layout->setSpacing(0);
    not_prompted_within_3_days_hbox_layout->addSpacing(22);
    not_prompted_within_3_days_hbox_layout->addWidget(not_prompted_within_3_days_check_box_);
    not_prompted_within_3_days_hbox_layout->addSpacing(8);
    not_prompted_within_3_days_hbox_layout->addWidget(not_prompted_within_3_days_label);
    not_prompted_within_3_days_hbox_layout->addStretch();

    // 按钮
    auto *cache_btn = new QPushButton(this);
    cache_btn->setText(tr("Cache"));
    connect(cache_btn, &QPushButton::clicked, this, [this]() {
        close();
    });

    auto *clean_btn = new QPushButton(this);
    clean_btn->setProperty("isImportant", true);
    clean_btn->setText(tr("Clean up immediately"));
    connect(clean_btn, &QPushButton::clicked, this, [this]() {
        QProcess process;
        process.setProgram("kylin-os-manager");
        process.setArguments({"--jumpTab=JunkClean", "--pluginMessage=MaxDiskCapacity"});
        process.startDetached();
        close();
    });

    auto *btn_hbox_layout = new QHBoxLayout;
    btn_hbox_layout->setMargin(0);
    btn_hbox_layout->setSpacing(0);
    btn_hbox_layout->addStretch();
    btn_hbox_layout->addWidget(cache_btn);
    btn_hbox_layout->addSpacing(8);
    btn_hbox_layout->addWidget(clean_btn);

    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setContentsMargins(24, 8, 24, 24);
    vbox_layout->setSpacing(0);
    vbox_layout->addLayout(title_hbox_layout);
    vbox_layout->addSpacing(8);
    vbox_layout->addLayout(not_prompted_within_3_days_hbox_layout);
    vbox_layout->addStretch();
    vbox_layout->addLayout(btn_hbox_layout);

    baseBar()->setLayout(vbox_layout);
}

DiskMonitorExceptionPopup::~DiskMonitorExceptionPopup() {

}

void DiskMonitorExceptionPopup::SetTitle(const QString &title) const {
    title_label_->SetText(title);
}

/*
 * {
 *     "start_display_time": "2024-10-29 12:34:55"
 * }
 */
void DiskMonitorExceptionPopup::GenerateConfigFile(bool enabel_start_display_time) const {
    QJsonObject root;
    if (enabel_start_display_time) {
        // 显示时间比当前时间晚 3 天
        auto current_time = QDateTime::currentDateTime();
        auto display_time = current_time.addSecs(3 * 24 * 60 * 60);
        root.insert("start_display_time", display_time.toString("yyyy-MM-dd hh:mm:ss"));
    } else {
        root.insert("start_display_time", "");
    }

    // 序列化
    QJsonDocument doc;
    doc.setObject(root);
    const auto data = doc.toJson();

    // 判断存放配置文件的目录是否存在
    const auto home_path = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::HomeLocation);
    const auto config_file_dir = QString("%1/.kylin-os-manager").arg(home_path);
    QDir dir(config_file_dir);
    if (!dir.exists()) {
        if (!dir.mkpath(config_file_dir)) {
            qCritical() << "Disk monitor exception popup generate config file create dir fail";
            return;
        }
    }

    // 写入文件
    const auto config_file_path = QString("%1/.kylin-os-manager/%2").arg(home_path, config_file_name_);
    QFile file(config_file_path);
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "Disk monitor exception popup generate config file open file fail";
        return;
    }
    const auto size = file.write(data);
    if (size != data.size()) {
        qWarning() << "Disk monitor exception popup generate config file write incomplete";
    }
    file.close();
}

void DiskMonitorExceptionPopup::on_NotPromptedWithinThreeDays(int state) {
    // 读入配置文件内容
    const auto home_path = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::HomeLocation);
    const auto config_file_path = QString("%1/.kylin-os-manager/%2").arg(home_path, config_file_name_);
    QFile file(config_file_path);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Disk monitor exception popup not prompted within 3 days open config file fail";
        state == Qt::CheckState::Unchecked? GenerateConfigFile(false): GenerateConfigFile(true);
        return;
    }
    const auto data = file.readAll();
    file.close();

    // 反序列化
    auto doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qCritical() << "Disk monitor exception popup not prompted within 3 days unmarshal fail";
        state == Qt::CheckState::Unchecked? GenerateConfigFile(false): GenerateConfigFile(true);
        return;
    }
    if (!doc.isObject()) {
        qCritical() << "Disk monitor exception popup not prompted within 3 days format error";
        state == Qt::CheckState::Unchecked? GenerateConfigFile(false): GenerateConfigFile(true);
        return;
    }

    // 修改 start_display_time 字段
    auto root = doc.object();
    if (!root.contains("start_display_time")) {
        qCritical() << "Disk monitor exception popup not prompted within 3 days start_display_time field miss";
        state == Qt::CheckState::Unchecked? GenerateConfigFile(false): GenerateConfigFile(true);
        return;
    }
    if (state == Qt::CheckState::Unchecked) {
        root["start_display_time"] = "";
    } else {
        auto current_time = QDateTime::currentDateTime();
        auto dispaly_time = current_time.addSecs(3 * 24 * 60 * 60);
        root["start_display_time"] = dispaly_time.toString("yyyy-MM-dd hh:mm:ss");
    }

    // 序列化
    QJsonDocument modify_doc;
    modify_doc.setObject(root);
    const auto modify_data = modify_doc.toJson();

    // 写入文件
    QFile modify_file(config_file_path);
    if (!modify_file.open(QIODevice::WriteOnly)) {
        qCritical() << "Disk monitor exception popup not prompted within 3 days modify config file open fail";
        return;
    }
    const auto size = modify_file.write(modify_data);
    if (size != modify_data.size()) {
        qCritical() << "Disk monitor exception popup not prompted within 3 days modify config file write incomplete";
    }
    modify_file.close();
}

// 默认行为是显示
void DiskMonitorExceptionPopup::Show() {
    not_prompted_within_3_days_check_box_->blockSignals(true);
    not_prompted_within_3_days_check_box_->setCheckState(Qt::CheckState::Unchecked);
    not_prompted_within_3_days_check_box_->blockSignals(false);

    // 判断当前时间是否显示
    bool display{true};
    do {
        // 读取配置文件中的值
        const auto home_path = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::HomeLocation);
        const auto config_file_path = QString("%1/.kylin-os-manager/%2").arg(home_path, config_file_name_);
        QFile file(config_file_path);
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << "Disk monitor exception popup show open config file fail";
            break;
        }
        const auto data = file.readAll();
        file.close();

        // 反序列化
        const auto doc = QJsonDocument::fromJson(data);
        if (doc.isNull()) {
            qCritical() << "Disk monitor exception popup show unmarshal fail";
            break;
        }
        if (!doc.isObject()) {
            qCritical() << "Disk monitor exception popup show config format error";
            break;
        }

        const auto root = doc.object();
        if (!root.contains("start_display_time")) {
            qCritical() << "Disk monitor exception popup show start_display_time field miss";
            break;
        }
        const auto display_time = root.value("start_display_time").toString();
        if (display_time.isEmpty()) {
            break;
        }
        if (QDateTime::currentDateTime() < QDateTime::fromString(display_time, "yyyy-MM-dd hh:mm:ss")) {
            display = false;
        }
    } while (0);

    if (display) {
        show();
    } else {
        hide();
    }
}
