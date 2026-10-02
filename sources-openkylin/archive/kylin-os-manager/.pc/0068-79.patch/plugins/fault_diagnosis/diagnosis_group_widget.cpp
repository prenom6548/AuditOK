#include <QHBoxLayout>
#include <QIcon>
#include "diagnosis_group_widget.h"

namespace fault_diagnosis {

DiagnosisGroupWidget::DiagnosisGroupWidget(QWidget *parent)
    : QWidget(parent),
      icon_label_(new K::IconLabel(this)),
      category_label_(new K::TextLabel(this)),
      total_label_(new K::TextLabel(this)),
      status_label_(new K::TextLabel(this)),
      expand_or_close_btn_(new kdk::KToolButton(this)),
      is_expand_(false)
{
    setMinimumHeight(48);
    setFixedWidth(820);
    icon_label_->setFixedSize(16, 16);
    category_label_->SetPixelSize(16, true);
    total_label_->SetPixelSize(12);
    expand_or_close_btn_->setFixedSize(16, 16);
    expand_or_close_btn_->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    expand_or_close_btn_->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    connect(expand_or_close_btn_, &kdk::KToolButton::clicked, this, &DiagnosisGroupWidget::on_UpdateExpandOrCloseIcon);

    auto *hbox_layout = new QHBoxLayout;
    hbox_layout->setSpacing(0);
    hbox_layout->setContentsMargins(40, 0, 40, 0);
    hbox_layout->addWidget(icon_label_);
    hbox_layout->addSpacing(12);
    hbox_layout->addWidget(category_label_);
    hbox_layout->addSpacing(12);
    hbox_layout->addWidget(total_label_);
    hbox_layout->addStretch();
    hbox_layout->addSpacing(16);
    hbox_layout->addWidget(status_label_);
    hbox_layout->addSpacing(12);
    hbox_layout->addWidget(expand_or_close_btn_);
    setLayout(hbox_layout);
}

DiagnosisGroupWidget::~DiagnosisGroupWidget() {

}

void DiagnosisGroupWidget::on_UpdateExpandOrCloseIcon() {
    is_expand_ = !is_expand_;
    if (is_expand_) {
        expand_or_close_btn_->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
    } else {
        expand_or_close_btn_->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    }

    Q_EMIT sig_Expanded(is_expand_);
}

void DiagnosisGroupWidget::SetIcon(const QString &name) {
    icon_label_->SetIcon(QIcon::fromTheme(name));
}

void DiagnosisGroupWidget::SetCategory(const QString &category_name) {
    category_label_->SetText(category_name);
}

void DiagnosisGroupWidget::SetTotal(int total) {
    total_label_->SetText(tr("Scanned %1 items").arg(total));
}

void DiagnosisGroupWidget::SetStatus(Status status) {
    switch (status) {
    case DiagnosisGroupWidget::Status::kNoState:
        status_label_->SetText("");
        break;
    case DiagnosisGroupWidget::Status::kNormal:
        status_label_->SetFontColor(Qt::green);
        status_label_->SetText(tr("Normal"));
        break;
    case DiagnosisGroupWidget::Status::kException:
        status_label_->SetFontColor(Qt::red);
        status_label_->SetText(tr("Exception"));
        break;
    case DiagnosisGroupWidget::Status::kFix:
        status_label_->SetFontColor(Qt::green);
        status_label_->SetText(tr("Fixed"));
        break;
    default:
        break;
    }
}

QString DiagnosisGroupWidget::Category() const {
    return category_label_->text();
}

void DiagnosisGroupWidget::SetExpanded(bool is_expand) {
    if (is_expand != is_expand_)
        on_UpdateExpandOrCloseIcon();
}

void DiagnosisGroupWidget::Reset() {
    SetTotal(0);
    SetStatus(DiagnosisGroupWidget::Status::kNoState);
    SetExpanded(false);
}

}
