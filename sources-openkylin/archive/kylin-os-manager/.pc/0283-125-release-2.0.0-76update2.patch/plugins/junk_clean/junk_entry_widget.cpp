#include "junk_entry_widget.h"
#include <QHBoxLayout>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include "utils.h"

namespace junk_clean
{

JunkEntryWidget::JunkEntryWidget(quint64 mark, enum Type type, const QString &cleaner_mark, QWidget *parent)
    : QWidget(parent),
      select_checkbox_(new QCheckBox(this)),
      path_label_(new K::TextLabel(this)),
      size_label_(new K::TextLabel(this)),
      mark_(mark),
      type_(type),
      cleaner_mark_(cleaner_mark),
      size_(0),
      file_count_(0)
{
    setFixedSize(824, 36);

    select_checkbox_->setFixedSize(18, 18);
    connect(select_checkbox_, &QCheckBox::stateChanged, this, &JunkEntryWidget::sig_CheckBoxStateChanged);
    path_label_->setMaximumWidth(540);
    if (type == Type::kUsageTraces) {
        size_label_->SetText(tr("0 entries"));
    } else {
        size_label_->SetText(tr("0 B"));
    }
    auto *hbox_layout = new QHBoxLayout;
    hbox_layout->setContentsMargins(96, 0, 68, 0);
    hbox_layout->setSpacing(0);
    hbox_layout->addWidget(select_checkbox_);
    hbox_layout->addSpacing(12);
    hbox_layout->addWidget(path_label_);
    hbox_layout->addStretch();
    hbox_layout->addSpacing(16);
    hbox_layout->addWidget(size_label_);
    setLayout(hbox_layout);
}

Qt::CheckState JunkEntryWidget::CheckState() const {
    return (select_checkbox_->checkState() == Qt::CheckState::Unchecked) ? Qt::CheckState::Unchecked : Qt::CheckState::Checked;
}

void JunkEntryWidget::SetCheckState(Qt::CheckState check_state) const {
    select_checkbox_->blockSignals(true);
    switch (check_state) {
        case Qt::CheckState::Unchecked:
            select_checkbox_->setCheckState(Qt::CheckState::Unchecked);
            break;
        default:
            select_checkbox_->setCheckState(Qt::CheckState::Checked);
            break;
    }
    select_checkbox_->blockSignals(false);
}

void JunkEntryWidget::SetContent(const QString &content) {
    path_label_->SetText(content);
    file_count_ = CalculateFileCount(content);
}

void JunkEntryWidget::SetSize(quint64 size) {
    size_ = size;
    switch (type_) {
        case Type::kSystemJunk:
            [[fallthrough]];
        case Type::kInternetJunk:
            size_label_->SetText(K::Utils::ByteToString(size));
            break;
        case Type::kUsageTraces:
            size_label_->SetText(tr("%1 entries").arg(size));
            break;
        default:
            break;
    }
}

quint64 JunkEntryWidget::Size() const {
    return size_;
}

quint64 JunkEntryWidget::Mark() const {
    return mark_;
}

QString JunkEntryWidget::Content() const {
    return path_label_->text();
}

quint64 JunkEntryWidget::CalculateFileCount(const QString &path) const {
    if (type_ == Type::kUsageTraces) {
        return 0;
    }
    // 如果是卸载残留、回收站清理者，则每一条垃圾项算作一个文件
    if (cleaner_mark_ == "residual_cleaner" ||
        cleaner_mark_ == "trash_cleaner")
    {
        return 1;
    }

    QFileInfo file_info(path);
    if (!file_info.exists()) {
        return 0;
    }

    quint64 total_file_count{0};
    if (file_info.isFile()) {
        total_file_count++;
    } else if (file_info.isDir()) {
        QDir dir(path);
        auto entries = dir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto &entry: entries) {
            auto absolute_file_path = dir.absoluteFilePath(entry);
            QFileInfo entry_info(absolute_file_path);
            if (entry_info.isFile()) {
                total_file_count++;
            } else {
                total_file_count += CalculateFileCount(absolute_file_path);
            }
        }
    }

    return total_file_count;
}

quint64 JunkEntryWidget::FileCount() const {
    return file_count_;
}

void JunkEntryWidget::SetDisable(bool ok) {
    setDisabled(ok);
}

bool JunkEntryWidget::AllowSelect() const {
    return isEnabled();
}

}
