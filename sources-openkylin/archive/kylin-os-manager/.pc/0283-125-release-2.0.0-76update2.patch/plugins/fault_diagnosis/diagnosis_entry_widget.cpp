#include "diagnosis_entry_widget.h"
#include <QHBoxLayout>
#include <QDebug>
#include <QPalette>
#include <QApplication>
#include "frame.h"

namespace fault_diagnosis {

DiagnosisEntryWidget::DiagnosisEntryWidget(const DiagnosticEntry &diagnostic_entry, QWidget *parent)
    : QWidget(parent),
      diagnostic_entry_(diagnostic_entry),
      status_(Status::kNoState),
      select_seat_label_(new QLabel(this)),
      select_seat_vbox_layout_(new QVBoxLayout(select_seat_label_)),
      select_checkbox_(new QCheckBox(this)),
      name_label_(new K::TextLabel(this)),
      desc_label_(new K::TextLabel(this)),
      status_label_(new K::TextLabel(this)),
      contact_technical_service_btn_(new kdk::KBorderlessButton(this)),
      go_clean_up_btn_(new kdk::KBorderlessButton(this)),
      prompt_label_(new Prompt(this)),
      expand_or_close_seat_label_(new QLabel(this)),
      expand_or_close_seat_vbox_layout_(new QVBoxLayout(expand_or_close_seat_label_)),
      expand_or_close_btn_(new kdk::KToolButton(this)),
      is_expand_(false)
{
    setAutoFillBackground(true);    // 解决 v11 上展开列表时文字重叠问题
    setMinimumHeight(36);
    setFixedWidth(820);

    select_seat_label_->setFixedSize(18, 18);
    select_seat_vbox_layout_->setSpacing(0);
    select_seat_vbox_layout_->setContentsMargins(0, 0, 0, 0);
    select_checkbox_->hide();
    name_label_->SetText(diagnostic_entry_.item.base.name);
    desc_label_->SetFontColor(K::TextLabel::FontColor::kPlaceholderText);
    desc_label_->SetText(diagnostic_entry_.item.base.describe);
    contact_technical_service_btn_->setText(tr("Please contact technical service"));
    contact_technical_service_btn_->hide();
    connect(contact_technical_service_btn_, &kdk::KBorderlessButton::clicked, this, []() {Frame::SwitchToProblemFeedback();});
    go_clean_up_btn_->setText(tr("Go clean up"));
    go_clean_up_btn_->hide();
    connect(go_clean_up_btn_, &kdk::KBorderlessButton::clicked, this, []() {Frame::RunJunkClean();});
    prompt_label_->setFixedSize(16, 16);
    prompt_label_->hide();
    expand_or_close_seat_label_->setFixedSize(16, 16);
    expand_or_close_seat_vbox_layout_->setSpacing(0);
    expand_or_close_seat_vbox_layout_->setContentsMargins(0, 0, 0, 0);
    expand_or_close_btn_->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    expand_or_close_btn_->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    if (!diagnostic_entry_.item.subitem.empty()) {
        // 该基础检测项存在子检测项
        expand_or_close_seat_vbox_layout_->addWidget(expand_or_close_btn_);
        expand_or_close_btn_->show();
    } else {
        expand_or_close_btn_->hide();
    }
    connect(expand_or_close_btn_, &kdk::KToolButton::clicked, this, &DiagnosisEntryWidget::on_UpdateExpandOrCloseIcon);
    connect(select_checkbox_, &QCheckBox::stateChanged, this, &DiagnosisEntryWidget::sig_SelectChange);

    auto *hbox_Layout = new QHBoxLayout;
    hbox_Layout->setSpacing(0);
    hbox_Layout->setContentsMargins(69, 0, 40, 0);
    hbox_Layout->addWidget(select_seat_label_);
    hbox_Layout->addSpacing(8);
    hbox_Layout->addWidget(name_label_);
    hbox_Layout->addSpacing(8);
    hbox_Layout->addWidget(desc_label_);
    hbox_Layout->addStretch();
    hbox_Layout->addSpacing(8);
    hbox_Layout->addWidget(status_label_);
    // TODO: 当两个控件都隐藏时，间距会变成 16
    hbox_Layout->addSpacing(8);
    hbox_Layout->addWidget(contact_technical_service_btn_);
    hbox_Layout->addWidget(go_clean_up_btn_);
    hbox_Layout->addWidget(prompt_label_);
    hbox_Layout->addSpacing(8);
    hbox_Layout->addWidget(expand_or_close_seat_label_);
    setLayout(hbox_Layout);
}

DiagnosisEntryWidget::~DiagnosisEntryWidget() {

}

void DiagnosisEntryWidget::on_UpdateExpandOrCloseIcon() {
    is_expand_ = !is_expand_;
    if (is_expand_) {
        expand_or_close_btn_->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
    } else {
        expand_or_close_btn_->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    }

    Q_EMIT sig_Expanded(is_expand_);
}

void DiagnosisEntryWidget::SetStatus(enum Status status) {
    switch (status) {
        case DiagnosisEntryWidget::Status::kNoState:
            status_ = DiagnosisEntryWidget::Status::kNoState;
            status_label_->SetText("");
            break;
        case DiagnosisEntryWidget::Status::kNormal:
            status_ = DiagnosisEntryWidget::Status::kNormal;
            status_label_->SetFontColor(Qt::green);
            status_label_->SetText(tr("Normal"));
            break;
        case DiagnosisEntryWidget::Status::kException:
            status_ = DiagnosisEntryWidget::Status::kException;
            status_label_->SetFontColor(Qt::red);
            status_label_->SetText(tr("Exception"));
            break;
        case DiagnosisEntryWidget::Status::kDiagnosising:
            status_ = DiagnosisEntryWidget::Status::kDiagnosising;
            status_label_->SetFontColor(K::TextLabel::FontColor::kText);
            status_label_->SetText(tr("Diagnosising..."));
            break;
        case DiagnosisEntryWidget::Status::kFixed:
            status_ = DiagnosisEntryWidget::Status::kFixed;
            status_label_->SetFontColor(Qt::green);
            status_label_->SetText(tr("Fixed"));
            break;
        case DiagnosisEntryWidget::Status::kRepairing:
            status_ = DiagnosisEntryWidget::Status::kRepairing;
            status_label_->SetFontColor(K::TextLabel::FontColor::kText);
            status_label_->SetText(tr("Repairing..."));
            break;
        case DiagnosisEntryWidget::Status::KReapriFailed:
            status_ = DiagnosisEntryWidget::Status::KReapriFailed;
            status_label_->SetFontColor(Qt::red);
            status_label_->SetText(tr("Repair failed"));
            break;
        default:
            break;
    }
}

void DiagnosisEntryWidget::SetCheckedState(Qt::CheckState state) {
    select_checkbox_->blockSignals(true);
    select_checkbox_->setCheckState(state);
    select_checkbox_->blockSignals(false);
}

void DiagnosisEntryWidget::SetCheckedEnable(bool enable) {
    select_checkbox_->setEnabled(enable);
}

QString DiagnosisEntryWidget::DiagnosisMark() const {
    return diagnostic_entry_.diagnosis_mark;
}

QString DiagnosisEntryWidget::Mark() const {
    return diagnostic_entry_.item.base.mark;
}

QString DiagnosisEntryWidget::Name() const {
    return diagnostic_entry_.item.base.name;
}

bool DiagnosisEntryWidget::ExistError() const {
    return (status_ == DiagnosisEntryWidget::Status::kException) || (status_ == DiagnosisEntryWidget::Status::KReapriFailed);
}

bool DiagnosisEntryWidget::Selected() {
    return (select_checkbox_->isEnabled() && (select_checkbox_->checkState() != Qt::CheckState::Unchecked));
}

void DiagnosisEntryWidget::SetExpanded(bool is_expand) {
    if (diagnostic_entry_.item.subitem.empty()) {
        return;
    }
    if (is_expand != is_expand_) {
        on_UpdateExpandOrCloseIcon();
    }
}

void DiagnosisEntryWidget::SetDiagnosisResult(bool ok, const QString &error_code, const QString &describe) {
    select_seat_vbox_layout_->addWidget(select_checkbox_);
    select_checkbox_->show();

    if (ok) {
        // 检测结果正确
        SetStatus(DiagnosisEntryWidget::Status::kNormal);
        SetCheckedState(Qt::CheckState::Unchecked);
        SetCheckedEnable(false);
        return;
    }

    // 检测结果错误
    error_code_ = error_code;
    error_desc_ = describe;

    SetStatus(DiagnosisEntryWidget::Status::kException);

    QString flag;
    QString content;
    if (describe.contains("*|*")) {
        auto list = describe.split("*|*");
        if (list.size() > 1) {
            flag = list.at(0).trimmed();
            content = list.at(1).trimmed();
        } else {
            content = describe;
        }
    } else if (describe == "0") {
        flag = "0";
    } else if (describe == "1") {
        flag = "1";
    } else if (describe == "3") {
        flag = "3";
    } else {
        content = describe;
    }

    // 如果描述为空则当作可以自动修复的错误来处理
    if (describe.isEmpty() || flag == "0") {
        SetCheckedEnable(true);
        SetCheckedState(Qt::CheckState::Checked);
    } else if (flag == "1") {
        contact_technical_service_btn_->show();
        SetCheckedState(Qt::CheckState::Unchecked);
        SetCheckedEnable(false);
    } else if (flag == "3") {
        go_clean_up_btn_->show();
        SetCheckedState(Qt::CheckState::Unchecked);
        SetCheckedEnable(false);
    } else {
        prompt_label_->SetPromptContent(content);
        prompt_label_->show();
        SetCheckedState(Qt::CheckState::Unchecked);
        SetCheckedEnable(false);
    }
}

void DiagnosisEntryWidget::SetRepairResult(bool ok) {
    if (ok) {
        SetStatus(DiagnosisEntryWidget::Status::kFixed);
        SetCheckedState(Qt::CheckState::Unchecked);
        SetCheckedEnable(false);
    } else {
        SetStatus(DiagnosisEntryWidget::Status::KReapriFailed);
    }
}

enum DiagnosisEntryWidget::Status DiagnosisEntryWidget::Status() const {
    return status_;
}

QString DiagnosisEntryWidget::ErrorCode() const {
    return error_code_;
}

void DiagnosisEntryWidget::Disable() {
    setDisabled(true);
}

}
