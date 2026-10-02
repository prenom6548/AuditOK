#include "user_experience_dialog.h"
#include <QVariant>
#include <QVBoxLayout>
#include <QPalette>
#include <QApplication>
#include <kwindowbuttonbar.h>
#include <ukuistylehelper/ukuistylehelper.h>
#include <gsettingmonitor.h>
#include "text_label.h"

namespace K {

UserExperienceDialog::UserExperienceDialog(QWidget *parent)
    : QDialog(parent),
      feedback_btn_(new QPushButton(this)),
      fixed_btn_(new QPushButton(this))
{
    setFixedSize(424, 215);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::ColorRole::Base);
    kdk::UkuiStyleHelper::self()->removeHeader(this);

    // 标题栏按钮
    auto *window_button_bar = new kdk::KWindowButtonBar(this);
    window_button_bar->menuButton()->hide();
    window_button_bar->minimumButton()->hide();
    window_button_bar->maximumButton()->hide();
    connect(window_button_bar->closeButton(), &QPushButton::clicked, this, &UserExperienceDialog::on_Fixed);

    // 图标和标题
    auto *icon_btn = new QPushButton(this);
    icon_btn->setFlat(true);
    icon_btn->setAttribute(Qt::WA_TransparentForMouseEvents);
    icon_btn->setFixedSize(22, 22);
    icon_btn->setIcon(QIcon::fromTheme("dialog-info"));
    auto *title_label = new K::TextLabel(this);
    title_label->SetPixelSize(14, true);
    title_label->SetText(tr("Have all issues been fixed?"));
    auto *title_hbox_layout = new QHBoxLayout;
    title_hbox_layout->setSpacing(0);
    title_hbox_layout->setContentsMargins(0, 0, 0, 0);
    title_hbox_layout->addSpacing(25);
    title_hbox_layout->addWidget(icon_btn);
    title_hbox_layout->addSpacing(9);
    title_hbox_layout->addWidget(title_label);
    title_hbox_layout->addStretch();

    // 详细描述
    auto *information_label = new QLabel(this);
    information_label->setWordWrap(true);
    auto palette = information_label->palette();
    palette.setColor(QPalette::ColorRole::Text, qApp->palette().color(QPalette::ColorRole::PlaceholderText));
    information_label->setPalette(palette);
    information_label->setText(tr("If there are any unresolved issues, you can click \"Submit feedback\" to let us know about the problem."));
    auto *information_hbox_layout = new QHBoxLayout;
    information_hbox_layout->setSpacing(0);
    information_hbox_layout->setContentsMargins(56, 0, 22, 0);
    information_hbox_layout->addWidget(information_label);

    // 按钮
    feedback_btn_->resize(96, 36);
    feedback_btn_->setText(tr("Submit feedback"));
    connect(feedback_btn_, &QPushButton::clicked, this, &UserExperienceDialog::on_Feedback);

    fixed_btn_->resize(96, 36);
    fixed_btn_->setText(tr("Fixed"));
    connect(fixed_btn_, &QPushButton::clicked, this, &UserExperienceDialog::on_Fixed);

    auto *button_hbox_layout = new QHBoxLayout;
    button_hbox_layout->setSpacing(0);
    button_hbox_layout->setContentsMargins(0, 0, 22, 0);
    button_hbox_layout->addStretch();
    button_hbox_layout->addWidget(feedback_btn_);
    button_hbox_layout->addSpacing(10);
    button_hbox_layout->addWidget(fixed_btn_);

    auto vbox_layout = new QVBoxLayout;
    vbox_layout->setSpacing(0);
    vbox_layout->setContentsMargins(0, 0, 0, 24);
    vbox_layout->addWidget(window_button_bar);
    vbox_layout->addSpacing(5);
    vbox_layout->addLayout(title_hbox_layout);
    vbox_layout->addSpacing(8);
    vbox_layout->addLayout(information_hbox_layout);
    vbox_layout->addStretch();
    vbox_layout->addLayout(button_hbox_layout);
    setLayout(vbox_layout);

    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [information_label]() {
        // 主题变换后，直接取 qApp 调色板的 PlaceholderText 颜色与实际的颜色相反，不知道为什么，此时手动设置颜色
        QColor color;
        const auto theme = kdk::GsettingMonitor::getInstance()->getSystemTheme().toString();
        if (theme == "ukui-default" || theme == "ukui-light") {
            color.setRed(0);
            color.setGreen(0);
            color.setBlue(0);
            color.setAlphaF(0.35);
        } else {
            color.setRed(255);
            color.setGreen(255);
            color.setBlue(255);
            color.setAlphaF(0.35);
        }
        auto palette = information_label->palette();
        palette.setColor(QPalette::ColorRole::Text, color);
        information_label->setPalette(palette);
    });
}

void UserExperienceDialog::on_Feedback() {
    accept();

    if (feedback_callback_) {
        feedback_callback_();
    }
}

void UserExperienceDialog::on_Fixed() {
    accept();

    if (fixed_callback_) {
        fixed_callback_();
    }
}

void UserExperienceDialog::SetFeedBackCallback(FeedBackCallback &&callback) {
    feedback_callback_ = callback;
}

void UserExperienceDialog::SetFixedCallback(FixedCallback &&callback) {
    fixed_callback_ = callback;
}

}
