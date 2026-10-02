#include "repair_specific_error_widget.h"
#include <QPixmap>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDesktopServices>
#include <QDebug>
#include <QProcess>
#include <QMessageBox>
#include <QTimer>
#include <QUrl>
#include <kborderlessbutton.h>
#include <kbackgroundgroup.h>
#include "frame.h"

namespace fault_diagnosis {

RepairSpecificErrorWidget::RepairSpecificErrorWidget(QWidget *parent)
    : QWidget(parent),
      title_label_(new K::TextLabel(this)),
      body_vbox_layout_(new QVBoxLayout),
      state_(RepairSpecificErrorWidget::State::kNone),
      current_normal_image_num_(0),
      upgrade_run_btn_(nullptr),
      upgrade_state_icon_label_(nullptr),
      upgrade_state_text_label_(nullptr),
      software_run_btn_(nullptr),
      software_state_icon_label_(nullptr),
      software_state_text_label_(nullptr)
{
    title_label_->SetPixelSize(24, true);
    body_vbox_layout_->setSpacing(0);
    body_vbox_layout_->setContentsMargins(0, 0, 0, 0);

    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setSpacing(0);
    vbox_layout->setContentsMargins(40, 16, 40, 0);
    vbox_layout->addWidget(title_label_);
    vbox_layout->addLayout(body_vbox_layout_);
    setLayout(vbox_layout);
}

RepairSpecificErrorWidget::~RepairSpecificErrorWidget() {

}

void RepairSpecificErrorWidget::SetTitle(const QString &title) const {
    title_label_->SetText(title);
}

void RepairSpecificErrorWidget::SwitchRunning() {
    Clear();

    state_ = RepairSpecificErrorWidget::State::kDiagnosising;
    current_normal_image_num_ = 0;

    auto *widget = new QWidget;

    auto *icon_label = new QLabel(widget);
    icon_label->setFixedSize(128, 128);
    icon_label->setPixmap(QPixmap(":/fault_diagnosis/data/icons/normal_0.svg")
                          .scaled(128, 128, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
    // 设置动图
    auto *timer = new QTimer(icon_label);
    timer->setInterval(200);
    connect(timer, &QTimer::timeout, this, [this, icon_label]() {
        if (current_normal_image_num_ == 6) {
            current_normal_image_num_ = 0;
            icon_label->setPixmap(QPixmap(":/fault_diagnosis/data/icons/normal_0.svg")
                                  .scaled(128, 128, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
            return;
        }

        icon_label->setPixmap(QPixmap(QString(":/fault_diagnosis/data/icons/normal_%1.svg").arg(++current_normal_image_num_))
                              .scaled(128, 128, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
    });
    timer->start();

    auto *icon_hbox_layout = new QHBoxLayout;
    icon_hbox_layout->setSpacing(0);
    icon_hbox_layout->setContentsMargins(0, 0, 0, 0);
    icon_hbox_layout->addStretch();
    icon_hbox_layout->addWidget(icon_label);
    icon_hbox_layout->addStretch();

    auto *desc_label = new K::TextLabel(widget);
    desc_label->SetPixelSize(18);
    desc_label->SetText(tr("Detecting and attempting to fix the issue"));
    auto *desc_hbox_layout = new QHBoxLayout;
    desc_hbox_layout->setSpacing(0);
    desc_hbox_layout->setContentsMargins(0, 0, 0, 0);
    desc_hbox_layout->addStretch();
    desc_hbox_layout->addWidget(desc_label);
    desc_hbox_layout->addStretch();

    auto *cancel_btn = new QPushButton(widget);
    cancel_btn->setText(tr("Cancel"));
    connect(cancel_btn, &QPushButton::clicked, this, [this]() {
        SetState(State::kCancel);
        Q_EMIT sig_Cancel();
    });
    auto *cancel_hbox_layout = new QHBoxLayout;
    cancel_hbox_layout->setSpacing(0);
    cancel_hbox_layout->setContentsMargins(0, 0, 0, 0);
    cancel_hbox_layout->addStretch();
    cancel_hbox_layout->addWidget(cancel_btn);
    cancel_hbox_layout->addStretch();

    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setSpacing(0);
    vbox_layout->setContentsMargins(0, 0, 0, 0);
    vbox_layout->addSpacing(112);
    vbox_layout->addLayout(icon_hbox_layout);
    vbox_layout->addSpacing(16);
    vbox_layout->addLayout(desc_hbox_layout);
    vbox_layout->addSpacing(32);
    vbox_layout->addLayout(cancel_hbox_layout);
    vbox_layout->addStretch();
    widget->setLayout(vbox_layout);

    body_vbox_layout_->addWidget(widget);
}

void RepairSpecificErrorWidget::SwitchResult(const ShowContent &show_content) {
    Clear();

    show_content_ = show_content;
    state_ = RepairSpecificErrorWidget::State::kResult;

    body_vbox_layout_->addSpacing(40);
    body_vbox_layout_->addWidget(DescWidget(show_content.ok, show_content.desc));
    body_vbox_layout_->addSpacing(30);

    // 结果正确
    if (show_content.ok) {
        auto *background_group = new kdk::KBackgroundGroup;
        background_group->setBackgroundRole(QPalette::ColorRole::Window);
        background_group->addWidget(ServiceSupportWidget());

        body_vbox_layout_->addWidget(background_group);
        body_vbox_layout_->addSpacing(8);
        body_vbox_layout_->addWidget(ReturnWidget());
        body_vbox_layout_->addStretch();
        return;
    }

    // 结果错误时，显示“解决建议”文案
    //body_vbox_layout_->addWidget(SuggestWidget());

    auto *backgroud_group = new kdk::KBackgroundGroup;
    backgroud_group->setBackgroundRole(QPalette::ColorRole::Window);

    // 网络状态
    if (show_content.network_state != NetworkState::kUnknow) {
        backgroud_group->addWidget(NetworkStateWidget(show_content.network_state));
    }
    // 系统更新修复脚本运行
    if (show_content.upgrade_run) {
        backgroud_group->addWidget(UpgradeRunWidget());
    }
    // 软件商店修复脚本运行
    if (show_content.software_run) {
        backgroud_group->addWidget(SoftwareRunWidget());
    }
    // 垃圾清理
    if (show_content.junk_clean) {
        backgroud_group->addWidget(JunkCleanWidget());
    }
    // 问题上报
    if (show_content.report_issues) {
        if (show_content.network_state != NetworkState::kUnknow ||
            show_content.upgrade_run ||
            show_content.software_run ||
            show_content.junk_clean)
        {
            backgroud_group->addWidget(ReportIssuesWidget(true));
        } else {
            backgroud_group->addWidget(ReportIssuesWidget(false));
        }
    }
    backgroud_group->addWidget(ServiceSupportWidget());

    body_vbox_layout_->addWidget(backgroud_group);
    body_vbox_layout_->addSpacing(8);
    body_vbox_layout_->addWidget(ReturnWidget());
    body_vbox_layout_->addStretch();
}

QWidget *RepairSpecificErrorWidget::DescWidget(bool ok, const QString &desc) {
    auto *widget = new QWidget;

    auto *icon_label = new QLabel(widget);
    icon_label->setFixedSize(64, 64);
    if (ok) {
        icon_label->setPixmap(QPixmap(":/fault_diagnosis/data/icons/success.svg")
                              .scaled(64, 64, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
    } else {
        icon_label->setPixmap(QPixmap(":/fault_diagnosis/data/icons/error.svg")
                                      .scaled(64, 64, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
    }
    auto *desc_label = new K::TextLabel(widget);
    desc_label->SetPixelSize(24);
    desc_label->SetText(desc);

    auto *hbox_layout = new QHBoxLayout;
    hbox_layout->setSpacing(0);
    hbox_layout->setContentsMargins(0, 0, 0, 0);
    hbox_layout->addWidget(icon_label);
    hbox_layout->addSpacing(16);
    hbox_layout->addWidget(desc_label);
    hbox_layout->addStretch();
    widget->setLayout(hbox_layout);

    return widget;
}

QWidget *RepairSpecificErrorWidget::SuggestWidget() {
    auto *widget = new QWidget;

    auto *suggest_label = new K::TextLabel(widget);
    suggest_label->SetText(tr("Solution suggestions"));
    auto *suggest_hbox_layout = new QHBoxLayout;
    suggest_hbox_layout->setSpacing(0);
    suggest_hbox_layout->setContentsMargins(16, 0, 16, 0);
    suggest_hbox_layout->addWidget(suggest_label);
    suggest_hbox_layout->addStretch();
    widget->setLayout(suggest_hbox_layout);

    return widget;
}

QWidget *RepairSpecificErrorWidget::ReportIssuesWidget(bool has_prepair) {
    auto *widget = new QWidget;

    auto *desc_label = new K::TextLabel(widget);
    if (has_prepair) {
        desc_label->SetText(tr("If the above suggestions still fail to solve the problem, you may need to report the issue to the system maintenance personnel."));
    } else {
        desc_label->SetText(tr("To attempt to resolve the issue, you may need to report the issue to the system maintenance personnel."));
    }
    auto *report_issues_btn = new QPushButton(widget);
    report_issues_btn->setText(tr("Report issues"));
    connect(report_issues_btn, &QPushButton::clicked, this, &RepairSpecificErrorWidget::on_ProblemReport);

    auto *hbox_layout = new QHBoxLayout;
    hbox_layout->setSpacing(0);
    hbox_layout->setContentsMargins(16, 0, 16, 0);
    hbox_layout->addWidget(desc_label);
    hbox_layout->addStretch();
    hbox_layout->addSpacing(40);
    hbox_layout->addWidget(report_issues_btn);
    widget->setLayout(hbox_layout);

    return widget;
}

QWidget *RepairSpecificErrorWidget::UpgradeRunWidget() {
    auto *widget = new QWidget;

    auto *desc_label = new K::TextLabel(widget);
    desc_label->SetText(tr("The network is normal but the update cannot be completed, allow the system to run automated scripts to attempt to fix the problem."));
    upgrade_run_btn_ = new QPushButton(widget);
    upgrade_run_btn_->setText(tr("Run"));
    connect(upgrade_run_btn_, &QPushButton::clicked, this, [this]() {
        upgrade_run_btn_->setText(tr("Running..."));
        upgrade_run_btn_->setEnabled(false);
        Q_EMIT sig_Repair(show_content_.error_code);
    });
    upgrade_state_icon_label_ = new QLabel(widget);
    upgrade_state_icon_label_->setFixedSize(16, 16);
    upgrade_state_icon_label_->hide();
    upgrade_state_text_label_ = new K::TextLabel(widget);
    upgrade_state_text_label_->hide();

    auto *hbox_layout = new QHBoxLayout;
    hbox_layout->setSpacing(0);
    hbox_layout->setContentsMargins(16, 0, 16, 0);
    hbox_layout->addWidget(desc_label);
    hbox_layout->addStretch();
    hbox_layout->addSpacing(40);
    hbox_layout->addWidget(upgrade_run_btn_);
    hbox_layout->addWidget(upgrade_state_icon_label_);
    hbox_layout->addWidget(upgrade_state_text_label_);
    widget->setLayout(hbox_layout);

    return widget;
}

QWidget *RepairSpecificErrorWidget::NetworkStateWidget(enum NetworkState network_state) {
    auto *widget = new QWidget;

    auto *desc_label = new K::TextLabel(widget);
    desc_label->SetText(tr("Please check if the the network connection is normal and update again."));
    auto *icon_label = new QLabel(widget);
    icon_label->setFixedSize(16, 16);
    auto *state_label = new K::TextLabel(widget);
    switch (network_state) {
        case NetworkState::kNormal:
            icon_label->setPixmap(QIcon::fromTheme("ukui-dialog-success").pixmap(16, 16));
            state_label->SetText(tr("Network normal"));
            break;
        default:
            icon_label->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(16, 16));
            state_label->SetText(tr("Network exception"));
            break;
    }

    auto *hbox_layout = new QHBoxLayout;
    hbox_layout->setSpacing(0);
    hbox_layout->setContentsMargins(16, 0, 26, 0);
    hbox_layout->addWidget(desc_label);
    hbox_layout->addStretch();
    hbox_layout->addSpacing(50);
    hbox_layout->addWidget(icon_label);
    hbox_layout->addSpacing(4);
    hbox_layout->addWidget(state_label);
    widget->setLayout(hbox_layout);

    return widget;
}

QWidget *RepairSpecificErrorWidget::ServiceSupportWidget() {
    auto *widget = new QWidget;
    widget->setFixedHeight(150);

    auto *desc_front_label = new K::TextLabel(widget);
    desc_front_label->SetText(tr("Access"));
    auto *official_website_btn = new kdk::KBorderlessButton(widget);
    official_website_btn->setText(tr("KylinOS official website"));
    connect(official_website_btn, &kdk::KBorderlessButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://wx.kylinos.cn/zsk"));
    });
    auto *desc_last_label = new K::TextLabel(widget);
    desc_last_label->SetText(tr("view common problems and solutions. For more help, please scan the QR code below: "));
    auto *desc_hbox_layout = new QHBoxLayout;
    desc_hbox_layout->setSpacing(0);
    desc_hbox_layout->setContentsMargins(0, 0, 0, 0);
    desc_hbox_layout->addWidget(desc_front_label);
    desc_hbox_layout->addWidget(official_website_btn);
    desc_hbox_layout->addWidget(desc_last_label);
    desc_hbox_layout->addStretch();

    auto *QR_code_label = new QLabel(widget);
    QR_code_label->setFixedSize(96, 96);
    QR_code_label->setPixmap(QPixmap(":/fault_diagnosis/data/icons/technical_service_QR.png")
                             .scaled(96, 96, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
    auto *QR_hbox_layout = new QHBoxLayout;
    QR_hbox_layout->setSpacing(0);
    QR_hbox_layout->setContentsMargins(0, 0, 0, 0);
    QR_hbox_layout->addWidget(QR_code_label);
    QR_hbox_layout->addStretch();

    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setSpacing(0);
    vbox_layout->setContentsMargins(16, 0, 24, 0);
    vbox_layout->addStretch();
    vbox_layout->addLayout(desc_hbox_layout);
    vbox_layout->addSpacing(7);
    vbox_layout->addLayout(QR_hbox_layout);
    vbox_layout->addStretch();
    widget->setLayout(vbox_layout);

    return widget;
}

QWidget *RepairSpecificErrorWidget::SoftwareRunWidget() {
    auto *widget = new QWidget;
    widget->setFixedHeight(135);

    auto *desc_front_label = new K::TextLabel(widget);
    desc_front_label->SetText(tr("The system will run automated scripts to try to solve the problem, "
                           "which will uninstall some software packages or lower the version of some software packages ("));
    auto *details_btn = new kdk::KBorderlessButton(widget);
    details_btn->setText(tr("click to view details"));
    auto *desc_last_label = new K::TextLabel(widget);
    desc_last_label->SetText(tr(")."));
    auto *desc_hbox_layout = new QHBoxLayout;
    desc_hbox_layout->setSpacing(0);
    desc_hbox_layout->setContentsMargins(16, 0, 16, 0);
    desc_hbox_layout->addWidget(desc_front_label);
    desc_hbox_layout->addWidget(details_btn);
    desc_hbox_layout->addWidget(desc_last_label);
    desc_hbox_layout->addStretch();

    auto *ask_label = new K::TextLabel(widget);
    ask_label->SetText(tr("Is it running?"));
    auto *ask_hbox_layout = new QHBoxLayout;
    ask_hbox_layout->setSpacing(0);
    ask_hbox_layout->setContentsMargins(16, 0, 16, 0);
    ask_hbox_layout->addWidget(ask_label);
    ask_hbox_layout->addStretch();

    software_run_btn_ = new QPushButton(widget);
    software_run_btn_->setText(tr("Run"));
    connect(software_run_btn_, &QPushButton::clicked, this, [this]() {
        software_run_btn_->setText(tr("Running..."));
        software_run_btn_->setEnabled(false);
        Q_EMIT sig_Repair(show_content_.error_code);
    });
    software_state_icon_label_ = new QLabel(widget);
    software_state_icon_label_->setFixedSize(16, 16);
    software_state_icon_label_->hide();
    software_state_text_label_ = new K::TextLabel(widget);
    software_state_text_label_->hide();
    auto *run_hbox_layout = new QHBoxLayout;
    run_hbox_layout->setSpacing(0);
    run_hbox_layout->setContentsMargins(16, 0, 16, 0);
    run_hbox_layout->addWidget(software_run_btn_);
    run_hbox_layout->addWidget(software_state_icon_label_);
    run_hbox_layout->addWidget(software_state_text_label_);
    run_hbox_layout->addStretch();

    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setSpacing(0);
    vbox_layout->setContentsMargins(0, 0, 0, 0);
    vbox_layout->addStretch();
    vbox_layout->addLayout(desc_hbox_layout);
    vbox_layout->addLayout(ask_hbox_layout);
    vbox_layout->addSpacing(16);
    vbox_layout->addLayout(run_hbox_layout);
    vbox_layout->addStretch();
    widget->setLayout(vbox_layout);

    return widget;
}

QWidget* RepairSpecificErrorWidget::JunkCleanWidget() {
    auto *widget = new QWidget;

    auto *desc_label = new K::TextLabel(widget);
    desc_label->SetText(tr("Your disk space is insufficient, please click the button on the right to clean it up."));
    auto *junk_clean_btn = new QPushButton(widget);
    junk_clean_btn->setText(tr("Clear"));
    connect(junk_clean_btn, &QPushButton::clicked, this, &RepairSpecificErrorWidget::on_JunkClean);

    auto *hbox_layout = new QHBoxLayout;
    hbox_layout->setSpacing(0);
    hbox_layout->setContentsMargins(16, 0, 16, 0);
    hbox_layout->addWidget(desc_label);
    hbox_layout->addStretch();
    hbox_layout->addSpacing(40);
    hbox_layout->addWidget(junk_clean_btn);
    widget->setLayout(hbox_layout);

    return widget;
}

QWidget *RepairSpecificErrorWidget::ReturnWidget() {
    auto *widget = new QWidget;

    auto *return_btn = new QPushButton(widget);
    return_btn->setText(tr("Return"));
    connect(return_btn, &QPushButton::clicked, this, &RepairSpecificErrorWidget::sig_Return);
    auto *return_hbox_layout = new QHBoxLayout;
    return_hbox_layout->setSpacing(0);
    return_hbox_layout->setContentsMargins(16, 0, 16, 0);
    return_hbox_layout->addWidget(return_btn);
    return_hbox_layout->addStretch();
    widget->setLayout(return_hbox_layout);

    return widget;
}

void RepairSpecificErrorWidget::Clear() {
    QLayoutItem *child;
    while ((child = body_vbox_layout_->takeAt(0)) != nullptr) {
        if (child->spacerItem()) {
            delete child->spacerItem();
        } else if (child->widget()) {
            child->widget()->deleteLater();
            delete child;
        }
    }

    upgrade_run_btn_ = nullptr;
    upgrade_state_icon_label_ = nullptr;
    upgrade_state_text_label_ = nullptr;
    software_run_btn_ = nullptr;
    software_state_icon_label_ = nullptr;
    software_state_text_label_ = nullptr;
}

void RepairSpecificErrorWidget::SetRepairResult(enum SpecificErrorModule module, bool ok) {
    switch (module) {
        case SpecificErrorModule::kUpgrade:
            if (upgrade_run_btn_ != nullptr &&
                upgrade_state_icon_label_ != nullptr &&
                upgrade_state_text_label_ != nullptr)
            {
                upgrade_run_btn_->hide();
                if (ok) {
                    upgrade_state_icon_label_->setPixmap(QIcon::fromTheme("ukui-dialog-success").pixmap(16, 16));
                    upgrade_state_text_label_->SetText(tr("Repair successful"));
                } else {
                    upgrade_state_icon_label_->setPixmap(QIcon::fromTheme("dialog-error").pixmap(16, 16));
                    upgrade_state_text_label_->SetText(tr("Repair failed"));
                }
                upgrade_state_icon_label_->show();
                upgrade_state_text_label_->show();
            }
            break;
        case SpecificErrorModule::kSoftware:
            if (software_run_btn_ != nullptr &&
                software_state_icon_label_ != nullptr &&
                software_state_text_label_ != nullptr)
            {
                software_run_btn_->hide();
                if (ok) {
                    software_state_icon_label_->setPixmap(QIcon::fromTheme("ukui-dialog-success").pixmap(16, 16));
                    software_state_text_label_->SetText(tr("Repair successful"));
                } else {
                    software_state_icon_label_->setPixmap(QIcon::fromTheme("dialog-error").pixmap(16, 16));
                    software_state_text_label_->SetText(tr("Repair failed"));
                }
                software_state_icon_label_->show();
                software_state_text_label_->show();
            }
            break;
        default:
            break;
    }
}

void RepairSpecificErrorWidget::on_ProblemReport() {
    auto *report_btn = static_cast<QPushButton *>(sender());
    report_btn->setText(tr("Reporting..."));
    report_btn->setEnabled(false);

    auto *process = new QProcess(this);
    process->setProgram("/usr/lib/kylin-os-manager/bin/service-support-mini");
    QString repair_module;
    switch (module_) {
        case SpecificErrorModule::kUpgrade:
            repair_module = "Upgrade";
            break;
        case SpecificErrorModule::kSoftware:
            repair_module = "Software";
            break;
        default:
            repair_module = "Unknow";
            break;
    }
    auto exe_option = QString("-exe=%1").arg(repair_module);
    auto code_option = QString("-code=%1").arg(raw_error_code_);
    auto path_option = QString("-path=%1").arg(show_content_.report_path);
    process->setArguments({"-mode=3", exe_option, code_option, path_option});
    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [this, report_btn](int exit_code, QProcess::ExitStatus exit_status) {
        static_cast<QProcess *>(sender())->deleteLater();

        report_btn->setText(tr("Report issues"));
        report_btn->setEnabled(true);

        if (!exit_code) {
            QMessageBox::information(this, "", tr("Successfully reported"));
        } else {
            QMessageBox::information(this, "", tr("Report failed"));
        }
    });
    process->start();
}

void RepairSpecificErrorWidget::on_JunkClean() {
    Frame::RunJunkClean();
}

enum RepairSpecificErrorWidget::State RepairSpecificErrorWidget::State() const {
    return state_;
}

void RepairSpecificErrorWidget::SetState(enum RepairSpecificErrorWidget::State state) {
    state_ = state;
}

void RepairSpecificErrorWidget::SetRawData(fault_diagnosis::SpecificErrorModule module, QString raw_error_code) {
    module_ = module;
    raw_error_code_ = raw_error_code;
}

}
