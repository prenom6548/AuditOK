#include "diagnosis_subentry_widget.h"
#include <QPalette>
#include <QApplication>
#include <QHBoxLayout>
#include "frame.h"

namespace fault_diagnosis {

DiagnosisSubentryWidget::DiagnosisSubentryWidget(const DiagnosticItem &diagnostic_item, QWidget *parent)
    : QWidget(parent),
      diagnostic_item_(diagnostic_item),
      select_seat_label_(new QLabel(this)),
      select_seat_vbox_layout_(new QVBoxLayout(select_seat_label_)),
      select_checkbox_(new QCheckBox(this)),
      name_label_(new K::TextLabel(this)),
      desc_label_(new K::TextLabel(this)),
      status_label_(new K::TextLabel(this)),
      contact_technical_service_btn_(new kdk::KBorderlessButton(this)),
      go_clean_up_btn_(new kdk::KBorderlessButton(this)),
      prompt_label_(new Prompt(this)),
      status_(Status::kNoState)
{
    setAutoFillBackground(true);    // 解决 v11 上展开列表时文字重叠问题
    setMinimumHeight(36);
    setFixedWidth(824);

    select_seat_label_->setFixedSize(18, 18);
    select_seat_vbox_layout_->setSpacing(0);
    select_seat_vbox_layout_->setContentsMargins(0, 0, 0, 0);
    select_checkbox_->hide();
    connect(select_checkbox_, &QCheckBox::stateChanged, this, &DiagnosisSubentryWidget::sig_SelectChange);

    name_label_->SetText(diagnostic_item_.name);
    desc_label_->SetFontColor(K::TextLabel::FontColor::kPlaceholderText);
    desc_label_->SetText(diagnostic_item_.describe);

    contact_technical_service_btn_->setText(tr("Please contact technical service"));
    contact_technical_service_btn_->hide();
    connect(contact_technical_service_btn_, &kdk::KBorderlessButton::clicked, this, []() {Frame::SwitchToProblemFeedback();});

    go_clean_up_btn_->setText(tr("Go clean up"));
    go_clean_up_btn_->hide();
    connect(go_clean_up_btn_, &kdk::KBorderlessButton::clicked, this, []() {Frame::RunJunkClean();});

    prompt_label_->setFixedSize(16, 16);
    prompt_label_->hide();

    auto *hbox_layout = new QHBoxLayout;
    hbox_layout->setSpacing(0);
    hbox_layout->setContentsMargins(104, 0, 64, 0);
    hbox_layout->addWidget(select_seat_label_);
    hbox_layout->addSpacing(8);
    hbox_layout->addWidget(name_label_);
    hbox_layout->addSpacing(8);
    hbox_layout->addWidget(desc_label_);
    hbox_layout->addStretch();
    hbox_layout->addSpacing(8);
    hbox_layout->addWidget(status_label_);
    // TODO: 如果两个都不显示时，会有 8 像素的间距
    hbox_layout->addSpacing(8);
    hbox_layout->addWidget(contact_technical_service_btn_);
    hbox_layout->addWidget(go_clean_up_btn_);
    hbox_layout->addWidget(prompt_label_);
    setLayout(hbox_layout);
}

DiagnosisSubentryWidget::~DiagnosisSubentryWidget() {

}

void DiagnosisSubentryWidget::SetStatus(enum Status status) {
    switch (status) {
        case Status::kNoState:
            status_ = DiagnosisSubentryWidget::Status::kNoState;
            status_label_->SetText("");
            break;
        case Status::kDiagnosising:
            status_ = DiagnosisSubentryWidget::Status::kDiagnosising;
            status_label_->SetFontColor(K::TextLabel::FontColor::kText);
            status_label_->SetText(tr("Diagnosising..."));
            break;
        case Status::kNormal:
            status_ = DiagnosisSubentryWidget::Status::kNormal;
            status_label_->SetFontColor("#52C429");
            status_label_->SetText(tr("Normal"));
            break;
        case Status::kException:
            status_ = DiagnosisSubentryWidget::Status::kException;
            status_label_->SetFontColor(Qt::red);
            status_label_->SetText(tr("Exception"));
            break;
        case Status::kRepairing:
            status_ = DiagnosisSubentryWidget::Status::kRepairing;
            status_label_->SetFontColor(K::TextLabel::FontColor::kText);
            status_label_->SetText(tr("Repairing..."));
            break;
        case Status::kFixed:
            status_ = DiagnosisSubentryWidget::Status::kFixed;
            status_label_->SetFontColor("#52C429");
            status_label_->SetText(tr("Fixed"));
            break;
        case Status::kRepairFailed:
            status_ = DiagnosisSubentryWidget::Status::kRepairFailed;
            status_label_->SetFontColor(Qt::red);
            status_label_->SetText(tr("Repair failed"));
            break;
        default:
            break;
    }
}

QString DiagnosisSubentryWidget::Mark() const {
    return diagnostic_item_.mark;
}

void DiagnosisSubentryWidget::SetCheckedState(Qt::CheckState state) {
    select_checkbox_->blockSignals(true);
    switch (state) {
        case Qt::CheckState::Unchecked: {
            select_checkbox_->setCheckState(Qt::CheckState::Unchecked);
            break;
        }
        default: {
            select_checkbox_->setCheckState(Qt::CheckState::Checked);
            break;
        }
    }
    select_checkbox_->blockSignals(false);
}

void DiagnosisSubentryWidget::SetCheckedEnable(bool enable) {
    select_checkbox_->setEnabled(enable);
}

void DiagnosisSubentryWidget::SetDiagnosisResult(bool ok, const QString &error_code, const QString &describe) {
    select_seat_vbox_layout_->addWidget(select_checkbox_);
    select_checkbox_->show();

    if (ok) {
        // 检测结果正确
        SetStatus(DiagnosisSubentryWidget::Status::kNormal);
        SetCheckedState(Qt::CheckState::Unchecked);
        SetCheckedEnable(false);
        return;
    }

    // 检测结果错误
    error_code_ = error_code;
    error_desc_ = describe;

    SetStatus(DiagnosisSubentryWidget::Status::kException);

    // 如果描述为空则当作可以自动修复的错误来处理
    if (describe.isEmpty() || describe == "0") {
        SetCheckedEnable(true);
        SetCheckedState(Qt::CheckState::Checked);
    } else if (describe == "1") {
        contact_technical_service_btn_->show();
        SetCheckedState(Qt::CheckState::Unchecked);
        SetCheckedEnable(false);
    } else if (describe == "3") {
        go_clean_up_btn_->show();
        SetCheckedState(Qt::CheckState::Unchecked);
        SetCheckedEnable(false);
    } else {
        prompt_label_->SetPromptContent(error_desc_);
        prompt_label_->show();
        SetCheckedState(Qt::CheckState::Unchecked);
        SetCheckedEnable(false);
    }
}

void DiagnosisSubentryWidget::SetRepairResult(bool ok) {
    if (ok) {
        SetStatus(DiagnosisSubentryWidget::Status::kFixed);
        SetCheckedState(Qt::CheckState::Unchecked);
        SetCheckedEnable(false);
    } else {
        SetStatus(DiagnosisSubentryWidget::Status::kRepairFailed);
    }
}

bool DiagnosisSubentryWidget::Selected() const {
    return (select_checkbox_->isEnabled() && (select_checkbox_->checkState() != Qt::CheckState::Unchecked));
}

enum DiagnosisSubentryWidget::Status DiagnosisSubentryWidget::Status() const {
    return status_;
}

bool DiagnosisSubentryWidget::ExistError() const {
    return (status_ == DiagnosisSubentryWidget::Status::kException) || (status_ == DiagnosisSubentryWidget::Status::kRepairFailed);
}

bool DiagnosisSubentryWidget::AllowSelect() const {
    return select_checkbox_->isEnabled();
}

QString DiagnosisSubentryWidget::ErrorCode() const {
    return error_code_;
}

QString DiagnosisSubentryWidget::Name() const {
    return diagnostic_item_.name;
}

}
