#include <QHBoxLayout>
#include <QDebug>
#include "diagnosis_entry_widget.h"

namespace fault_diagnosis {

DiagnosisEntryWidget::DiagnosisEntryWidget(const DiagnosticEntry &diagnostic_entry, QWidget *parent)
    : QWidget(parent),
      diagnostic_entry_(diagnostic_entry),
      select_seat_label_(new QLabel(this)),
      select_seat_vbox_layout_(new QVBoxLayout(select_seat_label_)),
      select_checkbox_(new QCheckBox(this)),
      name_label_(new K::TextLabel(this)),
      diagnosis_result_label_(new K::TextLabel(this)),
      status_label_(new K::TextLabel(this)),
      expand_or_close_seat_label_(new QLabel(this)),
      expand_or_close_seat_vbox_layout_(new QVBoxLayout(expand_or_close_seat_label_)),
      expand_or_close_btn_(new kdk::KToolButton(this)),
      is_expand_(false)
{
    setMinimumHeight(36);
    setFixedWidth(820);

    select_checkbox_->hide();
    expand_or_close_btn_->hide();

    select_seat_label_->setFixedSize(16, 16);
    select_seat_vbox_layout_->setSpacing(0);
    select_seat_vbox_layout_->setContentsMargins(0, 0, 0, 0);
    name_label_->SetText(diagnostic_entry_.item.name);
    diagnosis_result_label_->SetFontColor(Qt::red);
    expand_or_close_seat_label_->setFixedSize(16, 16);
    expand_or_close_seat_vbox_layout_->setSpacing(0);
    expand_or_close_seat_vbox_layout_->setContentsMargins(0, 0, 0, 0);
    expand_or_close_btn_->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    expand_or_close_btn_->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    connect(expand_or_close_btn_, &kdk::KToolButton::clicked, this, &DiagnosisEntryWidget::on_UpdateExpandOrCloseIcon);
    connect(select_checkbox_, &QCheckBox::stateChanged, this, &DiagnosisEntryWidget::sig_SelectChange);

    auto *hbox_Layout = new QHBoxLayout;
    hbox_Layout->setSpacing(0);
    hbox_Layout->setContentsMargins(69, 0, 40, 0);
    hbox_Layout->addWidget(select_seat_label_);
    hbox_Layout->addSpacing(13);
    hbox_Layout->addWidget(name_label_);
    hbox_Layout->addSpacing(94);
    hbox_Layout->addWidget(diagnosis_result_label_);
    hbox_Layout->addStretch();
    hbox_Layout->addSpacing(16);
    hbox_Layout->addWidget(status_label_);
    hbox_Layout->addSpacing(12);
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

void DiagnosisEntryWidget::SetStatus(Status status) {
    switch (status) {
    case DiagnosisEntryWidget::Status::kNoState:
        status_label_->SetText("");
        break;
    case DiagnosisEntryWidget::Status::kNormal:
        status_label_->SetFontColor(Qt::green);
        status_label_->SetText(tr("Normal"));
        break;
    case DiagnosisEntryWidget::Status::kException:
        status_label_->SetFontColor(Qt::red);
        status_label_->SetText(tr("Exception"));
        break;
    case DiagnosisEntryWidget::Status::kDiagnosising:
        //TODO: 根据主题选择颜色
        status_label_->SetFontColor(Qt::white);
        status_label_->SetText(tr("Diagnosising..."));
        break;
    case DiagnosisEntryWidget::Status::kFixed:
        status_label_->SetFontColor(Qt::green);
        status_label_->SetText(tr("Fixed"));
    default:
        break;
    }
}

void DiagnosisEntryWidget::SetDiagnosticResult(const CheckEntryResult &check_entry_result) {
    check_entry_result_ = check_entry_result;
    select_checkbox_->show();
    select_seat_vbox_layout_->addWidget(select_checkbox_);

    if (!check_entry_result.ok) {
        diagnosis_result_label_->SetText(tr("Backend detection error, please retest or provide feedback"));
        SetStatus(DiagnosisEntryWidget::Status::kException);
        return;
    }

    if (!check_entry_result.result.details.isEmpty()) {
        diagnosis_result_label_->SetText(check_entry_result.result.overview);
        SetStatus(DiagnosisEntryWidget::Status::kException);
        expand_or_close_btn_->show();
        expand_or_close_seat_vbox_layout_->addWidget(expand_or_close_btn_);
    } else {
        SetStatus(DiagnosisEntryWidget::Status::kNormal);
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
   return  diagnostic_entry_.diagnosis_mark;
}

qint64 DiagnosisEntryWidget::Mark() const {
    return diagnostic_entry_.item.mark;
}

bool DiagnosisEntryWidget::ExistError() const {
    return (!check_entry_result_.result.details.isEmpty() || !check_entry_result_.ok);
}

void DiagnosisEntryWidget::Reset() {
    select_seat_vbox_layout_->removeWidget(select_checkbox_);
    SetCheckedState(Qt::CheckState::Unchecked);
    select_checkbox_->hide();
    diagnosis_result_label_->SetText("");
    SetStatus(DiagnosisEntryWidget::Status::kNoState);
    SetExpanded(false);
    expand_or_close_seat_vbox_layout_->removeWidget(expand_or_close_btn_);
    expand_or_close_btn_->hide();
}

bool DiagnosisEntryWidget::Selected() {
    return (select_checkbox_->isEnabled() && select_checkbox_->checkState() != Qt::CheckState::Unchecked);
}

void DiagnosisEntryWidget::SetExpanded(bool is_expand) {
    if (is_expand != is_expand_) {
        on_UpdateExpandOrCloseIcon();
    }
}

}
