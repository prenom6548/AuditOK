#include "prompt.h"
#include <QIcon>
#include <QHBoxLayout>
#include <QDebug>
#include <QApplication>
#include <QPalette>
#include "utils.h"

namespace fault_diagnosis {

Prompt::Prompt(QWidget *parent)
    : QLabel(parent),
      bubble_widget_(new kdk::KBubbleWidget),
      content_label_(new QLabel(this)),
      fault_diagnosis_mainwindow_(nullptr)
{
    setWindowFlags(Qt::FramelessWindowHint);

    auto pixmap = QIcon::fromTheme("preferences-system-details-symbolic").pixmap(16, 16);
    setPixmap(K::Utils::ChangePixmapColor(pixmap, qApp->palette().color(QPalette::Text)));

    QWidget *pointer = this;
    while ((pointer = pointer->parentWidget()) != nullptr) {
        if (pointer->objectName() == "fault_diagnosis_mainwindow") {
            bubble_widget_->setParent(pointer);
            fault_diagnosis_mainwindow_ = pointer;
            break;
        }
    }
    bubble_widget_->setMaximumWidth(735);
    bubble_widget_->setBorderRadius(12, 12, 12, 12);
    bubble_widget_->setTailPosition(kdk::TailDirection::BottomDirection, kdk::TailLocation::RightLocation);
    bubble_widget_->hide();

    content_label_->setWordWrap(true);
    content_label_->setAlignment(Qt::AlignCenter);
    auto *hbox_layout = new QHBoxLayout;
    hbox_layout->setSpacing(0);
    hbox_layout->setContentsMargins(20, 13, 20, 20);
    hbox_layout->addWidget(content_label_);
    bubble_widget_->setLayout(hbox_layout);
}

Prompt::~Prompt() {

}

void Prompt::SetPromptContent(const QString &content) {
    content_ = content;
    content_label_->setText(content);
}

void Prompt::enterEvent([[maybe_unused]]QEvent *event) {
    bubble_widget_->adjustSize();
    bubble_widget_->show();
    if (fault_diagnosis_mainwindow_) {
        // FIXME: 获取到的 position 中的横坐标是错误的，此处先写死了横坐标，还需要排查
        auto position = fault_diagnosis_mainwindow_->mapFromGlobal(mapToGlobal(pos()));
        bubble_widget_->move(752 - bubble_widget_->width() + 30, position.y() - bubble_widget_->height() - 10);
    } else {
        qCritical() << "Fail to obtain display location";
    }
}

void Prompt::leaveEvent([[maybe_unused]]QEvent *event) {
    bubble_widget_->hide();
}

}
