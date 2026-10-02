#include "clean_up_finish_widget.h"
#include <QPixmap>
#include <QVariant>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QProcess>
#include "utils.h"

namespace junk_clean {

CleanUpFinishWidget::CleanUpFinishWidget(QWidget *parent)
    : QWidget(parent),
      clean_success_icon_label_(new K::IconLabel(this)),
      clean_success_describe_label_(new K::TextLabel(this)),
      clean_file_icon_label_(new K::IconLabel(this)),
      clean_file_describe_label_(new K::TextLabel(this)),
      clean_time_icon_label_(new K::IconLabel(this)),
      clean_time_describe_label_(new K::TextLabel(this)),
      complete_btn_(new QPushButton(this)),
      deep_clean_btn_(new kdk::KBorderlessButton(this))
{
    clean_success_icon_label_->setFixedSize(64, 64);
    clean_success_icon_label_->SetPixmap(QPixmap(":/data/icons/clean_success.svg"));
    auto *clean_success_icon_hbox_layout = new QHBoxLayout;
    clean_success_icon_hbox_layout->setContentsMargins(0, 0, 0, 0);
    clean_success_icon_hbox_layout->setSpacing(0);
    clean_success_icon_hbox_layout->addStretch();
    clean_success_icon_hbox_layout->addWidget(clean_success_icon_label_);
    clean_success_icon_hbox_layout->addStretch();

    clean_success_describe_label_->SetPixelSize(24, true);
    auto *clean_success_describe_hbox_layout = new QHBoxLayout;
    clean_success_describe_hbox_layout->setContentsMargins(0, 0, 0, 0);
    clean_success_describe_hbox_layout->setSpacing(0);
    clean_success_describe_hbox_layout->addStretch();
    clean_success_describe_hbox_layout->addWidget(clean_success_describe_label_);
    clean_success_describe_hbox_layout->addStretch();

    clean_file_icon_label_->setFixedSize(16, 16);
    clean_file_icon_label_->SetIcon(QIcon::fromTheme("edit-delete-symbolic"));
    clean_time_icon_label_->setFixedSize(16, 16);
    clean_time_icon_label_->SetIcon(QIcon::fromTheme("document-open-recent-symbolic"));
    auto *separator = new QFrame(this);
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Shadow::Raised);
    auto palette = separator->palette();
    palette.setColor(QPalette::Dark, QColor(217, 217, 217));
    separator->setPalette(palette);
    auto *clean_info_hbox_layout = new QHBoxLayout;
    clean_info_hbox_layout->setContentsMargins(0, 0, 0, 0);
    clean_info_hbox_layout->setSpacing(0);
    clean_info_hbox_layout->addStretch();
    clean_info_hbox_layout->addWidget(clean_file_icon_label_);
    clean_info_hbox_layout->addSpacing(8);
    clean_info_hbox_layout->addWidget(clean_file_describe_label_);
    clean_info_hbox_layout->addSpacing(12);
    clean_info_hbox_layout->addWidget(separator);
    clean_info_hbox_layout->addSpacing(12);
    clean_info_hbox_layout->addWidget(clean_time_icon_label_);
    clean_info_hbox_layout->addSpacing(8);
    clean_info_hbox_layout->addWidget(clean_time_describe_label_);
    clean_info_hbox_layout->addStretch();

    complete_btn_->setMinimumSize(96, 36);
    complete_btn_->setText(tr("Complete"));
    complete_btn_->setProperty("isImportant", true);
    auto *complete_hbox_layout = new QHBoxLayout;
    complete_hbox_layout->setContentsMargins(0, 0, 0, 0);
    complete_hbox_layout->setSpacing(0);
    complete_hbox_layout->addStretch();
    complete_hbox_layout->addWidget(complete_btn_);
    complete_hbox_layout->addStretch();
    connect(complete_btn_, &QPushButton::clicked, this, &CleanUpFinishWidget::sig_Complete);

    deep_clean_btn_->setText(tr("Deep clean"));
    auto *deep_clean_hboxy_layout = new QHBoxLayout;
    deep_clean_hboxy_layout->setContentsMargins(0, 0, 0, 0);
    deep_clean_hboxy_layout->setSpacing(0);
    deep_clean_hboxy_layout->addStretch();
    deep_clean_hboxy_layout->addWidget(deep_clean_btn_);
    deep_clean_hboxy_layout->addStretch();
    connect(deep_clean_btn_, &kdk::KBorderlessButton::clicked, this, []() {
        QProcess process;
        process.setProgram("/usr/lib/kylin-os-manager/bin/kylin-system-cleanup");
        process.startDetached();
    });

    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setContentsMargins(0, 167, 0, 0);
    vbox_layout->setSpacing(0);
    vbox_layout->addLayout(clean_success_icon_hbox_layout);
    vbox_layout->addSpacing(24);
    vbox_layout->addLayout(clean_success_describe_hbox_layout);
    vbox_layout->addSpacing(8);
    vbox_layout->addLayout(clean_info_hbox_layout);
    vbox_layout->addSpacing(40);
    vbox_layout->addLayout(complete_hbox_layout);
    vbox_layout->addSpacing(16);
    vbox_layout->addLayout(deep_clean_hboxy_layout);
    vbox_layout->addStretch();

    setLayout(vbox_layout);
}

void CleanUpFinishWidget::SetCleanedSize(quint64 byte) {
    clean_success_describe_label_->SetText(tr("Successfully cleaned up %1 of garbage").arg(K::Utils::ByteToString(byte)));
}

void CleanUpFinishWidget::SetCleanedFileCount(quint64 count) {
    clean_file_describe_label_->SetText(tr("Cleaning files: %1 entries").arg(count));
}

void CleanUpFinishWidget::SetCleanTime(quint64 second) {
    clean_time_describe_label_->SetText(tr("Cleaning time: %1 second").arg(second));
}

}
