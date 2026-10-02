#include "clean_up_entry_widget.h"
#include <QHBoxLayout>
#include <QApplication>
#include <QPalette>
#include "utils.h"

namespace junk_clean {

CleanUpEntryWidget::CleanUpEntryWidget(const QString &mark, enum Type type, enum Level level, QWidget *parent)
    : QWidget(parent),
      mark_(mark),
      type_(type),
      level_(level),
      select_checkbox_(new QCheckBox(this)),
      select_checkbox_seat_label_(new QLabel(this)),
      select_checkbox_seat_vbox_layout_(new QVBoxLayout(select_checkbox_seat_label_)),
      name_label_(new K::TextLabel(this)),
      describe_label_(new K::TextLabel(this)),
      state_label_(new K::TextLabel(this)),
      expand_or_close_btn_(new kdk::KToolButton(this)),
      expand_or_close_seat_label_(new QLabel(this)),
      expand_or_close_seat_vbox_layout_(new QVBoxLayout(expand_or_close_seat_label_)),
      is_expand_(false),
      size_(0)
{
    this->setObjectName("Kylin-os-manager_QWidget_CleanUpEntryWidget");
    this->setAccessibleName(this->objectName());
    setAutoFillBackground(true);    // 解决 v11 上展开列表时文字重叠问题
    setFixedSize(824, 36);

    select_checkbox_->setFixedSize(18, 18);
    select_checkbox_->hide();
    connect(select_checkbox_, &QCheckBox::stateChanged, this, &CleanUpEntryWidget::sig_CheckBoxStateChanged);
    select_checkbox_seat_label_->setFixedSize(18, 18);
    select_checkbox_seat_vbox_layout_->setContentsMargins(0, 0, 0, 0);
    select_checkbox_seat_vbox_layout_->setSpacing(0);
    expand_or_close_btn_->setFixedSize(16, 16);
    expand_or_close_btn_->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    expand_or_close_btn_->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    expand_or_close_btn_->hide();
    connect(expand_or_close_btn_, &kdk::KToolButton::clicked, this, &CleanUpEntryWidget::on_ExpandOrCloseBtnClicked);
    expand_or_close_seat_label_->setFixedSize(16, 16);
    expand_or_close_seat_vbox_layout_->setContentsMargins(0, 0, 0, 0);
    expand_or_close_seat_vbox_layout_->setSpacing(0);

    auto *hbox_layout = new QHBoxLayout;
    hbox_layout->setContentsMargins(68, 0, 40, 0);
    hbox_layout->setSpacing(0);
    hbox_layout->addWidget(select_checkbox_seat_label_);
    hbox_layout->addSpacing(13);
    hbox_layout->addWidget(name_label_);
    hbox_layout->addSpacing(16);
    hbox_layout->addWidget(describe_label_);
    hbox_layout->addStretch();
    hbox_layout->addSpacing(16);
    hbox_layout->addWidget(state_label_);
    hbox_layout->addSpacing(12);
    hbox_layout->addWidget(expand_or_close_seat_label_);
    setLayout(hbox_layout);
}

void CleanUpEntryWidget::on_ExpandOrCloseBtnClicked() {
    is_expand_ = !is_expand_;
    if (is_expand_) {
        expand_or_close_btn_->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
    } else {
        expand_or_close_btn_->setIcon(QIcon::fromTheme("ukui-down-symbolic"));
    }
    Q_EMIT sig_Expand(is_expand_);
}

void CleanUpEntryWidget::SetName(const QString &name) {
    name_label_->SetText(name);
}

void CleanUpEntryWidget::SetDescribe(const QString &describe) {
    describe_label_->SetText(describe);
}

void CleanUpEntryWidget::SetSize(quint64 size) {
    size_ = size;

    switch (type_) {
        case Type::kSystemJunk:
            [[fallthrough]];
        case Type::kInternetJunk:
            if (level_ == Level::kCareful) {
                state_label_->SetFontColor(Qt::red);
                state_label_->SetText(tr("%1 in total, clean carefully").arg(K::Utils::ByteToString(size)));
            } else {
                state_label_->SetFontColor(K::TextLabel::FontColor::kText);
                state_label_->SetText(K::Utils::ByteToString(size));
            }
            break;
        case Type::kUsageTraces:
            if (level_ == Level::kCareful) {
                state_label_->SetFontColor(Qt::red);
                state_label_->SetText(tr("%1 entries in total, carefully clean up").arg(size));
            } else {
                state_label_->SetFontColor(K::TextLabel::FontColor::kText);
                state_label_->SetText(tr("%1 entries").arg(size));
            }
            break;
        default:
            break;
    }
}

void CleanUpEntryWidget::SetSize(enum SpecialSize special_size) {
    size_ = 0;
    switch (special_size) {
        case SpecialSize::kScanning:
            state_label_->SetFontColor(K::TextLabel::FontColor::kText);
            state_label_->SetText(tr("Scanning..."));
            break;
        case SpecialSize::kVeryClean:
            state_label_->SetFontColor("#52C429");
            state_label_->SetText(tr("Very clean"));
            break;
        default:
            break;
    }
}

QString CleanUpEntryWidget::Mark() const {
    return mark_;
}

void CleanUpEntryWidget::ShowCheckBox(bool show) const {
    if (show) {
        select_checkbox_seat_vbox_layout_->addWidget(select_checkbox_);
        select_checkbox_->show();
    } else {
        select_checkbox_seat_vbox_layout_->removeWidget(select_checkbox_);
        select_checkbox_->hide();
    }
}

void CleanUpEntryWidget::ShowExpandBtn(bool show) {
    if (show) {
        if (is_expand_) {
            // 默认为合并状态
            on_ExpandOrCloseBtnClicked();
        }
        expand_or_close_seat_vbox_layout_->addWidget(expand_or_close_btn_);
        expand_or_close_btn_->show();
    } else {
        expand_or_close_seat_vbox_layout_->removeWidget(expand_or_close_btn_);
        expand_or_close_btn_->hide();
    }
}

void CleanUpEntryWidget::SetCheckState(Qt::CheckState check_state) {
    select_checkbox_->blockSignals(true);
    select_checkbox_->setCheckState(check_state);
    select_checkbox_->blockSignals(false);
}

Qt::CheckState CleanUpEntryWidget::CheckState() const {
    return select_checkbox_->checkState();
}

void CleanUpEntryWidget::SetCheckable(bool checkable) {
    select_checkbox_->setEnabled(checkable);
}

enum Type CleanUpEntryWidget::Type() const {
    return type_;
}

enum Level CleanUpEntryWidget::Level() const {
    return level_;
}

quint64 CleanUpEntryWidget::Size() const {
    return size_;
}

void CleanUpEntryWidget::Reset() {
    SetSize(0);
    ShowCheckBox(false);
    ShowExpandBtn(false);
}

}
