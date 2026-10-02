#include "clean_up_finish_widget.h"
#include <QPixmap>
#include <QVariant>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QProcess>
#include <QDebug>
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
      clean_need_deep_label_(new K::TextLabel(this)),
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

    clean_success_describe_label_->SetPixelSize(20, true);
    auto *clean_success_describe_hbox_layout = new QHBoxLayout;
    clean_success_describe_hbox_layout->setContentsMargins(0, 0, 0, 0);
    clean_success_describe_hbox_layout->setSpacing(0);
    clean_success_describe_hbox_layout->addStretch();
    clean_success_describe_hbox_layout->addWidget(clean_success_describe_label_);
    clean_success_describe_hbox_layout->addStretch();

    clean_need_deep_label_->SetPixelSize(20, true);
    clean_need_deep_label_->hide();
    auto *clean_need_deep_hbox_layout = new QHBoxLayout;
    clean_need_deep_hbox_layout->setContentsMargins(0, 0, 0, 0);
    clean_need_deep_hbox_layout->setSpacing(0);
    clean_need_deep_hbox_layout->addStretch();
    clean_need_deep_hbox_layout->addWidget(clean_need_deep_label_);
    clean_need_deep_hbox_layout->addStretch();

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
    connect(deep_clean_btn_, &kdk::KBorderlessButton::clicked, this, &CleanUpFinishWidget::startDeepClean);

    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setContentsMargins(0, 167, 0, 0);
    vbox_layout->setSpacing(0);
    vbox_layout->addLayout(clean_success_icon_hbox_layout);
    vbox_layout->addSpacing(24);
    vbox_layout->addLayout(clean_success_describe_hbox_layout);
    vbox_layout->addLayout(clean_need_deep_hbox_layout);
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
    clean_success_describe_label_->SetText(tr("%1 of garbage has been cleared").arg(K::Utils::ByteToString(byte)));
}

void CleanUpFinishWidget::SetCleanedFileCount(quint64 count) {
    clean_file_describe_label_->SetText(tr("Cleaning files: %1 entries").arg(count));
}

void CleanUpFinishWidget::SetCleanTime(quint64 second) {
    clean_time_describe_label_->SetText(tr("Cleaning time: %1 second").arg(second));
}

void junk_clean::CleanUpFinishWidget::checkDiskAgain()
{
    QProcess process;
    process.setProgram("df");
    process.setArguments({"-h"});
    process.start();
    process.waitForFinished();

    auto output = process.readAllStandardOutput();
    auto lines = QString::fromUtf8(output).split('\n');
    QString sizeStr;
    for (const auto &line: lines) {
        auto elementes = line.split(" ");
        elementes.removeAll("");
        if (elementes.size() != 6) {
            qCritical() << "Disk capacity check element count error: " << line << " [" << elementes << "]";
            continue;
        }

        auto mount_point = elementes.at(5);
        if (mount_point != "/data")
        {
            continue;
        }

        bool ok{false};
        auto used = elementes.at(4).trimmed().remove("%");
        auto percent = used.toDouble(&ok);
        if (!ok) {
            qCritical() << "Convert " <<  mount_point << "mount point disk usage fail.";
            continue;
        }

        if (percent > 90.0) {
            sizeStr = elementes.at(3);
            break;
        }
    }
    needDeepClean(sizeStr);
}

void junk_clean::CleanUpFinishWidget::needDeepClean(QString size)
{
    disconnect(complete_btn_, &QPushButton::clicked, nullptr, nullptr);
    disconnect(deep_clean_btn_, &kdk::KBorderlessButton::clicked, nullptr, nullptr);
    if (!size.isEmpty()) {
        complete_btn_->setText(tr("Deep clean"));
        deep_clean_btn_->setText(tr("Complete"));
        connect(complete_btn_, &QPushButton::clicked, this, &CleanUpFinishWidget::startDeepClean);
        connect(deep_clean_btn_, &kdk::KBorderlessButton::clicked, this, &CleanUpFinishWidget::sig_Complete);
        clean_need_deep_label_->SetText(tr("There is only %1 left on the disk, Please perform a deep cleaning").arg(size));
        clean_need_deep_label_->show();
        return;
    }
    complete_btn_->setText(tr("Complete"));
    deep_clean_btn_->setText(tr("Deep clean"));
    connect(complete_btn_, &QPushButton::clicked, this, &CleanUpFinishWidget::sig_Complete);
    connect(deep_clean_btn_, &kdk::KBorderlessButton::clicked, this, &CleanUpFinishWidget::startDeepClean);
    clean_need_deep_label_->hide();
}

void junk_clean::CleanUpFinishWidget::startDeepClean()
{
    QProcess process;
    process.setProgram("/usr/lib/kylin-os-manager/bin/kylin-system-cleanup");
    process.startDetached();
}

}
