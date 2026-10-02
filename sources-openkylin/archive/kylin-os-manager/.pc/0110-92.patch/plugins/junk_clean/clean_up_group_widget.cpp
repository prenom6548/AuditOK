#include "clean_up_group_widget.h"
#include <QHBoxLayout>
#include "utils.h"

namespace junk_clean {

CleanUpGroupWidget::CleanUpGroupWidget(enum Type type, QWidget *parent)
    : QWidget(parent),
      type_(type),
      icon_label_(new K::IconLabel(this)),
      category_label_(new K::TextLabel(this)),
      describe_label_(new K::TextLabel(this)),
      expand_or_close_btn_(new kdk::KToolButton(this)),
      is_expand_(false)
{
    setFixedHeight(40);
    icon_label_->setFixedSize(16, 16);
    icon_label_->SetIcon(QIcon::fromTheme(TypeIcon(type)));
    category_label_->SetPixelSize(16, true);
    category_label_->SetText(TypeToString(type));
    expand_or_close_btn_->setFixedSize(16, 16);
    expand_or_close_btn_->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    expand_or_close_btn_->setIconSize(QSize(16, 16));
    expand_or_close_btn_->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    connect(expand_or_close_btn_, &kdk::KToolButton::clicked, this, &CleanUpGroupWidget::on_ExpandOrCloseBtnClicked);

    auto *hbox_layout = new QHBoxLayout;
    hbox_layout->setContentsMargins(40, 8, 40, 0);
    hbox_layout->setSpacing(0);
    hbox_layout->addWidget(icon_label_);
    hbox_layout->addSpacing(6);
    hbox_layout->addWidget(category_label_);
    hbox_layout->addSpacing(12);
    hbox_layout->addWidget(describe_label_);
    hbox_layout->addStretch();
    hbox_layout->addSpacing(16);
    hbox_layout->addWidget(expand_or_close_btn_);
    setLayout(hbox_layout);
}

void CleanUpGroupWidget::on_ExpandOrCloseBtnClicked() {
    is_expand_ = !is_expand_;
    if (is_expand_) {
        expand_or_close_btn_->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
    } else {
        expand_or_close_btn_->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    }
    Q_EMIT sig_Expand(is_expand_);
}

void CleanUpGroupWidget::SetDescribe(const QString &describe) {
    describe_label_->SetText(describe);
}

void CleanUpGroupWidget::SetDescribe(int total_items, quint64 total_size, quint64 select_size) {
    switch (type_) {
        case Type::kSystemJunk:
            [[fallthrough]];
        case Type::kInternetJunk:
            describe_label_->SetText(tr("%1 items of %2 in total, %3 selected")
                                     .arg(total_items)
                                     .arg(K::Utils::ByteToString(total_size))
                                     .arg(K::Utils::ByteToString(select_size))) ;
            break;
        case Type::kUsageTraces:
            describe_label_->SetText(tr("%1 items with %2 traces, %3 selected")
                                     .arg(total_items)
                                     .arg(total_size)
                                     .arg(select_size));
            break;
        default:
            break;
    }
}

void CleanUpGroupWidget::SetExpand(bool is_expand) {
    if (is_expand_ == is_expand) {
        return;
    }
    on_ExpandOrCloseBtnClicked();
}

enum Type CleanUpGroupWidget::Type() const {
    return type_;
}

QString CleanUpGroupWidget::TypeToString(enum Type type) const {
    switch (type) {
        case Type::kSystemJunk:
            return tr("System junk");
        case Type::kInternetJunk:
            return tr("Internet junk");
        case Type::kUsageTraces:
            return tr("Usage traces");
        default:
            return tr("Other");
    }
}

QString CleanUpGroupWidget::TypeIcon(enum Type type) const {
    switch (type) {
        case Type::kSystemJunk:
            return "applications-system-symbolic";
        case Type::kInternetJunk:
            return "ukui-network-agent-symbolic";
        case Type::kUsageTraces:
            return "document-open-recent-symbolic";
        default:
            return "";
    }
}

}
